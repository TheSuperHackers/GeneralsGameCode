module;
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Water.Geometry;

namespace Graphics
{
export struct WaterVertex final
{
    std::array<float, 3> position{};
    std::array<float, 4> color{1, 1, 1, 1};
    std::array<float, 2> uv{};
    std::array<float, 2> secondary_uv{};
    std::array<float, 3> normal{0,0,1};
};
static_assert(sizeof(WaterVertex) == 56);

// Indexed, prelit triangle geometry shared by terrain decorations. The input
// topology is preserved, including segment boundaries and overlapping layers.
export std::vector<std::uint32_t> Expand_Water_Strip(std::span<const std::uint32_t> strip)
{
    std::vector<std::uint32_t> triangles;
    if (strip.size() < 3) return triangles;
    triangles.reserve((strip.size()-2)*3);
    for (std::size_t i=2;i<strip.size();++i) {
        const auto a = strip[i-2], b = strip[i-1], c = strip[i];
        if (a==b || b==c || a==c) continue;
        if (i%2==0) triangles.insert(triangles.end(),{a,b,c});
        else triangles.insert(triangles.end(),{b,a,c});
    }
    return triangles;
}

export class WaterGeometry final
{
public:
    bool Assign_Surface_Patch(std::span<const std::array<float,3>,4> corners, float spacing)
    {
        if (!std::isfinite(spacing) || spacing <= 0) return false;
        for (const auto& corner : corners)
            for (const float value : corner) if (!std::isfinite(value)) return false;
        const auto edge_length = [&](unsigned first, unsigned second) {
            const auto& a = corners[first];
            const auto& b = corners[second];
            return std::hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2]);
        };
        const auto subdivisions = [&](float length) {
            return static_cast<unsigned>(std::clamp(std::ceil(length/spacing),1.0f,250.0f));
        };
        const unsigned columns = subdivisions((std::max)(edge_length(0,1),edge_length(3,2)));
        const unsigned rows = subdivisions((std::max)(edge_length(0,3),edge_length(1,2)));
        m_vertices.resize((columns+1)*(rows+1));
        m_indices.resize(columns*rows*6);
        for (unsigned y = 0; y <= rows; ++y) {
            const float v = static_cast<float>(y)/rows;
            for (unsigned x = 0; x <= columns; ++x) {
                const float u = static_cast<float>(x)/columns;
                auto& vertex = m_vertices[y*(columns+1)+x];
                vertex = {};
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const float first = std::lerp(corners[0][axis],corners[1][axis],u);
                    const float second = std::lerp(corners[3][axis],corners[2][axis],u);
                    vertex.position[axis] = std::lerp(first,second,v);
                }
                vertex.uv = {vertex.position[0]/150,vertex.position[1]/150};
                vertex.secondary_uv = {vertex.position[0]/50,(vertex.position[1]+0.3f*vertex.position[0])/50};
            }
        }
        std::size_t index = 0;
        for (unsigned y = 0; y < rows; ++y) {
            for (unsigned x = 0; x < columns; ++x) {
                const unsigned first = y*(columns+1)+x;
                for (const unsigned vertex : {first,first+columns+2,first+columns+1,
                    first,first+1,first+columns+2}) m_indices[index++] = vertex;
            }
        }
        return true;
    }

    bool Assign(std::span<const WaterVertex> vertices, std::span<const std::uint32_t> indices)
    {
        constexpr auto maximum_bytes = std::numeric_limits<std::uint32_t>::max();
        if (vertices.size() > maximum_bytes / sizeof(WaterVertex)
            || indices.size() > maximum_bytes / sizeof(std::uint32_t)
            || indices.size() % 3 != 0) return false;
        for (const auto &vertex : vertices) {
            for (float value : vertex.position) if (!std::isfinite(value)) return false;
            for (float value : vertex.color) if (!std::isfinite(value)) return false;
            for (float value : vertex.normal) if (!std::isfinite(value)) return false;
            for (float value : vertex.uv) if (!std::isfinite(value)) return false;
        }
        for (std::uint32_t index : indices) if (index >= vertices.size()) return false;
        m_vertices.assign(vertices.begin(), vertices.end());
        m_indices.assign(indices.begin(), indices.end());
        return true;
    }

    std::span<const WaterVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

private:
    std::vector<WaterVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
}
