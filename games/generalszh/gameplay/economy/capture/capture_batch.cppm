module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.economy.capture.capture_batch;
export import games.generalszh.gameplay.economy.components.capture_range;
export import games.generalszh.gameplay.economy.algorithms.auto_deposit;
export import engine.ecs.core.component_registry;
export namespace generalszh::economy
{
struct CaptureRequest { std::uint64_t tick{}; std::int32_t player{-1}; };
// Stable owner partition. Each chunk row writes only its disjoint result range.
// Recording is a joined input boundary, not a concurrent producer queue.
class CaptureBatch
{
public:
	explicit CaptureBatch(std::size_t capacity) : m_capacity(capacity)
	{ m_records.reserve(capacity); m_requests.resize(capacity); m_results.resize(capacity); m_locations.resize(capacity); }
	CaptureBatch(const CaptureBatch &) = delete;
	CaptureBatch &operator=(const CaptureBatch &) = delete;
	void Record(std::size_t owner, CaptureRequest request)
	{
		if (m_prepared) throw std::logic_error("Capture batch is prepared");
		if (m_records.size() == m_capacity) throw std::length_error("Capture capacity exhausted");
		m_records.push_back({owner, request});
	}
	std::span<const CaptureRange> Prepare(std::size_t owners)
	{
		if (m_prepared) throw std::logic_error("Capture batch already prepared");
		m_ranges.assign(owners, {}); m_cursors.assign(owners, 0);
		for (const auto &record : m_records)
		{
			if (record.owner >= owners) throw std::out_of_range("Invalid capture owner");
			++m_ranges[record.owner].count;
		}
		std::size_t offset = 0;
		for (auto &range : m_ranges) { range.first = offset; offset += range.count; }
		for (std::size_t sequence = 0; sequence != m_records.size(); ++sequence)
		{
			const auto &record = m_records[sequence];
			const auto index = m_ranges[record.owner].first + m_cursors[record.owner]++;
			m_locations[sequence] = index; m_requests[index] = record.request; m_results[index] = {};
		}
		m_prepared = true; return m_ranges;
	}
	std::span<const CaptureRequest> Requests(CaptureRange range) const noexcept
	{
		assert(m_prepared && range.first <= m_capacity && range.count <= m_capacity - range.first);
		return std::span<const CaptureRequest>(m_requests).subspan(range.first, range.count);
	}
	std::span<Payout> Results(CaptureRange range) noexcept
	{
		assert(m_prepared && range.first <= m_capacity && range.count <= m_capacity - range.first);
		return std::span<Payout>(m_results).subspan(range.first, range.count);
	}
	const Payout &Result(std::size_t sequence) const
	{
		if (!m_prepared || sequence >= m_records.size()) throw std::logic_error("Capture result unavailable");
		return m_results[m_locations[sequence]];
	}
	void Clear() noexcept { m_records.clear(); m_prepared = false; }
private:
	struct RecordValue { std::size_t owner; CaptureRequest request; };
	std::size_t m_capacity;
	bool m_prepared{false};
	std::vector<RecordValue> m_records;
	std::vector<CaptureRequest> m_requests;
	std::vector<Payout> m_results;
	std::vector<std::size_t> m_locations, m_cursors;
	std::vector<CaptureRange> m_ranges;
};
}
