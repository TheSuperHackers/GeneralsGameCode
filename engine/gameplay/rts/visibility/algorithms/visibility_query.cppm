module;

#include <cstdint>

export module engine.gameplay.rts.visibility.algorithms.visibility_query;
export import engine.gameplay.rts.visibility.components.visibility_observer;

export namespace engine::gameplay::rts::visibility
{
enum class VisibilityStatus : std::uint8_t
{
	Shrouded,
	Fogged,
	Clear
};

constexpr VisibilityStatus StatusFor(const ObserverMask explored, const ObserverMask visible,
	const ObserverSlot observer) noexcept
{
	const auto bit = ObserverBit(observer);
	if ((visible & bit) != 0) return VisibilityStatus::Clear;
	if ((explored & bit) != 0) return VisibilityStatus::Fogged;
	return VisibilityStatus::Shrouded;
}

constexpr bool CanTargetEntity(const ObserverMask visible, const ObserverSlot observer) noexcept
{
	return (visible & ObserverBit(observer)) != 0;
}

// AttackPosition is a positional order, not an entity target. The caller
// supplies the existing bounded/in-bounds result; fog does not invent a target
// entity or force a position read before this policy is evaluated.
constexpr bool CanTargetPosition(const bool inBounds) noexcept { return inBounds; }
} // namespace engine::gameplay::rts::visibility
