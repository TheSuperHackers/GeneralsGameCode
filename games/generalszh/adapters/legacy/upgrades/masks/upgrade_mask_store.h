#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; struct PersistentResetParticipant; }
namespace generalszh::legacy
{
class UpgradeCatalogBridge;
using LegacyUpgradeWords = std::array<std::uint64_t, 8>;
struct UpgradeMaskSnapshot
{
	LegacyUpgradeWords completed{};
	LegacyUpgradeWords inProgress{};
};
class UpgradeMaskLease;
class UpgradeMaskStore final
{
public:
	explicit UpgradeMaskStore(GameplayState &state);
	~UpgradeMaskStore() noexcept;
	UpgradeMaskStore(const UpgradeMaskStore &) = delete;
	UpgradeMaskStore &operator=(const UpgradeMaskStore &) = delete;
	std::size_t Count() const noexcept;
	PersistentResetParticipant ResetParticipant();
private:
	struct Impl;
	Impl *m_impl;
	friend class UpgradeMaskLease;
};

// Sole state is in ECS word columns. Snapshots are temporary boundary values in
// native bit order, not retained caches or a second authoritative mask owner.
class UpgradeMaskLease final
{
public:
	UpgradeMaskLease(UpgradeMaskStore &store, const UpgradeCatalogBridge &catalog);
	~UpgradeMaskLease() noexcept;
	UpgradeMaskLease(const UpgradeMaskLease &) = delete;
	UpgradeMaskLease &operator=(const UpgradeMaskLease &) = delete;
	UpgradeMaskSnapshot Read() const;
	void Clear();
	void Start(const LegacyUpgradeWords &bits);
	void Complete(const LegacyUpgradeWords &bits);
	void Remove(const LegacyUpgradeWords &bits);
	void MergeProgress(const UpgradeMaskLease &other);
	bool HasCompleted(const LegacyUpgradeWords &bits) const;
	bool HasInProgress(const LegacyUpgradeWords &bits) const;
	void Restore(const UpgradeMaskSnapshot &snapshot);
private:
	UpgradeMaskStore &m_store;
	std::size_t m_slot;
};
}
