module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.production.rally.components.rally_point;
export import engine.gameplay.rts.orders.systems.engagement_system;
export namespace generalszh::production::rally
{
// Producer-owned state. InvalidCell explicitly means no user rally. Enrollment
// is opt-in for the ordinary ground/default-exit production path.
struct RallyPoint
{
    engine::gameplay::navigation::Cell destination{engine::gameplay::navigation::InvalidCell};
};
// DefaultProductionExitUpdate.cpp:103 and QueueProductionExitUpdate.cpp:140
// sample current rally at exit, then issue aiFollowExitProductionPath. Model the
// user destination as Move, not AttackMove. Natural waypoints/doors/aircraft and
// supply-specific exit dispatch remain outside this bounded modern path.
inline engine::gameplay::rts::orders::UnitOrder InitialRallyOrder(RallyPoint point) noexcept
{
    using namespace engine::gameplay::rts::orders;
    return point.destination==engine::gameplay::navigation::InvalidCell ? UnitOrder{} :
        UnitOrder{OrderKind::Move,point.destination,{}};
}
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::production::rally::RallyPoint>
{
    static constexpr std::string_view StableName="games.generalszh.production.rally_point";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
