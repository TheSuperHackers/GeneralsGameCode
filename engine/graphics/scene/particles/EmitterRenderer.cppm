module;

#include "../../profiling/Tracy.h"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

export module Graphics.Scene.Particles.EmitterRenderer;

import Assets.Math;
import Assets.Particles;
import Graphics.Materials.State;
import Graphics.RHI;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Beams.SegmentedLine;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Particles.EmitterKinematics;
import Graphics.Scene.Particles.EmitterVisualState;
import Graphics.Scene.Particles.LineGroupGeometry;
import Graphics.Scene.Particles.SpriteGeometry;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.View;

namespace Graphics
{

export struct EmitterDrawInput final
{
    Matrix4x4 projection = Matrix4x4::Identity();
    Matrix4x4 view = Matrix4x4::Identity();
    Matrix4x4 world = Matrix4x4::Identity();
    std::array<float, 9> line_rotation{1, 0, 0, 0, 1, 0, 0, 0, 1};
    SceneDrawParameters scene;
    std::array<float, 3> source_position{};
    std::uint8_t source_group = 0;
    std::uint32_t rendered_frame = 0;
    std::uint32_t sync_time = 0;
    std::uint32_t logic_time = 0;
    bool source_active = false;
    bool reflection = false;
    bool sorting_enabled = true;
};

export bool Emitter_Slot_Is_Visible(std::uint32_t slot, std::uint32_t threshold) noexcept
{
    constexpr std::array<unsigned, 16> permutation{
        11, 3, 7, 14, 0, 13, 1, 2, 5, 12, 15, 6, 9, 8, 4, 10};
    return permutation[slot & 15u] >= threshold;
}

// Owns active draw selection and preparation for authored emitter shapes.
// Simulation advances separately, including when an emitter is not drawn.
export class EmitterRenderer final
{
public:
    EmitterRenderer(const Assets::EmitterAssetDesc &description, MaterialState shader,
        std::uint32_t capacity, std::uint32_t logic_time)
        : m_mode(description.geometry_mode), m_frame_mode(std::countr_zero(description.atlas.columns)),
          m_shader(shader), m_capacity(capacity)
    {
        m_shader.Enable_Fog_For_Blend();
        m_indices.reserve(static_cast<std::size_t>(capacity) + 1);
        if (m_mode == Assets::EmitterGeometryMode::Line) {
            m_line.Set_Description(Line_Description(description, m_shader));
            m_line.Set_Width(description.size.start);
            m_line.Reset_Line(logic_time);
        }
    }

    EmitterRenderer(const EmitterRenderer &source)
        : m_mode(source.m_mode), m_frame_mode(source.m_frame_mode), m_shader(source.m_shader),
          m_capacity(source.m_capacity), m_line(source.m_line)
    {
        m_indices.reserve(static_cast<std::size_t>(m_capacity) + 1);
    }
    EmitterRenderer &operator=(const EmitterRenderer &) = delete;

    MaterialState Shader() const noexcept { return m_shader; }
    Assets::EmitterGeometryMode Mode() const noexcept { return m_mode; }
    void Scale(float scale) noexcept
    {
        if (m_mode == Assets::EmitterGeometryMode::Line)
            m_line.Scale(scale);
    }

    template<class Source, class Resolve, class RandomVector, class ResetRandom>
    bool Submit(Device &device, PropRenderer &renderer, PropSubmission &submission,
        const EmitterKinematics &particles, const EmitterVisualState &visuals,
        std::uint32_t decimation, const EmitterDrawInput &input,
        Source texture, Resolve resolve, RandomVector random_vector, ResetRandom reset_random)
    {
        Select(particles, decimation);
        if (m_mode == Assets::EmitterGeometryMode::Line)
            return Submit_Lines(device, renderer, submission, particles, visuals, input,
                texture, resolve, random_vector, reset_random);
        const std::size_t count = Selected_Count();
        if (count == 0)
            return true;
        const auto positions = particles.Positions(input.rendered_frame);
        const bool line_group = m_mode == Assets::EmitterGeometryMode::LineGroupTetra
            || m_mode == Assets::EmitterGeometryMode::LineGroupPrism;

        if (line_group) {
            const auto shape = m_mode == Assets::EmitterGeometryMode::LineGroupTetra
                ? LineGroupShape::Tetrahedron : LineGroupShape::Prism;
            if (!m_line_group.Build(count, shape, input.line_rotation, [&](std::size_t ordinal) {
                const auto slot = Selected_Slot(ordinal);
                const auto color = visuals.Color(slot);
                return LineGroupPoint{positions[slot], visuals.Tail(slot), color,
                    Tail_Color(color, texture != Source{}), visuals.Size(slot), visuals.Frame(slot)};
            }))
                return false;
        } else {
            if (m_frame_mode > 4)
                return false;
            const auto shape = m_mode == Assets::EmitterGeometryMode::SpriteTriangles
                ? SpriteShape::Triangle : SpriteShape::Quad;
            const unsigned columns = 1u << m_frame_mode;
            if (!m_sprite.Build(count, shape, input.view.values, [&](std::size_t ordinal) {
                const auto slot = Selected_Slot(ordinal);
                SpritePoint point;
                point.position = positions[slot];
                point.color = visuals.Color(slot);
                point.size = visuals.Size(slot);
                point.angle = visuals.Orientation(slot) * (6.283185307179586f / 256.0f);
                const unsigned frame = static_cast<int>(visuals.Frame(slot)) & 255;
                Get_Sprite_Atlas_Region(frame, columns, columns, point.texture_region);
                return point;
            }))
                return false;
        }

        MaterialState shader = m_shader;
        if (line_group)
            shader.Set_Cull_Mode(MaterialState::CULL_MODE_ENABLE);
        const auto color = visuals.Default_Color();
        const bool white = color[0] > .9961f && color[1] > .9961f
            && color[2] > .9961f && color[3] > .9961f;
        const bool textured = texture != Source{};
        shader.Set_Primary_Gradient(visuals.Has_Diffuse() || !white || !textured
            ? MaterialState::GRADIENT_MODULATE : MaterialState::GRADIENT_DISABLE);
        shader.Set_Texturing(textured ? MaterialState::TEXTURING_ENABLE : MaterialState::TEXTURING_DISABLE);
        if (line_group)
            m_shader = shader;
        PropParameters parameters;
        parameters.view_projection = line_group
            ? Compose_Matrices(input.projection, input.view).values : input.projection.values;
        PropMaterialDrawContext context;
        context.scene = input.scene;
        context.reflection = input.reflection;
        context.milliseconds = input.sync_time;
        if (Sorted(shader, input)) {
            context.sorting_depth = line_group
                ? std::array{input.view.values[8], input.view.values[9], input.view.values[10], input.view.values[11]}
                : std::array{0.0f, 0.0f, 1.0f, 0.0f};
        }
        const auto vertices = line_group ? m_line_group.Vertices() : m_sprite.Vertices();
        const auto indices = line_group ? m_line_group.Indices() : m_sprite.Indices();
        return Submit_Prop_Material(device, renderer, submission, vertices, indices, shader,
            std::array<Source, 2>{texture, Source{}}, resolve, parameters, context);
    }

private:
    static SegmentedLineDescription Line_Description(const Assets::EmitterAssetDesc &description,
        MaterialState shader)
    {
        const auto &properties = description.line_properties;
        SegmentedLineDescription line;
        switch (properties.texture_mapping) {
        case Assets::EmitterLineTextureMapping::UniformLength:
            line.texture_mapping = RibbonTextureMapping::Along;
            break;
        case Assets::EmitterLineTextureMapping::Tiled:
            line.texture_mapping = RibbonTextureMapping::Tiled;
            break;
        default:
            line.texture_mapping = RibbonTextureMapping::Across;
            break;
        }
        line.merge_intersections = properties.merge_intersections;
        line.freeze_random = properties.freeze_random;
        line.disable_sorting = properties.disable_sorting;
        line.end_caps = properties.end_caps;
        line.subdivision_level = properties.subdivision_level;
        line.noise_amplitude = properties.noise_amplitude;
        line.merge_abort_factor = properties.merge_abort_factor;
        line.texture_tile_factor = properties.texture_tile_factor;
        line.uv_offset_rate = {properties.uv_offset_rate.x, properties.uv_offset_rate.y};
        line.shader = shader;
        return line;
    }

    static bool Sorted(MaterialState shader, const EmitterDrawInput &input) noexcept
    {
        return input.sorting_enabled && shader.Get_Dst_Blend_Func() != MaterialState::DSTBLEND_ZERO
            && shader.Get_Alpha_Test() == MaterialState::ALPHATEST_DISABLE;
    }

    void Select(const EmitterKinematics &particles, std::uint32_t threshold)
    {
        m_indices.clear();
        m_direct_count = 0;
        const bool line = m_mode == Assets::EmitterGeometryMode::Line;
        if (!line && particles.Count() == particles.Capacity() && threshold == 0) {
            m_direct_count = particles.Count();
            return;
        }
        for (const auto range : particles.Active_Ranges(!line))
            for (std::uint32_t slot = range.begin; slot < range.end; ++slot)
                if (Emitter_Slot_Is_Visible(slot, threshold))
                    m_indices.push_back(slot);
    }

    std::size_t Selected_Count() const noexcept
    {
        return m_direct_count != 0 ? m_direct_count : m_indices.size();
    }
    std::uint32_t Selected_Slot(std::size_t ordinal) const noexcept
    {
        return m_direct_count != 0 ? static_cast<std::uint32_t>(ordinal) : m_indices[ordinal];
    }

    std::array<float, 4> Tail_Color(std::array<float, 4> head, bool textured) const noexcept
    {
        if (textured)
            return head;
        const auto source = m_shader.Get_Src_Blend_Func();
        const auto destination = m_shader.Get_Dst_Blend_Func();
        if (destination == MaterialState::DSTBLEND_SRC_COLOR)
            return {1, 1, 1, 1};
        if (source == MaterialState::SRCBLEND_ONE
            && (destination == MaterialState::DSTBLEND_ONE || destination == MaterialState::DSTBLEND_ONE_MINUS_SRC_COLOR))
            return {0, 0, 0, 0};
        if ((source == MaterialState::SRCBLEND_SRC_ALPHA && destination == MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA)
            || m_shader.Get_Alpha_Test() == MaterialState::ALPHATEST_ENABLE)
            head[3] = 0;
        return head;
    }

    template<class Source, class Resolve, class RandomVector, class ResetRandom>
    bool Submit_Lines(Device &device, PropRenderer &renderer, PropSubmission &submission,
        const EmitterKinematics &particles, const EmitterVisualState &visuals,
        const EmitterDrawInput &input, Source texture, Resolve resolve,
        RandomVector random_vector, ResetRandom reset_random)
    {
        const auto groups = particles.Groups();
        const auto positions = particles.Positions(input.rendered_frame);
        const std::uint8_t last_group = m_indices.empty() ? 0 : groups[m_indices.back()];
        const auto last_color = m_indices.empty() ? visuals.Default_Color() : visuals.Color(m_indices.back());
        // Only the final run connects to the emitter. Group IDs can wrap and
        // occur in earlier runs, which must remain disconnected from the source.
        if (input.source_active && last_group == input.source_group)
            m_indices.push_back(particles.Capacity());
        const auto group_at = [&](std::size_t ordinal) {
            const auto slot = m_indices[ordinal];
            return slot == particles.Capacity() ? last_group : groups[slot];
        };

        SegmentedLineDrawInput line_input;
        line_input.build.view = input.view.values;
        line_input.build.world = input.world.values;
        line_input.build.time_milliseconds = input.logic_time;
        line_input.build.use_point_colors = true;
        line_input.projection = input.projection.values;
        line_input.scene = input.scene;
        line_input.reflection = input.reflection;
        line_input.material_time_milliseconds = input.sync_time;
        if (!m_line.Is_Sorting_Disabled() && Sorted(m_shader, input))
            line_input.sorting_depth = {0, 0, 1, 0};
        bool success = true;
        std::size_t begin = 0;
        while (begin < m_indices.size()) {
            std::size_t end = begin + 1;
            while (end < m_indices.size() && group_at(end) == group_at(begin))
                ++end;
            if (end - begin > 1) {
                const bool submitted = m_line.Submit(device, renderer, submission, line_input,
                    end - begin, texture, resolve, [&](std::size_t ordinal) {
                        const auto slot = m_indices[begin + ordinal];
                        return slot == particles.Capacity()
                            ? RibbonPoint{input.source_position, last_color, 0}
                            : RibbonPoint{positions[slot], visuals.Color(slot), 0};
                    }, [&] { return random_vector(m_line.Is_Freeze_Random()); }, reset_random);
                success = submitted && success;
            }
            begin = end;
        }
        return success;
    }

    Assets::EmitterGeometryMode m_mode;
    unsigned m_frame_mode;
    MaterialState m_shader;
    std::uint32_t m_capacity;
    SegmentedLineRenderer m_line;
    SpriteGeometry m_sprite;
    LineGroupGeometry m_line_group;
    std::vector<std::uint32_t> m_indices;
    std::uint32_t m_direct_count = 0;
};

}
