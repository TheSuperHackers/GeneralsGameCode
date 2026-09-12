module;

#include <cstdint>

export module games.generalszh.gameplay.production.presentation.definitions.construction_marker_timing;

export import engine.time.simulation_time;

export namespace generalszh::production
{
struct ConstructionMarkerTiming
{
	std::uint64_t duration;
};

inline ConstructionMarkerTiming AuthorConstructionMarkerTiming(
	const engine::time::Duration duration,
	const engine::time::FixedStep step)
{
	return ConstructionMarkerTiming{step.TicksFor(duration)};
}
}
