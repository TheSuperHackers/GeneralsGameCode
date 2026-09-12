module;
#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.power.components.power_suppression;
export import engine.ecs.core.component_registry;

export namespace generalszh::power
{
// Separate SoA column from the integral power ledger. Zero cancels the pending
// recovery, matching Zero Hour's policy; there is no hidden countdown or clock.
struct PowerSuppression
{
    std::uint64_t untilTick{0};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::power::PowerSuppression>
{
    static constexpr std::string_view StableName = "games.generalszh.power.suppression";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
