#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; }

namespace generalszh::legacy
{
// Cold legacy transfer value. The live, authoritative state remains in the
// three ECS components owned by the bound entity; this record is not a
// serializer or a second runtime state owner.
struct PendingConditionSnapshot
{
	std::array<std::uint64_t, 2> clear{};
	std::array<std::uint64_t, 2> set{};
	bool dirty{false};
};

class PendingConditionLease;
class PendingConditionStore
{
public:
	PendingConditionStore();
	explicit PendingConditionStore(GameplayState &state);
	~PendingConditionStore() noexcept;
	PendingConditionStore(const PendingConditionStore &) = delete;
	PendingConditionStore &operator=(const PendingConditionStore &) = delete;
	PendingConditionStore(PendingConditionStore &&) = delete;
	PendingConditionStore &operator=(PendingConditionStore &&) = delete;

	std::size_t Count() const noexcept;
	void Reset();

private:
	struct Impl;
	Impl *m_impl;
	friend class PendingConditionLease;
};

class PendingConditionLease
{
public:
	explicit PendingConditionLease(PendingConditionStore &store);
	~PendingConditionLease() noexcept;
	PendingConditionLease(const PendingConditionLease &) = delete;
	PendingConditionLease &operator=(const PendingConditionLease &) = delete;
	PendingConditionLease(PendingConditionLease &&) = delete;
	PendingConditionLease &operator=(PendingConditionLease &&) = delete;

	void QueueSet(std::uint16_t canonicalCondition);
	void QueueClear(std::uint16_t canonicalCondition);
	void MarkDirty();
	PendingConditionSnapshot Read() const;
	void Restore(const PendingConditionSnapshot &snapshot);
	void Clear();

private:
	PendingConditionStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}
