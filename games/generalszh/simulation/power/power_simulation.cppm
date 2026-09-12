module;
#include <string_view>
#include "games/generalszh/simulation/execution/gameplay_workers.h"
export module games.generalszh.simulation.power.power_simulation;
export import games.generalszh.gameplay.power.systems.power_recovery_system;
export import engine.ecs.scheduler.scheduler;
import games.generalszh.simulation.execution.gameplay_workers;
export namespace generalszh
{
// Recovery plan only. Contribution/disabled-object execution is not yet migrated.
class PowerSimulation
{
public:
    PowerSimulation(ecs::World &world, GameplayWorkers &workers) : world(world),
        scheduler(world, registry, GameplayWorkerAccess::Jobs(workers)) {}
    void Finalize(engine::time::FixedStep step)
    {
        registry.Register(recovery); registry.Finalize(world.Components()); scheduler.Finalize(step);
    }
    void Execute(engine::time::SimulationTime time) { scheduler.Execute(time); }
private:
    ecs::World &world;
    power::PowerRecoverySystem recovery;
    ecs::SystemRegistry registry;
    ecs::Scheduler scheduler;
};
}
