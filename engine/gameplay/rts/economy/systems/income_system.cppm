module;
#include <cstddef>
#include <string_view>
export module engine.gameplay.rts.economy.systems.income_system;
export import engine.gameplay.rts.economy.components.periodic_income;
export import engine.gameplay.rts.economy.components.income_dispatch;
export import engine.ecs.system.system;

export namespace engine::gameplay::rts::economy
{
struct IncomeSystem
{
	using Query = ecs::Query<ecs::Write<IncomeSchedule>, ecs::Read<IncomeRate>,
		ecs::Read<IncomeEligibility>, ecs::Write<IncomePulse>, ecs::Optional<IncomeDispatch>>;
	void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
	{
		auto schedules = chunk.Get<IncomeSchedule>();
		const auto rates = chunk.Get<IncomeRate>();
		const auto eligibility = chunk.Get<IncomeEligibility>();
		auto pulses = chunk.Get<IncomePulse>();
		const auto dispatch = chunk.Get<IncomeDispatch>();
		const auto tick = context.Time().Tick();
		// Select the optional column once per chunk. The ungated path keeps the
		// original contiguous loop; gated rows never evaluate/rearm while blocked.
		if (dispatch.empty())
		{
			for (std::size_t row = 0; row < schedules.size(); ++row)
				pulses[row] = EvaluateIncome(schedules[row], rates[row], eligibility[row], tick);
		}
		else
		{
			for (std::size_t row = 0; row < schedules.size(); ++row)
				pulses[row] = dispatch[row].enabled
					? EvaluateIncome(schedules[row], rates[row], eligibility[row], tick) : IncomePulse{};
		}
	}
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::economy::IncomeSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.economy.income_system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
