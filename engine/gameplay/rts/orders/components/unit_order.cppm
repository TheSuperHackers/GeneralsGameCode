module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.orders.components.unit_order;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.components.movement;
export namespace engine::gameplay::rts::orders
{
enum class OrderKind : std::uint8_t { None, Move, Stop, AttackTarget, AttackMove, GuardPosition, AttackPosition };
struct GuardPositionParameters
{
    std::uint32_t innerRadiusCells{};
    std::uint32_t outerRadiusCells{};
};
struct UnitOrder
{
    OrderKind kind{OrderKind::None};
    navigation::Cell destination{navigation::InvalidCell};
    ecs::Entity target{};
    GuardPositionParameters guard{};
};
// Target eligibility belongs to the composing game; order progression doesn't
// query opponents, diplomacy, visibility, content or world state through callbacks.
struct EngagementObservation
{
    ecs::Entity target{};
    navigation::Cell position{navigation::InvalidCell};
    bool inRange{};
    bool positionValid{};
};
struct EngagementResult
{
    ecs::Entity target{};
    bool controlsWeapon{};
    navigation::Cell position{navigation::InvalidCell};
    bool positionValid{};
};
}
export namespace ecs
{
#define ORDER_COMPONENT(Type, Policy, SchemaVersion) template<> struct ComponentTraits<engine::gameplay::rts::orders::Type> { \
 static constexpr std::string_view StableName = "engine.gameplay.rts.orders." #Type; static constexpr std::uint32_t Version=SchemaVersion; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Policy; };
ORDER_COMPONENT(UnitOrder,Serializable,3)
ORDER_COMPONENT(EngagementObservation,Transient,2)
ORDER_COMPONENT(EngagementResult,Transient,2)
#undef ORDER_COMPONENT
}
