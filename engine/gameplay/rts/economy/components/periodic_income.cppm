module;
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.rts.economy.components.periodic_income;
export import engine.ecs.core.component_registry;
export import engine.time.simulation_time;

namespace engine::gameplay::rts::economy::detail
{
// Keep exception construction in its owning translation unit. Clang 22.1.8's
// Windows ABI can omit the exception copy constructor when emitting an imported
// inline throw, corrupting std::exception_ptr transfer from a worker. Only the
// cold failure path calls this function; the normal contiguous loop is unchanged.
// A module-local subtype also prevents a malformed std::overflow_error catchable
// type array emitted by another imported inline throw from winning COMDAT folding.
class IncomeTickOverflow final : public std::overflow_error
{
public:
	IncomeTickOverflow() : std::overflow_error("Income schedule exceeds simulation tick range") {}
};
[[noreturn]] void ThrowIncomeTickOverflow()
{
	throw IncomeTickOverflow{};
}
}

export namespace engine::gameplay::rts::economy
{
struct IncomeSchedule { std::uint64_t nextTick{0}; };
struct IncomeRate
{
	std::uint64_t intervalTicks{0};
	std::uint64_t quantity{0};
};
struct IncomeEligibility { bool enabled{false}; };
// Output for the current execution, not an account balance or accumulated copy.
// The game composes a consumer after IncomeSystem and owns transfer semantics.
struct IncomePulse
{
	std::uint64_t quantity{0};
	bool due{false};
};

inline IncomeRate MakeIncomeRate(time::Duration interval, std::uint64_t quantity, time::FixedStep step)
{
	return {step.TicksFor(interval), quantity};
}
inline bool IsIncomeDue(const IncomeSchedule &schedule, std::uint64_t tick) noexcept
{
	return tick >= schedule.nextTick;
}
inline void RearmIncome(IncomeSchedule &schedule, std::uint64_t tick, std::uint64_t interval)
{
	if (interval > (std::numeric_limits<std::uint64_t>::max)() - tick)
		detail::ThrowIncomeTickOverflow();
	schedule.nextTick = tick + interval;
}
inline IncomePulse EvaluateIncome(IncomeSchedule &schedule, const IncomeRate &rate,
	IncomeEligibility eligibility, std::uint64_t tick)
{
	if (!IsIncomeDue(schedule, tick)) return {};
	RearmIncome(schedule, tick, rate.intervalTicks);
	return {eligibility.enabled ? rate.quantity : 0, true};
}
}

export namespace ecs
{
#define INCOME_COMPONENT(Type, Name, Policy) \
template<> struct ComponentTraits<engine::gameplay::rts::economy::Type> \
{ \
	static constexpr std::string_view StableName = Name; \
	static constexpr std::uint32_t Version = 1; \
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; \
};
INCOME_COMPONENT(IncomeSchedule, "engine.gameplay.rts.economy.income_schedule", Serializable)
INCOME_COMPONENT(IncomeRate, "engine.gameplay.rts.economy.income_rate", Serializable)
INCOME_COMPONENT(IncomeEligibility, "engine.gameplay.rts.economy.income_eligibility", Transient)
INCOME_COMPONENT(IncomePulse, "engine.gameplay.rts.economy.income_pulse", Transient)
#undef INCOME_COMPONENT
}
