module;
#include <cstddef>
#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.power.components.power_recovery;
export import engine.ecs.core.component_registry;

export namespace generalszh::power
{
// Local adapter routing only; neither slots nor player indices are wire identity.
struct PowerRecoveryOwner { std::size_t slot{}; std::int32_t player{-1}; };
// A fact requesting a joined brownout refresh, not a cached power balance.
struct PowerRecovery { bool recovered{false}; };
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::power::PowerRecoveryOwner>
{
    static constexpr std::string_view StableName = "games.generalszh.power.recovery_owner";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
template<> struct ComponentTraits<generalszh::power::PowerRecovery>
{
    static constexpr std::string_view StableName = "games.generalszh.power.recovery";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
