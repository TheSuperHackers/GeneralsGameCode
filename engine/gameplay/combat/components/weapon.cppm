module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.components.weapon;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.spatial.algorithms.grid_radius;
export namespace engine::gameplay::combat
{
enum class RadiusDamageAffect : std::uint8_t {
    Allies = 1U,
    Enemies = 2U,
    Neutrals = 4U,
    Self = 8U,
};
struct RadiusDamageAffects {
    static constexpr std::uint8_t SupportedBits =
        static_cast<std::uint8_t>(RadiusDamageAffect::Allies) |
        static_cast<std::uint8_t>(RadiusDamageAffect::Enemies) |
        static_cast<std::uint8_t>(RadiusDamageAffect::Neutrals) |
        static_cast<std::uint8_t>(RadiusDamageAffect::Self);
    std::uint8_t bits{static_cast<std::uint8_t>(RadiusDamageAffect::Allies) |
                      static_cast<std::uint8_t>(RadiusDamageAffect::Enemies) |
                      static_cast<std::uint8_t>(RadiusDamageAffect::Neutrals)};
    [[nodiscard]] constexpr bool Contains(const RadiusDamageAffect affect) const noexcept
    {
        return (bits & static_cast<std::uint8_t>(affect)) != 0U;
    }
    [[nodiscard]] constexpr bool IsValid() const noexcept
    {
        return (bits & static_cast<std::uint8_t>(~SupportedBits)) == 0U;
    }
};
enum class AmmunitionPolicy : std::uint8_t { Limited, Unlimited };
enum class PrefirePolicy : std::uint8_t { PerShot, PerAttack, PerClip };
struct WeaponDefinition
{
    static constexpr std::uint32_t Version = 5;
    std::uint64_t damage{}, shotTicks{}, reloadTicks{}, flightTicks{1};
    std::uint32_t rangeCells{4}, clipSize{1};
    bool autoReload{true};
    AmmunitionPolicy ammunition{AmmunitionPolicy::Limited};
    damage::DamageTypeId damageType{};
    damage::DamageChannelId damageChannel{};
    std::uint64_t preAttackTicks{};
    PrefirePolicy preAttackType{PrefirePolicy::PerShot};
    std::uint64_t secondaryDamage{};
    std::uint32_t primaryRadiusCells{};
    std::uint32_t secondaryRadiusCells{};
    RadiusDamageAffects radiusDamageAffects{};
};
struct WeaponState
{
    std::uint64_t readyTick{};
    std::uint32_t ammo{};
    bool reloading{};
    std::uint64_t preAttackReadyTick{};
    ecs::Entity preAttackTarget{};
    ecs::Entity lastAttackTarget{};
    bool preAttackActive{};
    spatial::GridPoint preAttackPosition{};
    spatial::GridPoint lastAttackPosition{};
    bool preAttackPositionValid{};
    bool lastAttackPositionValid{};
    // Explicit validity keeps simulation tick zero a real launch tick.
    bool lastFireTickValid{};
    std::uint64_t lastFireTick{};
};
struct WeaponTarget
{
    // Kept as the first field for source compatibility with existing entity
    // target construction and inspection. positionValid is the discriminant;
    // exactly one representation may be active.
    ecs::Entity entity{};
    spatial::GridPoint position{};
    bool positionValid{};
    [[nodiscard]] bool IsEntity() const noexcept { return entity.IsValid() && !positionValid; }
    [[nodiscard]] bool IsPosition() const noexcept { return !entity.IsValid() && positionValid; }
    [[nodiscard]] bool IsValid() const noexcept { return IsEntity() || IsPosition(); }
    static WeaponTarget ForEntity(const ecs::Entity value) noexcept { return {value, {}, false}; }
    static WeaponTarget ForPosition(const spatial::GridPoint value) noexcept { return {{}, value, true}; }
};
struct WeaponContact { bool valid{}, inRange{}; };
// Derived launch metadata, supplied by the composing game's targeting policy.
// This is not an alternate authority for actor ownership.
struct DamageEmitter { ecs::Entity account{}; std::uint32_t definition{}; };
}
export namespace ecs
{
#define WEAPON_COMPONENT(Type, Key, Policy) template<> struct ComponentTraits<engine::gameplay::combat::Type> { \
 static constexpr std::string_view StableName = Key; static constexpr std::uint32_t Version = 1; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; };
template<> struct ComponentTraits<engine::gameplay::combat::WeaponDefinition> {
 static constexpr std::string_view StableName = "engine.gameplay.combat.weapon_definition";
 static constexpr std::uint32_t Version = engine::gameplay::combat::WeaponDefinition::Version;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::WeaponState> {
 static constexpr std::string_view StableName = "engine.gameplay.combat.weapon_state";
 static constexpr std::uint32_t Version = 4;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::WeaponTarget> {
 static constexpr std::string_view StableName = "engine.gameplay.combat.weapon_target";
 static constexpr std::uint32_t Version = 2;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
WEAPON_COMPONENT(WeaponContact, "engine.gameplay.combat.weapon_contact", Transient)
WEAPON_COMPONENT(DamageEmitter, "engine.gameplay.combat.damage_emitter", Transient)
#undef WEAPON_COMPONENT
}
