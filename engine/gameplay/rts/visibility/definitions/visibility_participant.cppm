module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

export module engine.gameplay.rts.visibility.definitions.visibility_participant;
export import engine.ecs.core.entity;
export import engine.gameplay.rts.visibility.components.visibility_observer;

export namespace engine::gameplay::rts::visibility
{
// The game composition owns one table of account enrollments.  This is a
// narrow startup mapping, not live ownership state and not a service locator.
// Entries are never retired or reused during a simulation run: an entity
// generation therefore remains part of the lookup key for the whole run.
class VisibilityParticipantTable final
{
public:
	bool CanEnroll() const noexcept { return nextSlot_ < MaxObserverSlots; }

	ParticipantHandle Enroll(const ecs::Entity account)
	{
		if (!account.IsValid()) throw std::invalid_argument("Visibility participant account must be valid");
		if (Resolve(account).IsValid()) throw std::logic_error("Visibility participant account is already enrolled");
		if (!CanEnroll()) throw std::length_error("Visibility participant slot capacity exhausted");
		const auto slot = static_cast<ObserverSlot>(nextSlot_++);
		entries_[slot] = account;
		return ParticipantHandle{slot, account.generation};
	}

	ParticipantHandle Resolve(const ecs::Entity account) const noexcept
	{
		if (!account.IsValid()) return {};
		for (std::size_t slot = 0; slot != nextSlot_; ++slot)
			if (entries_[slot] == account)
				return ParticipantHandle{static_cast<ObserverSlot>(slot), account.generation};
		return {};
	}

	std::size_t Count() const noexcept { return nextSlot_; }

private:
	std::array<ecs::Entity, MaxObserverSlots> entries_{};
	std::size_t nextSlot_{};
};
} // namespace engine::gameplay::rts::visibility
