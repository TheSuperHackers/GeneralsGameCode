module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

export module games.generalszh.gameplay.ai.systems.force_production_system;

export import engine.ecs.system.system;
export import engine.gameplay.rts.ai.algorithms.select_force_production;
export import engine.gameplay.rts.ai.components.force_production_controller;
export import games.generalszh.gameplay.production.admission.build_catalog;
export import games.generalszh.gameplay.production.algorithms.prerequisite_presence;
export import games.generalszh.gameplay.production.algorithms.production_prerequisite_evaluation;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import engine.gameplay.rts.production.algorithms.production_quantity;
export import games.generalszh.gameplay.production.boundary.production_operations;
export import games.generalszh.gameplay.production.inputs.build_inputs;
export import games.generalszh.gameplay.production.systems.production_admission_system;
export import games.generalszh.gameplay.economy.systems.account_system;
export import games.generalszh.gameplay.selling.systems.sell_system;
export import games.generalszh.gameplay.match.systems.defeat_system;
export import games.generalszh.gameplay.capture.systems.capture_system;

export namespace generalszh::ai
{

namespace detail
{

using Controller = engine::gameplay::rts::ai::ForceProductionController;
using Producer = generalszh::production::Producer;
using Queue = generalszh::production::Queue;
using Member = generalszh::production::Member;
using Life = generalszh::production::Life;
using Position = generalszh::production::Position;
using BuildOrder = generalszh::production::BuildOrder;
using Quantity = generalszh::production::Quantity;
using ProducedUnit = generalszh::production::ProducedUnit;
using Balance = generalszh::production::Balance;
using UnlockState = engine::gameplay::rts::unlocks::UnlockState;

using Producers = ecs::Query<ecs::Read<Producer>, ecs::Read<Queue>,
	ecs::Read<Life>, ecs::Read<Position>>;
using Orders = ecs::Query<ecs::Read<BuildOrder>, ecs::Read<Quantity>, ecs::Read<Member>>;

using ProducerRow = engine::gameplay::rts::ai::ForceProductionProducerSnapshot;
using RawQuantityRow = engine::gameplay::rts::ai::ForceProductionQuantitySnapshot;
using DefinitionRow = engine::gameplay::rts::ai::ForceProductionDefinitionSnapshot;

struct DefinitionNeed
{
	ecs::Entity account{};
	std::uint32_t definitionKey{};
};

struct AccountRange
{
	ecs::Entity account{};
	std::size_t producerOffset{};
	std::size_t producerCount{};
	std::size_t quantityOffset{};
	std::size_t quantityCount{};
	std::size_t definitionOffset{};
	std::size_t definitionCount{};
};

struct DecisionSlot
{
	bool valid{};
	ecs::Entity account{};
	ecs::Entity producer{};
	std::uint32_t definitionKey{};
};

struct UnlockRow
{
	ecs::Entity account{};
	UnlockState state{};
};

inline bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}

inline bool EntityEqual(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return left.index == right.index && left.generation == right.generation;
}

template<typename Row>
inline bool AccountRowLess(const Row &left, const Row &right) noexcept
{
	return EntityLess(left.account, right.account);
}

template<typename Row>
inline std::pair<std::size_t, std::size_t> AccountSlice(
	const std::vector<Row> &rows, const ecs::Entity account)
{
	const auto first = std::lower_bound(rows.begin(), rows.end(), account,
		[](const Row &row, const ecs::Entity value) { return EntityLess(row.account, value); });
	const auto last = std::upper_bound(first, rows.end(), account,
		[](const ecs::Entity value, const Row &row) { return EntityLess(value, row.account); });
	return {static_cast<std::size_t>(first - rows.begin()), static_cast<std::size_t>(last - first)};
}

template<typename Row>
inline std::span<const Row> Slice(const std::vector<Row> &rows,
	const std::size_t offset, const std::size_t count) noexcept
{
	return count == 0 ? std::span<const Row>{} : std::span<const Row>(rows.data() + offset, count);
}

inline bool ProducerKeyLess(const ProducerRow &left, const ProducerRow &right) noexcept
{
	if (left.account != right.account) return EntityLess(left.account, right.account);
	return EntityLess(left.producer, right.producer);
}

inline bool QuantityKeyLess(const RawQuantityRow &left, const RawQuantityRow &right) noexcept
{
	if (left.account != right.account) return EntityLess(left.account, right.account);
	if (left.definitionKey != right.definitionKey) return left.definitionKey < right.definitionKey;
	return EntityLess(left.producer, right.producer);
}

inline bool SameQuantityKey(const RawQuantityRow &left, const RawQuantityRow &right) noexcept
{
	return EntityEqual(left.account, right.account) &&
		EntityEqual(left.producer, right.producer) &&
		left.definitionKey == right.definitionKey;
}

inline std::uint64_t SaturatingAdd(const std::uint64_t left, const std::uint64_t right) noexcept
{
	return right > (std::numeric_limits<std::uint64_t>::max)() - left
		? (std::numeric_limits<std::uint64_t>::max)() : left + right;
}

inline bool DefinitionKeyLess(const DefinitionNeed &left, const DefinitionNeed &right) noexcept
{
	if (left.account != right.account) return EntityLess(left.account, right.account);
	return left.definitionKey < right.definitionKey;
}

inline bool SameDefinitionKey(const DefinitionNeed &left, const DefinitionNeed &right) noexcept
{
	return EntityEqual(left.account, right.account) && left.definitionKey == right.definitionKey;
}

} // namespace detail

class ForceProductionSystem final
{
public:
	using Query = ecs::Query<
		ecs::Write<engine::gameplay::rts::ai::ForceProductionController>,
		ecs::Read<generalszh::production::Balance>>;

	using AuxiliaryAccess = ecs::Query<
		ecs::Read<generalszh::production::Producer>,
		ecs::Read<generalszh::production::Queue>,
		ecs::Read<generalszh::production::Member>,
		ecs::Read<generalszh::production::Life>,
		ecs::Read<generalszh::production::Position>,
		ecs::Read<generalszh::production::BuildOrder>,
		ecs::Read<generalszh::production::Quantity>,
		ecs::Read<generalszh::production::ProducedUnit>,
		ecs::Read<generalszh::construction::Structure>,
		ecs::Read<detail::UnlockState>>;
	using Units = ecs::Query<ecs::Read<detail::ProducedUnit>, ecs::Read<detail::Life>>;

	ForceProductionSystem(ecs::World &world,
		const engine::gameplay::rts::ai::ForceProductionPlanCatalog &plans,
		const generalszh::production::BuildCatalog &catalog,
		generalszh::BuildBatch &batch,
		const std::size_t controllerCapacity,
		const std::size_t observationCapacity) :
		world(world), plans(plans), catalog(catalog), batch(batch),
		producers(world), orders(world), units(world), unlockStates(world),
		presence(world, observationCapacity),
		controllerCapacity(controllerCapacity), observationCapacity(observationCapacity)
	{
		if (controllerCapacity == 0 || observationCapacity == 0)
			throw std::invalid_argument("Force production capacities must be positive");
		if (!(plans.Step() == catalog.Step()))
			throw std::invalid_argument("Force production plan and build catalog steps differ");

		rowBases.reserve(controllerCapacity);
		slots.reserve(controllerCapacity);
		controllerAccounts.reserve(controllerCapacity);
		accountRanges.reserve(controllerCapacity);
		producerRows.reserve(observationCapacity);
		rawQuantityRows.reserve(observationCapacity);
		quantityRows.reserve(observationCapacity);
		definitionNeeds.reserve(observationCapacity);
		definitionRows.reserve(observationCapacity);
		unlockRows.reserve(controllerCapacity);
		decisions.reserve(controllerCapacity);
		generatedInputs.reserve(batch.Capacity());
		ValidatePlanDefinitions();
	}

	// The composition/content boundary calls this after controller entities and
	// authored policy keys exist, before scheduler execution is finalized.  The
	// worker path can therefore treat a missing policy as an invariant failure,
	// while malformed authored configuration remains an explicit startup error.
	void ValidateConfiguredControllers() const
	{
		if (!world.ComponentsFinalized() || world.IsScheduledExecutionActive())
			throw std::logic_error("Force production controller validation requires a startup boundary");
		ecs::Query<ecs::Read<engine::gameplay::rts::ai::ForceProductionController>> controllers(world);
		controllers.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<engine::gameplay::rts::ai::ForceProductionController>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto account = chunk.Entities()[row];
				if (!world.IsAlive(account))
					throw std::logic_error("Force production controller owner is not alive");
				if (world.Get<generalszh::production::Balance>(account) == nullptr)
					throw std::invalid_argument("Force production controller requires an account balance");
				if (catalog.HasSciencePrerequisites() && world.Get<detail::UnlockState>(account) == nullptr)
					throw std::invalid_argument("Force production controller requires account unlock state");
				if (plans.FindPolicy(values[row].policyKey) == nullptr)
					throw std::invalid_argument("Force production controller references unknown policy");
			}
		});
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		assert(context.Time().Step() == catalog.Step());

		rowBases.clear();
		std::size_t rowCount = 0;
		query.ForEachPreparedChunk([&](auto chunk)
		{
			if (chunk.Count() > controllerCapacity - rowCount)
				throw std::length_error("Force production controller capacity exhausted");
			rowBases.push_back(rowCount);
			rowCount += chunk.Count();
		});
		slots.assign(rowCount, detail::DecisionSlot{});

		controllerAccounts.clear();
		definitionNeeds.clear();
		query.ForEachPreparedChunk([&](auto chunk)
		{
			const auto controllers = chunk.template Get<engine::gameplay::rts::ai::ForceProductionController>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto account = chunk.Entities()[row];
				assert(world.IsAlive(account));
				const auto *policy = plans.FindPolicy(controllers[row].policyKey);
				assert(policy != nullptr);
				controllerAccounts.push_back(account);
				for (const auto &candidate : plans.Candidates(*policy))
					for (const auto &entry : plans.Entries(candidate))
						AppendBounded(definitionNeeds, detail::DefinitionNeed{account, entry.definitionKey}, observationCapacity,
							"Force production definition observation capacity exhausted");
			}
		});

		std::sort(controllerAccounts.begin(), controllerAccounts.end(), detail::EntityLess);
		controllerAccounts.erase(std::unique(controllerAccounts.begin(), controllerAccounts.end(), detail::EntityEqual), controllerAccounts.end());
		std::sort(definitionNeeds.begin(), definitionNeeds.end(), detail::DefinitionKeyLess);
		definitionNeeds.erase(std::unique(definitionNeeds.begin(), definitionNeeds.end(), detail::SameDefinitionKey), definitionNeeds.end());

		CollectUnlockStates();
		if (catalog.HasPrerequisites())
			presence.Rebuild();
		CollectProducers();
		CollectQuantities();
		CollectDefinitions();
		BuildAccountRanges();
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &context)
	{
		assert(context.ChunkOrder() < rowBases.size());
		const auto base = rowBases[context.ChunkOrder()];
		const auto controllers = chunk.template Get<engine::gameplay::rts::ai::ForceProductionController>();
		const auto balances = chunk.template Get<generalszh::production::Balance>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto slotIndex = base + row;
			assert(slotIndex < slots.size());
			auto &slot = slots[slotIndex];
			slot = {};
			auto &controller = controllers[row];
			if (!controller.enabled || context.Tick() < controller.nextDecisionTick)
				continue;

			const auto account = chunk.Entities()[row];
			const auto *range = FindAccountRange(account);
			assert(range != nullptr);
			const engine::gameplay::rts::ai::ForceProductionAccountSnapshot snapshot{
				account, balances[row].quantity,
				detail::Slice(producerRows, range->producerOffset, range->producerCount),
				detail::Slice(quantityRows, range->quantityOffset, range->quantityCount),
				detail::Slice(definitionRows, range->definitionOffset, range->definitionCount)};
			const auto choice = engine::gameplay::rts::ai::SelectForceProduction(plans, controller.policyKey, snapshot);
			const auto *policy = plans.FindPolicy(controller.policyKey);
			assert(policy != nullptr);
			const auto cooldownTicks = choice ? choice->retryCooldownTicks : policy->fallbackCooldownTicks;
			controller.nextDecisionTick = NextDeadline(context.Tick(), cooldownTicks);
			if (!choice)
				continue;
			slot.valid = true;
			slot.account = choice->account;
			slot.producer = choice->producer;
			slot.definitionKey = choice->definitionKey;
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &context)
	{
		decisions.clear();
		for (const auto &slot : slots)
			if (slot.valid)
				AppendBounded(decisions, slot, controllerCapacity,
					"Force production decision capacity exhausted");
		std::sort(decisions.begin(), decisions.end(), [](const auto &left, const auto &right)
		{
			if (left.account != right.account) return detail::EntityLess(left.account, right.account);
			if (left.producer != right.producer) return detail::EntityLess(left.producer, right.producer);
			return left.definitionKey < right.definitionKey;
		});

		generatedInputs.clear();
		for (const auto &decision : decisions)
		{
			if (generatedInputs.size() == batch.Capacity())
				throw std::length_error("Force production generated input capacity exhausted");
			generatedInputs.push_back({decision.producer, decision.definitionKey});
		}
		if (!generatedInputs.empty())
			batch.AppendGenerated(context.GetWorld(), generatedInputs);
	}

private:
	template<typename T>
	static void AppendBounded(std::vector<T> &values, T value, const std::size_t capacity, const char *message)
	{
		if (values.size() == capacity)
			throw std::length_error(message);
		values.push_back(std::move(value));
	}

	static std::uint64_t NextDeadline(const std::uint64_t now, const std::uint64_t cooldownTicks)
	{
		assert(cooldownTicks != 0);
		if (cooldownTicks > (std::numeric_limits<std::uint64_t>::max)() - now)
			throw std::overflow_error("Force production decision deadline overflow");
		return now + cooldownTicks;
	}

	void ValidatePlanDefinitions() const
	{
		for (const auto &policy : plans.Policies())
			for (const auto &candidate : plans.Candidates(policy))
				for (const auto &entry : plans.Entries(candidate))
				{
					const auto index = catalog.DefinitionIndex(entry.definitionKey);
					if (index == catalog.Definitions().size() || catalog.Definitions()[index].key != entry.definitionKey ||
						catalog.Definitions()[index].kind != generalszh::production::EntryKind::Unit)
						throw std::invalid_argument("Force production plan references a non-unit or unknown definition");
				}
	}

	void CollectProducers()
	{
		producerRows.clear();
		producers.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<detail::Producer>();
			const auto queues = chunk.template Get<detail::Queue>();
			const auto life = chunk.template Get<detail::Life>();
			const auto positions = chunk.template Get<detail::Position>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (!world.IsAlive(values[row].account))
					continue;
				AppendBounded(producerRows, detail::ProducerRow{values[row].account, chunk.Entities()[row],
					queues[row].count, values[row].queueLimit, values[row].active, life[row].alive,
					!positions.empty()}, observationCapacity,
					"Force production producer observation capacity exhausted");
			}
		});
		std::sort(producerRows.begin(), producerRows.end(), detail::ProducerKeyLess);
	}

	const detail::ProducerRow *FindProducer(const ecs::Entity account, const ecs::Entity producer) const noexcept
	{
		const auto found = std::lower_bound(producerRows.begin(), producerRows.end(),
			std::pair{account, producer}, [](const auto &row, const auto &key)
			{
				if (row.account != key.first) return detail::EntityLess(row.account, key.first);
				return detail::EntityLess(row.producer, key.second);
			});
		if (found == producerRows.end() || !detail::EntityEqual(found->account, account) ||
			!detail::EntityEqual(found->producer, producer))
			return nullptr;
		return &*found;
	}

	void CollectQuantities()
	{
		rawQuantityRows.clear();
		units.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<detail::ProducedUnit>();
			const auto life = chunk.template Get<detail::Life>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
				if (life[row].alive && world.IsAlive(values[row].account))
					AppendBounded(rawQuantityRows, detail::RawQuantityRow{values[row].account, values[row].producer,
						values[row].definition, 1, 0}, observationCapacity,
						"Force production quantity observation capacity exhausted");
		});
		orders.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<detail::BuildOrder>();
			const auto quantities = chunk.template Get<detail::Quantity>();
			const auto members = chunk.template Get<detail::Member>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto remaining = engine::gameplay::rts::production::Remaining(quantities[row]);
				if (remaining <= 0 || !members[row].queue.IsValid() || members[row].queue != values[row].producer ||
					!world.IsAlive(values[row].account))
					continue;
				const auto *producer = FindProducer(values[row].account, values[row].producer);
				if (producer == nullptr || !producer->active || !producer->alive)
					continue;
				AppendBounded(rawQuantityRows, detail::RawQuantityRow{values[row].account, values[row].producer,
					values[row].definition, 0, static_cast<std::uint64_t>(remaining)}, observationCapacity,
					"Force production quantity observation capacity exhausted");
			}
		});

		std::sort(rawQuantityRows.begin(), rawQuantityRows.end(), detail::QuantityKeyLess);
		quantityRows.clear();
		for (const auto &row : rawQuantityRows)
		{
			if (!quantityRows.empty() && detail::SameQuantityKey(quantityRows.back(), row))
			{
				quantityRows.back().liveQuantity = detail::SaturatingAdd(
					quantityRows.back().liveQuantity, row.liveQuantity);
				quantityRows.back().queuedQuantity = detail::SaturatingAdd(
					quantityRows.back().queuedQuantity, row.queuedQuantity);
			}
			else
				AppendBounded(quantityRows, row, observationCapacity,
					"Force production quantity range capacity exhausted");
		}
	}

	void CollectUnlockStates()
	{
		unlockRows.clear();
		if (!catalog.HasSciencePrerequisites())
			return;
		unlockStates.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<detail::UnlockState>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto account = chunk.Entities()[row];
				const auto found = std::lower_bound(controllerAccounts.begin(), controllerAccounts.end(), account,
					[](const auto left, const auto right) { return detail::EntityLess(left, right); });
				if (found != controllerAccounts.end() && detail::EntityEqual(*found, account))
					AppendBounded(unlockRows, detail::UnlockRow{account, values[row]}, controllerCapacity,
						"Force production unlock-state observation capacity exhausted");
			}
		});
		std::sort(unlockRows.begin(), unlockRows.end(), detail::AccountRowLess<detail::UnlockRow>);
		for (const auto account : controllerAccounts)
			if (FindUnlockState(account) == nullptr)
				throw std::logic_error("Force production controller account has no unlock state");
	}

	const detail::UnlockRow *FindUnlockState(const ecs::Entity account) const noexcept
	{
		const auto found = std::lower_bound(unlockRows.begin(), unlockRows.end(), account,
			[](const auto &row, const ecs::Entity value) { return detail::EntityLess(row.account, value); });
		return found == unlockRows.end() || !detail::EntityEqual(found->account, account) ? nullptr : &*found;
	}

	void CollectDefinitions()
	{
		definitionRows.clear();
		for (const auto &need : definitionNeeds)
		{
			const auto index = catalog.DefinitionIndex(need.definitionKey);
			if (index == catalog.Definitions().size() || catalog.Definitions()[index].key != need.definitionKey)
				throw std::logic_error("Force production definition observation is stale");
			const auto &definition = catalog.Definitions()[index];
			const auto objectKeys = presence.Keys(need.account);
			bool prerequisitesMet = generalszh::production::SatisfiesObjectPrerequisites(
				catalog, index, objectKeys);
			if (catalog.HasSciencePrerequisites())
			{
				const auto *unlockState = FindUnlockState(need.account);
				if (unlockState == nullptr)
					throw std::logic_error("Force production definition has no account unlock state");
				prerequisitesMet = prerequisitesMet && generalszh::production::SatisfiesSciencePrerequisites(
					catalog.SciencePrerequisites(index), unlockState->state);
			}
			AppendBounded(definitionRows, detail::DefinitionRow{need.account, definition.key,
				definition.cost, static_cast<std::uint32_t>(definition.quantity), prerequisitesMet}, observationCapacity,
				"Force production definition range capacity exhausted");
		}
	}

	void BuildAccountRanges()
	{
		accountRanges.clear();
		for (const auto account : controllerAccounts)
		{
			const auto producerSlice = detail::AccountSlice(producerRows, account);
			const auto quantitySlice = detail::AccountSlice(quantityRows, account);
			const auto definitionSlice = detail::AccountSlice(definitionRows, account);
			AppendBounded(accountRanges, detail::AccountRange{account,
				producerSlice.first, producerSlice.second, quantitySlice.first, quantitySlice.second,
				definitionSlice.first, definitionSlice.second}, controllerCapacity,
				"Force production account range capacity exhausted");
		}
	}

	const detail::AccountRange *FindAccountRange(const ecs::Entity account) const noexcept
	{
		const auto found = std::lower_bound(accountRanges.begin(), accountRanges.end(), account,
			[](const auto &range, const ecs::Entity value) { return detail::EntityLess(range.account, value); });
		return found == accountRanges.end() || !detail::EntityEqual(found->account, account) ? nullptr : &*found;
	}

	ecs::World &world;
	const engine::gameplay::rts::ai::ForceProductionPlanCatalog &plans;
	const generalszh::production::BuildCatalog &catalog;
	generalszh::BuildBatch &batch;
	detail::Producers producers;
	detail::Orders orders;
	Units units;
	ecs::Query<ecs::Read<detail::UnlockState>> unlockStates;
	generalszh::production::PrerequisitePresence presence;
	const std::size_t controllerCapacity;
	const std::size_t observationCapacity;
	std::vector<std::size_t> rowBases;
	std::vector<detail::DecisionSlot> slots;
	std::vector<ecs::Entity> controllerAccounts;
	std::vector<detail::AccountRange> accountRanges;
	std::vector<detail::ProducerRow> producerRows;
	std::vector<detail::RawQuantityRow> rawQuantityRows;
	std::vector<detail::RawQuantityRow> quantityRows;
	std::vector<detail::DefinitionNeed> definitionNeeds;
	std::vector<detail::DefinitionRow> definitionRows;
	std::vector<detail::UnlockRow> unlockRows;
	std::vector<detail::DecisionSlot> decisions;
	std::vector<generalszh::BuildInput> generatedInputs;
};

// Startup-only composition contract.  The root calls this after registering
// the participating systems; it owns the instances and scheduler lifetime.
inline void RegisterForceProductionOrdering(ecs::SystemRegistry &registry)
{
	registry.OrderBefore<generalszh::economy::AccountSystem, ForceProductionSystem>();
	registry.OrderBefore<generalszh::selling::SellSystem, ForceProductionSystem>();
	registry.OrderBefore<generalszh::match::DefeatSystem, ForceProductionSystem>();
	registry.OrderBefore<generalszh::capture::CaptureSystem, ForceProductionSystem>();
	registry.OrderBefore<ForceProductionSystem, generalszh::production::ProductionAdmissionSystem>();
}

} // namespace generalszh::ai

export namespace ecs
{

template<>
struct SystemTraits<generalszh::ai::ForceProductionSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.ai.force_production";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<generalszh::production::ProductionAdmissionSystem>;
	using After = SystemTypeList<>;
};

} // namespace ecs
