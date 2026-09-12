module;
#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.power.components.power_state;
export import engine.ecs.core.component_registry;

export namespace generalszh::power
{
struct PowerPolicy { bool affected{true}; };
struct PowerState
{
    bool brownout{};
    std::uint32_t speedNumerator{10000}, speedDenominator{10000};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::power::PowerPolicy>
{
    static constexpr std::string_view StableName = "games.generalszh.power.policy";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::power::PowerState>
{
    static constexpr std::string_view StableName = "games.generalszh.power.state";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
