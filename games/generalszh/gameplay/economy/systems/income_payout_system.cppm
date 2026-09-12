module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.economy.systems.income_payout_system;
export import games.generalszh.gameplay.economy.income.income_components;
export namespace generalszh::economy
{
struct IncomePayoutSystem
{
	using Query = ecs::Query<ecs::Read<engine::gameplay::rts::economy::IncomePulse>,
		ecs::Read<IncomeDefinition>, ecs::Read<IncomeObservation>, ecs::Write<CaptureRewardState>, ecs::Write<IncomePayout>>;
	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		const auto pulses = chunk.Get<engine::gameplay::rts::economy::IncomePulse>();
		const auto definitions = chunk.Get<IncomeDefinition>();
		const auto observations = chunk.Get<IncomeObservation>();
		auto rewards = chunk.Get<CaptureRewardState>();
		auto outputs = chunk.Get<IncomePayout>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			outputs[row] = {};
			if (!pulses[row].due) continue;
			OnFirstIncomeDue(rewards[row]);
			if (pulses[row].quantity == 0) continue;
			outputs[row].value = PlanValidatedIncome(definitions[row].base, observations[row].boost,
				definitions[row].actualMoney, observations[row].visible);
		}
	}
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::economy::IncomePayoutSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.income.payout";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<engine::gameplay::rts::economy::IncomeSystem>;
};
}
