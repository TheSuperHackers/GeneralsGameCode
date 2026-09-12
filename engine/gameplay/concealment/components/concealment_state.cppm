module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.concealment.components.concealment_state;
export import engine.ecs.core.component_registry;
export import engine.gameplay.concealment.algorithms.detection_lease;
export import engine.gameplay.concealment.definitions.concealment_definition;

export namespace engine::gameplay::concealment
{
// One authoritative global detection lease is stored per concealed entity.
// Visibility from a particular observer is derived by the consuming game
// policy; it is deliberately not duplicated here.
struct ConcealmentState
{
    bool enabled{true};
    bool concealed{};
    bool initialized{};
    std::uint64_t concealAllowedTick{};
    std::uint64_t detectedUntilTick{};
};

inline ConcealmentState StartConcealment(const CompiledConcealmentDefinition &definition, std::uint64_t tick)
{
    return {definition.enabledByDefault, false, true,
        ConcealmentDeadline(tick, definition.stealthDelayTicks), 0};
}
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::concealment::ConcealmentState>
{
    static constexpr std::string_view StableName="engine.gameplay.concealment.state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
