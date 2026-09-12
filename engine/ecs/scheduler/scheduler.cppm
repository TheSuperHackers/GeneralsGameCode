module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

export module engine.ecs.scheduler.scheduler;

export import engine.jobs.job_system;
export import engine.ecs.system.system;

export namespace ecs
{

class DependencyGraph
{
public:
	std::size_t NodeCount() const noexcept { return m_edges.size(); }
	const std::vector<SystemId> &Edges(SystemId system) const { return m_edges.at(system); }

	bool HasPath(SystemId from, SystemId to) const;

private:
	void AddEdge(SystemId from, SystemId to);

	std::vector<std::vector<SystemId>> m_edges;

	friend class Scheduler;
};

struct ExecutionPlan
{
	struct PhasePlan
	{
		SystemPhase phase{SystemPhase::Simulation};
		std::vector<std::vector<SystemId>> waves;
	};

	const std::vector<PhasePlan> &Phases() const noexcept { return phases; }

private:
	std::vector<PhasePlan> phases;

	friend class Scheduler;
};

extern "C++"
{
class Scheduler
{
public:
	Scheduler(World &world, SystemRegistry &systems) :
		Scheduler(world, systems, engine::jobs::JobSystemConfig{})
	{
	}
	Scheduler(World &world, SystemRegistry &systems, engine::jobs::JobSystemConfig config);
	Scheduler(World &world, SystemRegistry &systems, engine::jobs::JobSystem &jobs) noexcept;

	~Scheduler() noexcept;

	Scheduler(const Scheduler &) = delete;
	Scheduler &operator=(const Scheduler &) = delete;

	void Finalize(engine::time::FixedStep step);
	bool IsFinalized() const noexcept { return m_finalized; }
	bool IsFailed() const noexcept { return m_failed; }

	const DependencyGraph &Graph() const noexcept { return m_graph; }
	const ExecutionPlan &Plan() const noexcept { return m_plan; }

	void Execute(engine::time::SimulationTime time);

private:
	struct QueryBinding
	{
		void *query{nullptr};
		SystemInfo::DestroyQueryFunction destroy{nullptr};
	};

	static DependencyGraph BuildGraph(const SystemRegistry &systems);
	static ExecutionPlan BuildPlan(const SystemRegistry &systems, const DependencyGraph &graph);
	struct PreparedSystem
	{
		const SystemInfo *info{nullptr};
		void *query{nullptr};
		std::size_t chunkCount{0};
		std::size_t beforeContext{0}, afterContext{0};
	};

	struct ChunkExecution
	{
		SystemInfo::ExecuteChunkFunction execute{nullptr};
		void *instance{nullptr};
		void *query{nullptr};
		std::size_t chunkIndex{0};
		SystemContext *context{nullptr};
	};

	static void ExecuteChunkJob(void *context);
	void ExecuteWave(const ExecutionPlan::PhasePlan &phase,
		const std::vector<SystemId> &wave,
		engine::time::SimulationTime time);
	void DestroyQueries() noexcept;

	World *m_world;
	SystemRegistry *m_systems;
	engine::jobs::JobSystem *m_jobSystem{nullptr};
	std::unique_ptr<engine::jobs::JobSystem> m_ownedJobSystem;
	DependencyGraph m_graph;
	ExecutionPlan m_plan;
	std::vector<QueryBinding> m_queries;
	bool m_finalized{false};
	std::optional<engine::time::FixedStep> m_step;
	bool m_failed{false};
	bool m_executing{false};
};
} // extern "C++"

} // namespace ecs

namespace ecs
{

void DependencyGraph::AddEdge(const SystemId from, const SystemId to)
{
	if (from >= m_edges.size() || to >= m_edges.size())
		throw std::out_of_range("Invalid ECS scheduler dependency node");
	m_edges[from].push_back(to);
}

namespace
{

SystemId ResolveDependency(const SystemRegistry &systems,
	const SystemInfo &source,
	const SystemDependency &dependency)
{
	if (dependency.type == nullptr || *dependency.type == typeid(void))
		throw std::logic_error("ECS scheduler dependency is missing its system type");

	const SystemId targetId = systems.TryGet(*dependency.type);
	if (targetId == InvalidSystemId)
	{
		throw std::logic_error("ECS scheduler system '" + std::string(source.stableName) +
			"' references unregistered system '" + std::string(dependency.stableName) + "'");
	}

	const SystemInfo &target = systems.Get(targetId);
	if (target.stableKey != dependency.stableKey)
	{
		throw std::logic_error("ECS scheduler dependency system type and stable key disagree for '" +
			std::string(dependency.stableName) + "'");
	}
	return targetId;
}

bool FindCycle(const DependencyGraph &graph,
	const SystemId node,
	std::vector<std::uint8_t> &colors,
	std::vector<SystemId> &stack,
	std::vector<SystemId> &cycle)
{
	colors[node] = 1;
	stack.push_back(node);
	for (const SystemId next : graph.Edges(node))
	{
		if (colors[next] == 0)
		{
			if (FindCycle(graph, next, colors, stack, cycle))
				return true;
		}
		else if (colors[next] == 1)
		{
			const auto start = std::find(stack.begin(), stack.end(), next);
			cycle.assign(start, stack.end());
			cycle.push_back(next);
			return true;
		}
	}
	stack.pop_back();
	colors[node] = 2;
	return false;
}

std::string CycleDescription(const SystemRegistry &systems, const std::vector<SystemId> &cycle)
{
	std::string result = "ECS scheduler dependency cycle:";
	for (std::size_t index = 0; index < cycle.size(); ++index)
	{
		result += " " + std::string(systems.Get(cycle[index]).stableName);
		if (index + 1 < cycle.size())
			result += " ->";
	}
	return result;
}

SystemPhase PhaseFromIndex(const std::size_t index)
{
	switch (index)
	{
	case 0:
		return SystemPhase::PreSimulation;
	case 1:
		return SystemPhase::Simulation;
	case 2:
		return SystemPhase::PostSimulation;
	default:
		throw std::out_of_range("Invalid ECS scheduler phase index");
	}
}

} // namespace

bool DependencyGraph::HasPath(const SystemId from, const SystemId to) const
{
	if (from >= m_edges.size() || to >= m_edges.size())
		return false;
	if (from == to)
		return true;

	std::vector<bool> visited(m_edges.size(), false);
	std::vector<SystemId> pending{from};
	visited[from] = true;
	while (!pending.empty())
	{
		const SystemId current = pending.back();
		pending.pop_back();
		for (const SystemId next : m_edges[current])
		{
			if (next == to)
				return true;
			if (!visited[next])
			{
				visited[next] = true;
				pending.push_back(next);
			}
		}
	}
	return false;
}

extern "C++"
{
DependencyGraph Scheduler::BuildGraph(const SystemRegistry &systems)
{
	DependencyGraph graph;
	graph.m_edges.resize(systems.Count());
	const auto &ordered = systems.OrderedSystems();

	for (const SystemInfo *left : ordered)
	{
		for (const SystemInfo *right : ordered)
		{
			if (SystemPhaseIndex(left->phase) < SystemPhaseIndex(right->phase))
				graph.AddEdge(left->id, right->id);
		}
	}

	for (const SystemInfo *info : ordered)
	{
		for (const SystemDependency &dependency : info->Before())
			graph.AddEdge(info->id, ResolveDependency(systems, *info, dependency));
		for (const SystemDependency &dependency : info->After())
			graph.AddEdge(ResolveDependency(systems, *info, dependency), info->id);
	}
	for (const auto &[before, after] : systems.m_ordering)
	{
		const SystemId source = systems.TryGet(*before.type);
		if (source == InvalidSystemId)
			throw std::logic_error("ECS composition ordering references unregistered system '" + std::string(before.stableName) + "'");
		const SystemInfo &info = systems.Get(source);
		graph.AddEdge(ResolveDependency(systems, info, before), ResolveDependency(systems, info, after));
	}

	for (std::vector<SystemId> &edges : graph.m_edges)
	{
		std::sort(edges.begin(), edges.end());
		edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
	}

	std::vector<std::uint8_t> colors(graph.NodeCount(), 0);
	std::vector<SystemId> stack;
	std::vector<SystemId> cycle;
	for (SystemId system = 0; system < graph.NodeCount(); ++system)
	{
		if (colors[system] == 0 && FindCycle(graph, system, colors, stack, cycle))
			throw std::logic_error(CycleDescription(systems, cycle));
	}

	for (SystemId leftId = 0; leftId < graph.NodeCount(); ++leftId)
	{
		const SystemInfo &left = systems.Get(leftId);
		for (SystemId rightId = leftId + 1; rightId < graph.NodeCount(); ++rightId)
		{
			const SystemInfo &right = systems.Get(rightId);
			if (left.phase != right.phase || !left.access.ConflictsWith(right.access))
				continue;
			if (!graph.HasPath(leftId, rightId) && !graph.HasPath(rightId, leftId))
			{
				throw std::logic_error("Ambiguous ECS scheduler conflict between systems '" +
					std::string(left.stableName) + "' and '" + std::string(right.stableName) +
					"'; add an explicit Before/After dependency");
			}
		}
	}

	return graph;
}

ExecutionPlan Scheduler::BuildPlan(const SystemRegistry &systems, const DependencyGraph &graph)
{
	std::vector<std::uint32_t> indegrees(graph.NodeCount(), 0);
	for (SystemId system = 0; system < graph.NodeCount(); ++system)
	{
		for (const SystemId next : graph.Edges(system))
			++indegrees[next];
	}

	std::vector<std::vector<SystemId>> waves;
	std::size_t remaining = graph.NodeCount();
	while (remaining != 0)
	{
		std::vector<SystemId> ready;
		for (SystemId system = 0; system < graph.NodeCount(); ++system)
		{
			if (indegrees[system] == 0)
				ready.push_back(system);
		}
		if (ready.empty())
			throw std::logic_error("ECS scheduler cannot build an execution plan because the graph contains a cycle");

		// Execution may join chunk groups around caller-side batch work, but
		// never introduce extra structural visibility boundaries in a ready set.
		waves.push_back(ready);
		for (const SystemId system : ready)
		{
			indegrees[system] = (std::numeric_limits<std::uint32_t>::max)();
			--remaining;
			for (const SystemId next : graph.Edges(system))
				--indegrees[next];
		}
	}

	ExecutionPlan plan;
	plan.phases.reserve(SystemPhaseCount);
	for (std::size_t index = 0; index < SystemPhaseCount; ++index)
		plan.phases.push_back(ExecutionPlan::PhasePlan{PhaseFromIndex(index), {}});

	for (const std::vector<SystemId> &wave : waves)
	{
		if (wave.empty())
			continue;
		const SystemPhase phase = systems.Get(wave.front()).phase;
		for (const SystemId system : wave)
		{
			if (systems.Get(system).phase != phase)
				throw std::logic_error("ECS scheduler produced a wave spanning multiple phases");
		}
		plan.phases[SystemPhaseIndex(phase)].waves.push_back(wave);
	}

	return plan;
}

Scheduler::~Scheduler() noexcept
{
	DestroyQueries();
}

Scheduler::Scheduler(World &world,
	SystemRegistry &systems,
	const engine::jobs::JobSystemConfig config) :
	m_world(&world),
	m_systems(&systems),
	m_ownedJobSystem(std::make_unique<engine::jobs::JobSystem>(config))
{
	m_jobSystem = m_ownedJobSystem.get();
}

Scheduler::Scheduler(World &world,
	SystemRegistry &systems,
	engine::jobs::JobSystem &jobs) noexcept :
	m_world(&world),
	m_systems(&systems),
	m_jobSystem(&jobs)
{
}

void Scheduler::DestroyQueries() noexcept
{
	for (std::size_t index = 0; index < m_queries.size(); ++index)
	{
		if (m_queries[index].query != nullptr)
		{
			m_queries[index].destroy(m_queries[index].query);
			m_queries[index].query = nullptr;
		}
	}
	m_queries.clear();
}

void Scheduler::Finalize(const engine::time::FixedStep step)
{
	if (m_finalized)
	{
		if (*m_step != step)
			throw std::logic_error("Cannot change a finalized ECS scheduler's simulation step");
		return;
	}
	if (!m_world->ComponentsFinalized())
		throw std::logic_error("ECS component registry must be finalized before scheduler finalization");
	if (!m_systems->IsFrozen())
		throw std::logic_error("ECS system registry must be finalized before scheduler finalization");
	if (m_systems->ComponentSchema() != m_world->Components().SchemaHash())
		throw std::logic_error("ECS system access metadata was finalized against a different component schema");

	DependencyGraph graph = BuildGraph(*m_systems);
	ExecutionPlan plan = BuildPlan(*m_systems, graph);

	std::vector<QueryBinding> queries(m_systems->Count());
	try
	{
		for (const SystemInfo *info : m_systems->OrderedSystems())
		{
			void *query = info->createQuery(*m_world);
			try
			{
				queries[info->id] = QueryBinding{query, info->destroyQuery};
			}
			catch (...)
			{
				info->destroyQuery(query);
				throw;
			}
		}
	}
	catch (...)
	{
		for (std::size_t index = 0; index < queries.size(); ++index)
		{
			if (queries[index].query != nullptr)
				queries[index].destroy(queries[index].query);
		}
		throw;
	}

	m_graph = std::move(graph);
	m_plan = std::move(plan);
	m_queries = std::move(queries);
	m_step = step;
	m_finalized = true;
}

void Scheduler::ExecuteChunkJob(void *rawContext)
{
	ChunkExecution &job = *static_cast<ChunkExecution *>(rawContext);
	job.execute(job.instance, job.query, job.chunkIndex, *job.context);
}

void Scheduler::ExecuteWave(const ExecutionPlan::PhasePlan &phase,
	const std::vector<SystemId> &wave,
	const engine::time::SimulationTime time)
{
	std::vector<PreparedSystem> prepared;
	prepared.reserve(wave.size());

	std::size_t totalChunks = 0;
	std::size_t lifecycleContexts = 0;
	for (const SystemId systemId : wave)
	{
		const SystemInfo &info = m_systems->Get(systemId);
		if (info.prepareQuery == nullptr || info.executeChunk == nullptr)
			throw std::logic_error("ECS scheduler system is missing chunk execution metadata");
		const std::size_t chunkCount = info.prepareQuery(m_queries[systemId].query);
		const bool lifecycle = info.beforeChunks || info.afterChunks;
		if (chunkCount > (std::numeric_limits<std::uint32_t>::max)() - (lifecycle ? 2u : 0u))
			throw std::length_error("ECS scheduler logical chunk order exceeds its representation");
		if (totalChunks > (std::numeric_limits<std::size_t>::max)() - chunkCount)
			throw std::length_error("ECS scheduler chunk batch is too large");
		totalChunks += chunkCount;
		if (lifecycle)
		{
			if (lifecycleContexts > (std::numeric_limits<std::size_t>::max)() - 2)
				throw std::length_error("ECS lifecycle context capacity overflow");
			lifecycleContexts += 2;
		}
		prepared.push_back(PreparedSystem{&info, m_queries[systemId].query, chunkCount});
	}

	if (totalChunks == 0 && lifecycleContexts == 0)
		return;
	if (lifecycleContexts > (std::numeric_limits<std::size_t>::max)() - totalChunks)
		throw std::length_error("ECS lifecycle context capacity overflow");

	std::vector<CommandBuffer> commands;
	std::vector<SystemContext> contexts;
	std::vector<ChunkExecution> executions;
	std::vector<engine::jobs::Job> jobs;
	std::vector<CommandBuffer *> commandPointers;
	commands.reserve(totalChunks + lifecycleContexts);
	contexts.reserve(totalChunks + lifecycleContexts);
	executions.reserve(totalChunks);
	jobs.reserve(totalChunks);
	commandPointers.reserve(totalChunks + lifecycleContexts);

	for (PreparedSystem &system : prepared)
	{
		const bool lifecycle = system.info->beforeChunks || system.info->afterChunks;
		if (lifecycle)
		{
			system.beforeContext = contexts.size();
			commands.emplace_back(CommandBufferOrder{static_cast<std::uint32_t>(SystemPhaseIndex(phase.phase)),system.info->id,0,0});
			contexts.emplace_back(*m_world,commands.back(),time,system.info->id,phase.phase,0,0);
		}
		for (std::size_t chunkIndex = 0; chunkIndex < system.chunkCount; ++chunkIndex)
		{
			const std::uint32_t logicalOrder = static_cast<std::uint32_t>(chunkIndex);
			commands.emplace_back(CommandBufferOrder{
				static_cast<std::uint32_t>(SystemPhaseIndex(phase.phase)),
				system.info->id,
				logicalOrder + (lifecycle ? 1u : 0u),
				logicalOrder});
			contexts.emplace_back(*m_world,
				commands.back(),
				time,
				system.info->id,
				phase.phase,
				logicalOrder + (lifecycle ? 1u : 0u),
				logicalOrder);
			executions.push_back(ChunkExecution{
				system.info->executeChunk,
				system.info->instance,
				system.query,
				chunkIndex,
				&contexts.back()});
			jobs.push_back(engine::jobs::Job{&ExecuteChunkJob, &executions.back()});
		}
		if (lifecycle)
		{
			system.afterContext = contexts.size();
			const auto order = static_cast<std::uint32_t>(system.chunkCount + 1);
			commands.emplace_back(CommandBufferOrder{static_cast<std::uint32_t>(SystemPhaseIndex(phase.phase)),system.info->id,order,order});
			contexts.emplace_back(*m_world,commands.back(),time,system.info->id,phase.phase,order,order);
		}
	}

	m_world->BeginScheduledExecution();
	bool scheduledExecutionActive = true;
	try
	{
		// Caller-side hooks run once, including empty queries. No structural
		// visibility changes here: every buffer commits only after the whole wave.
		for (const PreparedSystem &system : prepared)
			if (system.info->beforeChunks)
				system.info->beforeChunks(system.info->instance,system.query,contexts[system.beforeContext]);
		std::size_t firstJob = 0, nextJob = 0;
		for (const PreparedSystem &system : prepared)
		{
			if (system.info->batch)
			{
				// Join outstanding compatible chunk work, then allow a typed
				// batch node to borrow the pool on the caller. All commands stay
				// deferred until the ENTIRE original wave has succeeded.
				if (nextJob != firstJob)
					m_jobSystem->Execute(std::span<engine::jobs::Job>(jobs).subspan(firstJob, nextJob - firstJob));
				ExecuteChunkJob(&executions[nextJob]);
				++nextJob;
				firstJob = nextJob;
			}
			else nextJob += system.chunkCount;
		}
		if (nextJob != firstJob)
			m_jobSystem->Execute(std::span<engine::jobs::Job>(jobs).subspan(firstJob, nextJob - firstJob));
		for (const PreparedSystem &system : prepared)
			if (system.info->afterChunks)
				system.info->afterChunks(system.info->instance,system.query,contexts[system.afterContext]);
		m_world->EndScheduledExecution();
		scheduledExecutionActive = false;
	}
	catch (...)
	{
		if (scheduledExecutionActive)
			m_world->EndScheduledExecution();
		throw;
	}

	for (CommandBuffer &command : commands)
	{
		if (!command.Empty())
			commandPointers.push_back(&command);
	}

	// Commit is a deterministic barrier. World::Commit sorts the worker-local
	// buffers by their logical scheduler order before applying them.
	if (!commandPointers.empty())
		m_world->Commit(std::span<CommandBuffer *>(commandPointers));
}

void Scheduler::Execute(const engine::time::SimulationTime time)
{
	if (!m_finalized)
		throw std::logic_error("ECS scheduler must be finalized before execution");
	if (m_failed)
		throw std::logic_error("ECS scheduler cannot execute after a failed simulation tick");
	if (m_executing)
		throw std::logic_error("ECS scheduler execution is not reentrant");
	if (time.Step() != *m_step)
		throw std::invalid_argument("ECS execution time does not match the finalized simulation step");

	m_executing = true;
	try
	{
		for (const ExecutionPlan::PhasePlan &phase : m_plan.Phases())
		{
			for (const std::vector<SystemId> &wave : phase.waves)
			{
				// Every wave has its own job-local command buffers. This is the
				// structural visibility boundary: later waves see this commit,
				// while jobs in this wave cannot see one another's commands.
				ExecuteWave(phase, wave, time);
			}
		}
		m_executing = false;
	}
	catch (...)
	{
		m_executing = false;
		m_failed = true;
		throw;
	}
}

} // extern "C++"
} // namespace ecs
