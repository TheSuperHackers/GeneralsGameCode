module;
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Trees.Geometry;

namespace Graphics
{
export struct TreeVertex final
{
    std::array<float, 3> position{};
    std::array<float, 4> color{1, 1, 1, 1};
    std::array<float, 2> uv{};
    std::array<float, 2> reserved{};
    std::array<float, 3> sway{1,1,0}; // Type (1..10), darkening, base height.
};
static_assert(sizeof(TreeVertex) == 56);

// Indexed, prelit triangle geometry shared by terrain decorations. The input
// topology is preserved, including segment boundaries and overlapping layers.
export class TreeGeometry final
{
public:
    bool Assign(std::span<const TreeVertex> vertices, std::span<const std::uint32_t> indices)
    {
        constexpr auto maximum_bytes = std::numeric_limits<std::uint32_t>::max();
        if (vertices.size() > maximum_bytes / sizeof(TreeVertex)
            || indices.size() > maximum_bytes / sizeof(std::uint32_t)
            || indices.size() % 3 != 0) return false;
        for (const auto &vertex : vertices) {
            for (float value : vertex.position) if (!std::isfinite(value)) return false;
            for (float value : vertex.color) if (!std::isfinite(value)) return false;
            for (float value : vertex.sway) if (!std::isfinite(value)) return false;
            for (float value : vertex.uv) if (!std::isfinite(value)) return false;
        }
        for (std::uint32_t index : indices) if (index >= vertices.size()) return false;
        m_vertices.assign(vertices.begin(), vertices.end());
        m_indices.assign(indices.begin(), indices.end());
        return true;
    }

    std::span<const TreeVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

private:
    std::vector<TreeVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
}
