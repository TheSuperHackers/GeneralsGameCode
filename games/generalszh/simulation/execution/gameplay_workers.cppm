module;
#include "gameplay_workers.h"
export module games.generalszh.simulation.execution.gameplay_workers;
export import engine.jobs.job_system;

extern "C++"
{
struct generalszh::GameplayWorkers::Impl
{
	engine::jobs::JobSystem jobs;
	explicit Impl(std::size_t count) : jobs(engine::jobs::JobSystemConfig{count}) {}
};
generalszh::GameplayWorkers::GameplayWorkers(std::size_t workers) : m_impl(new Impl(workers)) {}
generalszh::GameplayWorkers::~GameplayWorkers() noexcept { delete m_impl; }
std::size_t generalszh::GameplayWorkers::Count() const noexcept { return m_impl->jobs.WorkerCount(); }
}

export namespace generalszh
{
extern "C++"
{
class GameplayWorkerAccess
{
public:
	static engine::jobs::JobSystem &Jobs(GameplayWorkers &owner) noexcept { return owner.m_impl->jobs; }
};
}
}
