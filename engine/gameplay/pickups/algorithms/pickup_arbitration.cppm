module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>

export module engine.gameplay.pickups.algorithms.pickup_arbitration;
export import engine.ecs.core.entity;

export namespace engine::gameplay::pickups
{
// The generic layer knows only identity and the authored multi-pickup policy.
// Game-specific reward eligibility is supplied as a pure success policy by the
// owning gameplay system.  A failed policy call never consumes a crate.
struct PickupCandidate final
{
	ecs::Entity crate{};
	ecs::Entity collector{};
	std::uint32_t sequence{};
	std::size_t ordinal{};
	bool allowMultiPickup{};
	bool eligible{};
};

struct PickupClaim final
{
	ecs::Entity crate{};
	ecs::Entity collector{};
	std::uint32_t sequence{};
	std::size_t candidateOrdinal{};
	bool allowMultiPickup{};
};

inline bool PickupEntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}

// Candidates are sorted once at the input boundary by crate, collector,
// sequence, and ordinal.  This validation makes the linear group traversal a
// contract rather than an accidental dependence on registration order.
inline void ValidateCanonicalCandidates(std::span<const PickupCandidate> candidates)
{
	for (std::size_t index = 1; index < candidates.size(); ++index)
	{
		const auto &previous = candidates[index - 1];
		const auto &current = candidates[index];
		const auto previousKey = std::tuple{previous.crate.index, previous.crate.generation,
			previous.collector.index, previous.collector.generation, previous.sequence,
			previous.ordinal};
		const auto currentKey = std::tuple{current.crate.index, current.crate.generation,
			current.collector.index, current.collector.generation, current.sequence,
			current.ordinal};
		if (currentKey < previousKey)
			throw std::logic_error("Pickup candidates are not in canonical order");
	}
}

template<typename SuccessPolicy>
std::size_t SelectSuccessAware(std::span<const PickupCandidate> candidates,
	std::span<PickupClaim> output, SuccessPolicy &&successPolicy)
{
	ValidateCanonicalCandidates(candidates);
	// The output bound is checked before invoking a policy.  A policy may
	// update its bounded projection, so capacity failure must not leave a
	// partially applied arbitration state behind.
	if (output.size() < candidates.size())
		throw std::length_error("Pickup claim capacity is smaller than candidate capacity");

	std::size_t count = 0;
	for (std::size_t first = 0; first != candidates.size();)
	{
		const auto crate = candidates[first].crate;
		bool consumed = false;
		while (first != candidates.size() && candidates[first].crate == crate)
		{
			const auto collector = candidates[first].collector;
			// Canonical order makes this an O(N) distinct-collector traversal.
			// Duplicate contacts for one collector never invoke the reward policy
			// twice, including when AllowMultiPickup is authored.
			const auto &candidate = candidates[first];
			if (!consumed && candidate.eligible &&
				static_cast<bool>(successPolicy(candidate)))
			{
				output[count++] = {candidate.crate, candidate.collector, candidate.sequence,
					candidate.ordinal, candidate.allowMultiPickup};
				if (!candidate.allowMultiPickup)
					consumed = true;
			}
			while (first != candidates.size() && candidates[first].crate == crate &&
				candidates[first].collector == collector)
				++first;
		}
	}
	return count;
}
} // namespace engine::gameplay::pickups
