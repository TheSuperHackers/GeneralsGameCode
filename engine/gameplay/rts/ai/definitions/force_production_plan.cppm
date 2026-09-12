module;

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

export module engine.gameplay.rts.ai.definitions.force_production_plan;

export import engine.time.simulation_time;

export namespace engine::gameplay::rts::ai
{

struct ForceProductionEntry
{
	std::uint32_t definitionKey{};
	std::uint32_t minimum{};
	std::uint32_t maximum{};
};

struct ForceProductionCandidateInput
{
	std::uint32_t stableKey{};
	std::int32_t productionPriority{};
	engine::time::Duration retryCooldown{};
	std::span<const ForceProductionEntry> entries{};
};

struct ForceProductionPolicyInput
{
	std::uint32_t stableKey{};
	std::uint32_t version{1};
	engine::time::Duration fallbackCooldown{};
	std::span<const ForceProductionCandidateInput> candidates{};
};

struct ForceProductionPlanLimits
{
	std::size_t maxPolicies{};
	std::size_t maxCandidates{};
	std::size_t maxEntries{};
};

// Immutable flat startup catalog.  Entries and candidates are copied into
// contiguous storage and referenced by validated ranges; no generic capacity
// is derived from a particular game's content format.
class ForceProductionPlanCatalog final
{
public:
	struct CandidateRange
	{
		std::uint32_t stableKey{};
		std::int32_t productionPriority{};
		std::uint64_t retryCooldownTicks{};
		std::size_t entryOffset{};
		std::size_t entryCount{};
	};

	struct PolicyRange
	{
		std::uint32_t stableKey{};
		std::uint32_t version{};
		std::uint64_t fallbackCooldownTicks{};
		std::size_t candidateOffset{};
		std::size_t candidateCount{};
	};

	ForceProductionPlanCatalog(std::span<const ForceProductionPolicyInput> source,
		engine::time::FixedStep step, ForceProductionPlanLimits limits) :
		step(step), limits(limits)
	{
		if (limits.maxPolicies == 0 || limits.maxCandidates == 0 || limits.maxEntries == 0)
			throw std::invalid_argument("Force production plan limits must be positive");
		if (source.size() > limits.maxPolicies)
			throw std::length_error("Force production policy capacity exhausted");

		policies.reserve(source.size());
		candidates.reserve(limits.maxCandidates);
		entries.reserve(limits.maxEntries);

		for (const auto &policy : source)
		{
			if (policy.version == 0)
				throw std::invalid_argument("Force production policy version must be nonzero");
			if (policy.candidates.empty())
				throw std::invalid_argument("Force production policy requires a candidate");
			const auto fallbackTicks = PositiveTicks(policy.fallbackCooldown,
				"Force production fallback cooldown must be positive");

			if (std::any_of(policies.begin(), policies.end(), [&](const auto &existing) {
				return existing.stableKey == policy.stableKey;
			}))
				throw std::invalid_argument("Duplicate force production policy key");

			std::vector<NormalizedCandidate> normalized;
			normalized.reserve(policy.candidates.size());
			for (const auto &candidate : policy.candidates)
			{
				if (candidate.entries.empty())
					throw std::invalid_argument("Force production candidate requires an entry");
				if (std::any_of(normalized.begin(), normalized.end(), [&](const auto &existing) {
					return existing.stableKey == candidate.stableKey;
				}))
					throw std::invalid_argument("Duplicate force production candidate key");
				const auto cooldownTicks = PositiveTicks(candidate.retryCooldown,
					"Force production retry cooldown must be positive");

				std::vector<ForceProductionEntry> candidateEntries(candidate.entries.begin(), candidate.entries.end());
				std::sort(candidateEntries.begin(), candidateEntries.end(), [](const auto &left, const auto &right) {
					return left.definitionKey < right.definitionKey;
				});
				for (std::size_t index = 0; index != candidateEntries.size(); ++index)
				{
					const auto &entry = candidateEntries[index];
					if (entry.maximum == 0 || entry.minimum > entry.maximum)
						throw std::invalid_argument("Invalid force production minimum/maximum");
					if (index != 0 && candidateEntries[index - 1].definitionKey == entry.definitionKey)
						throw std::invalid_argument("Duplicate force production definition key");
				}

				normalized.push_back({candidate.stableKey, candidate.productionPriority,
					cooldownTicks, std::move(candidateEntries)});
			}

			std::sort(normalized.begin(), normalized.end(), [](const auto &left, const auto &right) {
				if (left.productionPriority != right.productionPriority)
					return left.productionPriority > right.productionPriority;
				return left.stableKey < right.stableKey;
			});

			if (normalized.size() > limits.maxCandidates - candidates.size())
				throw std::length_error("Force production candidate capacity exhausted");
			const auto candidateOffset = candidates.size();
			for (const auto &candidate : normalized)
			{
				if (candidate.entries.size() > limits.maxEntries - entries.size())
					throw std::length_error("Force production entry capacity exhausted");
				const auto entryOffset = entries.size();
				entries.insert(entries.end(), candidate.entries.begin(), candidate.entries.end());
				candidates.push_back({candidate.stableKey, candidate.productionPriority,
					candidate.retryCooldownTicks, entryOffset, candidate.entries.size()});
			}
			policies.push_back({policy.stableKey, policy.version, fallbackTicks,
				candidateOffset, normalized.size()});
		}

		std::sort(policies.begin(), policies.end(), [](const auto &left, const auto &right) {
			return left.stableKey < right.stableKey;
		});
	}

	const PolicyRange *FindPolicy(const std::uint32_t stableKey) const noexcept
	{
		const auto found = std::lower_bound(policies.begin(), policies.end(), stableKey,
			[](const PolicyRange &value, const std::uint32_t key) { return value.stableKey < key; });
		return found == policies.end() || found->stableKey != stableKey ? nullptr : &*found;
	}

	std::span<const CandidateRange> Candidates(const PolicyRange &policy) const noexcept
	{
		return {candidates.data() + policy.candidateOffset, policy.candidateCount};
	}

	std::span<const ForceProductionEntry> Entries(const CandidateRange &candidate) const noexcept
	{
		return {entries.data() + candidate.entryOffset, candidate.entryCount};
	}

	const std::vector<PolicyRange> &Policies() const noexcept { return policies; }
	const engine::time::FixedStep &Step() const noexcept { return step; }

private:
	struct NormalizedCandidate
	{
		std::uint32_t stableKey{};
		std::int32_t productionPriority{};
		std::uint64_t retryCooldownTicks{};
		std::vector<ForceProductionEntry> entries;
	};

	static std::uint64_t PositiveTicks(const engine::time::Duration duration, const char *message,
		const engine::time::FixedStep &step)
	{
		if (duration <= engine::time::Duration::zero())
			throw std::invalid_argument(message);
		const auto ticks = step.TicksFor(duration);
		if (ticks == 0)
			throw std::invalid_argument(message);
		return ticks;
	}

	std::uint64_t PositiveTicks(const engine::time::Duration duration, const char *message) const
	{
		return PositiveTicks(duration, message, step);
	}

	engine::time::FixedStep step;
	ForceProductionPlanLimits limits;
	std::vector<PolicyRange> policies;
	std::vector<CandidateRange> candidates;
	std::vector<ForceProductionEntry> entries;
};

} // namespace engine::gameplay::rts::ai
