#pragma once

#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; struct PersistentResetParticipant; }

namespace generalszh::legacy
{
class UpgradeStatusLease;
class UpgradeStatusStore
{
public:
	explicit UpgradeStatusStore(GameplayState &state);
	~UpgradeStatusStore() noexcept;
	UpgradeStatusStore(const UpgradeStatusStore &) = delete;
	UpgradeStatusStore &operator=(const UpgradeStatusStore &) = delete;
	UpgradeStatusStore(UpgradeStatusStore &&) = delete;
	UpgradeStatusStore &operator=(UpgradeStatusStore &&) = delete;

	std::size_t Count() const noexcept;
	PersistentResetParticipant ResetParticipant();

private:
	struct Impl;
	Impl *m_impl;
	friend class UpgradeStatusLease;
};

class UpgradeStatusLease
{
public:
	explicit UpgradeStatusLease(UpgradeStatusStore &store);
	~UpgradeStatusLease() noexcept;
	UpgradeStatusLease(const UpgradeStatusLease &) = delete;
	UpgradeStatusLease &operator=(const UpgradeStatusLease &) = delete;
	UpgradeStatusLease(UpgradeStatusLease &&) = delete;
	UpgradeStatusLease &operator=(UpgradeStatusLease &&) = delete;

	std::int32_t Get() const;
	void Set(std::int32_t value);
	void Restore(std::int32_t value);

private:
	UpgradeStatusStore &m_store;
	std::size_t m_slot;
};
}
