module;

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <mutex>
#include <span>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

export module engine.jobs.job_system;

export namespace engine::jobs
{

struct Job
{
	using Function = void (*)(void *);

	Function function{nullptr};
	void *context{nullptr};
};

struct JobSystemConfig
{
	// Zero selects the process hardware-concurrency value, with one worker as
	// the deterministic fallback when the platform does not report a value.
	std::size_t workerCount{0};
};

class JobSystem
{
public:
	explicit JobSystem(JobSystemConfig config = {});
	~JobSystem() noexcept;

	JobSystem(const JobSystem &) = delete;
	JobSystem &operator=(const JobSystem &) = delete;

	// Execute is an explicit batch barrier. The caller owns the Job array and
	// must keep it alive until this function returns. A thrown job exception is
	// rethrown after all workers have stopped processing the batch.
	void Execute(std::span<Job> jobs);
	void Shutdown() noexcept;

	[[nodiscard]] std::size_t WorkerCount() const noexcept { return m_workerCount; }

private:
	void WorkerLoop();

	std::vector<std::thread> m_workers;
	std::mutex m_mutex;
	std::condition_variable m_workAvailable;
	std::condition_variable m_workComplete;

	const Job *m_jobs{nullptr};
	std::size_t m_jobCount{0};
	std::size_t m_activeWorkers{0};
	std::size_t m_workerCount{0};
	std::uint64_t m_generation{0};
	std::exception_ptr m_failure{};
	bool m_stopping{false};
	bool m_executing{false};

	// These are independently modified by all workers while claiming jobs.
	// Keeping them on separate cache lines avoids making the claim/failure
	// protocol an obvious false-sharing point.
	alignas(64) std::atomic<std::size_t> m_nextJob{0};
	alignas(64) std::atomic<bool> m_batchFailed{false};
};

} // namespace engine::jobs

namespace engine::jobs
{

JobSystem::JobSystem(const JobSystemConfig config)
{
	m_workerCount = config.workerCount;
	if (m_workerCount == 0)
	{
		m_workerCount = std::thread::hardware_concurrency();
		if (m_workerCount == 0)
			m_workerCount = 1;
	}

	try
	{
		m_workers.reserve(m_workerCount);
		for (std::size_t index = 0; index < m_workerCount; ++index)
			m_workers.emplace_back([this] { WorkerLoop(); });
	}
	catch (...)
	{
		Shutdown();
		throw;
	}
}

JobSystem::~JobSystem() noexcept
{
	Shutdown();
}

void JobSystem::Shutdown() noexcept
{
	{
		std::lock_guard lock(m_mutex);
		if (m_stopping)
			return;
		// Shutdown is a lifetime operation and must not race Execute. The
		// assertion catches misuse in debug builds; the worker joins below are
		// still safe for the normal constructor/destructor path.
		assert(!m_executing);
		m_stopping = true;
	}
	m_workAvailable.notify_all();
	for (std::thread &worker : m_workers)
	{
		if (worker.joinable())
			worker.join();
	}
}

void JobSystem::Execute(const std::span<Job> jobs)
{
	for (const Job &job : jobs)
	{
		if (job.function == nullptr)
			throw std::invalid_argument("Engine job batches cannot contain an empty job");
	}
	if (jobs.empty())
		return;

	{
		std::lock_guard lock(m_mutex);
		if (m_stopping)
			throw std::logic_error("Cannot execute jobs after job-system shutdown");
		if (m_executing)
			throw std::logic_error("Engine job-system execution is not reentrant");

		m_executing = true;
		m_jobs = jobs.data();
		m_jobCount = jobs.size();
		m_activeWorkers = m_workerCount;
		m_failure = nullptr;
		m_nextJob.store(0, std::memory_order_relaxed);
		m_batchFailed.store(false, std::memory_order_release);
		++m_generation;
	}
	m_workAvailable.notify_all();

	std::exception_ptr failure;
	{
		std::unique_lock lock(m_mutex);
		m_workComplete.wait(lock, [this] { return m_activeWorkers == 0; });
		failure = m_failure;
		m_jobs = nullptr;
		m_jobCount = 0;
		m_executing = false;
	}

	if (failure != nullptr)
		std::rethrow_exception(failure);
}

void JobSystem::WorkerLoop()
{
	std::uint64_t observedGeneration = 0;
	for (;;)
	{
		const Job *jobs = nullptr;
		std::size_t jobCount = 0;
		{
			std::unique_lock lock(m_mutex);
			m_workAvailable.wait(lock, [this, observedGeneration] {
				return m_stopping || m_generation != observedGeneration;
			});
			if (m_stopping)
				return;
			observedGeneration = m_generation;
			jobs = m_jobs;
			jobCount = m_jobCount;
		}

		while (!m_batchFailed.load(std::memory_order_acquire))
		{
			const std::size_t index = m_nextJob.fetch_add(1, std::memory_order_relaxed);
			if (index >= jobCount)
				break;

			try
			{
				jobs[index].function(jobs[index].context);
			}
			catch (...)
			{
				if (!m_batchFailed.exchange(true, std::memory_order_acq_rel))
				{
					std::lock_guard lock(m_mutex);
					m_failure = std::current_exception();
				}
				break;
			}
		}

		{
			std::lock_guard lock(m_mutex);
			assert(m_activeWorkers > 0);
			--m_activeWorkers;
			if (m_activeWorkers == 0)
			{
				m_jobs = nullptr;
				m_jobCount = 0;
				m_workComplete.notify_one();
			}
		}
	}
}

} // namespace engine::jobs
