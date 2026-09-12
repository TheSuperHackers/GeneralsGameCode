module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.components.projectile;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.combat.components.weapon;
export namespace engine::gameplay::combat
{
struct ProjectileImpact {
    ecs::Entity source{}, target{}; std::uint64_t damage{}, launchTick{}, arrivalTick{};
    damage::DamageTypeId damageType{}; damage::DamageChannelId damageChannel{};
    ecs::Entity sourceAccount{}; std::uint32_t sourceDefinition{};
    std::uint64_t secondaryDamage{};
    std::uint32_t primaryRadiusCells{}, secondaryRadiusCells{};
    RadiusDamageAffects radiusDamageAffects{};
    bool detonationPositionValid{};
    std::uint32_t detonationX{}, detonationY{};
    bool positionalTarget{};
};
struct ImpactDue { bool value{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::combat::ProjectileImpact> {
 static constexpr std::string_view StableName = "engine.gameplay.combat.projectile_impact";
 static constexpr std::uint32_t Version = 4;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::ImpactDue> {
 static constexpr std::string_view StableName = "engine.gameplay.combat.impact_due";
 static constexpr std::uint32_t Version = 1;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
