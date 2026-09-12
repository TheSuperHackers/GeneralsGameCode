module;
#include <cstdint>
#include <optional>

export module games.generalszh.gameplay.power.algorithms.power_contribution;
export import engine.gameplay.rts.power.algorithms.power_ledger;

export namespace generalszh::power
{
enum class PowerChannel : std::uint8_t
{
    Production,
    Consumption
};

struct PowerContribution
{
    PowerChannel channel;
    std::int32_t delta;
};

inline std::int32_t NegatePowerDelta(std::int32_t value)
{
    return engine::gameplay::rts::power::CheckedNegate(value);
}

inline std::int32_t CheckedPowerDelta(std::int32_t total, std::int32_t delta)
{
    return engine::gameplay::rts::power::CheckedAdd(total, delta);
}

inline std::optional<PowerContribution> PlanContribution(std::int32_t configured, bool adding)
{
    if (configured == 0)
        return std::nullopt;

    if (configured > 0)
        return PowerContribution{
            PowerChannel::Production,
            adding ? configured : engine::gameplay::rts::power::CheckedNegate(configured)};

    return PowerContribution{
        PowerChannel::Consumption,
        adding ? engine::gameplay::rts::power::CheckedNegate(configured) : configured};
}
}
