module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Water.OceanPatches;

namespace Graphics {
export struct OceanPatchGrid final {
    std::array<float,2> minimum{};
    std::array<float,2> maximum{};
    std::array<float,3> position{};
    float width = 1;
    float scale = 1;
};



export class OceanPatches final {
public:
    bool Prepare(const OceanPatchGrid& grid) {
        const float spacing = grid.width * grid.scale;
        assert(std::isfinite(spacing) && spacing > 0);
        assert(std::isfinite(grid.position[0]) && std::isfinite(grid.position[1]) && std::isfinite(grid.position[2]));
        Key key;
        key.position = grid.position; key.width = grid.width; key.scale = grid.scale;
        for (unsigned axis=0; axis<2; ++axis) {
            const double first = std::floor((grid.minimum[axis]-grid.position[axis]) / spacing);
            const double end = std::ceil((grid.maximum[axis]-grid.position[axis]) / spacing);
            constexpr auto limit = (std::numeric_limits<std::int32_t>::max)() - 1;
            assert(std::isfinite(first) && std::isfinite(end));
            assert(first >= -limit && first <= limit && end >= -limit && end <= limit);
            key.first[axis] = static_cast<std::int32_t>(first);
            key.end[axis] = static_cast<std::int32_t>(end);


            const auto origin = [&](std::int32_t patch) { return grid.position[axis] + patch * grid.width * grid.scale; };
            while (key.end[axis] < limit && origin(key.end[axis]) < grid.maximum[axis]) ++key.end[axis];
            while (key.end[axis] > -limit && origin(key.end[axis]-1) >= grid.maximum[axis]) --key.end[axis];
        }
        if (m_valid && key == m_key) return true;
        const auto columns = (std::max)(std::int64_t{0}, std::int64_t{key.end[0]}-key.first[0]);
        const auto rows = (std::max)(std::int64_t{0}, std::int64_t{key.end[1]}-key.first[1]);
        constexpr auto maximum = (std::numeric_limits<std::uint32_t>::max)() / 64u;
        if (columns > maximum || rows > maximum || columns * rows > maximum) return false;
        m_worlds.resize(static_cast<std::size_t>(columns * rows));
        std::size_t index = 0;
        for (auto y = key.first[1]; y < key.end[1]; ++y) {
            const float world_y = grid.position[1]+y*spacing;
            for (auto x = key.first[0]; x < key.end[0]; ++x) {
                const float world_x = grid.position[0]+x*spacing;
                m_worlds[index++] = {grid.scale,0,0,world_x,
                    0,0,grid.scale,world_y,0,1,0,grid.position[2],0,0,0,1};
            }
        }
        assert(index == m_worlds.size());
        m_key = key; m_valid = true; ++m_revision;
        return true;
    }
    std::span<const std::array<float,16>> Worlds() const noexcept { return m_worlds; }
    std::uint64_t Revision() const noexcept { return m_revision; }
private:
    struct Key final {
        std::array<std::int32_t,2> first{}, end{};
        std::array<float,3> position{};
        float width=1, scale=1;
        bool operator==(const Key&) const = default;
    };
    Key m_key;
    bool m_valid = false;
    std::uint64_t m_revision = 0;
    std::vector<std::array<float,16>> m_worlds;
};
}
