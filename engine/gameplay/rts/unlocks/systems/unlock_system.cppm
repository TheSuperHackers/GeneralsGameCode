module;

#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.unlocks.systems.unlock_system;
export import engine.ecs.system.system;
export import engine.gameplay.rts.unlocks.components.unlock_inbox;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import engine.gameplay.rts.unlocks.inputs.unlock_batch;

export namespace engine::gameplay::rts::unlocks
{
class UnlockSystem final
{
public:
	using Query = ecs::Query<ecs::Write<UnlockState>, ecs::Write<UnlockInbox>>;

	UnlockSystem(const UnlockCatalog &catalog, UnlockBatch &batch) :
		catalog_(catalog),
		batch_(batch)
	{
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		batch_.Prepare(context.Tick());
		query.ForEachPreparedChunk([this](Query::Chunk chunk) {
			auto inboxes = chunk.Get<UnlockInbox>();
			const auto entities = chunk.Entities();
			for (std::size_t row = 0; row < entities.size(); ++row)
			{
				inboxes[row] = {};
				std::size_t begin = 0;
				std::size_t count = 0;
				if (batch_.FindRange(entities[row], begin, count))
					inboxes[row] = UnlockInbox{begin, count};
			}
		});
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		auto states = chunk.Get<UnlockState>();
		auto inboxes = chunk.Get<UnlockInbox>();
		const auto order = batch_.Order();
		for (std::size_t row = 0; row < states.size(); ++row)
		{
			const UnlockInbox inbox = inboxes[row];
			for (std::size_t offset = 0; offset < inbox.count; ++offset)
			{
				const std::size_t requestIndex = order[inbox.begin + offset];
				const UnlockRequest &request = batch_.Input(requestIndex);
				UnlockReceipt &receipt = batch_.Receipt(requestIndex);
				Apply(request, states[row], receipt);
			}
			// The inbox is tick-local routing state; all gameplay mutation and
			// receipt writes have completed before it is cleared.
			inboxes[row] = {};
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &)
	{
		batch_.Publish();
	}

private:
	void Apply(const UnlockRequest &request, UnlockState &state,
		UnlockReceipt &receipt) const
	{
		receipt.id = InvalidUnlockId;
		receipt.creditsBefore = state.credits;
		receipt.creditsAfter = state.credits;
		receipt.owned = false;
		if (request.operation != UnlockOperation::Purchase &&
			request.operation != UnlockOperation::Grant &&
			request.operation != UnlockOperation::Availability)
		{
			receipt.result = UnlockResultCode::InvalidOperation;
			return;
		}

		const UnlockId id = catalog_.Find(request.key);
		if (id == InvalidUnlockId)
		{
			receipt.result = UnlockResultCode::UnknownUnlock;
			return;
		}
		const UnlockEntry &definition = catalog_.Get(id);
		receipt.id = id;
		receipt.owned = state.IsOwned(id);

		if (request.operation == UnlockOperation::Grant)
		{
			// Grant follows the legacy grant boundary: grantability is checked,
			// but prerequisites, availability and credits are not consulted.
			if (!definition.grantable)
				receipt.result = UnlockResultCode::NotGrantable;
			else if (receipt.owned)
				receipt.result = UnlockResultCode::AlreadyOwned;
			else
			{
				state.SetOwned(id);
				receipt.result = UnlockResultCode::Granted;
				receipt.owned = true;
			}
			receipt.creditsAfter = state.credits;
			return;
		}

		if (receipt.owned)
		{
			receipt.result = UnlockResultCode::AlreadyOwned;
			return;
		}
		if (state.IsDisabled(id))
		{
			receipt.result = UnlockResultCode::Disabled;
			return;
		}
		if (state.IsHidden(id))
		{
			receipt.result = UnlockResultCode::Hidden;
			return;
		}
		for (const UnlockId prerequisite : catalog_.Prerequisites(id))
		{
			if (!state.IsOwned(prerequisite))
			{
				receipt.result = UnlockResultCode::MissingPrerequisite;
				return;
			}
		}
		if (definition.cost == 0)
		{
			receipt.result = UnlockResultCode::ZeroCost;
			return;
		}
		if (definition.cost > state.credits)
		{
			receipt.result = UnlockResultCode::InsufficientCredits;
			return;
		}

		if (request.operation == UnlockOperation::Availability)
		{
			receipt.result = UnlockResultCode::Available;
			return;
		}
		state.credits -= definition.cost;
		state.SetOwned(id);
		receipt.result = UnlockResultCode::Purchased;
		receipt.creditsAfter = state.credits;
		receipt.owned = true;
	}

	const UnlockCatalog &catalog_;
	UnlockBatch &batch_;
};
} // namespace engine::gameplay::rts::unlocks

export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::unlocks::UnlockSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.unlocks.system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
} // namespace ecs
