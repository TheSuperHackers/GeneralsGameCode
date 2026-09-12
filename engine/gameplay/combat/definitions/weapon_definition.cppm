module;
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
export module engine.gameplay.combat.definitions.weapon_definition;
export import engine.gameplay.combat.components.weapon;
export import engine.time.simulation_time;
export namespace engine::gameplay::combat
{
struct WeaponConfig
{
    std::uint64_t damage{};
    std::uint32_t rangeCells{4}, clipSize{1};
    engine::time::Duration shotInterval{std::chrono::milliseconds{100}}, reloadTime{std::chrono::milliseconds{100}}, flightTime{std::chrono::milliseconds{50}};
    bool autoReload{true};
    AmmunitionPolicy ammunition{AmmunitionPolicy::Limited};
    damage::DamageTypeId damageType{};
    damage::DamageChannelId damageChannel{};
    engine::time::Duration preAttackDelay{};
    PrefirePolicy preAttackType{PrefirePolicy::PerShot};
    std::uint64_t secondaryDamage{};
    std::uint32_t primaryRadiusCells{};
    std::uint32_t secondaryRadiusCells{};
    RadiusDamageAffects radiusDamageAffects{};
};
inline WeaponDefinition AuthorWeapon(const WeaponConfig &config, engine::time::FixedStep step)
{
    if (config.ammunition != AmmunitionPolicy::Limited && config.ammunition != AmmunitionPolicy::Unlimited)
        throw std::invalid_argument("Invalid weapon ammunition policy");
    if (config.ammunition == AmmunitionPolicy::Limited && !config.clipSize)
        throw std::invalid_argument("Limited weapon clips must have positive capacity");
    if (config.preAttackType != PrefirePolicy::PerShot && config.preAttackType != PrefirePolicy::PerAttack &&
        config.preAttackType != PrefirePolicy::PerClip)
        throw std::invalid_argument("Invalid weapon pre-attack policy");
    if (!config.radiusDamageAffects.IsValid())
        throw std::invalid_argument("Invalid radius damage affects mask");
    return {config.damage, step.TicksFor(config.shotInterval), step.TicksFor(config.reloadTime),
        std::max(std::uint64_t{1}, step.TicksFor(config.flightTime)), config.rangeCells,
        config.ammunition == AmmunitionPolicy::Limited ? config.clipSize : 0u, config.autoReload, config.ammunition,
        config.damageType,config.damageChannel,step.TicksFor(config.preAttackDelay),config.preAttackType,
        config.secondaryDamage,config.primaryRadiusCells,config.secondaryRadiusCells,config.radiusDamageAffects};
}
}
