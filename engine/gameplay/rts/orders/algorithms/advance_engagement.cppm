module;
#include <algorithm>
#include <cassert>
#include <cstdint>
export module engine.gameplay.rts.orders.algorithms.advance_engagement;
export import engine.gameplay.rts.orders.components.unit_order;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.spatial.algorithms.grid_radius;
export namespace engine::gameplay::rts::orders
{
inline bool IsWithinGridRadius(const navigation::Cell center, const navigation::Cell position,
    const std::uint32_t width, const std::uint32_t radius) noexcept
{
    // NavigationGrid validates a non-zero width before this helper is called.
    assert(width != 0);

    return spatial::IsWithinGridRadius({center % width, center / width},
        {position % width, position / width}, radius);
}

inline void AdvanceEngagement(UnitOrder &order, const EngagementObservation &observed, const combat::LifeState &life,
    const navigation::GridPosition &position, navigation::MoveGoal &destination, navigation::MoveCredit &credit,
    EngagementResult &result) noexcept
{
            result={}; auto goal=navigation::InvalidCell;
            if (!life.alive) { order={}; result.controlsWeapon=true; }
            else
            {
                if(order.kind==OrderKind::None) return;
                result.controlsWeapon=true;
                switch(order.kind)
                {
                case OrderKind::Move:
                    goal=order.destination;
                    if(position.cell==goal) order={};
                    break;
                case OrderKind::Stop: order={}; break;
                case OrderKind::AttackTarget:
                    if(!observed.target.IsValid()) { order={}; break; }
                    result.target=observed.target;
                    if(!observed.inRange) goal=observed.position;
                    break;
                case OrderKind::AttackPosition:
                    if(!observed.positionValid) { order={}; break; }
                    result.position=observed.position;
                    result.positionValid=true;
                    break;
                case OrderKind::AttackMove:
                    if(observed.target.IsValid())
                    { result.target=observed.target; if(!observed.inRange) goal=observed.position; }
                    else { goal=order.destination; if(position.cell==goal) order={}; }
                    break;
                case OrderKind::GuardPosition:
                    if (observed.target.IsValid())
                    {
                        order.target=observed.target;
                        result.target=observed.target;
                        if (!observed.inRange) goal=observed.position;
                    }
                    else
                    {
                        order.target={};
                        goal=position.cell==order.destination ? navigation::InvalidCell : order.destination;
                    }
                    break;
                case OrderKind::None: break;
                }
            }
            if(destination.cell!=goal) credit.remainder=0;
            destination.cell=goal;
}
}
