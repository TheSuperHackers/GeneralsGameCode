module;

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <tuple>

export module engine.gameplay.rts.ai.algorithms.select_force_production;

export import engine.ecs.core.entity;
export import engine.gameplay.rts.ai.definitions.force_production_plan;

export namespace engine::gameplay::rts::ai
{

struct ForceProductionProducerSnapshot
{
	ecs::Entity account{};
	ecs::Entity producer{};
	std::uint32_t queueCount{};
	std::uint32_t queueLimit{};
	bool active{};
	bool alive{};
	bool hasPosition{};
};

struct ForceProductionQuantitySnapshot
{
	ecs::Entity account{};
	ecs::Entity producer{};
	std::uint32_t definitionKey{};
	std::uint64_t liveQuantity{};
	std::uint64_t queuedQuantity{};
};

struct ForceProductionDefinitionSnapshot
{
	ecs::Entity account{};
	std::uint32_t definitionKey{};
	std::uint32_t cost{};
	// One admission request creates this many authored units.  Selection only
	// returns a request when the whole normal recipe fits the remaining maximum.
	std::uint32_t recipeQuantity{};
	bool prerequisitesMet{};
};

// Every range is explicitly owner-keyed.  The system supplies sorted,
// generation-safe records; the pure selector never consults ECS or legacy
// object state.
struct ForceProductionAccountSnapshot
{
	ecs::Entity account{};
	std::uint64_t balance{};
	std::span<const ForceProductionProducerSnapshot> producers{};
	std::span<const ForceProductionQuantitySnapshot> quantities{};
	std::span<const ForceProductionDefinitionSnapshot> definitions{};
};

struct ForceProductionDecision
{
	ecs::Entity account{};
	ecs::Entity producer{};
	std::uint32_t candidateKey{};
	std::uint32_t definitionKey{};
	std::uint64_t retryCooldownTicks{};
};

inline bool ForceProductionEntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}

inline bool ForceProductionEntityEqual(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return left.index == right.index && left.generation == right.generation;
}

namespace detail
{

inline const ForceProductionDefinitionSnapshot *FindDefinition(
	const ForceProductionAccountSnapshot &snapshot, const std::uint32_t key) noexcept
{
	const auto found = std::lower_bound(snapshot.definitions.begin(), snapshot.definitions.end(), key,
		[](const auto &value, const std::uint32_t target) { return value.definitionKey < target; });
	if (found == snapshot.definitions.end() || found->definitionKey != key ||
		!ForceProductionEntityEqual(found->account, snapshot.account))
		return nullptr;
	return &*found;
}

inline std::uint64_t QuantityFor(const ForceProductionAccountSnapshot &snapshot,
	const std::uint32_t key) noexcept
{
	std::uint64_t total{};
	const auto add = [&total](const std::uint64_t value) noexcept
	{
		if (value > (std::numeric_limits<std::uint64_t>::max)() - total)
			total = (std::numeric_limits<std::uint64_t>::max)();
		else
			total += value;
	};
	for (const auto &quantity : snapshot.quantities)
	{
		if (!ForceProductionEntityEqual(quantity.account, snapshot.account) || quantity.definitionKey != key)
			continue;
		add(quantity.liveQuantity);
		add(quantity.queuedQuantity);
	}
	return total;
}

inline const ForceProductionProducerSnapshot *FindProducer(
	const ForceProductionAccountSnapshot &snapshot, const std::uint32_t definitionKey,
	const ForceProductionDefinitionSnapshot &definition) noexcept
{
	const ForceProductionProducerSnapshot *best = nullptr;
	for (const auto &producer : snapshot.producers)
	{
		if (!ForceProductionEntityEqual(producer.account, snapshot.account) ||
			!producer.active || !producer.alive || !producer.hasPosition ||
			producer.queueCount >= producer.queueLimit)
			continue;
		if (!ForceProductionEntityEqual(definition.account, snapshot.account) ||
			definition.definitionKey != definitionKey || !definition.prerequisitesMet ||
			definition.cost > snapshot.balance)
			continue;
		if (best == nullptr || ForceProductionEntityLess(producer.producer, best->producer))
			best = &producer;
	}
	return best;
}

inline std::optional<ForceProductionDecision> SelectPhase(
	const ForceProductionPlanCatalog &catalog,
	const ForceProductionPlanCatalog::PolicyRange &policy,
	const ForceProductionAccountSnapshot &snapshot,
	const bool minimumPhase)
{
	for (const auto &candidate : catalog.Candidates(policy))
	{
		for (const auto &entry : catalog.Entries(candidate))
		{
			const auto quantity = QuantityFor(snapshot, entry.definitionKey);
			const auto *definition = FindDefinition(snapshot, entry.definitionKey);
			if (definition == nullptr || definition->recipeQuantity == 0)
				continue;
			// Admission creates the authored recipe quantity as one order.  Do
			// not enqueue an order that would overshoot the authored maximum.
			if (quantity > entry.maximum ||
				static_cast<std::uint64_t>(definition->recipeQuantity) >
					static_cast<std::uint64_t>(entry.maximum) - quantity)
				continue;
			const bool deficit = minimumPhase ? quantity < entry.minimum : quantity < entry.maximum;
			if (!deficit)
				continue;
			const auto *producer = FindProducer(snapshot, entry.definitionKey, *definition);
			if (producer == nullptr)
				continue;
			return ForceProductionDecision{snapshot.account, producer->producer,
				candidate.stableKey, entry.definitionKey, candidate.retryCooldownTicks};
		}
	}
	return std::nullopt;
}

} // namespace detail

// Minimum deficits have global precedence over optional fill.  Candidate
// priority is then applied in each phase, with stable candidate and definition
// keys replacing the reference game's random highest-priority tie choice.
inline std::optional<ForceProductionDecision> SelectForceProduction(
	const ForceProductionPlanCatalog &catalog, const std::uint32_t policyKey,
	const ForceProductionAccountSnapshot &snapshot)
{
	const auto *policy = catalog.FindPolicy(policyKey);
	if (policy == nullptr || !snapshot.account.IsValid())
		return std::nullopt;
	if (const auto required = detail::SelectPhase(catalog, *policy, snapshot, true))
		return required;
	return detail::SelectPhase(catalog, *policy, snapshot, false);
}

} // namespace engine::gameplay::rts::ai
