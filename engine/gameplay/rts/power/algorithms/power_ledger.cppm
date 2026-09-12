module;
#include <cassert>
#include <bit>
#include <cstdint>
#include <limits>

export module engine.gameplay.rts.power.algorithms.power_ledger;
export import engine.gameplay.rts.power.components.power_ledger;

export namespace engine::gameplay::rts::power
{
constexpr bool CanAdd(PowerValue total, PowerValue delta) noexcept
{
	const auto result = static_cast<std::int64_t>(total)
		+ static_cast<std::int64_t>(delta);
	return result >= (std::numeric_limits<PowerValue>::min)()
		&& result <= (std::numeric_limits<PowerValue>::max)();
}

constexpr bool CanNegate(PowerValue value) noexcept
{
	return value != (std::numeric_limits<PowerValue>::min)();
}

// Internal simulation preconditions are debug-only. Unsigned arithmetic and
// bit_cast keep even an invalid Release overflow defined (modulo 2^32), never UB.
// The Checked names refer to debug checks, not runtime exception contracts.
inline PowerValue CheckedAdd(PowerValue total, PowerValue delta) noexcept
{
	assert(CanAdd(total, delta));
	return std::bit_cast<PowerValue>(static_cast<std::uint32_t>(total) + static_cast<std::uint32_t>(delta));
}

inline PowerValue CheckedNegate(PowerValue value) noexcept
{
	assert(CanNegate(value));
	return std::bit_cast<PowerValue>(std::uint32_t{0} - static_cast<std::uint32_t>(value));
}

inline void ApplyProduction(PowerLedger &ledger, PowerValue delta) noexcept
{
	ledger.production = CheckedAdd(ledger.production, delta);
}

inline void ApplyConsumption(PowerLedger &ledger, PowerValue delta) noexcept
{
	ledger.consumption = CheckedAdd(ledger.consumption, delta);
}
}
