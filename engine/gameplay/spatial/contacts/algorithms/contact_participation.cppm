module;

#include <cstdint>
#include <limits>
#include <stdexcept>

export module engine.gameplay.spatial.contacts.algorithms.contact_participation;
export import engine.gameplay.spatial.contacts.components.contact_participation;

export namespace engine::gameplay::spatial::contacts
{
	inline constexpr bool IsValidParticipation(const ContactParticipation value) noexcept
	{
		return value.categoryMask != 0;
	}

	inline void ValidateParticipation(const ContactParticipation value)
	{
		if (!IsValidParticipation(value))
			throw std::invalid_argument("Contact participation requires a nonzero category mask");
	}

	inline constexpr bool CanCollide(const ContactParticipation left,
		const ContactParticipation right) noexcept
	{
		return left.enabled && right.enabled &&
			(left.categoryMask & right.collisionMask) != 0 &&
			(right.categoryMask & left.collisionMask) != 0;
	}
}
