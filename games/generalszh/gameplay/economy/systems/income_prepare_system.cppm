module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.economy.systems.income_prepare_system;
export import games.generalszh.gameplay.economy.income.income_components;
export namespace generalszh::economy
{
struct IncomePrepareSystem
{
	using Query = ecs::Query<ecs::Read<IncomeDefinition>, ecs::Read<IncomeObservation>,
		ecs::Write<engine::gameplay::rts::economy::IncomeEligibility>, ecs::Write<engine::gameplay::rts::economy::IncomeDispatch>>;
	void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
	{
		using namespace engine::gameplay::rts::economy;
		const auto definitions = chunk.Get<IncomeDefinition>();
		const auto observations = chunk.Get<IncomeObservation>();
		auto eligibility = chunk.Get<IncomeEligibility>();
		auto dispatch = chunk.Get<IncomeDispatch>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto &input = observations[row];
			dispatch[row].enabled = input.active && input.dispatchEnabled;
			eligibility[row].enabled = CanAccrue(input.neutral, input.constructionComplete, definitions[row].base);
		}
	}
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::economy::IncomePrepareSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.income.prepare";
	static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
	using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
