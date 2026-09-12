module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

export module engine.gameplay.rts.rank.systems.rank_system;
export import engine.ecs.system.system;
export import engine.gameplay.rts.rank.components.rank_inbox;
export import engine.gameplay.rts.rank.components.rank_state;
export import engine.gameplay.rts.rank.definitions.rank_definition;
export import engine.gameplay.rts.rank.inputs.rank_batch;
export import engine.gameplay.rts.unlocks.components.unlock_state;

export namespace engine::gameplay::rts::rank
{
class RankSystem final
{
public:
	using Query = ecs::Query<ecs::Write<RankState>, ecs::Write<RankInbox>,
		ecs::Write<unlocks::UnlockState>>;

	RankSystem(const RankCatalog &catalog, RankBatch &batch) :
		catalog_(catalog), batch_(batch)
	{
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		batch_.Prepare(context.Tick());
		query.ForEachPreparedChunk([this](Query::Chunk chunk) {
			auto inboxes = chunk.Get<RankInbox>();
			const auto entities = chunk.Entities();
			for (std::size_t row = 0; row < entities.size(); ++row)
			{
				inboxes[row] = {};
				std::size_t begin = 0;
				std::size_t count = 0;
				if (batch_.FindRange(entities[row], begin, count))
					inboxes[row] = RankInbox{begin, count};
			}
		});
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		auto states = chunk.Get<RankState>();
		auto inboxes = chunk.Get<RankInbox>();
		auto unlockStates = chunk.Get<unlocks::UnlockState>();
		const auto order = batch_.Order();
		for (std::size_t row = 0; row < states.size(); ++row)
		{
			assert(ValidState(states[row]));
			const RankInbox inbox = inboxes[row];
			for (std::size_t offset = 0; offset < inbox.count; ++offset)
			{
				const std::size_t requestIndex = order[inbox.begin + offset];
				Apply(requestIndex, states[row], unlockStates[row]);
			}
			inboxes[row] = {};
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &) { batch_.Publish(); }

private:
	[[nodiscard]] bool ValidState(const RankState &state) const noexcept
	{
		const auto limit = catalog_.LevelLimit().value;
		if (state.level == 0 || state.level > limit)
			return false;
		const auto &current = catalog_.Get(state.level);
		const auto cap = catalog_.Get(limit).skillPointsNeeded;
		if (state.skillPoints < current.skillPointsNeeded || state.skillPoints > cap)
			return false;
		return state.level == limit ||
			state.skillPoints < catalog_.Get(state.level + 1).skillPointsNeeded;
	}

	void Apply(const std::size_t requestIndex, RankState &state,
		unlocks::UnlockState &unlockState) const
	{
		const auto &input = batch_.Input(requestIndex);
		auto &result = batch_.Result(requestIndex);
		result.skillPointsBefore = state.skillPoints;
		result.skillPointsAfter = state.skillPoints;
		result.oldLevel = state.level;
		result.newLevel = state.level;
		result.creditsBefore = unlockState.credits;
		result.creditsAfter = unlockState.credits;

		const std::uint64_t cap = catalog_.Get(catalog_.LevelLimit().value).skillPointsNeeded;
		// Startup enrollment and catalog validation establish state.skillPoints <=
		// cap. Keep the release arithmetic defined even if external corruption
		// violates that internal invariant after the debug assertion above.
		const std::uint64_t room = state.skillPoints <= cap ? cap - state.skillPoints : 0;
		result.applied = (std::min)(input.amount, room);
		result.outcome = result.applied == input.amount ? RankOutcome::Applied : RankOutcome::Clamped;
		state.skillPoints += result.applied;

		while (state.level < catalog_.LevelLimit().value)
		{
			const std::uint32_t nextLevel = state.level + 1;
			if (state.skillPoints < catalog_.Get(nextLevel).skillPointsNeeded)
				break;
			const auto &definition = catalog_.Get(nextLevel);
			if (definition.unlockCreditsGranted >
				(std::numeric_limits<std::uint64_t>::max)() - unlockState.credits)
				throw std::overflow_error("Rank reward credit overflow");
			unlockState.credits += definition.unlockCreditsGranted;
			for (const auto unlock : definition.unlocksGranted)
			{
				const bool owned = unlockState.SetOwned(unlock);
				(void)owned;
				assert(owned);
			}
			state.level = nextLevel;
		}

		result.skillPointsAfter = state.skillPoints;
		result.newLevel = state.level;
		result.creditsAfter = unlockState.credits;
	}

	const RankCatalog &catalog_;
	RankBatch &batch_;
};
} // namespace engine::gameplay::rts::rank

export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::rank::RankSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.rank.system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
} // namespace ecs
