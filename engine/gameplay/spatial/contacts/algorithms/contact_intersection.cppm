module;

#include <cassert>
#include <cstdint>

export module engine.gameplay.spatial.contacts.algorithms.contact_intersection;
export import engine.gameplay.spatial.contacts.algorithms.contact_geometry;

export namespace engine::gameplay::spatial::contacts
{
	inline constexpr std::uint64_t Square(const std::uint64_t value) noexcept
	{
		assert(value <= MaxBroadphaseMicrocells);
		return value * value;
	}

	inline constexpr std::uint64_t AbsoluteDifference(const FixedCoordinate left,
		const FixedCoordinate right) noexcept
	{
		return left > right ? left - right : right - left;
	}

	inline constexpr bool WithinCircle(const FixedCoordinate dx,
		const FixedCoordinate dy, const std::uint64_t radius) noexcept
	{
		const auto radiusSquared = Square(radius);
		const auto dxSquared = Square(dx);
		if (dxSquared > radiusSquared) return false;
		const auto dySquared = Square(dy);
		return dySquared <= radiusSquared - dxSquared;
	}

	inline constexpr bool CircleCircle(const ContactGeometry left, const FixedCoordinate leftX,
		const FixedCoordinate leftY, const ContactGeometry right, const FixedCoordinate rightX,
		const FixedCoordinate rightY) noexcept
	{
		const auto radius = std::uint64_t{left.extentX} + right.extentX;
		return WithinCircle(AbsoluteDifference(leftX, rightX), AbsoluteDifference(leftY, rightY), radius);
	}

	inline constexpr bool BoxBox(const ContactGeometry left, const FixedCoordinate leftX,
		const FixedCoordinate leftY, const ContactGeometry right, const FixedCoordinate rightX,
		const FixedCoordinate rightY) noexcept
	{
		return AbsoluteDifference(leftX, rightX) <= std::uint64_t{left.extentX} + right.extentX &&
			AbsoluteDifference(leftY, rightY) <= std::uint64_t{left.extentY} + right.extentY;
	}

	inline constexpr bool BoxCircle(const ContactGeometry box, const FixedCoordinate boxX,
		const FixedCoordinate boxY, const ContactGeometry circle, const FixedCoordinate circleX,
		const FixedCoordinate circleY) noexcept
	{
		const auto dx = AbsoluteDifference(boxX, circleX);
		const auto dy = AbsoluteDifference(boxY, circleY);
		const auto closestX = dx > box.extentX ? dx - box.extentX : 0;
		const auto closestY = dy > box.extentY ? dy - box.extentY : 0;
		return WithinCircle(closestX, closestY, circle.extentX);
	}

	inline constexpr bool Intersects(const ContactGeometry left, const FixedCoordinate leftX,
		const FixedCoordinate leftY, const ContactGeometry right, const FixedCoordinate rightX,
		const FixedCoordinate rightY) noexcept
	{
		assert(IsValidGeometry(left) && IsValidGeometry(right));
		if (left.shape == ContactShape::Circle && right.shape == ContactShape::Circle)
			return CircleCircle(left, leftX, leftY, right, rightX, rightY);
		if (left.shape == ContactShape::AxisAlignedBox && right.shape == ContactShape::AxisAlignedBox)
			return BoxBox(left, leftX, leftY, right, rightX, rightY);
		if (left.shape == ContactShape::AxisAlignedBox)
			return BoxCircle(left, leftX, leftY, right, rightX, rightY);
		return BoxCircle(right, rightX, rightY, left, leftX, leftY);
	}
}
