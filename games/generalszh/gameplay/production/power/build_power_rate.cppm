module;
#include <algorithm>
export module games.generalszh.gameplay.production.power.build_power_rate;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.power.data.power_frame;
export namespace generalszh::production
{
// Power policy is expressed in basis points. Paused orders retain their authored
// speed and fractional credit until they are eligible to advance again.
inline void ApplyBuildPowerRate(engine::gameplay::rts::production::BuildWork &work,
    bool enabled, EntryKind kind, const power::PowerState *state) noexcept
{
    if (!enabled) return;
    const auto old = std::max(work.speedDenominator, 1u);
    work.remainder = (work.remainder % old) * 10000 / old;
    work.speedNumerator = state && kind == EntryKind::Unit ? state->speedNumerator : 10000u;
    work.speedDenominator = 10000;
}
}
