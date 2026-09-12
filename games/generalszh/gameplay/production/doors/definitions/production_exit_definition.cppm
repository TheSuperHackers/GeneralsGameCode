module;
export module games.generalszh.gameplay.production.doors.definitions.production_exit_definition;
export import engine.time.simulation_time;
export import games.generalszh.gameplay.production.doors.components.production_exit;
export namespace generalszh::production
{
struct ProductionExitDefinition
{
    engine::time::Duration opening{}, waiting{}, closing{};
};
inline ProductionExitTiming AuthorProductionExit(const ProductionExitDefinition &definition,
    engine::time::FixedStep step)
{
    return {step.TicksFor(definition.opening),step.TicksFor(definition.waiting),step.TicksFor(definition.closing)};
}
}
