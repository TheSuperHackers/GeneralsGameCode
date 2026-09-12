module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import engine.ecs.core.component_registry;

export namespace generalszh::bounty
{
enum class CashBountyCostBasis : std::uint8_t
{
	DecodedBuildCost = 0
};

// This is an authored BuildCost snapshot. It is deliberately not the paid
// production receipt and does not claim to include the legacy owner-dependent
// template/KindOf/handicap modifier stack, which has no modern typed owner.
struct CashBountyCostBinding final
{
	std::uint32_t authoredBuildCost{};
	CashBountyCostBasis basis{CashBountyCostBasis::DecodedBuildCost};
	// Explicit content-bound enrollment policy. The adapter sets this only for
	// victims that are not KINDOF_IGNORED_IN_GUI; it is independent of the
	// veterancy/XP binding and fails closed when enrollment omitted the policy.
	bool eligible{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::bounty::CashBountyCostBinding>
{
	static constexpr std::string_view StableName = "games.generalszh.bounty.cost_binding";
	static constexpr std::uint32_t Version = 2;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
