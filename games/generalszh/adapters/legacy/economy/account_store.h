#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
namespace generalszh { class GameplayState; struct PersistentResetParticipant; }
namespace generalszh::legacy
{
// Plain legacy configuration/transfer value. Never retained beside a live binding.
struct AccountValue
{
	std::uint32_t balance{0};
	std::array<std::uint32_t, 60> buckets{};
	std::uint32_t current{0};
	std::uint32_t total{0};
	std::uint32_t QuoteWithdrawal(std::uint32_t requested) const;
	void ApplyWithdrawal(std::uint32_t approved);
	void ApplyDeposit(std::uint32_t amount, bool trackIncome);
	void Reset(std::uint32_t starting);
	void RecordIncome(std::uint32_t amount);
	void AdvanceBucket(std::uint32_t next);
	void ClearIncome() noexcept;
	void Validate() const;
};
class AccountLease;
class AccountStore
{
public:
	explicit AccountStore(GameplayState &state);
	~AccountStore() noexcept;
	AccountStore(const AccountStore &) = delete;
	AccountStore &operator=(const AccountStore &) = delete;
	std::size_t Count() const noexcept;
	// Narrow participant descriptor for the game composition's explicit reset.
	PersistentResetParticipant ResetParticipant();
	// Single-store compatibility convenience. Multi-store resets use the coordinator.
	// All entities must belong to the supplied participant set.
	// Bindings survive; raw World references/entities do not. Preflight rejection
	// leaves state intact. Failure after release begins is fatal to further use,
	// but bindings can still be destroyed safely.
	void ResetWorld();
private:
	struct Impl;
	Impl *m_impl;
	friend class AccountLease;
};
class AccountLease
{
public:
	AccountLease(AccountStore &store, std::uint32_t initial);
	AccountLease(AccountStore &store, const AccountValue &initial);
	~AccountLease() noexcept;
	AccountLease(const AccountLease &) = delete;
	AccountLease &operator=(const AccountLease &) = delete;
	std::uint32_t Value() const;
	void Set(std::uint32_t value);
	// Quote is read-only. Application rejects an amount no longer affordable;
	// callers must not use presentation consumers to mutate authoritative state.
	std::uint32_t QuoteWithdrawal(std::uint32_t requested) const;
	void ApplyWithdrawal(std::uint32_t approved);
	void ApplyDeposit(std::uint32_t amount, bool trackIncome);
	void Reset(std::uint32_t starting);
	std::uint32_t IncomePerMinute() const;
	void RecordIncome(std::uint32_t amount);
	void AdvanceBucket(std::uint32_t next);
	void ClearIncome();
	AccountValue State() const;
	void Restore(const AccountValue &state);
private:
	AccountStore &m_store;
	std::size_t m_slot;
};
}
