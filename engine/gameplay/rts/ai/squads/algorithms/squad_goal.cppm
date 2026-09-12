module;

#include <cstdint>
#include <limits>
#include <stdexcept>

export module engine.gameplay.rts.ai.squads.algorithms.squad_goal;

export import engine.gameplay.rts.ai.squads.components.squad_goal;

export namespace engine::gameplay::rts::ai::squads
{

// Typed goal setup uses ValidateSquadGoal at its boundary.  The runtime system
// only checks this side-effect-free invariant in debug builds after setup.
inline bool IsValidSquadGoal(const SquadGoal &goal,
	const navigation::NavigationGrid &grid) noexcept
{
	const auto noTarget = !goal.target.IsValid();
	const auto noDestination = goal.destination == navigation::InvalidCell;

	switch (goal.kind)
	{
	case SquadGoalKind::None:
		return noTarget && noDestination && goal.searchRadiusCells == 0;
	case SquadGoalKind::Stop:
		return goal.owner.IsValid() && noTarget && noDestination && goal.searchRadiusCells == 0;
	case SquadGoalKind::AttackTarget:
		return goal.owner.IsValid() && goal.target.IsValid() && noDestination &&
			goal.searchRadiusCells == 0;
	case SquadGoalKind::AttackMove:
		return goal.owner.IsValid() && noTarget && !noDestination && goal.searchRadiusCells == 0 &&
			grid.Walkable(goal.destination);
	case SquadGoalKind::Hunt:
		return goal.owner.IsValid() && noTarget && noDestination && goal.searchRadiusCells != 0;
	default:
		return false;
	}
}

inline void ValidateSquadGoal(const SquadGoal &goal, const navigation::NavigationGrid &grid)
{
	if (IsValidSquadGoal(goal, grid))
		return;
	throw std::invalid_argument("Squad goal has invalid kind or payload");
}

inline std::uint64_t NextSquadGoalRevision(const std::uint64_t revision)
{
	if (revision == (std::numeric_limits<std::uint64_t>::max)())
		throw std::overflow_error("Squad goal revision overflow");
	return revision + 1;
}

} // namespace engine::gameplay::rts::ai::squads
