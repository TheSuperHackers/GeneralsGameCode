module;

#include <cassert>
#include <cstdint>
#include <stdexcept>

export module engine.gameplay.spatial.contacts.algorithms.contact_geometry;
export import engine.gameplay.spatial.contacts.components.contact_geometry;

export namespace engine::gameplay::spatial::contacts
{
using FixedCoordinate = std::uint64_t;
inline constexpr std::uint64_t FixedScale = 1'000'000;
inline constexpr std::uint32_t MaxMapDimensionCells = 2'048;
inline constexpr std::uint32_t MaxExtentCells = 512;
inline constexpr std::uint32_t MaxBroadphaseRadiusCells = 2'048;
inline constexpr std::uint32_t MaxExtentMicrocells =
	MaxExtentCells * static_cast<std::uint32_t>(FixedScale);
inline constexpr std::uint64_t MaxBroadphaseMicrocells =
	std::uint64_t{MaxBroadphaseRadiusCells} * FixedScale;

inline constexpr ContactGeometry Circle(const MicroCells radius) noexcept
{
	return {ContactShape::Circle, radius, 0};
}

inline constexpr ContactGeometry AxisAlignedBox(const MicroCells halfWidth,
	const MicroCells halfHeight) noexcept
{
	return {ContactShape::AxisAlignedBox, halfWidth, halfHeight};
}

inline constexpr bool IsValidGeometry(const ContactGeometry geometry) noexcept
{
	if (geometry.extentX == 0 || geometry.extentX > MaxExtentMicrocells)
		return false;
	if (geometry.shape == ContactShape::Circle)
		return geometry.extentY == 0;
	return geometry.shape == ContactShape::AxisAlignedBox &&
		geometry.extentY != 0 && geometry.extentY <= MaxExtentMicrocells;
}

inline void ValidateGeometry(const ContactGeometry geometry)
{
	if (!IsValidGeometry(geometry))
		throw std::invalid_argument("Invalid 2D contact geometry");
}

inline constexpr std::uint32_t CeilCells(const MicroCells value) noexcept
{
	assert(value <= MaxExtentMicrocells);
	return value / static_cast<std::uint32_t>(FixedScale) +
		(value % static_cast<std::uint32_t>(FixedScale) != 0 ? 1u : 0u);
}

inline constexpr std::uint32_t BroadphaseRadiusCells(const ContactGeometry geometry) noexcept
{
	assert(IsValidGeometry(geometry));
	const auto bound = geometry.shape == ContactShape::Circle
		? std::uint64_t{CeilCells(geometry.extentX)}
		: std::uint64_t{CeilCells(geometry.extentX)} + CeilCells(geometry.extentY);
	assert(bound <= MaxBroadphaseRadiusCells);
	return static_cast<std::uint32_t>(bound);
}

inline constexpr FixedCoordinate CellCoordinate(const std::uint32_t cell) noexcept
{
	assert(cell <= MaxMapDimensionCells);
	return FixedCoordinate{cell} * FixedScale;
}
} // namespace engine::gameplay::spatial::contacts
