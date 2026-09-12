module;
#include <algorithm>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
export module engine.events.storage.event_batch;
export import engine.events.ordering.batch_order;

export namespace engine::events
{
enum class RecordingState { Idle, Recording, Sealed, Consumed, Failed };

template<class T> class PublishedBatch;

// Storage primitive, not a dynamically typed bus. T is the exact C++ payload
// type; stable/schema identities will belong to the message declaration layer.
template<class T>
class RecordedBatch
{
	static_assert(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>,
		"Batch payloads require noexcept move construction and destruction");
	static_assert(!std::is_polymorphic_v<T>, "Batch payloads must be value data");
	static_assert(!std::uses_allocator_v<T, std::pmr::polymorphic_allocator<T>>,
		"Payloads must not have allocator-dependent move construction");
public:
	RecordedBatch(std::size_t capacity, std::pmr::memory_resource &memory) :
		m_values(&memory), m_limit(capacity) { m_values.reserve(capacity); }
	RecordedBatch(const RecordedBatch &) = delete;
	RecordedBatch &operator=(const RecordedBatch &) = delete;
	RecordedBatch(RecordedBatch &&) = delete;
	RecordedBatch &operator=(RecordedBatch &&) = delete;

	void Begin(BatchOrder order)
	{
		if (m_state != RecordingState::Idle && m_state != RecordingState::Consumed)
			throw std::logic_error("Discard or consume an event batch before beginning another recording");
		m_order = order;
		m_state = RecordingState::Recording;
	}
	template<class... Args> void Emplace(Args &&...args)
	{
		if (m_state != RecordingState::Recording)
			throw std::logic_error("Event batch is not recording");
		try
		{
			if (m_values.size() == m_limit)
				throw std::length_error("Event recording capacity exhausted");
			m_values.emplace_back(std::forward<Args>(args)...);
		}
		catch (...)
		{
			m_values.clear(); // A caught producer exception cannot publish a prefix.
			m_state = RecordingState::Failed;
			throw;
		}
	}
	void Seal()
	{
		if (m_state != RecordingState::Recording)
			throw std::logic_error("Only a successful recording can be sealed");
		m_state = RecordingState::Sealed;
	}
	// Explicit cancellation/reclamation, never publication. The owner must first
	// join any recording jobs. Failed simulation work must not simply be retried.
	void Discard() noexcept { m_values.clear(); m_state = RecordingState::Idle; }
	[[nodiscard]] RecordingState State() const noexcept { return m_state; }
	[[nodiscard]] BatchOrder Order() const noexcept { return m_order; }
	[[nodiscard]] std::size_t Size() const noexcept { return m_values.size(); }
	[[nodiscard]] std::size_t Capacity() const noexcept { return m_limit; }
private:
	std::pmr::vector<T> m_values;
	const std::size_t m_limit;
	BatchOrder m_order{};
	RecordingState m_state{RecordingState::Idle};
	friend class PublishedBatch<T>;
};

struct BatchSegment
{
	BatchOrder order{};
	std::size_t first{0};
	std::size_t count{0};
};

template<class T>
class PublishedBatch
{
public:
	PublishedBatch(std::size_t capacity, std::size_t maxBatches, std::pmr::memory_resource &memory) :
		m_values(&memory), m_segments(&memory), m_ordered(&memory),
		m_limit(capacity), m_batchLimit(maxBatches)
	{
		m_values.reserve(capacity);
		m_segments.reserve(maxBatches);
		m_ordered.reserve(maxBatches);
	}
	PublishedBatch(const PublishedBatch &) = delete;
	PublishedBatch &operator=(const PublishedBatch &) = delete;
	PublishedBatch(PublishedBatch &&) = delete;
	PublishedBatch &operator=(PublishedBatch &&) = delete;

	void Publish(BatchBoundary boundary, std::span<RecordedBatch<T> *const> batches)
	{
		if (m_published) throw std::logic_error("Release the previous publication before publishing again");
		if (batches.size() > m_batchLimit) throw std::length_error("Too many event recording batches");
		m_ordered.clear();
		struct ClearScratch
		{
			std::pmr::vector<RecordedBatch<T> *> &pointers;
			~ClearScratch() noexcept { pointers.clear(); }
		} clearScratch{m_ordered};
		std::size_t total = 0;
		// Validate the entire set before moving any payload. A validation failure
		// leaves sealed producers intact and no publication visible.
		for (auto *batch : batches)
		{
			if (!batch || batch->m_state != RecordingState::Sealed)
				throw std::logic_error("Publication requires sealed event batches");
			if (batch->m_order.boundary != boundary)
				throw std::logic_error("Event batch belongs to another tick or phase");
			if (batch->Size() > m_limit - total)
				throw std::length_error("Event publication capacity exhausted");
			total += batch->Size();
			m_ordered.push_back(batch);
		}
		std::sort(m_ordered.begin(), m_ordered.end(), [](const auto *a, const auto *b) {
			return a->m_order < b->m_order;
		});
		for (std::size_t i = 1; i < m_ordered.size(); ++i)
			if (m_ordered[i - 1]->m_order == m_ordered[i]->m_order)
				throw std::logic_error("Duplicate logical event batch order");

		// Reserved capacity + ordinary noexcept moves/destructors: the destructive
		// transfer has no allocating or throwing user operation. No callback dispatch.
		for (auto *batch : m_ordered)
		{
			m_segments.push_back({batch->m_order, m_values.size(), batch->Size()});
			for (auto &value : batch->m_values) m_values.emplace_back(std::move(value));
			batch->m_values.clear();
			batch->m_state = RecordingState::Consumed;
		}
		m_boundary = boundary;
		m_published = true;
	}
	[[nodiscard]] bool IsPublished() const noexcept { return m_published; }
	[[nodiscard]] std::span<const T> Values() const { RequirePublished(); return m_values; }
	[[nodiscard]] std::span<const BatchSegment> Segments() const { RequirePublished(); return m_segments; }
	[[nodiscard]] BatchBoundary Boundary() const { RequirePublished(); return m_boundary; }
	[[nodiscard]] std::size_t Capacity() const noexcept { return m_limit; }
	// Reader spans expire here. The caller must join all consumers first.
	void Release() noexcept
	{
		m_values.clear(); m_segments.clear(); m_ordered.clear(); m_published = false;
	}
private:
	void RequirePublished() const
	{
		if (!m_published) throw std::logic_error("No event batch has been published");
	}
	std::pmr::vector<T> m_values;
	std::pmr::vector<BatchSegment> m_segments;
	std::pmr::vector<RecordedBatch<T> *> m_ordered;
	const std::size_t m_limit, m_batchLimit;
	BatchBoundary m_boundary{};
	bool m_published{false};
};
}
