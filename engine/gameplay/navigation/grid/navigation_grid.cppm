module;
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.navigation.grid.navigation_grid;
export namespace engine::gameplay::navigation
{
using Cell = std::uint32_t;
inline constexpr Cell InvalidCell = (std::numeric_limits<Cell>::max)();
// Immutable uniform-cost ground connectivity, independent of game file formats.
class NavigationGrid
{
public:
    NavigationGrid(std::uint32_t width, std::uint32_t height, std::span<const std::uint8_t> walkable) : width(width)
    {
        const auto count = std::uint64_t{width} * height;
        if (!width || !height || count >= InvalidCell || count != walkable.size())
            throw std::invalid_argument("Invalid navigation grid dimensions");
        cells.assign(walkable.begin(), walkable.end());
        for (const auto cell : cells) if (cell > 1) throw std::invalid_argument("Invalid navigation cell");
    }
    std::uint32_t Width() const noexcept { return width; }
    std::uint32_t Count() const noexcept { return static_cast<std::uint32_t>(cells.size()); }
    bool Walkable(Cell cell) const noexcept { return cell < cells.size() && cells[cell] != 0; }
private:
    std::uint32_t width;
    std::vector<std::uint8_t> cells;
};
}
