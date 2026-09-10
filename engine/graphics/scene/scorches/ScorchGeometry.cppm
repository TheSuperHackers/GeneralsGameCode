module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
export module Graphics.Scene.Scorches.Geometry;
export import Graphics.Scene.Surfaces.Geometry;

namespace Graphics
{
export struct ScorchDescription final
{
    std::array<float, 2> center{};
    float radius = 1;
    unsigned atlas_index = 0;
};

export struct ScorchGrid final
{
    int width = 0;
    int height = 0;
    int border = 0;
    float spacing = 1;
    float elevation = 0;
};

// Call Append newest first. A mark that exceeds the budget leaves the existing
// geometry intact so capacity cannot evict more recent marks.
export class ScorchGeometry final
{
public:
    std::vector<SurfaceVertex> vertices;
    std::vector<std::uint32_t> indices;

    template<class HeightSample, class DiagonalSample>
    bool Append(const ScorchDescription &mark, const ScorchGrid &grid,
        const std::array<float, 4> &color, HeightSample height, DiagonalSample diagonal,
        std::size_t vertex_limit = 8194, std::size_t index_limit = 49164)
    {
        if (!(mark.radius > 0) || !std::isfinite(mark.radius)
            || !std::isfinite(mark.center[0]) || !std::isfinite(mark.center[1])
            || !(grid.spacing > 0) || !std::isfinite(grid.spacing)
            || grid.width < 2 || grid.height < 2 || grid.border < 0) return false;
        // Clamp in floating point before integer conversion, including marks
        // wholly outside the map and large radii.
        const auto lower = [&](float center, int extent) {
            return static_cast<int>(std::clamp(std::floor((center - mark.radius) / grid.spacing),
                -float(grid.border), float(extent - grid.border)));
        };
        const auto upper = [&](float center, int extent) {
            return static_cast<int>(std::clamp(std::ceil((center + mark.radius) / grid.spacing) + 1,
                -float(grid.border), float(extent - grid.border)));
        };
        const int min_x = lower(mark.center[0], grid.width), max_x = upper(mark.center[0], grid.width);
        const int min_y = lower(mark.center[1], grid.height), max_y = upper(mark.center[1], grid.height);
        const int width = max_x - min_x, rows = max_y - min_y;
        if (width < 2 || rows < 2) return true;
        const std::size_t vertex_count = std::size_t(width) * rows;
        const std::size_t index_count = std::size_t(width - 1) * (rows - 1) * 6;
        if (vertices.size() > vertex_limit || vertex_count > vertex_limit - vertices.size()
            || indices.size() > index_limit || index_count > index_limit - indices.size()) return false;
        const auto start = static_cast<std::uint32_t>(vertices.size());
        const unsigned type = mark.atlas_index < 9 ? mark.atlas_index : 0;
        for (int y = min_y; y < max_y; ++y) {
            for (int x = min_x; x < max_x; ++x) {
                SurfaceVertex vertex;
                vertex.position = {x * grid.spacing, y * grid.spacing,
                    height(x + grid.border, y + grid.border) + grid.elevation};
                vertex.color = color;
                vertex.uv = {(float(type % 3) * 1.5f + 0.5f + (vertex.position[0] - mark.center[0]) / (2 * mark.radius)) / 4,
                    (float(type / 3) * 1.5f + 0.5f + (vertex.position[1] - mark.center[1]) / (2 * mark.radius)) / 4};
                vertices.push_back(vertex);
            }
        }
        for (int y = 0; y < rows - 1; ++y) {
            for (int x = 0; x < width - 1; ++x) {
                const auto a = start + y * width + x;
                const auto b = a + 1, c = a + width, d = c + 1;
                const std::array<std::uint32_t, 6> cell = diagonal(x + min_x + grid.border, y + min_y + grid.border)
                    ? std::array<std::uint32_t, 6>{b,c,a,b,d,c} : std::array<std::uint32_t, 6>{a,d,c,a,b,d};
                indices.insert(indices.end(), cell.begin(), cell.end());
            }
        }
        return true;
    }
};
}
