module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.bounty.components.bounty_policy;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::bounty
{
struct BountyRate final
{
	std::uint32_t numerator{};
	std::uint32_t denominator{1};

	friend constexpr bool operator==(const BountyRate &, const BountyRate &) noexcept = default;
};

constexpr bool IsValidBountyRate(const BountyRate rate) noexcept
{
	return rate.denominator != 0 && rate.numerator <= rate.denominator;
}

// Account-owned persistent maximum. UnlockState remains the science source of
// truth; this component is the durable policy projection used by reward joins.
struct BountyPolicy final
{
	BountyRate maximum{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::bounty::BountyPolicy>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.bounty.policy";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
