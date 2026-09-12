module;
#include <string_view>
#include "games/generalszh/simulation/execution/gameplay_workers.h"
export module games.generalszh.simulation.income.income_simulation;
export import games.generalszh.gameplay.economy.systems.capture_system;
export import games.generalszh.gameplay.economy.systems.income_prepare_system;
export import games.generalszh.gameplay.economy.systems.income_payout_system;
export import engine.ecs.scheduler.scheduler;
import games.generalszh.simulation.execution.gameplay_workers;
export namespace generalszh
{
class IncomeSimulation
{
public:
	IncomeSimulation(ecs::World &world, economy::CaptureBatch &batch, GameplayWorkers &workers) :
		world(world), capture(batch), scheduler(world, registry, GameplayWorkerAccess::Jobs(workers)) {}
	void Finalize(engine::time::FixedStep step)
	{
		registry.Register(capture); registry.Register(prepare); registry.Register(income); registry.Register(payout);
		registry.Finalize(world.Components()); scheduler.Finalize(step);
	}
	void Execute(engine::time::SimulationTime time) { scheduler.Execute(time); }
private:
	ecs::World &world;
	economy::CaptureSystem capture;
	economy::IncomePrepareSystem prepare;
	engine::gameplay::rts::economy::IncomeSystem income;
	economy::IncomePayoutSystem payout;
	ecs::SystemRegistry registry;
	ecs::Scheduler scheduler;
};
}
