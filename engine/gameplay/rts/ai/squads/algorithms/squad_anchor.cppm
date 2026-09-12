module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>

export module engine.gameplay.rts.ai.squads.algorithms.squad_anchor;

export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;

export namespace engine::gameplay::rts::ai::squads
{

struct SquadAnchorCandidate
{
	ecs::Entity actor{};
	navigation::Cell position{navigation::InvalidCell};
};

struct SquadAnchor
{
	ecs::Entity actor{};
	navigation::Cell position{navigation::InvalidCell};
};

inline bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return left.index < right.index || (left.index == right.index && left.generation < right.generation);
}

// The lowest generation-safe actor identity is the stable squad anchor.  The
// caller supplies only currently eligible members, so this helper remains a
// reusable deterministic reduction and knows nothing about a game's units or
// visibility policy.
inline std::optional<SquadAnchor> SelectSquadAnchor(
	const std::span<const SquadAnchorCandidate> candidates) noexcept
{
	std::optional<SquadAnchor> result;
	for (const auto &candidate : candidates)
	{
		if (!candidate.actor.IsValid() || candidate.position == navigation::InvalidCell)
			continue;
		if (!result || EntityLess(candidate.actor, result->actor) ||
			(candidate.actor == result->actor && candidate.position < result->position))
			result = SquadAnchor{candidate.actor, candidate.position};
	}
	return result;
}

} // namespace engine::gameplay::rts::ai::squads
