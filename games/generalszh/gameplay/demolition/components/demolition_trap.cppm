module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.demolition.components.demolition_trap;
export import engine.ecs.core.component_registry;

export namespace generalszh::demolition
{
enum class DemolitionDetonationMode : std::uint8_t
{
    Proximity,
    Manual
};

struct DemolitionIgnorePolicy
{
    bool airborne{};
    bool structures{};
    // Projectile entities do not carry the health/life target columns used by
    // the bounded proximity query; retaining the authored bit makes that
    // exclusion explicit at the adapter boundary.
    bool projectiles{};
    bool unattackable{};
};

// Runtime state only.  The trap's authored mode/radius/weapon key lives in
// the immutable catalog; this component records scan cadence and the once-only
// detonation transition.
struct DemolitionTrapState
{
    std::uint64_t nextScanTick{};
    DemolitionDetonationMode mode{DemolitionDetonationMode::Proximity};
    bool detonated{};
    bool selfDeathApplied{};
};

// Optional target metadata is a typed adapter/runtime projection, not a copy
// of the trap definition.  A producer must attach it when the resolved target
// authoring/runtime state supplies these classifications; absence does not
// invent a legacy object flag.
struct DemolitionTargetClassification
{
    bool airborne{};
    bool unattackable{};
    bool disarmingDozer{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::demolition::DemolitionTrapState>
{
    static constexpr std::string_view StableName="games.generalszh.demolition.trap_state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::demolition::DemolitionTargetClassification>
{
    static constexpr std::string_view StableName="games.generalszh.demolition.target_classification";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
}
