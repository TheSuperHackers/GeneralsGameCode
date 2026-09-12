module;
export module engine.gameplay.rts.production.definitions.build_definition;
export import engine.gameplay.rts.production.components.build_work;
export import engine.time.simulation_time;
export namespace engine::gameplay::rts::production
{
inline BuildWork AuthorBuild(engine::time::Duration duration,engine::time::FixedStep step)
{return {step.TicksFor(duration),0};}
}
