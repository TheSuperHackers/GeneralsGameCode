#pragma once

#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; }

namespace generalszh::legacy
{
class UpgradeExecutionLease;
class UpgradeExecutionStore final
{
public:
	explicit UpgradeExecutionStore(GameplayState &state);
	~UpgradeExecutionStore() noexcept;
	UpgradeExecutionStore(const UpgradeExecutionStore &) = delete;
	UpgradeExecutionStore &operator=(const UpgradeExecutionStore &) = delete;
	UpgradeExecutionStore(UpgradeExecutionStore &&) = delete;
	UpgradeExecutionStore &operator=(UpgradeExecutionStore &&) = delete;
	std::size_t Count() const noexcept;
private:
	struct Impl;
	Impl *m_impl;
	friend class UpgradeExecutionLease;
};

// One non-persistent module-instance binding. No cached execution value or
// per-lease allocation; the ECS column is authoritative throughout its lifetime.
class UpgradeExecutionLease final
{
public:
	explicit UpgradeExecutionLease(UpgradeExecutionStore &store);
	~UpgradeExecutionLease() noexcept;
	UpgradeExecutionLease(const UpgradeExecutionLease &) = delete;
	UpgradeExecutionLease &operator=(const UpgradeExecutionLease &) = delete;
	UpgradeExecutionLease(UpgradeExecutionLease &&) = delete;
	UpgradeExecutionLease &operator=(UpgradeExecutionLease &&) = delete;
	bool Get() const;
	void Set(bool executed);
private:
	UpgradeExecutionStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}
