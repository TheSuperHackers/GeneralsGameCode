module;

#include <cstdint>
#include <limits>
#include <stdexcept>

export module games.generalszh.gameplay.production.progress.definitions.production_requirement;

export import engine.time.simulation_time;

export namespace generalszh::production
{
struct ProductionRequirement
{
	std::int64_t ticks;
};

inline ProductionRequirement AuthorRequirement(const engine::time::Duration duration,
	const engine::time::FixedStep step)
{
	const std::uint64_t ticks = step.TicksFor(duration);
	if (ticks > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
		throw std::overflow_error("Production requirement exceeds signed tick range");
	return ProductionRequirement{static_cast<std::int64_t>(ticks)};
}
}
