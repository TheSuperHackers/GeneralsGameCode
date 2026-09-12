module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.crates.inputs.crate_pickup_batch;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.pickups.algorithms.pickup_arbitration;
export import engine.gameplay.progression.inputs.progression_batch;

export namespace generalszh::crates
{
struct CratePickupInput final
{
	ecs::Entity crate{};
	ecs::Entity collector{};
	std::uint32_t sequence{};
};

struct CrateCandidate final
{
	engine::gameplay::pickups::PickupCandidate arbitration{};
	std::uint32_t definition{};
	ecs::Entity account{};
	engine::gameplay::navigation::Cell collectorCell{engine::gameplay::navigation::InvalidCell};
	bool hasCollectorCell{};
};

struct CrateClaim final
{
	ecs::Entity crate{};
	ecs::Entity collector{};
	ecs::Entity account{};
	std::uint32_t definition{};
	std::uint32_t sequence{};
	std::size_t ordinal{};
	engine::gameplay::navigation::Cell collectorCell{engine::gameplay::navigation::InvalidCell};
	bool hasCollectorCell{};
};

// One bounded transient handoff between the independently registered claim and
// reward systems.  It owns neither crate state nor progression state.
class CratePickupBatch final
{
public:
	CratePickupBatch(const std::size_t capacity, const std::size_t progressionPrefixCapacity) :
		inputs_(capacity), claims_(capacity), prefix_(progressionPrefixCapacity) {}

	std::size_t Capacity() const noexcept { return inputs_.size(); }
	std::size_t ProgressionPrefixCapacity() const noexcept { return prefix_.size(); }

	void SetInputs(std::span<const CratePickupInput> inputs)
	{
		if (inputs.size() > inputs_.size())
			throw std::length_error("Crate pickup input capacity exhausted");
		for (std::size_t index = 0; index != inputs.size(); ++index)
		{
			if (!inputs[index].crate.IsValid() || !inputs[index].collector.IsValid())
				throw std::invalid_argument("Crate pickup input contains an invalid entity");
			inputs_[index] = inputs[index];
		}
		inputCount_ = inputs.size();
		claimsCount_ = 0;
		prefixCount_ = 0;
	}

	std::span<const CratePickupInput> Inputs() const noexcept
	{
		return {inputs_.data(), inputCount_};
	}

	void SetProgressionPrefix(std::span<const engine::gameplay::progression::AcceptedExperience> prefix)
	{
		if (prefix.size() > prefix_.size())
			throw std::length_error("Crate progression prefix capacity exhausted");
		for (std::size_t index = 0; index != prefix.size(); ++index)
			prefix_[index] = prefix[index];
		prefixCount_ = prefix.size();
	}

	std::span<const engine::gameplay::progression::AcceptedExperience> ProgressionPrefix() const noexcept
	{
		return {prefix_.data(), prefixCount_};
	}

	void SetClaims(std::span<const CrateClaim> claims)
	{
		if (claims.size() > claims_.size())
			throw std::length_error("Crate claim capacity exhausted");
		for (std::size_t index = 0; index != claims.size(); ++index)
			claims_[index] = claims[index];
		claimsCount_ = claims.size();
	}

	std::span<const CrateClaim> Claims() const noexcept
	{
		return {claims_.data(), claimsCount_};
	}

private:
	std::vector<CratePickupInput> inputs_;
	std::vector<CrateClaim> claims_;
	std::vector<engine::gameplay::progression::AcceptedExperience> prefix_;
	std::size_t inputCount_{}, claimsCount_{}, prefixCount_{};
};

inline bool CrateEntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}

inline bool CrateContactLess(const CratePickupInput &left, const CratePickupInput &right) noexcept
{
	return std::tie(left.collector.index, left.collector.generation, left.crate.index,
		left.crate.generation, left.sequence) < std::tie(right.collector.index,
		right.collector.generation, right.crate.index, right.crate.generation, right.sequence);
}
} // namespace generalszh::crates
