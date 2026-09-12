module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.concealment.components.concealment_binding;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.concealment.definitions.concealment_definition;
export import engine.gameplay.concealment.definitions.detection_definition;

export namespace engine::gameplay::concealment
{
struct ConcealmentBinding { ConcealmentDefinitionId definition{}; };
struct DetectionBinding { DetectionDefinitionId definition{}; };

// The engine only compares opaque groups. Zero Hour maps its account relation
// to this binding; no diplomacy or observer-visibility policy lives here.
struct ConcealmentGroup { ecs::Entity value{}; };
}

export namespace ecs
{
#define CONCEALMENT_BINDING_TRAITS(Type, Name) \
template<> struct ComponentTraits<engine::gameplay::concealment::Type> { \
    static constexpr std::string_view StableName="engine.gameplay.concealment." Name; \
    static constexpr std::uint32_t Version=1; \
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable; \
};
CONCEALMENT_BINDING_TRAITS(ConcealmentBinding, "binding")
CONCEALMENT_BINDING_TRAITS(DetectionBinding, "detection_binding")
template<> struct ComponentTraits<engine::gameplay::concealment::ConcealmentGroup> {
    static constexpr std::string_view StableName="engine.gameplay.concealment.group";
    static constexpr std::uint32_t Version=1;
    // The ZH adapter refreshes this projection from ProducedUnit::account at
    // the joined policy boundary; it is never a serialized owner authority.
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
#undef CONCEALMENT_BINDING_TRAITS
}
