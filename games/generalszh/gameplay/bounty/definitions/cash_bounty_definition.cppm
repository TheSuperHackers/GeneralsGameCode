module;

#include <string_view>

export module games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;
export import engine.gameplay.rts.bounty.components.bounty_policy;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace generalszh::bounty
{
struct CashBountyDefinition final
{
	engine::gameplay::rts::unlocks::UnlockKey science{};
	engine::gameplay::rts::bounty::BountyRate rate{};
};
}
