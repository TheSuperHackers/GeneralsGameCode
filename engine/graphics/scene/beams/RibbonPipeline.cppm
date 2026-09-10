module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Scene.Beams.RibbonPipeline;

import Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonGeometry;
import Graphics.Scene.Beams.RibbonIntersections;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Props.Geometry;

namespace Graphics {

// The ribbon subdivision budget is fixed at 128 segments after
// subdivision. Source segments per chunk therefore shrink with the level.
// A chunk has one extra source point because adjacent chunks share their seam.
export inline constexpr std::size_t RibbonPipelineChunkSegments = 128;
export inline constexpr unsigned RibbonPipelineMaximumSubdivisionLevel = 7;

export struct RibbonPipelineSettings final {
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
    unsigned subdivision_level = 0;
    float noise_amplitude = 0;
    float width = 0;
    bool merge_intersections = true;
    float merge_abort_factor = 1.5f;
    RibbonTextureMapping texture_mapping = RibbonTextureMapping::Across;
    float texture_tile_factor = 1;
    std::array<float, 2> texture_offset{};
    bool use_uniform_color = true;
    std::array<float, 3> uniform_color{1, 1, 1};
    float uniform_opacity = 1;
};

// The source range includes the final point shared with the following chunk.
// Both spans refer to pipeline-owned scratch and are valid only while the sink
// callback is running, before the next chunk starts.
export struct RibbonPipelineChunk final {
    std::size_t first_source_point = 0;
    std::size_t source_point_count = 0;
    std::span<const PropVertex> vertices{};
    std::span<const std::uint32_t> indices{};
};

// Owns ribbon traversal and temporary storage while leaving source
// access, visual randomness, and submission to the caller. ReadPoint returns a
// RibbonPoint whose position and (when selected) color are in object space;
// its v component is replaced by the pipeline's absolute source-index UV.
export class RibbonPipeline final {
    using Matrix = std::array<float, 16>;
    using Position = std::array<float, 3>;

    static bool Finite(const Matrix &matrix) noexcept
    {
        for (const float value : matrix)
            if (!std::isfinite(value)) return false;
        return true;
    }

    static Matrix Multiply(const Matrix &left, const Matrix &right) noexcept
    {
        Matrix result{};
        for (std::size_t row = 0; row < 4; ++row)
            for (std::size_t column = 0; column < 4; ++column)
                for (std::size_t element = 0; element < 4; ++element)
                    result[row * 4 + column] += left[row * 4 + element]
                        * right[element * 4 + column];
        return result;
    }

    static Position Transform(const Matrix &matrix, const Position &point) noexcept
    {
        return {
            matrix[0] * point[0] + matrix[1] * point[1] + matrix[2] * point[2] + matrix[3],
            matrix[4] * point[0] + matrix[5] * point[1] + matrix[6] * point[2] + matrix[7],
            matrix[8] * point[0] + matrix[9] * point[1] + matrix[10] * point[2] + matrix[11]};
    }

public:
    template<class ReadPoint, class RandomVector, class ResetRandom, class EmitChunk>
    bool Build(std::size_t point_count, const RibbonPipelineSettings &settings,
        ReadPoint &&read_point, RandomVector &&random_vector, ResetRandom &&reset_random,
        EmitChunk &&emit_chunk)
    {
        // Short lines produce no geometry. Keep that no-op behavior without count - 1
        // underflow here. A level outside the renderer's fixed budget is an
        // invalid request and is rejected before reading source points.
        if (settings.subdivision_level > RibbonPipelineMaximumSubdivisionLevel)
            return false;
        if (point_count < 2)
            return true;
        if (!Finite(settings.view) || !Finite(settings.world))
            return false;

        const Matrix model_view = Multiply(settings.view, settings.world);
        if (!Finite(model_view))
            return false;

        const std::size_t segments_per_chunk =
            RibbonPipelineChunkSegments >> settings.subdivision_level;
        const std::size_t points_per_chunk = segments_per_chunk + 1;
        std::size_t first_source_point = 0;

        for (;;) {
            const std::size_t remaining = point_count - first_source_point;
            const std::size_t source_point_count =
                (std::min)(remaining, points_per_chunk);
            // The loop invariant guarantees two points, but retaining this
            // check makes the seam arithmetic explicit if the budget changes.
            if (source_point_count < 2)
                return false;

            m_eye_points.resize(source_point_count);
            for (std::size_t local_point = 0; local_point < source_point_count; ++local_point) {
                const std::size_t source_point = first_source_point + local_point;
                const RibbonPoint source = read_point(source_point);
                RibbonPoint &prepared = m_eye_points[local_point];
                prepared.position = Transform(model_view, source.position);
                prepared.color = settings.use_uniform_color
                    ? std::array<float, 4>{settings.uniform_color[0], settings.uniform_color[1],
                        settings.uniform_color[2], settings.uniform_opacity}
                    : source.color;
                prepared.v = Ribbon_Texture_V(settings.texture_mapping, source_point,
                    settings.texture_tile_factor);
            }

            // The old renderer constructed a fresh frozen/random generator for
            // every chunk. Reset is supplied by the adapter so this component
            // never owns or knows the game's RNG type.
            reset_random(first_source_point);
            if (!m_subdivision.Build(source_point_count, settings.subdivision_level,
                    settings.noise_amplitude,
                    [&](std::size_t local_point) { return m_eye_points[local_point]; },
                    [&] { return random_vector(); }))
                return false;
            if (!m_edges.Build(m_subdivision.Points(), settings.width))
                return false;
            if (!m_intersections.Build(m_edges, settings.merge_intersections,
                    settings.width, settings.merge_abort_factor))
                return false;
            if (!m_geometry.Build(m_edges.Points(), m_intersections.Top(),
                    m_intersections.Bottom(), settings.texture_mapping,
                    settings.texture_offset))
                return false;

            emit_chunk(RibbonPipelineChunk{
                first_source_point,
                source_point_count,
                m_geometry.Vertices(),
                m_geometry.Indices()});

            if (source_point_count == remaining)
                break;
            // The final source point is the first point of the next chunk.
            first_source_point += source_point_count - 1;
        }
        return true;
    }

private:
    std::vector<RibbonPoint> m_eye_points;
    RibbonSubdivision m_subdivision;
    RibbonEdges m_edges;
    RibbonIntersections m_intersections;
    RibbonGeometry m_geometry;
};

}
