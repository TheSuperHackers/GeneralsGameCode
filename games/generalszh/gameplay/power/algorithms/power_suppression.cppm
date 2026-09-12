module;
#include <cassert>
#include <cstdint>
#include <limits>

export module games.generalszh.gameplay.power.algorithms.power_suppression;
export import games.generalszh.gameplay.power.components.power_suppression;
export import engine.time.simulation_time;

export namespace generalszh::power
{
inline void SetSuppressionUntil(PowerSuppression &suppression, std::uint64_t deadline) noexcept
{
    suppression.untilTick = deadline;
}

// Typed authoring/setup entry point. Quantize duration once, not in hot reads.
inline void ArmSuppressionAfter(PowerSuppression &suppression,
    engine::time::SimulationTime now, engine::time::Duration duration)
{
    const auto ticks = now.Step().TicksFor(duration);
    assert(ticks <= (std::numeric_limits<std::uint64_t>::max)() - now.Tick());
    SetSuppressionUntil(suppression, now.Tick() + ticks);
}

inline bool IsSuppressed(const PowerSuppression &suppression, std::uint64_t now) noexcept
{
    return now < suppression.untilTick;
}

// Visibility recovers at equality; the explicit recovery action follows only
// after the deadline. Consume before emitting effects so reentrant rearming
// survives. This conditional is gameplay state transition, not a sanity guard.
inline bool ConsumeSuppressionRecovery(PowerSuppression &suppression, std::uint64_t now) noexcept
{
    const bool recovered = suppression.untilTick != 0 && now > suppression.untilTick;
    if (recovered) suppression.untilTick = 0;
    return recovered;
}
}
