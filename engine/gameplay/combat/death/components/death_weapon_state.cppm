module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.death.components.death_weapon_state;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.spatial.algorithms.grid_radius;

export namespace engine::gameplay::combat::death
{
// A compact immutable-definition reference.  The definition itself is owned by
// the composition catalog; entities retain only the stable key.
struct DeathWeaponBinding
{
    std::uint32_t definition{};
};

enum class DeathSourceDisposition : std::uint8_t
{
    Eligible,
    Exempt,
    Invalid
};

// Returned by a game's narrow source projection at the death boundary.  This
// is a transient capture, not an entity component and not a second owner of
// live position/ownership state.
struct DeathSourceCapture
{
    DeathSourceDisposition disposition{DeathSourceDisposition::Invalid};
    ecs::Entity account{};
    std::uint32_t definition{};
    spatial::GridPoint position{};
};

struct DeathWeaponSource
{
    ecs::Entity entity{};
    ecs::Entity account{};
    std::uint32_t definition{};
    spatial::GridPoint position{};
};

// The state is written once when the health/life transition is observed.  It
// makes initial/final emission and deferred destruction idempotent across
// ticks without duplicating the source's continuously changing components.
struct DeathWeaponState
{
    DeathWeaponSource source{};
    std::uint64_t deathTick{};
    std::uint64_t destructionDeadline{};
    bool armed{};
    bool initialEmitted{};
    bool finalEmitted{};
    bool destructionQueued{};
    bool suppressed{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::combat::death::DeathWeaponBinding>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.death_weapon_binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::death::DeathWeaponState>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.death_weapon_state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
