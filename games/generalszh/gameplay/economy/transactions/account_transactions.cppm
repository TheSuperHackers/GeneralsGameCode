module;
#include <cstdint>
#include <limits>
#include <stdexcept>

export module games.generalszh.gameplay.economy.transactions.account_transactions;
export import engine.gameplay.rts.economy.components.resource_balance;
export import games.generalszh.gameplay.economy.income_history;

export namespace generalszh::economy
{
using engine::gameplay::rts::economy::ResourceBalance;

// The legacy Zero Hour account boundary is uint32_t even though the generic
// ECS balance deliberately has a wider uint64_t quantity.  Keep this narrow
// conversion explicit so a malformed ECS value cannot be silently truncated.
inline std::uint32_t BalanceValue(const ResourceBalance &balance)
{
	if (balance.quantity > (std::numeric_limits<std::uint32_t>::max)())
		throw std::overflow_error("Zero Hour account balance exceeds uint32 range");
	return static_cast<std::uint32_t>(balance.quantity);
}

// Quote is read-only and returns the amount that can be withdrawn from the
// current bounded account.  The caller must apply the returned quote before
// another authoritative mutation changes the account.
inline std::uint32_t QuoteWithdrawal(const ResourceBalance &balance, std::uint32_t requested)
{
	const auto available = BalanceValue(balance);
	return requested < available ? requested : available;
}

// Applying a quote is intentionally strict: an unaffordable approved amount
// is a caller error, not a second implicit capped withdrawal.  This API does
// not attach a token or version to a quote; an older quote can still be used
// when it remains affordable under the caller's access discipline.
inline void ApplyWithdrawal(ResourceBalance &balance, std::uint32_t approved)
{
	const auto available = BalanceValue(balance);
	if (approved > available)
		throw std::invalid_argument("Approved withdrawal exceeds account balance");
	engine::gameplay::rts::economy::DebitUpTo(balance, approved);
}

// Deposit follows the original Zero Hour uint32 arithmetic boundary.  A zero
// deposit has no effects, including no validation or access to IncomeHistory.
// When tracking is requested, only the current bucket index is validated here;
// full ring consistency remains a cold restore/validation responsibility.
inline void ApplyDeposit(ResourceBalance &balance, IncomeHistory &history,
	std::uint32_t amount, bool trackIncome)
{
	if (amount == 0) return;

	const auto current = BalanceValue(balance);
	if (trackIncome) RecordIncome(history, amount);
	balance.quantity = static_cast<std::uint32_t>(
		static_cast<std::uint64_t>(current) + static_cast<std::uint64_t>(amount));
}

inline void ResetAccount(ResourceBalance &balance, IncomeHistory &history,
	std::uint32_t starting) noexcept
{
	// Both current component assignments are non-throwing.  This is a logical
	// reset, not a validation boundary; restore/configuration validation remains
	// the caller's responsibility.
	balance.quantity = starting;
	history = {};
}
}
