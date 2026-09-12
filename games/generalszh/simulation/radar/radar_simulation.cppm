module;
#include <string_view>
export module games.generalszh.simulation.radar.radar_simulation;
export import engine.gameplay.rts.radar.systems.radar_system;
export import engine.ecs.scheduler.scheduler;

export namespace generalszh
{
// Composition, not a second scheduler implementation. Lifetime is bound to the
// injected World; persistent world replacement destroys/rebuilds this owner.
class RadarSimulation
{
public:
	RadarSimulation(ecs::World &world, engine::gameplay::rts::radar::RadarBatch &batch,
		engine::jobs::JobSystemConfig jobs) : m_world(world), m_system(batch), m_scheduler(world, m_registry, jobs) {}
	RadarSimulation(ecs::World &world, engine::gameplay::rts::radar::RadarBatch &batch,
		engine::jobs::JobSystem &jobs) : m_world(world), m_system(batch), m_scheduler(world, m_registry, jobs) {}
	void Finalize(engine::time::FixedStep step)
	{
		m_registry.Register(m_system);
		m_registry.Finalize(m_world.Components());
		m_scheduler.Finalize(step);
	}
	void Execute(engine::time::SimulationTime time) { m_scheduler.Execute(time); }
private:
	ecs::World &m_world;
	engine::gameplay::rts::radar::RadarSystem m_system;
	ecs::SystemRegistry m_registry;
	ecs::Scheduler m_scheduler;
};
}
