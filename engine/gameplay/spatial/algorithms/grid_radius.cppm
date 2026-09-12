module;
#include <cstdint>
export module engine.gameplay.spatial.algorithms.grid_radius;
export namespace engine::gameplay::spatial
{
struct GridPoint
{
    std::uint32_t x{}, y{};
    friend constexpr bool operator==(const GridPoint &, const GridPoint &) noexcept = default;
};

inline constexpr bool IsWithinGridRadius(const GridPoint center, const GridPoint position,
    const std::uint32_t radius) noexcept
{
    const auto dx = center.x > position.x ? std::uint64_t{center.x - position.x} : std::uint64_t{position.x - center.x};
    const auto dy = center.y > position.y ? std::uint64_t{center.y - position.y} : std::uint64_t{position.y - center.y};
    const auto radiusSquared = std::uint64_t{radius} * radius;
    const auto dxSquared = dx * dx;
    if (dxSquared > radiusSquared) return false;
    const auto dySquared = dy * dy;
    return dySquared <= radiusSquared - dxSquared;
}
}
