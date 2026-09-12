module;
#include <algorithm>
#include <cassert>
#include <cstdint>
export module engine.gameplay.rts.production.algorithms.advance_build;
export import engine.gameplay.rts.production.components.build_work;
export namespace engine::gameplay::rts::production
{
// Rational work per eligible fixed step; no floating point percentage decides
// completion. Paused work retains integral/fractional progress; no catch-up burst.
// Shared arithmetic; callers retain their own typed chunk selection and policy.
inline void AdvanceBuild(BuildWork &value, const BuildEnabled &enabled, BuildReady &ready) noexcept
{
    ready.value = false;
    if (!enabled.value) return;
    assert(value.speedDenominator != 0);
    const auto denominator = std::max(value.speedDenominator, std::uint32_t{1});
    value.elapsed = std::min(value.elapsed, value.required);
    if (value.elapsed < value.required)
    {
        const auto credit = value.remainder % denominator + value.speedNumerator;
        value.elapsed += std::min(credit / denominator, value.required - value.elapsed);
        value.remainder = credit % denominator;
    }
    ready.value = value.elapsed >= value.required;
}

}
