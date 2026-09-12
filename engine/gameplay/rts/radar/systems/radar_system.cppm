module;
#include <cstddef>
#include <string_view>
export module engine.gameplay.rts.radar.systems.radar_system;
export import engine.gameplay.rts.radar.components.radar_availability;
export import engine.gameplay.rts.radar.algorithms.radar_availability;
export import engine.gameplay.rts.radar.commands.radar_batch;
export import engine.ecs.system.system;

export namespace engine::gameplay::rts::radar
{
struct RadarSystem
{
	using Query = ecs::Query<ecs::Write<RadarAvailability>, ecs::Write<RadarBatchRange>>;
	explicit RadarSystem(RadarBatch &batch) noexcept : m_batch(batch) {}
	void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
	{
		auto states = chunk.Get<RadarAvailability>();
		auto ranges = chunk.Get<RadarBatchRange>();
		for (std::size_t row = 0; row < states.size(); ++row)
		{
			auto &state = states[row];
			const auto actions = m_batch.Actions(ranges[row]);
			auto transitions = m_batch.Transitions(ranges[row]);
			for (std::size_t index = 0; index < actions.size(); ++index)
			{
				const bool before = HasRadar(state);
				switch (actions[index])
				{
				case RadarAction::Add: AddProvider(state, false); break;
				case RadarAction::AddResistant: AddProvider(state, true); break;
				case RadarAction::Remove: RemoveProvider(state, false); break;
				case RadarAction::RemoveResistant: RemoveProvider(state, true); break;
				case RadarAction::Suppress: SetSuppressed(state, true); break;
				case RadarAction::Unsuppress: SetSuppressed(state, false); break;
				}
				const bool after = HasRadar(state);
				transitions[index] = before == after ? RadarTransition::None :
					(after ? RadarTransition::Online : RadarTransition::Offline);
			}
			// Consume the row's slice. No input replays in a later execution.
			ranges[row] = {};
		}
	}
private:
	RadarBatch &m_batch;
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::radar::RadarSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.radar.system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
