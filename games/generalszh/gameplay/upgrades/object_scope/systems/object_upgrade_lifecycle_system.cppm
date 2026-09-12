module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.upgrades.object_scope.systems.object_upgrade_lifecycle_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.rts.radar.components.radar_provider_contribution;
export import engine.gameplay.rts.upgrades.components.upgrade_words;
export import engine.gameplay.rts.upgrades.object_scope.components.object_upgrade_scope;
export import engine.gameplay.rts.upgrades.object_scope.inputs.object_upgrade_batch;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.radar.components.radar_provider_binding;
export import games.generalszh.gameplay.radar.definitions.radar_provider_definition;
export import games.generalszh.gameplay.upgrades.identity.upgrade_catalog;
export import games.generalszh.gameplay.upgrades.object_scope.components.object_upgrade_lifecycle;

export namespace generalszh::upgrades::object_scope
{
class ObjectUpgradeLifecycleSystem final
{
public:
	using Target = engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeTarget;
	using WordMarker = engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeWordMarker;
	using WordOwner = engine::gameplay::rts::upgrades::UpgradeWordOwner;
	using WordOrdinal = engine::gameplay::rts::upgrades::UpgradeWordOrdinal;
	using CompletedWord = engine::gameplay::rts::upgrades::CompletedUpgradeWord;
	using Batch = engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeBatch;
	using Request = engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeRequest;
	using Operation = engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeOperation;

	using Query = ecs::Query<
		ecs::Write<ObjectUpgradeLifecycle>,
		ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Optional<generalszh::construction::Structure>,
		ecs::Optional<generalszh::production::ProducedUnit>,
		ecs::OptionalWrite<Target>,
		ecs::OptionalWrite<generalszh::radar::RadarProviderBinding>,
		ecs::OptionalWrite<generalszh::radar::RadarProviderGrant>,
		ecs::OptionalWrite<engine::gameplay::rts::radar::RadarProviderContribution>>;

	// Target and provider columns are in Query as OptionalWrite terms.  The
	// auxiliary declaration therefore contains only child columns created by
	// the deferred enrollment commands; no component is declared twice.
	using AuxiliaryAccess = ecs::Query<
		ecs::Write<WordOwner>,
		ecs::Write<WordOrdinal>,
		ecs::Write<WordMarker>,
		ecs::Write<CompletedWord>>;

	ObjectUpgradeLifecycleSystem(
		const generalszh::upgrades::UpgradeCatalog &catalog,
		Batch &batch,
		const std::size_t wordRowCapacity,
		const generalszh::radar::RadarProviderDefinitions *radarDefinitions = nullptr) :
		catalog_(catalog),
		batch_(batch),
		wordRowCapacity_(wordRowCapacity),
		radareDefinitions_(radarDefinitions)
	{
		if (!catalog_.IsFinalized())
			throw std::logic_error("Object upgrade lifecycle requires a finalized upgrade catalog");
		if (catalog_.Count() >
			static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) - 63u)
			throw std::length_error("Object upgrade catalog word count exceeds runtime range");
		if (wordRowCapacity_ == 0)
			throw std::invalid_argument("Object upgrade lifecycle word-row capacity must be positive");

		wordCount_ = static_cast<std::uint32_t>((catalog_.Count() + 63u) / 64u);
		chunkOffsets_.reserve(batch_.EntityIndexCapacity());
		rows_.reserve(batch_.EntityIndexCapacity());
		hostRequests_.reserve(batch_.Capacity());
		generatedRequests_.reserve(batch_.Capacity());
		mergeRequests_.reserve(batch_.Capacity());
	}

	ObjectUpgradeLifecycleSystem(const ObjectUpgradeLifecycleSystem &) = delete;
	ObjectUpgradeLifecycleSystem &operator=(const ObjectUpgradeLifecycleSystem &) = delete;

	// This hook only snapshots the host prefix and allocates deterministic row
	// slots.  Readiness and enrollment are gameplay decisions made by Execute.
	void BeforeChunks(Query &query, ecs::SystemContext &)
	{
		if (batch_.IsPublished())
			throw std::logic_error("Object upgrade lifecycle input was not reset before the next tick");

		hostRequests_.assign(batch_.Requests().begin(), batch_.Requests().end());
		if (hostRequests_.size() > batch_.Capacity())
			throw std::length_error("Object upgrade lifecycle host input capacity exhausted");

		chunkOffsets_.clear();
		rows_.clear();
		generatedRequests_.clear();
		mergeRequests_.clear();

		const std::size_t chunkCount = query.PreparedChunkCount();
		if (chunkCount > (std::numeric_limits<std::uint32_t>::max)())
			throw std::length_error("Object upgrade lifecycle chunk capacity exhausted");
		chunkOffsets_.reserve(chunkCount);

		std::size_t rowCount = 0;
		query.ForEachPreparedChunk([&](Query::Chunk chunk) {
			chunkOffsets_.push_back(rowCount);
			if (chunk.Count() > (std::numeric_limits<std::size_t>::max)() - rowCount)
				throw std::length_error("Object upgrade lifecycle row capacity exhausted");
			rowCount += chunk.Count();
		});
		rows_.resize(rowCount);
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &context)
	{
		if (context.ChunkOrder() >= chunkOffsets_.size())
			throw std::logic_error("Object upgrade lifecycle received an unprepared chunk order");
		const std::size_t offset = chunkOffsets_[context.ChunkOrder()];

		const auto lifecycles = chunk.Get<ObjectUpgradeLifecycle>();
		const auto lives = chunk.Get<engine::gameplay::combat::LifeState>();
		const auto structures = chunk.Get<generalszh::construction::Structure>();
		const auto units = chunk.Get<generalszh::production::ProducedUnit>();
		auto targets = chunk.Get<Target>();
		auto bindings = chunk.Get<generalszh::radar::RadarProviderBinding>();
		auto grants = chunk.Get<generalszh::radar::RadarProviderGrant>();
		auto contributions = chunk.Get<engine::gameplay::rts::radar::RadarProviderContribution>();

		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			PreparedRow &prepared = rows_[offset + row];
			prepared = {};
			prepared.entity = chunk.Entities()[row];
			prepared.nextStage = lifecycles[row].stage;

			if (lifecycles[row].stage == ObjectUpgradeLifecycleStage::RequestPublished ||
				lifecycles[row].stage == ObjectUpgradeLifecycleStage::SuppressedDead)
				continue;

			ValidateDefinition(lifecycles[row].definition);

			if (!lives[row].alive)
			{
				prepared.nextStage = ObjectUpgradeLifecycleStage::SuppressedDead;
				continue;
			}

			const bool hasStructure = !structures.empty();
			const bool hasProducedUnit = !units.empty();
			if (hasStructure == hasProducedUnit)
				throw std::logic_error(
					"Object upgrade lifecycle requires exactly one structure or produced-unit owner");

			const bool ready = hasProducedUnit || structures[row].complete;
			const auto *radarDefinition = FindRadarDefinition(lifecycles[row].definition.binding);

			if (targets.empty())
			{
				if (static_cast<std::size_t>(prepared.entity.index) >= batch_.EntityIndexCapacity())
					throw std::length_error("Object upgrade lifecycle target exceeds entity capacity");
				context.Commands().Add<Target>(prepared.entity,
					Target{wordCount_, catalog_.SchemaHash()});
				for (std::uint32_t ordinal = 0; ordinal != wordCount_; ++ordinal)
				{
					const auto word = context.Commands().Create();
					context.Commands().Add<WordOwner>(word, WordOwner{prepared.entity});
					context.Commands().Add<WordOrdinal>(word, WordOrdinal{ordinal});
					context.Commands().Add<WordMarker>(word);
					context.Commands().Add<CompletedWord>(word);
				}
				prepared.enrolledWordRows = wordCount_;
			}
			else
			{
				if (targets[row].schemaHash != catalog_.SchemaHash() ||
					targets[row].wordCount != wordCount_)
					throw std::invalid_argument(
						"Object upgrade lifecycle target does not match the finalized catalog");
			}

			if (radarDefinition != nullptr)
			{
				if (bindings.empty())
					context.Commands().Add<generalszh::radar::RadarProviderBinding>(
						prepared.entity, generalszh::radar::RadarProviderBinding{radarDefinition->id});
				else if (bindings[row].definitionId != radarDefinition->id)
					throw std::logic_error("Object upgrade lifecycle radar provider binding is stale");
				if (grants.empty())
					context.Commands().Add<generalszh::radar::RadarProviderGrant>(prepared.entity);
				if (contributions.empty())
					context.Commands().Add<engine::gameplay::rts::radar::RadarProviderContribution>(
						prepared.entity);
			}

			if (!ready)
			{
				prepared.nextStage = ObjectUpgradeLifecycleStage::AwaitingReadiness;
				continue;
			}

			// This is the actual producer/readiness decision.  The helper retains
			// the authored exemption validation; no event is silently upgraded.
			const auto input = MakeObjectUpgradeCreationInput(
				prepared.entity, lifecycles[row].definition, lifecycles[row].event);
			prepared.request = Request{
				prepared.entity, input.binding, Operation::Grant,
				GeneratedSequence(offset + row)};
			prepared.hasRequest = true;
			prepared.nextStage = ObjectUpgradeLifecycleStage::RequestPublished;
		}
	}

	// The scheduler commits this system's deferred buffers only after this hook
	// returns.  Capacity/conflict failures therefore escape as terminal failures;
	// the scheduler discards the current wave's command buffers in its catch path.
	void AfterChunks(Query &query, ecs::SystemContext &)
	{
		std::size_t newWordRows = 0;
		generatedRequests_.clear();
		for (const PreparedRow &row : rows_)
		{
			if (row.enrolledWordRows > wordRowCapacity_ -
				std::min(wordRowCapacity_, newWordRows))
				throw std::length_error("Object upgrade lifecycle word-row capacity exhausted");
			if (newWordRows > wordRowCapacity_ - row.enrolledWordRows)
				throw std::length_error("Object upgrade lifecycle word-row capacity exhausted");
			newWordRows += row.enrolledWordRows;
			if (row.hasRequest)
				generatedRequests_.push_back(row.request);
		}

		if (hostRequests_.size() > batch_.Capacity() ||
			generatedRequests_.size() > batch_.Capacity() - hostRequests_.size())
			throw std::length_error("Object upgrade lifecycle request capacity exhausted");

		std::sort(generatedRequests_.begin(), generatedRequests_.end(), RequestLess);
		mergeRequests_ = hostRequests_;
		mergeRequests_.insert(mergeRequests_.end(), generatedRequests_.begin(), generatedRequests_.end());
		std::sort(mergeRequests_.begin(), mergeRequests_.end(), RequestLess);
		for (std::size_t index = 1; index != mergeRequests_.size(); ++index)
		{
			if (!SameTargetBinding(mergeRequests_[index - 1], mergeRequests_[index]))
				continue;
			if (mergeRequests_[index - 1].operation != mergeRequests_[index].operation)
				throw std::logic_error("Object upgrade lifecycle host/generated operations conflict");
		}

		for (const Request &request : generatedRequests_)
			batch_.Append(request);

		std::size_t chunkIndex = 0;
		query.ForEachPreparedChunk([&](Query::Chunk chunk) {
			if (chunkIndex >= chunkOffsets_.size())
				throw std::logic_error("Object upgrade lifecycle prepared chunk count changed");
			auto lifecycles = chunk.Get<ObjectUpgradeLifecycle>();
			const std::size_t offset = chunkOffsets_[chunkIndex++];
			for (std::size_t row = 0; row != chunk.Count(); ++row)
				lifecycles[row].stage = rows_[offset + row].nextStage;
		});
	}

private:
	struct PreparedRow final
	{
		ecs::Entity entity{};
		Request request{};
		ObjectUpgradeLifecycleStage nextStage{
			ObjectUpgradeLifecycleStage::Enrolling};
		std::uint32_t enrolledWordRows{};
		bool hasRequest{};
	};

	static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return std::tie(left.index, left.generation) <
			std::tie(right.index, right.generation);
	}

	static bool SameTargetBinding(const Request &left, const Request &right) noexcept
	{
		return left.target == right.target && left.binding == right.binding;
	}

	static bool RequestLess(const Request &left, const Request &right) noexcept
	{
		if (EntityLess(left.target, right.target)) return true;
		if (EntityLess(right.target, left.target)) return false;
		if (left.binding.definitionId != right.binding.definitionId)
			return left.binding.definitionId < right.binding.definitionId;
		if (left.binding.wordOrdinal != right.binding.wordOrdinal)
			return left.binding.wordOrdinal < right.binding.wordOrdinal;
		if (left.binding.bitMask != right.binding.bitMask)
			return left.binding.bitMask < right.binding.bitMask;
		if (left.binding.schemaHash != right.binding.schemaHash)
			return left.binding.schemaHash < right.binding.schemaHash;
		if (left.operation != right.operation)
			return static_cast<std::uint8_t>(left.operation) <
				static_cast<std::uint8_t>(right.operation);
		return left.producerSequence < right.producerSequence;
	}

	static std::uint64_t GeneratedSequence(const std::size_t slot)
	{
		if (slot > (std::numeric_limits<std::uint64_t>::max)())
			throw std::length_error("Object upgrade lifecycle producer sequence exhausted");
		return static_cast<std::uint64_t>(slot);
	}

	void ValidateDefinition(const ObjectUpgradeCreationDefinition &definition) const
	{
		engine::gameplay::rts::upgrades::object_scope::ValidateCanonicalBinding(definition.binding);
		if (definition.binding.schemaHash != catalog_.SchemaHash())
			throw std::invalid_argument("Object upgrade lifecycle definition has a stale schema");
		if (definition.binding.definitionId >= catalog_.Count() ||
			definition.binding.wordOrdinal >= wordCount_)
			throw std::out_of_range("Object upgrade lifecycle definition is outside the catalog");
	}

	const generalszh::radar::RadarProviderDefinition *FindRadarDefinition(
		const engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeBinding &binding) const
		noexcept
	{
		if (radareDefinitions_ == nullptr)
			return nullptr;
		for (std::size_t index = 0; index != radareDefinitions_->Size(); ++index)
		{
			const auto &candidate = radareDefinitions_->Get(static_cast<std::uint32_t>(index));
			const auto &upgrade = candidate.upgrade;
			if (upgrade.definitionId == binding.definitionId &&
				upgrade.wordOrdinal == binding.wordOrdinal &&
				upgrade.bitMask == binding.bitMask &&
				upgrade.schemaHash == binding.schemaHash)
				return &candidate;
		}
		return nullptr;
	}

	const generalszh::upgrades::UpgradeCatalog &catalog_;
	Batch &batch_;
	const std::size_t wordRowCapacity_;
	const generalszh::radar::RadarProviderDefinitions *radareDefinitions_;
	std::uint32_t wordCount_{};
	std::vector<std::size_t> chunkOffsets_;
	std::vector<PreparedRow> rows_;
	std::vector<Request> hostRequests_;
	std::vector<Request> generatedRequests_;
	std::vector<Request> mergeRequests_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::upgrades::object_scope::ObjectUpgradeLifecycleSystem>
{
	static constexpr std::string_view StableName =
		"games.generalszh.upgrades.object_scope.lifecycle";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
