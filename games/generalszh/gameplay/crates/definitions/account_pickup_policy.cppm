module;

#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.gameplay.crates.definitions.account_pickup_policy;
export import engine.ecs.core.entity;

export namespace generalszh::crates
{
// Human/neutral classification is immutable account policy supplied by the
// composition root. It is not a second ECS owner or dynamic collector state.
struct AccountPickupPolicy final
{
	ecs::Entity account{};
	bool human{};
	bool neutral{};
};

class AccountPickupPolicies final
{
public:
	explicit AccountPickupPolicies(const std::size_t entityIndexCapacity) :
		records_(entityIndexCapacity)
	{
		if (entityIndexCapacity == 0)
			throw std::invalid_argument("Account pickup policy capacity must be positive");
	}

	AccountPickupPolicies(const std::size_t entityIndexCapacity,
		const std::span<const AccountPickupPolicy> entries) :
		AccountPickupPolicies(entityIndexCapacity)
	{
		for (const auto &entry : entries)
			Register(entry.account, entry.human, entry.neutral);
		Freeze();
	}

	void Register(const ecs::Entity account, const bool human, const bool neutral)
	{
		if (frozen_)
			throw std::logic_error("Account pickup policies are already frozen");
		if (!account.IsValid() || account.index >= records_.size())
			throw std::invalid_argument("Account pickup policy has an invalid entity");
		auto &record = records_[account.index];
		if (record.account.IsValid())
			throw std::invalid_argument("Account pickup policy contains a duplicate entity");
		record = AccountPickupPolicy{account, human, neutral};
	}

	void Freeze()
	{
		if (frozen_)
			throw std::logic_error("Account pickup policies are already frozen");
		frozen_ = true;
	}

	bool IsFrozen() const noexcept { return frozen_; }

	const AccountPickupPolicy *Find(const ecs::Entity account) const
	{
		RequireFrozen();
		if (!account.IsValid() || account.index >= records_.size())
			return nullptr;
		const auto &record = records_[account.index];
		return record.account.IsValid() && record.account == account ? &record : nullptr;
	}

	bool IsHuman(const ecs::Entity account) const
	{
		const auto *record = Find(account);
		return record != nullptr && record->human;
	}

	bool IsNeutral(const ecs::Entity account) const
	{
		const auto *record = Find(account);
		return record != nullptr && record->neutral;
	}

private:
	void RequireFrozen() const
	{
		if (!frozen_)
			throw std::logic_error("Account pickup policies are not frozen");
	}

	std::vector<AccountPickupPolicy> records_;
	bool frozen_{false};
};
} // namespace generalszh::crates
