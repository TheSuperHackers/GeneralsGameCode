module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.components.health;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export namespace engine::gameplay::combat
{
// Integer health quanta chosen by content adapters, not a hardcoded game unit.
// This slice applies already-resolved damage: armor/type modifiers are upstream.
struct Health { std::uint64_t current{}; };
struct LifeState { bool alive{true}; };
struct LethalSourceEvidence
{
    // Entity handles include their generation. The target handle is retained
    // even though this payload lives on that target, so delayed/reused rows
    // cannot validate an old proof accidentally.
    ecs::Entity target{};
    ecs::Entity source{};
    ecs::Entity sourceAccount{};
    std::uint64_t tick{};
    std::uint64_t healthAtCrossing{};
    std::uint64_t pendingBefore{};
    std::uint64_t pendingAfter{};
    bool valid{};
};

inline bool MatchesLethalTransition(const LethalSourceEvidence &evidence,
    const ecs::Entity target, const std::uint64_t tick,
    const std::uint64_t healthBefore, const std::uint64_t healthAfter,
    const std::uint64_t pendingAfter) noexcept
{
    return evidence.valid && evidence.target == target && evidence.tick == tick
        && healthBefore != 0 && healthAfter == 0
        && evidence.healthAtCrossing == healthBefore
        && evidence.pendingBefore < healthBefore
        && evidence.pendingAfter >= healthBefore
        // Extra unattributed damage may follow the crossing. It does not
        // change which projectile proved the crossing, but pending damage must
        // still be lethal for this actual health transition.
        && pendingAfter >= healthBefore;
}

// Keep the scalar quantity as the first aggregate member. Existing callers
// using PendingDamage{quantity} remain source-compatible; provenance is a
// transient defaulted suffix.
struct PendingDamage { std::uint64_t quantity{}; LethalSourceEvidence lethalSource{}; };
// Keep applied/killed as the first two aggregate members for existing result
// construction. The source proof is populated only for a matching kill.
struct DamageResult { std::uint64_t applied{}; bool killed{}; LethalSourceEvidence lethalSource{}; };
}
export namespace ecs
{
#define HEALTH_COMPONENT(Type, Key, Policy) template<> struct ComponentTraits<engine::gameplay::combat::Type> { \
 static constexpr std::string_view StableName = Key; static constexpr std::uint32_t Version = 1; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; };
HEALTH_COMPONENT(Health, "engine.gameplay.combat.health", Serializable)
HEALTH_COMPONENT(LifeState, "engine.gameplay.combat.life_state", Serializable)
template<> struct ComponentTraits<engine::gameplay::combat::PendingDamage>
{
 static constexpr std::string_view StableName = "engine.gameplay.combat.pending_damage";
 static constexpr std::uint32_t Version = 2;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
template<> struct ComponentTraits<engine::gameplay::combat::DamageResult>
{
 static constexpr std::string_view StableName = "engine.gameplay.combat.damage_result";
 static constexpr std::uint32_t Version = 2;
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
#undef HEALTH_COMPONENT
}
