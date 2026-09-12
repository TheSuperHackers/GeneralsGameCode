module;

#include <cstddef>
#include <string_view>

export module games.generalszh.gameplay.bounty.systems.cash_bounty_policy_system;
export import engine.ecs.system.system;
export import engine.gameplay.rts.bounty.algorithms.bounty_amount;
export import engine.gameplay.rts.bounty.components.bounty_policy;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import games.generalszh.gameplay.bounty.definitions.cash_bounty_catalog;
export import engine.gameplay.rts.unlocks.systems.unlock_system;

export namespace generalszh::bounty
{
class CashBountyPolicySystem final
{
public:
	using Policy = engine::gameplay::rts::bounty::BountyPolicy;
	using UnlockState = engine::gameplay::rts::unlocks::UnlockState;
	using Query = ecs::Query<ecs::Write<Policy>, ecs::Read<UnlockState>>;

	explicit CashBountyPolicySystem(const CashBountyCatalog &catalog) : catalog_(catalog) {}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		auto policies = chunk.Get<Policy>();
		const auto states = chunk.Get<UnlockState>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto owned = catalog_.MaximumOwned(states[row]);
			if (engine::gameplay::rts::bounty::GreaterBountyRate(owned, policies[row].maximum))
				policies[row].maximum = owned;
		}
	}

private:
	const CashBountyCatalog &catalog_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::bounty::CashBountyPolicySystem>
{
	static constexpr std::string_view StableName = "games.generalszh.bounty.policy";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	// The policy snapshots owned sciences before UnlockSystem publishes this
	// tick's purchase/grant. The root must also place this node before RankSystem
	// when rank publication is present; that edge is conditional on the root's
	// optional rank system and therefore cannot be encoded here without making
	// the standalone bounty graph require rank.
	using Before = SystemTypeList<engine::gameplay::rts::unlocks::UnlockSystem>;
	using After = SystemTypeList<>;
};
}
