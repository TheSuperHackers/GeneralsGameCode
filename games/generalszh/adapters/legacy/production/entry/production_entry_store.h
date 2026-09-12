#pragma once
#include <cstddef>
#include <cstdint>

class ProductionEntry;
namespace generalszh { class GameplayState; }
namespace generalszh::legacy
{
class ProductionEntryLease;
class ProductionQueueStore;
class ProductionQueueLease;
// Legacy entry-state ABI boundary; component storage belongs to the injected World.
class ProductionEntryStore
{
public:
	ProductionEntryStore();
	explicit ProductionEntryStore(GameplayState &state);
	~ProductionEntryStore() noexcept;
	ProductionEntryStore(const ProductionEntryStore &) = delete;
	ProductionEntryStore &operator=(const ProductionEntryStore &) = delete;
	std::size_t Count() const noexcept;
	void Reset(); // Queue entries must have released all leases first.
private:
	GameplayState &Gameplay() const noexcept;
	ProductionEntry *Resolve(std::uint32_t index, std::uint32_t generation) const;
	struct Impl;
	Impl *m_impl;
	friend class ProductionEntryLease;
	friend class ProductionQueueStore;
	friend class ProductionQueueLease;
};

class ProductionEntryLease
{
public:
	explicit ProductionEntryLease(ProductionEntryStore &store);
	~ProductionEntryLease() noexcept;
	ProductionEntryLease(const ProductionEntryLease &) = delete;
	ProductionEntryLease &operator=(const ProductionEntryLease &) = delete;
	void Bind(ProductionEntry *entry); // Non-owning legacy identity mapping only.
	std::int32_t Type() const;
	void SetType(std::int32_t type);
	std::int32_t Correlation() const;
	void SetCorrelation(std::int32_t correlation);
	std::int32_t Total() const;
	std::int32_t Completed() const;
	std::int32_t Remaining() const;
	void Set(std::int32_t total, std::int32_t completed = 0);
	std::int32_t ExitDoor() const;
	void SetExitDoor(std::int32_t exitDoor);
	void RestoreExitDoor(std::int32_t exitDoor);
	void CompleteOne();
	void AdvanceStep();
	void RefreshProgress(std::int32_t requiredFrames);
	float Percent() const;
	// Narrow legacy transfer boundary; live ECS elapsed ticks are 64-bit.
	std::int32_t LegacyElapsedFrames() const;
	void RestoreProgress(std::int32_t elapsedFrames, float percent);
private:
	friend class ProductionQueueLease;
	ProductionEntryStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}
