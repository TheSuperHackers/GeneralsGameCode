module;
#include <compare>
#include <cstdint>
export module engine.events.ordering.batch_order;

export namespace engine::events
{
// Assigned by the simulation owner before recording. A boundary is one explicit
// publication (e.g. one scheduler wave), not a wall-clock interval.
struct BatchBoundary
{
	std::uint64_t tick{0};
	std::uint32_t phase{0};
	friend constexpr auto operator<=>(const BatchBoundary &, const BatchBoundary &) = default;
};
struct BatchOrder
{
	BatchBoundary boundary{};
	std::uint32_t systemOrder{0};
	std::uint32_t jobOrder{0};
	std::uint32_t chunkOrder{0};
	friend constexpr auto operator<=>(const BatchOrder &, const BatchOrder &) = default;
};
// Local command/event sequence is the index within a recorded batch. There are
// no atomically generated IDs, pointer keys, worker IDs or completion timestamps.
}
