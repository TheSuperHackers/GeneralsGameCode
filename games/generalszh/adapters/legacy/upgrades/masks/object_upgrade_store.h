#pragma once

#include "upgrade_mask_store.h"

#include <cstddef>

namespace generalszh
{
class GameplayState;
}

namespace generalszh::legacy
{
class UpgradeCatalogBridge;

class ObjectUpgradeLease;
class ObjectUpgradeStore final
{
public:
	explicit ObjectUpgradeStore(GameplayState &state);
	~ObjectUpgradeStore() noexcept;
	ObjectUpgradeStore(const ObjectUpgradeStore &) = delete;
	ObjectUpgradeStore &operator=(const ObjectUpgradeStore &) = delete;
	ObjectUpgradeStore(ObjectUpgradeStore &&) = delete;
	ObjectUpgradeStore &operator=(ObjectUpgradeStore &&) = delete;

	std::size_t Count() const noexcept;

private:
	struct Impl;
	Impl *m_impl;
	friend class ObjectUpgradeLease;
};

// The ECS CompletedUpgradeWord columns are the sole authoritative object
// upgrade state. Native words are temporary values at this legacy boundary.
class ObjectUpgradeLease final
{
public:
	ObjectUpgradeLease(ObjectUpgradeStore &store, const UpgradeCatalogBridge &catalog);
	~ObjectUpgradeLease() noexcept;
	ObjectUpgradeLease(const ObjectUpgradeLease &) = delete;
	ObjectUpgradeLease &operator=(const ObjectUpgradeLease &) = delete;
	ObjectUpgradeLease(ObjectUpgradeLease &&) = delete;
	ObjectUpgradeLease &operator=(ObjectUpgradeLease &&) = delete;

	LegacyUpgradeWords Read() const;
	void Add(const LegacyUpgradeWords &bits);
	void Remove(const LegacyUpgradeWords &bits);
	bool HasAll(const LegacyUpgradeWords &bits) const;
	void Restore(const LegacyUpgradeWords &bits);

private:
	ObjectUpgradeStore &m_store;
	std::size_t m_slot;
};
}
