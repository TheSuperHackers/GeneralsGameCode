module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.orders.systems.engagement_system;
export import engine.gameplay.navigation.systems.movement_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.rts.orders.algorithms.advance_engagement;
export import engine.gameplay.rts.orders.inputs.order_input;
export namespace engine::gameplay::rts::orders
{
struct EngagementSystem
{
    using Query = ecs::Query<ecs::Write<UnitOrder>,ecs::Read<EngagementObservation>,ecs::Read<combat::LifeState>,
        ecs::Read<navigation::GridPosition>,ecs::Write<navigation::MoveGoal>,ecs::Write<navigation::MoveCredit>,
        ecs::OptionalWrite<navigation::MoveRangeGoal>,ecs::Write<EngagementResult>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        auto orders=chunk.Get<UnitOrder>(); const auto observed=chunk.Get<EngagementObservation>(); const auto lives=chunk.Get<combat::LifeState>();
        const auto positions=chunk.Get<navigation::GridPosition>(); auto goals=chunk.Get<navigation::MoveGoal>();
        auto credits=chunk.Get<navigation::MoveCredit>(); auto ranges=chunk.Get<navigation::MoveRangeGoal>();
        auto results=chunk.Get<EngagementResult>();
        for(std::size_t row=0;row!=chunk.Count();++row)
        {
            AdvanceEngagement(orders[row], observed[row], lives[row], positions[row], goals[row], credits[row], results[row]);
            if(!ranges.empty()) navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::orders::EngagementSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.rts.orders.engagement";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
