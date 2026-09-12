module;
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
export module games.generalszh.gameplay.economy.algorithms.auto_deposit;
export import games.generalszh.gameplay.economy.components.capture_reward_state;

export namespace generalszh::economy
{
inline void OnFirstIncomeDue(CaptureRewardState &state) noexcept
{
	if (!state.initialized) state = {true, true};
}
inline bool CanAccrue(bool neutral, bool constructionComplete, std::int32_t baseAmount) noexcept
{
	return !neutral && constructionComplete && baseAmount > 0;
}
struct Payout
{
	std::uint32_t money{0};
	std::int32_t score{0};
	std::int32_t displayAmount{0};
	bool pay{false};
	bool show{false};
};
inline void ValidateIncomeAmount(std::int32_t base, std::int32_t boost)
{
	const std::int64_t amount = static_cast<std::int64_t>(base) + boost;
	if (amount < (std::numeric_limits<std::int32_t>::min)() || amount > (std::numeric_limits<std::int32_t>::max)())
		throw std::overflow_error("Zero Hour income amount exceeds its supported range");
}
// Internal chunk path. External configuration/observation validation precedes
// execution. Both assertion predicates disappear under NDEBUG; required sum and
// defined integral conversions are outside assertions.
inline Payout PlanValidatedIncome(std::int32_t base, std::int32_t boost, bool actualMoney, bool visible) noexcept
{
	assert(static_cast<std::int64_t>(base) + boost >= (std::numeric_limits<std::int32_t>::min)());
	assert(static_cast<std::int64_t>(base) + boost <= (std::numeric_limits<std::int32_t>::max)());
	const std::int64_t amount = static_cast<std::int64_t>(base) + boost;
	// Explicitly preserve the legacy unsigned money boundary. Score credit is
	// deliberately the base quantity, not boosted income; these are game rules.
	return {static_cast<std::uint32_t>(amount), base, static_cast<std::int32_t>(amount), actualMoney, amount > 0 && visible};
}
inline Payout PlanIncome(std::int32_t base, std::int32_t boost, bool actualMoney, bool visible)
{
	ValidateIncomeAmount(base, boost);
	return PlanValidatedIncome(base, boost, actualMoney, visible);
}
inline Payout PlanCapture(bool available, bool hasPlayer, std::int32_t bonus) noexcept
{
	if (!available || !hasPlayer || bonus <= 0) return {};
	return {static_cast<std::uint32_t>(bonus), bonus, bonus, true, true};
}
}
