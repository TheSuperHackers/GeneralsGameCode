#pragma once

#include <cstddef>
#include <cstdint>

class ProductionEntry;
namespace generalszh { class GameplayState; }

namespace generalszh::legacy
{
class ProductionEntryStore;
class ProductionEntryLease;

class ProductionQueueLease;
class ProductionQueueStore
{
public:
	explicit ProductionQueueStore(ProductionEntryStore &entries);
	explicit ProductionQueueStore(GameplayState &state, ProductionEntryStore &entries);
	~ProductionQueueStore() noexcept;
	ProductionQueueStore(const ProductionQueueStore &) = delete;
	ProductionQueueStore &operator=(const ProductionQueueStore &) = delete;
	ProductionQueueStore(ProductionQueueStore &&) = delete;
	ProductionQueueStore &operator=(ProductionQueueStore &&) = delete;

	std::size_t Count() const noexcept;

private:
	struct Impl;
	Impl *m_impl;
	friend class ProductionQueueLease;
};

class ProductionQueueLease
{
public:
	explicit ProductionQueueLease(ProductionQueueStore &store,
		std::uint32_t initialCapacity = 0);
	~ProductionQueueLease() noexcept;
	ProductionQueueLease(const ProductionQueueLease &) = delete;
	ProductionQueueLease &operator=(const ProductionQueueLease &) = delete;
	ProductionQueueLease(ProductionQueueLease &&) = delete;
	ProductionQueueLease &operator=(ProductionQueueLease &&) = delete;

	std::uint32_t Count() const;
	ProductionEntry *First() const;
	ProductionEntry *Next(const ProductionEntryLease &entry) const;
	bool Contains(const ProductionEntryLease &entry) const;
	void Append(ProductionEntryLease &entry);
	void Remove(ProductionEntryLease &entry);

private:
	ProductionQueueStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}
