module;
#include <cstdint>
#include <limits>
#include <stdexcept>

export module engine.time.simulation_clock;
export import engine.time.simulation_time;

export namespace engine::time
{
// Explicitly owned fixed-step timeline. The driver executes Current(), then
// advances only after a successful step. Pausing means not dispatching a step;
// rendering and wall-clock pacing do not mutate simulation time themselves.
class SimulationClock
{
public:
	explicit constexpr SimulationClock(FixedStep step, std::uint64_t initialTick = 0) noexcept :
		m_step(step), m_tick(initialTick) {}
	SimulationClock(const SimulationClock &) = delete;
	SimulationClock &operator=(const SimulationClock &) = delete;
	constexpr SimulationTime Current() const noexcept { return {m_tick, m_step}; }
	constexpr void Advance()
	{
		if (m_tick == (std::numeric_limits<std::uint64_t>::max)())
			throw std::overflow_error("Simulation clock exhausted its tick range");
		++m_tick;
	}

private:
	const FixedStep m_step;
	std::uint64_t m_tick;
};
}
