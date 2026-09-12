module;

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <stdexcept>

export module engine.gameplay.rts.visibility.algorithms.visibility_page_capacity;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;

export namespace engine::gameplay::rts::visibility
{
inline std::size_t CheckedAdd(const std::size_t left, const std::size_t right)
{
	if (right > (std::numeric_limits<std::size_t>::max)() - left)
		throw std::overflow_error("Visibility capacity addition overflow");
	return left + right;
}

inline std::size_t CheckedMultiply(const std::size_t left, const std::size_t right)
{
	if (left != 0 && right > (std::numeric_limits<std::size_t>::max)() / left)
		throw std::overflow_error("Visibility capacity multiplication overflow");
	return left * right;
}

inline std::size_t CeilingDivide(const std::size_t value, const std::size_t divisor)
{
	if (!divisor) throw std::invalid_argument("Visibility capacity divisor must be positive");
	return value == 0 ? 0 : 1 + (value - 1) / divisor;
}

struct VisibilityPageCapacity final
{
	std::size_t maximumEntityIndexCapacity{};
	std::size_t maximumGameplayEntities{};
	std::size_t reservedEntityCapacity{};
	std::size_t cellCount{};
	std::size_t maximumObserverEntities{};
	std::size_t maximumRegionTransitionsPerTick{1};
};

struct VisibilityPageBudget final
{
	std::uint64_t graceTicks{};
	std::size_t leaseRecordCapacity{};
	std::size_t cellPageCount{};
	std::size_t historyPageCount{};
	std::size_t leasePageCount{};
	std::size_t visibilityPageEntityCount{};
	std::size_t requiredEntityIndexCapacity{};
};

struct VisibilitySessionBudget final
{
	VisibilityPageCapacity capacity{};
	VisibilityPageBudget pages{};
};

inline VisibilityPageBudget CalculateVisibilityPageBudget(const VisibilityPageCapacity &capacity,
	const std::uint64_t graceTicks)
{
	if (!capacity.cellCount || !capacity.maximumObserverEntities || !capacity.maximumRegionTransitionsPerTick)
		throw std::invalid_argument("Visibility page capacities must be positive");
	if (capacity.maximumGameplayEntities > capacity.maximumEntityIndexCapacity ||
		capacity.reservedEntityCapacity > capacity.maximumEntityIndexCapacity)
		throw std::length_error("Visibility entity capacities exceed the root index budget");

	if (graceTicks == (std::numeric_limits<std::uint64_t>::max)())
		throw std::overflow_error("Visibility grace tick count cannot be incremented");
	const auto graceWindow64 = graceTicks + 1;
	if (graceWindow64 > (std::numeric_limits<std::size_t>::max)())
		throw std::overflow_error("Visibility grace tick count exceeds size capacity");
	const auto graceWindow = static_cast<std::size_t>(graceWindow64);
	const auto recordsPerObserver = CheckedMultiply(capacity.maximumRegionTransitionsPerTick, graceWindow);
	const auto leaseRecords = CheckedMultiply(capacity.maximumObserverEntities, recordsPerObserver);
	const auto cellPages = CeilingDivide(capacity.cellCount, VisibilityPageLanes);
	const auto historyPages = CeilingDivide(capacity.maximumObserverEntities, VisibilityPageLanes);
	const auto leasePages = CeilingDivide(leaseRecords, VisibilityPageLanes);
	const auto pageEntities = CheckedAdd(CheckedAdd(cellPages, historyPages), leasePages);
	const auto required = CheckedAdd(CheckedAdd(capacity.maximumGameplayEntities, capacity.reservedEntityCapacity), pageEntities);
	if (required > capacity.maximumEntityIndexCapacity)
		throw std::length_error("Visibility pages exceed the total maximum entity-index budget");

	return VisibilityPageBudget{graceTicks, leaseRecords, cellPages, historyPages, leasePages,
		pageEntities, required};
}

// Fits the requested live-observer ceiling and the fixed page entities into
// one explicit entity-index budget.  maximumObserverEntities is a ceiling for
// a subset of gameplay entities, not a second entity-index reservation.  The
// returned maximumGameplayEntities is the remaining actor/entity budget after
// page and root-reserved slots; all arithmetic is checked and a zero result is
// fatal at startup.
inline VisibilitySessionBudget FitVisibilitySessionBudget(const std::size_t maximumEntityIndexCapacity,
	const std::size_t cellCount, const std::size_t requestedObserverCapacity,
	const std::size_t reservedEntityCapacity, const std::uint64_t graceTicks,
	const std::size_t maximumRegionTransitionsPerTick = 1)
{
	if (!maximumEntityIndexCapacity || !cellCount || !requestedObserverCapacity ||
		reservedEntityCapacity >= maximumEntityIndexCapacity)
		throw std::invalid_argument("Visibility session budget inputs must be positive and bounded");
	const auto available = maximumEntityIndexCapacity - reservedEntityCapacity;
	std::size_t low = 0;
	std::size_t high = (std::min)(requestedObserverCapacity, available);
	while (low < high)
	{
		const auto candidate = low + (high - low + 1) / 2;
		const VisibilityPageCapacity probe{
			maximumEntityIndexCapacity, 0, reservedEntityCapacity,
			cellCount, candidate, maximumRegionTransitionsPerTick};
		const auto pages = CalculateVisibilityPageBudget(probe, graceTicks);
		if (pages.visibilityPageEntityCount <= available) low = candidate;
		else high = candidate - 1;
	}
	if (!low) throw std::length_error("Visibility page pool leaves no observer capacity in the entity-index budget");
	const VisibilityPageCapacity pageProbe{
		maximumEntityIndexCapacity, 0, reservedEntityCapacity,
		cellCount, low, maximumRegionTransitionsPerTick};
	const auto pages = CalculateVisibilityPageBudget(pageProbe, graceTicks);
	if (pages.visibilityPageEntityCount > available)
		throw std::length_error("Visibility page pool exceeds the entity-index budget after observer fit");
	const auto maximumGameplayEntities = available - pages.visibilityPageEntityCount;
	if (!maximumGameplayEntities)
		throw std::length_error("Visibility page pool leaves no gameplay entity capacity");
	const VisibilityPageCapacity capacity{
		maximumEntityIndexCapacity,
		maximumGameplayEntities,
		reservedEntityCapacity,
		cellCount,
		low,
		maximumRegionTransitionsPerTick};
	return VisibilitySessionBudget{capacity, CalculateVisibilityPageBudget(capacity, graceTicks)};
}
} // namespace engine::gameplay::rts::visibility
