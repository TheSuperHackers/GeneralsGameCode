module;
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.repair.inputs.manual_repair_batches;
export import engine.ecs.core.entity;

export namespace generalszh::repair
{
struct ManualRepairInput
{
	ecs::Entity actor{};
	ecs::Entity target{};
};

inline bool ManualRepairInputLess(const ManualRepairInput &left,
	const ManualRepairInput &right) noexcept
{
	return std::tie(left.actor.index,left.actor.generation,left.target.index,left.target.generation) <
		std::tie(right.actor.index,right.actor.generation,right.target.index,right.target.generation);
}

// Input ownership ends at the joined SetInputs boundary.  This batch is
// transient and is never an ECS authority or a replay/network event.
class ManualRepairRequestBatch
{
public:
	explicit ManualRepairRequestBatch(std::size_t capacity) : capacity_(capacity)
	{
		values_.reserve(capacity);
	}

	void Set(std::span<const ManualRepairInput> values)
	{
		if (values.size()>capacity_) throw std::length_error("Manual repair input capacity exhausted");
		values_.assign(values.begin(),values.end());
		std::sort(values_.begin(),values_.end(),ManualRepairInputLess);
	}

	void Clear() noexcept { values_.clear(); }
	std::span<const ManualRepairInput> Values() const noexcept { return values_; }
	std::size_t Capacity() const noexcept { return capacity_; }

private:
	std::size_t capacity_;
	std::vector<ManualRepairInput> values_;
};
}
