module;
#include <string_view>
#include "games/generalszh/simulation/execution/gameplay_workers.h"
export module games.generalszh.simulation.lifetime.lifetime_simulation;
export import games.generalszh.gameplay.lifetime.systems.lifetime_effect_system;
export import engine.ecs.scheduler.scheduler;
import games.generalszh.simulation.execution.gameplay_workers;

export namespace generalszh
{
class LifetimeSimulation
{
public:
	LifetimeSimulation(ecs::World &world, GameplayWorkers &workers) : m_world(world),
		m_scheduler(world, m_registry, GameplayWorkerAccess::Jobs(workers)) {}
	void Finalize(engine::time::FixedStep step)
	{
		m_registry.Register(m_expiration);
		m_registry.Register(m_effects);
		m_registry.Finalize(m_world.Components());
		m_scheduler.Finalize(step);
	}
	void Execute(engine::time::SimulationTime time) { m_scheduler.Execute(time); }
private:
	ecs::World &m_world;
	engine::gameplay::lifetime::ExpirationSystem m_expiration;
	lifetime::LifetimeEffectSystem m_effects;
	ecs::SystemRegistry m_registry;
	ecs::Scheduler m_scheduler;
};
}
