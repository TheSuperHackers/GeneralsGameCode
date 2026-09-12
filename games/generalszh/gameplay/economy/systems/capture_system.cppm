module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.economy.systems.capture_system;
export import games.generalszh.gameplay.economy.income.income_components;
export import games.generalszh.gameplay.economy.capture.capture_batch;
export namespace generalszh::economy
{
struct CaptureSystem
{
	explicit CaptureSystem(CaptureBatch &batch) : batch(batch) {}
	using Query = ecs::Query<ecs::Write<engine::gameplay::rts::economy::IncomeSchedule>,
		ecs::Read<engine::gameplay::rts::economy::IncomeRate>, ecs::Write<CaptureRewardState>,
		ecs::Read<IncomeDefinition>, ecs::Read<IncomeObservation>, ecs::Write<CaptureRange>>;
	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		using namespace engine::gameplay::rts::economy;
		auto schedules = chunk.Get<IncomeSchedule>();
		const auto rates = chunk.Get<IncomeRate>();
		auto rewards = chunk.Get<CaptureRewardState>();
		const auto definitions = chunk.Get<IncomeDefinition>();
		const auto observations = chunk.Get<IncomeObservation>();
		const auto ranges = chunk.Get<CaptureRange>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto requests = batch.Requests(ranges[row]);
			auto results = batch.Results(ranges[row]);
			for (std::size_t index = 0; index != requests.size(); ++index)
			{
				RearmIncome(schedules[row], requests[index].tick, rates[row].intervalTicks);
				results[index] = PlanCapture(rewards[row].available,
					requests[index].player >= 0 && observations[row].active, definitions[row].captureBonus);
				if (results[index].pay) rewards[row].available = false;
			}
		}
	}
private:
	CaptureBatch &batch;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::economy::CaptureSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.income.capture";
	static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
	using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
