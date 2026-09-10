module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

export module Graphics.Scene.Beams.SegmentedLine;

export import Graphics.Scene.Beams.RibbonPipeline;
export import Graphics.Scene.Beams.RibbonTextureCoordinates;
export import Graphics.Scene.Props.Geometry;
import Graphics.Materials.State;
import Graphics.RHI;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;

namespace Graphics {

// The line renderer owns ribbon preparation. Texture objects and asset
// ownership remain with the caller, which supplies a resolver at submission.
export struct SegmentedLineDescription final {
    RibbonTextureMapping texture_mapping = RibbonTextureMapping::Across;
    bool merge_intersections = true;
    bool freeze_random = false;
    bool disable_sorting = false;
    bool end_caps = false;
    unsigned subdivision_level = 0;
    float width = 0.0f;
    std::array<float, 3> color{1.0f, 1.0f, 1.0f};
    float opacity = 1.0f;
    float noise_amplitude = 0.0f;
    float merge_abort_factor = 1.5f;
    float texture_tile_factor = 1.0f;
    std::array<float, 2> uv_offset_rate{};
    MaterialState shader = MaterialState::AdditiveSprite();
};

export struct SegmentedLineBuildInput final {
    std::array<float, 16> view{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    std::array<float, 16> world{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    std::uint32_t time_milliseconds = 0;
    bool use_point_colors = false;
};

export struct SegmentedLineDrawInput final {
    SegmentedLineBuildInput build{};
    std::array<float, 16> projection{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    SceneDrawParameters scene{};
    bool reflection = false;
    std::uint32_t material_time_milliseconds = 0;
    std::optional<std::array<float, 4>> sorting_depth;
};

export struct SegmentedLineBounds final {
    std::array<float, 3> minimum{};
    std::array<float, 3> maximum{};
    bool valid = false;
};

namespace SegmentedLineDetail {

inline bool Finite_Position(const std::array<float, 3> &position) noexcept
{
    return std::all_of(position.begin(), position.end(),
        [](float value) { return std::isfinite(value); });
}

inline std::array<float, 3> Midpoint(const std::array<float, 3> &first,
    const std::array<float, 3> &second) noexcept
{
    std::array<float, 3> midpoint{};
    for (std::size_t axis = 0; axis < midpoint.size(); ++axis)
        midpoint[axis] = (first[axis] + second[axis]) * 0.5f;
    return midpoint;
}

inline bool Expand(SegmentedLineBounds &bounds, float amount) noexcept
{
    if (!std::isfinite(amount)) return false;
    for (std::size_t axis = 0; axis < bounds.minimum.size(); ++axis) {
        bounds.minimum[axis] -= amount;
        bounds.maximum[axis] += amount;
    }
    return Finite_Position(bounds.minimum) && Finite_Position(bounds.maximum);
}

template<class ReadPosition>
SegmentedLineBounds Make_Bounds(std::size_t point_count, unsigned max_subdivision_levels,
    float width, float noise_amplitude, ReadPosition &&read_position) noexcept
{
    SegmentedLineBounds bounds;
    if (point_count < 2 || !std::isfinite(width) || !std::isfinite(noise_amplitude))
        return bounds;

    const auto first = read_position(0);
    if (!Finite_Position(first)) return bounds;
    bounds.minimum = bounds.maximum = first;
    for (std::size_t point = 1; point < point_count; ++point) {
        const auto position = read_position(point);
        if (!Finite_Position(position)) return SegmentedLineBounds{};
        for (std::size_t axis = 0; axis < bounds.minimum.size(); ++axis) {
            bounds.minimum[axis] = (std::min)(bounds.minimum[axis], position[axis]);
            bounds.maximum[axis] = (std::max)(bounds.maximum[axis], position[axis]);
        }
    }

    const float radius = width * 0.5f;
    if (!Expand(bounds, radius)) return SegmentedLineBounds{};
    if (max_subdivision_levels > 0) {
        SegmentedLineBounds midpoint_bounds;
        midpoint_bounds.minimum = midpoint_bounds.maximum =
            Midpoint(first, read_position(1));
        if (!Finite_Position(midpoint_bounds.minimum)) return SegmentedLineBounds{};
        for (std::size_t point = 1; point + 1 < point_count; ++point) {
            const auto midpoint = Midpoint(read_position(point), read_position(point + 1));
            if (!Finite_Position(midpoint)) return SegmentedLineBounds{};
            for (std::size_t axis = 0; axis < midpoint_bounds.minimum.size(); ++axis) {
                midpoint_bounds.minimum[axis] =
                    (std::min)(midpoint_bounds.minimum[axis], midpoint[axis]);
                midpoint_bounds.maximum[axis] =
                    (std::max)(midpoint_bounds.maximum[axis], midpoint[axis]);
            }
        }
        if (!Expand(midpoint_bounds, radius + 2.0f * noise_amplitude))
            return SegmentedLineBounds{};
        for (std::size_t axis = 0; axis < bounds.minimum.size(); ++axis) {
            bounds.minimum[axis] = (std::min)(bounds.minimum[axis], midpoint_bounds.minimum[axis]);
            bounds.maximum[axis] = (std::max)(bounds.maximum[axis], midpoint_bounds.maximum[axis]);
        }
    }
    bounds.valid = true;
    return bounds;
}

inline RibbonPipelineSettings Make_Settings(const SegmentedLineDescription &description,
    const SegmentedLineBuildInput &input) noexcept
{
    RibbonPipelineSettings settings;
    settings.view = input.view;
    settings.world = input.world;
    settings.subdivision_level = description.subdivision_level;
    settings.noise_amplitude = description.noise_amplitude;
    settings.width = description.width;
    settings.merge_intersections = description.merge_intersections;
    settings.merge_abort_factor = description.merge_abort_factor;
    settings.texture_mapping = description.texture_mapping;
    settings.texture_tile_factor = description.texture_tile_factor;
    settings.use_uniform_color = !input.use_point_colors;
    settings.uniform_color = description.color;
    settings.uniform_opacity = description.opacity;
    return settings;
}

} // namespace SegmentedLineDetail

// A focused owner of line state, ribbon preparation, and generic prop
// submission. It has no dependency on TextureClass or any game-facing type.
export class SegmentedLineRenderer final {
public:
    SegmentedLineRenderer() = default;
    explicit SegmentedLineRenderer(const SegmentedLineDescription &description,
        std::uint32_t time_milliseconds = 0)
        : m_description(description), m_texture_coordinates(time_milliseconds)
    {
        Set_Current_Subdivision_Level(description.subdivision_level);
        Set_Texture_Tile_Factor(description.texture_tile_factor);
        Set_UV_Offset_Rate(description.uv_offset_rate);
    }

    SegmentedLineRenderer(const SegmentedLineRenderer &) = default;
    SegmentedLineRenderer &operator=(const SegmentedLineRenderer &) = default;

    const SegmentedLineDescription &Description() const noexcept { return m_description; }
    void Set_Description(const SegmentedLineDescription &description) noexcept
    {
        m_description = description;
        Set_Current_Subdivision_Level(description.subdivision_level);
        Set_Texture_Tile_Factor(description.texture_tile_factor);
        Set_UV_Offset_Rate(description.uv_offset_rate);
    }

    RibbonTextureMapping Get_Texture_Mapping_Mode() const noexcept
    {
        return m_description.texture_mapping;
    }
    void Set_Texture_Mapping_Mode(RibbonTextureMapping mapping) noexcept
    {
        m_description.texture_mapping = mapping;
    }
    bool Is_Merge_Intersections() const noexcept { return m_description.merge_intersections; }
    void Set_Merge_Intersections(bool enabled) noexcept { m_description.merge_intersections = enabled; }
    bool Is_Freeze_Random() const noexcept { return m_description.freeze_random; }
    void Set_Freeze_Random(bool enabled) noexcept { m_description.freeze_random = enabled; }
    bool Is_Sorting_Disabled() const noexcept { return m_description.disable_sorting; }
    void Set_Disable_Sorting(bool disabled) noexcept { m_description.disable_sorting = disabled; }
    bool Are_End_Caps_Enabled() const noexcept { return m_description.end_caps; }
    void Set_End_Caps(bool enabled) noexcept { m_description.end_caps = enabled; }

    unsigned Get_Current_Subdivision_Level() const noexcept { return m_description.subdivision_level; }
    void Set_Current_Subdivision_Level(unsigned level) noexcept
    {
        m_description.subdivision_level = (std::min)(level, RibbonPipelineMaximumSubdivisionLevel);
    }
    float Get_Width() const noexcept { return m_description.width; }
    void Set_Width(float width) noexcept
    {
        m_description.width = width;
    }
    const std::array<float, 3> &Get_Color() const noexcept { return m_description.color; }
    void Set_Color(std::array<float, 3> color) noexcept
    {
        m_description.color = color;
    }
    float Get_Opacity() const noexcept { return m_description.opacity; }
    void Set_Opacity(float opacity) noexcept
    {
        m_description.opacity = opacity;
    }
    MaterialState Get_Shader() const noexcept { return m_description.shader; }
    void Set_Shader(MaterialState shader) noexcept { m_description.shader = shader; }
    float Get_Noise_Amplitude() const noexcept { return m_description.noise_amplitude; }
    void Set_Noise_Amplitude(float amplitude) noexcept
    {
        m_description.noise_amplitude = amplitude;
    }
    float Get_Merge_Abort_Factor() const noexcept { return m_description.merge_abort_factor; }
    void Set_Merge_Abort_Factor(float factor) noexcept
    {
        m_description.merge_abort_factor = factor;
    }
    float Get_Texture_Tile_Factor() const noexcept { return m_description.texture_tile_factor; }
    void Set_Texture_Tile_Factor(float factor) noexcept
    {
        if (!std::isfinite(factor)) factor = 0.0f;
        m_description.texture_tile_factor = (std::clamp)(factor, 0.0f, 50.0f);
    }
    std::array<float, 2> Get_UV_Offset_Rate() const noexcept
    {
        return m_texture_coordinates.Rate();
    }
    void Set_UV_Offset_Rate(std::array<float, 2> rate) noexcept
    {
        m_description.uv_offset_rate = rate;
        m_texture_coordinates.SetRate(rate);
    }
    void Set_Current_UV_Offset(std::array<float, 2> offset) noexcept
    {
        m_texture_coordinates.SetOffset(offset);
    }
    void Reset_Line(std::uint32_t time_milliseconds) noexcept
    {
        m_texture_coordinates.Reset(time_milliseconds);
    }
    void Scale(float scale) noexcept
    {
        m_description.width *= scale;
        m_description.noise_amplitude *= scale;
    }

    template<class ReadPosition>
    SegmentedLineBounds Get_Bounds(std::size_t point_count, unsigned max_subdivision_levels,
        ReadPosition &&read_position) const noexcept
    {
        return SegmentedLineDetail::Make_Bounds(point_count, max_subdivision_levels,
            m_description.width, m_description.noise_amplitude,
            std::forward<ReadPosition>(read_position));
    }

    template<class ReadPoint, class RandomVector, class ResetRandom, class EmitChunk>
    bool Build(const SegmentedLineBuildInput &input, std::size_t point_count,
        ReadPoint &&read_point, RandomVector &&random_vector, ResetRandom &&reset_random,
        EmitChunk &&emit_chunk)
    {
        if (point_count < 2) return true;
        auto settings = SegmentedLineDetail::Make_Settings(m_description, input);
        settings.texture_offset = m_texture_coordinates.Advance(input.time_milliseconds);
        return m_pipeline.Build(point_count, settings,
            std::forward<ReadPoint>(read_point), std::forward<RandomVector>(random_vector),
            std::forward<ResetRandom>(reset_random), std::forward<EmitChunk>(emit_chunk));
    }

    template<class ReadPoint, class RandomVector, class ResetRandom, class EmitChunk>
    bool Extract_Geometry(const SegmentedLineBuildInput &input, std::size_t point_count,
        ReadPoint &&read_point, RandomVector &&random_vector, ResetRandom &&reset_random,
        EmitChunk &&emit_chunk)
    {
        return Build(input, point_count, std::forward<ReadPoint>(read_point),
            std::forward<RandomVector>(random_vector), std::forward<ResetRandom>(reset_random),
            std::forward<EmitChunk>(emit_chunk));
    }

    template<class Source, class Resolve, class ReadPoint, class RandomVector, class ResetRandom>
    bool Submit(Device &device, PropRenderer &renderer, PropSubmission &submission,
        const SegmentedLineDrawInput &input, std::size_t point_count, Source source,
        Resolve &&resolve, ReadPoint &&read_point, RandomVector &&random_vector,
        ResetRandom &&reset_random)
    {
        bool submitted = true;
        MaterialState shader = m_description.shader;
        shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
        shader.Set_Primary_Gradient(MaterialState::GRADIENT_MODULATE);
        PropParameters parameters;
        parameters.view_projection = input.projection;
        PropMaterialDrawContext context;
        context.scene = input.scene;
        context.reflection = input.reflection;
        context.milliseconds = input.material_time_milliseconds;
        context.sorting_depth = input.sorting_depth;
        const std::array<Source, 2> sources{source, Source{}};
        const bool built = Build(input.build, point_count, std::forward<ReadPoint>(read_point),
            std::forward<RandomVector>(random_vector), std::forward<ResetRandom>(reset_random),
            [&](const RibbonPipelineChunk &chunk) {
                if (!Submit_Prop_Material(device, renderer, submission,
                    chunk.vertices, chunk.indices, shader, sources, resolve, parameters, context))
                    submitted = false;
            });
        return built && submitted;
    }

private:
    SegmentedLineDescription m_description{};
    RibbonTextureCoordinates m_texture_coordinates{};
    RibbonPipeline m_pipeline{};
};

} // namespace Graphics
