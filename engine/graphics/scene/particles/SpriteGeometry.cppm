module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Particles.SpriteGeometry;
import Assets.Math;
export import Graphics.Scene.Props.Geometry;

namespace Graphics {

export enum class SpriteShape { Triangle, Quad };

export bool Get_Sprite_Atlas_Region(std::uint32_t frame, std::uint32_t columns,
    std::uint32_t rows, std::array<float, 4>& result) noexcept
{
    if (columns == 0 || rows == 0) return false;
    const auto index = frame % (std::uint64_t{columns} * rows);
    const float u = static_cast<float>(index % columns) / columns;
    const float v = static_cast<float>(index / columns) / rows;
    result = {u, v, u + 1.0f / columns, v + 1.0f / rows};
    return true;
}

export struct SpritePoint final {
    std::array<float, 3> position{};
    std::array<float, 4> color{1, 1, 1, 1};
    float size = 1;
    float angle = 0;
    std::array<float, 4> texture_region{0, 0, 1, 1};
};

// Generates camera-facing geometry for material-shaded sprites. The caller
// supplies active points in draw order; the result owns all vertex attributes.
export class SpriteGeometry final {
public:
    template<class ReadPoint>
    bool Build(std::size_t count, SpriteShape shape,
        const std::array<float, 16>& view, ReadPoint&& read_point)
    {
        Clear();
        if (shape != SpriteShape::Triangle && shape != SpriteShape::Quad) return false;
        const std::size_t corners = shape == SpriteShape::Triangle ? 3 : 4;
        const std::size_t indices = shape == SpriteShape::Triangle ? 3 : 6;
        if (count > std::numeric_limits<std::uint32_t>::max() / indices || !Finite(view)) return false;
        m_vertices.resize(count * corners);
        m_indices.resize(count * indices);
        constexpr std::array<std::array<float, 2>, 4> quad_positions{{{-.5f, .5f}, {-.5f, -.5f}, {.5f, -.5f}, {.5f, .5f}}};
        constexpr std::array<std::array<float, 2>, 4> quad_uv{{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
        constexpr std::array<std::array<float, 2>, 3> triangle_positions{{{0, -2}, {-1.732f, 1}, {1.732f, 1}}};
        constexpr std::array<std::array<float, 2>, 3> triangle_uv{{{.5f, 0}, {0, .866f}, {1, .866f}}};
        constexpr std::array<unsigned, 6> quad_indices{0, 1, 2, 2, 3, 0};
        constexpr std::array<unsigned, 3> triangle_indices{0, 2, 1};
        for (std::size_t i = 0; i < count; ++i) {
            const SpritePoint point = read_point(i);
            if (!Finite(point.position) || !Finite(point.color) || !Finite(point.texture_region)
                || !std::isfinite(point.size) || !std::isfinite(point.angle)) {
                Clear();
                return false;
            }
            std::array<float, 3> center{};
            for (unsigned row = 0; row < 3; ++row)
                center[row] = view[row * 4] * point.position[0] + view[row * 4 + 1] * point.position[1]
                    + view[row * 4 + 2] * point.position[2] + view[row * 4 + 3];
            const float cosine = std::cos(point.angle), sine = std::sin(point.angle);
            const auto packed = Assets::Color_To_ARGB({point.color[0], point.color[1], point.color[2], point.color[3]});
            const std::array<float, 4> color{((packed >> 16) & 255) / 255.0f,
                ((packed >> 8) & 255) / 255.0f, (packed & 255) / 255.0f, ((packed >> 24) & 255) / 255.0f};
            for (std::size_t corner = 0; corner < corners; ++corner) {
                const auto offset = shape == SpriteShape::Triangle ? triangle_positions[corner] : quad_positions[corner];
                const auto uv = shape == SpriteShape::Triangle ? triangle_uv[corner] : quad_uv[corner];
                auto& vertex = m_vertices[i * corners + corner];
                vertex.position = {center[0] + (offset[0] * cosine - offset[1] * sine) * point.size,
                    center[1] + (offset[0] * sine + offset[1] * cosine) * point.size, center[2]};
                vertex.uv = {point.texture_region[0] + uv[0] * (point.texture_region[2] - point.texture_region[0]),
                    point.texture_region[1] + uv[1] * (point.texture_region[3] - point.texture_region[1])};
                vertex.color = color;
                if (!Finite(vertex.position) || !Finite(vertex.uv)) { Clear(); return false; }
            }
            for (std::size_t index = 0; index < indices; ++index)
                m_indices[i * indices + index] = static_cast<std::uint32_t>(i * corners
                    + (shape == SpriteShape::Triangle ? triangle_indices[index] : quad_indices[index]));
        }
        return true;
    }

    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

private:
    template<std::size_t N>
    static bool Finite(const std::array<float, N>& values) noexcept {
        return std::all_of(values.begin(), values.end(), [](float value) { return std::isfinite(value); });
    }
    void Clear() noexcept { m_vertices.clear(); m_indices.clear(); }
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
}
