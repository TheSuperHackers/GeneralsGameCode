module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.combat.components.acquisition_policy;
export import engine.ecs.system.system;
export namespace generalszh::combat
{
struct AcquisitionPolicy { bool enabled{}, attackBuildings{}; };
struct TargetIntent { bool explicitOrder{}; };
inline constexpr std::uint32_t UnitTarget = 1, BuildingTarget = 2;
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::combat::AcquisitionPolicy>
{
    static constexpr std::string_view StableName = "games.generalszh.combat.acquisition_policy";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::combat::TargetIntent>
{
    static constexpr std::string_view StableName = "games.generalszh.combat.target_intent";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
