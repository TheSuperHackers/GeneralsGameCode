module;
export module engine.gameplay.rts.construction.algorithms.builder_routing;
export import engine.gameplay.rts.construction.components.construction_components;
export namespace engine::gameplay::rts::construction
{
// Reusable task movement rule. Caller owns contiguous chunk iteration and scheduling.
inline void UpdateBuilderRoute(BuilderAssignment &assignment,combat::LifeState life,
    navigation::MoveGoal &goal,navigation::MoveCredit &credit,
    navigation::MoveRangeGoal *range=nullptr) noexcept
{
    if(range) navigation::ClearMoveRangeGoal(*range,credit);
    if(!assignment.site.IsValid()) return;
    if(!life.alive) { assignment={}; goal={}; credit={}; return; }
    if(goal.cell!=assignment.cell) credit={};
    goal.cell=assignment.cell;
}
}
