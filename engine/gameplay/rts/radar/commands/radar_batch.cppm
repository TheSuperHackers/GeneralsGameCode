module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.rts.radar.commands.radar_batch;
export import engine.gameplay.rts.radar.components.radar_batch_range;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::radar
{
enum class RadarAction : std::uint8_t
{
	Add, AddResistant, Remove, RemoveResistant, Suppress, Unsuppress
};
enum class RadarTransition : std::uint8_t { None, Online, Offline };

class RadarBatch
{
public:
	explicit RadarBatch(std::size_t capacity) : m_capacity(capacity)
	{
		m_requests.reserve(capacity);
		m_actions.resize(capacity);
		m_transitions.resize(capacity);
		m_locations.resize(capacity);
	}
	RadarBatch(const RadarBatch &) = delete;
	RadarBatch &operator=(const RadarBatch &) = delete;
	std::size_t Size() const noexcept { return m_requests.size(); }
	std::size_t Capacity() const noexcept { return m_capacity; }
	bool IsPrepared() const noexcept { return m_prepared; }

	// Input must already have a canonical producer order. This is not a shared
	// worker queue: parallel producers must merge before reaching this boundary.
	void Record(std::size_t owner, RadarAction action)
	{
		RequireRecording();
		if (Size() == m_capacity) throw std::length_error("Radar input capacity exhausted");
		m_requests.push_back({owner, action, true});
	}
	void Cancel(std::size_t owner)
	{
		RequireRecording();
		for (auto &request : m_requests)
			if (request.owner == owner) request.live = false;
	}

	// Stable counting partition: no comparison sort, hashes or pointer ordering.
	// Owner bookkeeping may grow here when composition adds owners, never in jobs.
	std::span<const RadarBatchRange> Prepare(std::size_t ownerCount)
	{
		RequireRecording();
		m_ranges.assign(ownerCount, {});
		m_cursors.assign(ownerCount, 0);
		for (const auto &request : m_requests)
			if (request.live)
			{
				if (request.owner >= ownerCount) throw std::out_of_range("Invalid radar batch owner");
				++m_ranges[request.owner].count;
			}
		std::size_t offset = 0;
		for (auto &range : m_ranges) { range.first = offset; offset += range.count; }
		for (std::size_t sequence = 0; sequence < Size(); ++sequence)
		{
			const auto &request = m_requests[sequence];
			if (!request.live) continue;
			const auto index = m_ranges[request.owner].first + m_cursors[request.owner]++;
			m_locations[sequence] = index;
			m_actions[index] = request.action;
			m_transitions[index] = RadarTransition::None;
		}
		m_prepared = true;
		return m_ranges;
	}
	std::span<const RadarAction> Actions(RadarBatchRange range) const noexcept
	{
		assert(m_prepared && range.first <= m_capacity && range.count <= m_capacity - range.first);
		return std::span<const RadarAction>{m_actions}.subspan(range.first, range.count);
	}
	std::span<RadarTransition> Transitions(RadarBatchRange range) noexcept
	{
		assert(m_prepared && range.first <= m_capacity && range.count <= m_capacity - range.first);
		return std::span<RadarTransition>{m_transitions}.subspan(range.first, range.count);
	}
	// Only after the scheduler has joined all jobs. Completion timing is not an
	// ordering input: consumers traverse the original canonical sequence.
	RadarTransition Result(std::size_t sequence) const
	{
		if (!m_prepared || sequence >= Size()) throw std::logic_error("Radar result unavailable");
		return m_requests[sequence].live ? m_transitions[m_locations[sequence]] : RadarTransition::None;
	}
	void Clear() noexcept { m_requests.clear(); m_prepared = false; }

private:
	void RequireRecording() const
	{
		if (m_prepared) throw std::logic_error("Cannot change a prepared radar batch");
	}
	struct Request { std::size_t owner; RadarAction action; bool live; };
	std::size_t m_capacity;
	bool m_prepared{false};
	std::vector<Request> m_requests;
	std::vector<RadarAction> m_actions;
	std::vector<RadarTransition> m_transitions;
	std::vector<std::size_t> m_locations, m_cursors;
	std::vector<RadarBatchRange> m_ranges;
};
}
