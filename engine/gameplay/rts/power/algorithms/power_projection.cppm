module;
#include <cstdint>
export module engine.gameplay.rts.power.algorithms.power_projection;
export import engine.gameplay.rts.power.components.power_source;
export namespace engine::gameplay::rts::power
{
// Pure source projection shared by independently composed power behaviours.
inline PowerContribution ProjectPower(const PowerSource &source, bool alive = true) noexcept
{
    if (!source.eligible || !alive) return {};
    const auto value = static_cast<std::int64_t>(source.authoredPower);
    return value >= 0 ? PowerContribution{static_cast<std::uint32_t>(value), 0}
        : PowerContribution{0, static_cast<std::uint32_t>(-value)};
}
}
