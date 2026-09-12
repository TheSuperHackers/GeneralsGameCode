module;
#include <cstdint>
export module games.generalszh.gameplay.production.doors.definitions.door_timing;
export import engine.time.simulation_time;
export namespace generalszh::production
{
struct DoorTiming
{
	std::uint64_t opening;
	std::uint64_t waiting;
	std::uint64_t closing;
};

inline DoorTiming AuthorDoorTiming(const engine::time::Duration opening,
	const engine::time::Duration waiting,
	const engine::time::Duration closing,
	const engine::time::FixedStep step)
{
	return DoorTiming{step.TicksFor(opening), step.TicksFor(waiting), step.TicksFor(closing)};
}
}
