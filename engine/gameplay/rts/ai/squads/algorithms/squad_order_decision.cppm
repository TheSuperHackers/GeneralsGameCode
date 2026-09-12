module;

#include <cstdint>

export module engine.gameplay.rts.ai.squads.algorithms.squad_order_decision;

export import engine.ecs.core.entity;
export import engine.gameplay.rts.ai.squads.components.squad_goal;
export import engine.gameplay.rts.orders.components.unit_order;

export namespace engine::gameplay::rts::ai::squads
{

enum class SquadDecisionRejection : std::uint8_t
{
	None,
	Dead,
	Contained,
	WrongOwner,
	InvalidPosition,
	MissingWeapon,
	NoTarget,
	InvalidGoal
};

struct SquadMemberDecisionInput
{
	ecs::Entity actor{};
	ecs::Entity owner{};
	bool alive{};
	bool contained{};
	bool positionValid{};
	bool hasWeapon{};
	ecs::Entity target{};
	bool targetVisible{};
};

struct SquadOrderDecision
{
	bool emit{};
	SquadDecisionRejection rejection{SquadDecisionRejection::None};
	engine::gameplay::rts::orders::UnitOrder order{};
};

// This is the policy-independent mapping from a validated goal plus a ZH-free
// member/visibility observation to one request.  ZH supplies targetVisible;
// this algorithm never imports or scans a game-specific TargetFrame.
inline SquadOrderDecision DecideSquadOrder(const SquadGoal &goal,
	const SquadMemberDecisionInput &member) noexcept
{
	using engine::gameplay::rts::orders::OrderKind;

	if (goal.kind == SquadGoalKind::None)
		return {};
	if (!member.alive)
		return {false, SquadDecisionRejection::Dead, {}};
	if (member.contained)
		return {false, SquadDecisionRejection::Contained, {}};
	if (!member.positionValid)
		return {false, SquadDecisionRejection::InvalidPosition, {}};
	if (member.owner != goal.owner)
		return {false, SquadDecisionRejection::WrongOwner, {}};

	switch (goal.kind)
	{
	case SquadGoalKind::Stop:
		return {true, SquadDecisionRejection::None,
			{OrderKind::Stop, engine::gameplay::navigation::InvalidCell, {}, {}}};
	case SquadGoalKind::AttackMove:
		if (!member.hasWeapon)
			return {false, SquadDecisionRejection::MissingWeapon, {}};
		return {true, SquadDecisionRejection::None,
			{OrderKind::AttackMove, goal.destination, {}, {}}};
	case SquadGoalKind::AttackTarget:
	case SquadGoalKind::Hunt:
		if (!member.hasWeapon)
			return {false, SquadDecisionRejection::MissingWeapon, {}};
		if (!member.target.IsValid() || !member.targetVisible)
			return {false, SquadDecisionRejection::NoTarget, {}};
		return {true, SquadDecisionRejection::None,
			{OrderKind::AttackTarget, engine::gameplay::navigation::InvalidCell, member.target, {}}};
	case SquadGoalKind::None:
		return {};
	default:
		return {false, SquadDecisionRejection::InvalidGoal, {}};
	}
}

} // namespace engine::gameplay::rts::ai::squads
