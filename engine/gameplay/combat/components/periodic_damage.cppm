module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.components.periodic_damage;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export namespace engine::gameplay::combat
{
struct PeriodicDamagePolicy { std::uint64_t interval{},duration{}; };
struct PeriodicDamage { ecs::Entity source{}; std::uint64_t quantity{},nextTick{},stopTick{}; bool active{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::combat::PeriodicDamagePolicy>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.periodic_damage_policy";
    static constexpr std::uint32_t Version=1;static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::PeriodicDamage>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.periodic_damage";
    static constexpr std::uint32_t Version=1;static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
