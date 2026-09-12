module;

#include <cstdint>
#include <limits>
#include <string_view>

export module engine.gameplay.rts.visibility.components.visibility_observer;
export import engine.ecs.core.component_registry;
export import engine.gameplay.navigation.grid.navigation_grid;

export namespace engine::gameplay::rts::visibility
{
inline constexpr std::uint32_t MaxObserverSlots = 64;
using ObserverMask = std::uint64_t;
using ObserverSlot = std::uint8_t;
using VisibilityDefinitionId = std::uint32_t;

struct ParticipantHandle final
{
	ObserverSlot slot{static_cast<ObserverSlot>(MaxObserverSlots)};
	std::uint32_t generation{};

	constexpr bool IsValid() const noexcept
	{
		return slot < MaxObserverSlots && generation != 0;
	}
	friend constexpr bool operator==(const ParticipantHandle &, const ParticipantHandle &) noexcept = default;
};

constexpr bool Less(const ParticipantHandle left, const ParticipantHandle right) noexcept
{
	return left.slot < right.slot || (left.slot == right.slot && left.generation < right.generation);
}

constexpr ObserverMask ObserverBit(const ObserverSlot slot) noexcept
{
	return slot < MaxObserverSlots ? ObserverMask{1} << slot : ObserverMask{};
}

struct VisibilityObserver final
{
	ParticipantHandle participant{};
	VisibilityDefinitionId definition{};
};

// This is a transient policy projection. It is never used as the historical
// source of grace state; VisibilityHistoryPage owns that state after a live
// actor disappears.
struct VisibilityEligibility final
{
	bool active{};
	bool resolvedCellValid{};
	engine::gameplay::navigation::Cell resolvedCell{engine::gameplay::navigation::InvalidCell};
	bool radiusOverride{};
	std::uint32_t radiusCells{};
};
} // namespace engine::gameplay::rts::visibility

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityObserver>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.observer";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityEligibility>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.eligibility";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
} // namespace ecs
