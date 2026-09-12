module;

#include <algorithm>
#include <cassert>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module engine.gameplay.rts.visibility.systems.visibility_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.visibility.algorithms.visibility_region_index;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;
export import engine.gameplay.rts.visibility.components.visibility_history_page;
export import engine.gameplay.rts.visibility.components.visibility_lease_page;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export import engine.gameplay.rts.visibility.algorithms.visibility_page_capacity;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;

export namespace engine::gameplay::rts::visibility
{
class VisibilitySystem final
{
public:
	using Query = ecs::Query<ecs::Read<VisibilityCellIndex>,
		ecs::Write<VisibilityExploredMaskPage>, ecs::Write<VisibilityVisibleMaskPage>>;
	using LiveObserverQuery = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<VisibilityObserver>, ecs::Read<VisibilityEligibility>,
		ecs::Optional<engine::gameplay::navigation::GridPosition>>;
	using HistoryPageQuery = ecs::Query<ecs::Write<VisibilityHistoryPage>>;
	using LeasePageQuery = ecs::Query<ecs::Write<VisibilityLeasePage>>;
	using AuxiliaryAccess = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<VisibilityObserver>, ecs::Read<VisibilityEligibility>,
		ecs::Optional<engine::gameplay::navigation::GridPosition>,
		ecs::Write<VisibilityHistoryPage>, ecs::Write<VisibilityLeasePage>>;

	VisibilitySystem(ecs::World &world, const VisibilityDefinitions &definitions,
		const VisibilityTopology &topology, const VisibilityPageCapacity &capacity) :
		definitions_(definitions), topology_(topology), capacity_(capacity),
		budget_(CalculateVisibilityPageBudget(capacity_, definitions_.MaximumGraceTicks())),
		liveObservers_(world), historyPages_(world), leasePages_(world),
		regionIndex_(topology_, CheckedAdd(capacity_.maximumObserverEntities, budget_.leaseRecordCapacity))
	{
		live_.reserve(capacity_.maximumObserverEntities);
		historyRows_.reserve(capacity_.maximumObserverEntities);
		historyPageRefs_.reserve(budget_.historyPageCount);
		leasePageRefs_.reserve(budget_.leasePageCount);
		historyFreeSlots_.reserve(CheckedMultiply(budget_.historyPageCount, VisibilityPageLanes));
		leaseFreeSlots_.reserve(CheckedMultiply(budget_.leasePageCount, VisibilityPageLanes));
		cellRanges_.reserve(budget_.cellPageCount);
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		if (context.Time().Step() != definitions_.Step())
			throw std::logic_error("Visibility system fixed step does not match its definitions");
		if (!cellPagesValidated_) ValidateCellPages(query);
		RefreshPageReferences();
		BuildLiveSnapshot();
		ExpireLeases(context.Tick());
		CollectAvailableSlots();
		BuildHistoryRows();
		ReconcileHistory(context.Tick());
		BuildRegionIndex(context.Tick());
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		assert(cellPagesValidated_);
		const auto indexes = chunk.Get<VisibilityCellIndex>();
		auto exploredPages = chunk.Get<VisibilityExploredMaskPage>();
		auto visiblePages = chunk.Get<VisibilityVisibleMaskPage>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto count = static_cast<std::size_t>(indexes[row].count);
			assert(count <= VisibilityPageLanes);
			for (std::size_t lane = count; lane != VisibilityPageLanes; ++lane)
				visiblePages[row].values[lane] = 0;
			for (std::size_t lane = 0; lane != count; ++lane)
			{
				const auto cell64 = std::uint64_t{indexes[row].firstCell} + lane;
				assert(cell64 < topology_.Count());
				const auto cell = static_cast<engine::gameplay::navigation::Cell>(cell64);
				ObserverMask visible{};
				regionIndex_.VisitCovering(cell, [&](const ParticipantHandle owner) noexcept {
					visible |= ObserverBit(owner.slot);
				});
				visiblePages[row].values[lane] = visible;
				exploredPages[row].values[lane] |= visible;
			}
		}
	}

	const VisibilityPageBudget &Budget() const noexcept { return budget_; }

private:
	struct LiveObserver final
	{
		ecs::Entity source{};
		ParticipantHandle owner{};
		VisibilityRegion region{};
		VisibilityDefinitionId definition{};
		std::uint64_t graceTicks{};
	};

	struct HistoryPageRef final
	{
		VisibilityHistoryPage *page{};
	};

	struct LeasePageRef final
	{
		VisibilityLeasePage *page{};
	};

	struct HistoryRowRef final
	{
		VisibilityHistoryPage *page{};
		std::size_t slot{};
		ecs::Entity source{};
	};

	struct HistorySlotRef final
	{
		VisibilityHistoryPage *page{};
		std::size_t slot{};
	};

	struct LeaseSlotRef final
	{
		VisibilityLeasePage *page{};
		std::size_t slot{};
	};

	struct CellRange final
	{
		std::uint32_t firstCell{};
		std::uint16_t count{};
	};

	static bool LessEntity(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
	}

	static bool EqualEntity(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return left.index == right.index && left.generation == right.generation;
	}

	void ValidateCellPages(Query &query)
	{
		cellRanges_.clear();
		query.ForEachPreparedChunk([&](Query::Chunk chunk) {
			const auto indexes = chunk.Get<VisibilityCellIndex>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (cellRanges_.size() >= budget_.cellPageCount)
					throw std::length_error("Visibility cell page count exceeds startup capacity");
				const auto count = static_cast<std::size_t>(indexes[row].count);
				if (count > VisibilityPageLanes)
					throw std::length_error("Visibility cell page lane count exceeds startup capacity");
				const auto end = std::uint64_t{indexes[row].firstCell} + count;
				if (end > topology_.Count())
					throw std::out_of_range("Visibility cell page extends beyond its topology");
				cellRanges_.push_back(CellRange{indexes[row].firstCell, indexes[row].count});
			}
		});
		if (cellRanges_.size() != budget_.cellPageCount)
			throw std::length_error("Visibility cell page pool does not match startup capacity");
		std::sort(cellRanges_.begin(), cellRanges_.end(), [](const CellRange left, const CellRange right) {
			return left.firstCell < right.firstCell;
		});
		std::uint64_t expected = 0;
		for (const auto range : cellRanges_)
		{
			if (range.firstCell != expected)
				throw std::logic_error("Visibility cell pages do not form a contiguous topology partition");
			expected += range.count;
		}
		if (expected != topology_.Count())
			throw std::logic_error("Visibility cell pages do not cover their topology");
		cellPagesValidated_ = true;
	}

	void RefreshPageReferences()
	{
		historyPageRefs_.clear();
		historyPages_.ForEachChunk([&](auto chunk) {
			auto pages = chunk.template Get<VisibilityHistoryPage>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (historyPageRefs_.size() >= budget_.historyPageCount)
					throw std::length_error("Visibility history page references exceed startup capacity");
				historyPageRefs_.push_back({&pages[row]});
			}
		});
		leasePageRefs_.clear();
		leasePages_.ForEachChunk([&](auto chunk) {
			auto pages = chunk.template Get<VisibilityLeasePage>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (leasePageRefs_.size() >= budget_.leasePageCount)
					throw std::length_error("Visibility lease page references exceed startup capacity");
				leasePageRefs_.push_back({&pages[row]});
			}
		});

		std::sort(historyPageRefs_.begin(), historyPageRefs_.end(), [](const auto &left, const auto &right) {
			return left.page->pageIndex < right.page->pageIndex;
		});
		std::sort(leasePageRefs_.begin(), leasePageRefs_.end(), [](const auto &left, const auto &right) {
			return left.page->pageIndex < right.page->pageIndex;
		});
		if (historyPageRefs_.size() != budget_.historyPageCount || leasePageRefs_.size() != budget_.leasePageCount)
			throw std::length_error("Visibility page pool does not match startup capacity");
		for (std::size_t index = 0; index != historyPageRefs_.size(); ++index)
			if (historyPageRefs_[index].page->pageIndex != index)
				throw std::logic_error("Visibility history page indices are not dense");
		for (std::size_t index = 0; index != leasePageRefs_.size(); ++index)
			if (leasePageRefs_[index].page->pageIndex != index)
				throw std::logic_error("Visibility lease page indices are not dense");
	}

	void BuildLiveSnapshot()
	{
		live_.clear();
		liveObservers_.ForEachChunk([&](auto chunk) {
			const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
			const auto observers = chunk.template Get<VisibilityObserver>();
			const auto eligibility = chunk.template Get<VisibilityEligibility>();
			const auto positions = chunk.template Get<engine::gameplay::navigation::GridPosition>();
			const auto entities = chunk.Entities();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto &observer = observers[row];
				if (!observer.participant.IsValid())
					throw std::invalid_argument("Visibility observer has an invalid participant handle");
				const auto &definition = definitions_.Get(observer.definition);
				LiveObserver current{entities[row], observer.participant, {}, observer.definition,
					definition.unlookPersistTicks};
				if (lives[row].alive && eligibility[row].active)
				{
					const auto cell = eligibility[row].resolvedCellValid ? eligibility[row].resolvedCell :
						(positions.empty() ? engine::gameplay::navigation::InvalidCell : positions[row].cell);
					if (!topology_.Contains(cell))
						throw std::invalid_argument("Active visibility observer has no valid resolved cell");
					const auto radius = eligibility[row].radiusOverride ? eligibility[row].radiusCells :
						definition.shroudClearingRadiusCells;
					if (radius != 0) current.region = VisibilityRegion{cell, radius, true};
				}
				if (live_.size() >= capacity_.maximumObserverEntities)
					throw std::length_error("Visibility live observer snapshot exceeds startup capacity");
				live_.push_back(current);
			}
		});
		std::sort(live_.begin(), live_.end(), [](const auto &left, const auto &right) {
			return LessEntity(left.source, right.source);
		});
		for (std::size_t index = 1; index < live_.size(); ++index)
			if (EqualEntity(live_[index - 1].source, live_[index].source))
				throw std::logic_error("Visibility live observer snapshot contains a duplicate entity");
	}

	void ExpireLeases(const std::uint64_t tick) noexcept
	{
		for (const auto reference : leasePageRefs_)
			for (std::size_t slot = 0; slot != VisibilityPageLanes; ++slot)
				if (reference.page->Occupied(slot) && reference.page->expiresAt[slot] < tick)
					reference.page->Clear(slot);
	}

	void BuildHistoryRows()
	{
		historyRows_.clear();
		for (const auto reference : historyPageRefs_)
			for (std::size_t slot = 0; slot != VisibilityPageLanes; ++slot)
				if (reference.page->Occupied(slot))
				{
					if (historyRows_.size() >= capacity_.maximumObserverEntities)
						throw std::length_error("Visibility history snapshot exceeds startup capacity");
					historyRows_.push_back({reference.page, slot, reference.page->sources[slot]});
				}
		std::sort(historyRows_.begin(), historyRows_.end(), [](const auto &left, const auto &right) {
			return LessEntity(left.source, right.source);
		});
		for (std::size_t index = 1; index < historyRows_.size(); ++index)
			if (EqualEntity(historyRows_[index - 1].source, historyRows_[index].source))
				throw std::logic_error("Visibility history contains a duplicate source generation");
	}

	void ReconcileHistory(const std::uint64_t tick)
	{
		RetireMissingHistories(tick);
		BuildHistoryRows();
		CollectAvailableHistorySlots();
		std::size_t historyIndex = 0, liveIndex = 0;
		while (historyIndex != historyRows_.size() || liveIndex != live_.size())
		{
			if (liveIndex == live_.size() || (historyIndex != historyRows_.size() &&
				LessEntity(historyRows_[historyIndex].source, live_[liveIndex].source)))
			{
				RetireHistory(historyRows_[historyIndex++], tick);
				continue;
			}
			if (historyIndex == historyRows_.size() || LessEntity(live_[liveIndex].source,
				historyRows_[historyIndex].source))
			{
				CreateHistory(live_[liveIndex++]);
				continue;
			}
			UpdateHistory(historyRows_[historyIndex++], live_[liveIndex++], tick);
		}
	}

	void RetireMissingHistories(const std::uint64_t tick)
	{
		std::size_t historyIndex = 0, liveIndex = 0;
		while (historyIndex != historyRows_.size())
		{
			if (liveIndex == live_.size() || LessEntity(historyRows_[historyIndex].source, live_[liveIndex].source))
			{
				RetireHistory(historyRows_[historyIndex], tick);
				++historyIndex;
				continue;
			}
			if (LessEntity(live_[liveIndex].source, historyRows_[historyIndex].source))
			{
				++liveIndex;
				continue;
			}
			++historyIndex;
			++liveIndex;
		}
	}

	void CollectAvailableSlots()
	{
		CollectAvailableHistorySlots();
		CollectAvailableLeaseSlots();
	}

	void CollectAvailableHistorySlots()
	{
		historyFreeSlots_.clear();
		for (const auto reference : historyPageRefs_)
			for (std::size_t word = 0; word != reference.page->occupied.size(); ++word)
			{
				auto freeBits = ~reference.page->occupied[word];
				while (freeBits != 0)
				{
					if (historyFreeSlots_.size() >=
						CheckedMultiply(historyPageRefs_.size(), VisibilityPageLanes))
						throw std::length_error("Visibility history free-slot collection exceeded capacity");
					const auto bit = std::countr_zero(freeBits);
					historyFreeSlots_.push_back({reference.page, word * 64 + bit});
					freeBits &= freeBits - 1;
				}
			}
		historyFreeCursor_ = 0;
	}

	void CollectAvailableLeaseSlots()
	{
		leaseFreeSlots_.clear();
		for (const auto reference : leasePageRefs_)
			for (std::size_t word = 0; word != reference.page->occupied.size(); ++word)
			{
				auto freeBits = ~reference.page->occupied[word];
				while (freeBits != 0)
				{
					if (leaseFreeSlots_.size() >= CheckedMultiply(leasePageRefs_.size(), VisibilityPageLanes))
						throw std::length_error("Visibility lease free-slot collection exceeded capacity");
					const auto bit = std::countr_zero(freeBits);
					leaseFreeSlots_.push_back({reference.page, word * 64 + bit});
					freeBits &= freeBits - 1;
				}
			}
		leaseFreeCursor_ = 0;
	}

	void RetireHistory(const HistoryRowRef row, const std::uint64_t tick)
	{
		const auto &page = *row.page;
		if (page.regions[row.slot].valid)
			AddLease(page.owners[row.slot], page.regions[row.slot], page.graceTicks[row.slot], tick);
		row.page->Clear(row.slot);
	}

	void CreateHistory(const LiveObserver &current)
	{
		if (historyFreeCursor_ == historyFreeSlots_.size())
			throw std::length_error("Visibility history page pool exhausted");
		const auto free = historyFreeSlots_[historyFreeCursor_++];
		free.page->SetOccupied(free.slot);
		free.page->sources[free.slot] = current.source;
		free.page->owners[free.slot] = current.owner;
		free.page->regions[free.slot] = current.region;
		free.page->definitions[free.slot] = current.definition;
		free.page->graceTicks[free.slot] = current.graceTicks;
		free.page->flags[free.slot] = current.region.valid ? 1u : 0u;
	}

	void UpdateHistory(const HistoryRowRef row, const LiveObserver &current, const std::uint64_t tick)
	{
		const auto oldOwner = row.page->owners[row.slot];
		const auto oldRegion = row.page->regions[row.slot];
		const auto changed = oldOwner != current.owner || oldRegion != current.region ||
			row.page->definitions[row.slot] != current.definition ||
			row.page->graceTicks[row.slot] != current.graceTicks;
		if (changed && oldRegion.valid)
			AddLease(oldOwner, oldRegion, row.page->graceTicks[row.slot], tick);
		row.page->owners[row.slot] = current.owner;
		row.page->regions[row.slot] = current.region;
		row.page->definitions[row.slot] = current.definition;
		row.page->graceTicks[row.slot] = current.graceTicks;
		row.page->flags[row.slot] = current.region.valid ? 1u : 0u;
	}

	void AddLease(const ParticipantHandle owner, const VisibilityRegion region,
		const std::uint64_t graceTicks, const std::uint64_t tick)
	{
		assert(owner.IsValid());
		assert(region.valid);
		if (graceTicks > (std::numeric_limits<std::uint64_t>::max)() - tick)
			throw std::overflow_error("Visibility lease deadline overflow");
		if (leaseFreeCursor_ == leaseFreeSlots_.size())
			throw std::length_error("Visibility lease pool exhausted; authoritative reveal was not dropped");
		const auto free = leaseFreeSlots_[leaseFreeCursor_++];
		free.page->SetOccupied(free.slot);
		free.page->owners[free.slot] = owner;
		free.page->regions[free.slot] = region;
		free.page->expiresAt[free.slot] = tick + graceTicks;
	}

	void BuildRegionIndex(const std::uint64_t tick)
	{
		regionIndex_.Clear();
		for (const auto &observer : live_)
			if (observer.region.valid) regionIndex_.Add(observer.owner, observer.region);
		for (const auto reference : leasePageRefs_)
			for (std::size_t slot = 0; slot != VisibilityPageLanes; ++slot)
				if (reference.page->Occupied(slot) && reference.page->expiresAt[slot] >= tick)
					regionIndex_.Add(reference.page->owners[slot], reference.page->regions[slot]);
		regionIndex_.Publish();
	}

	const VisibilityDefinitions &definitions_;
	const VisibilityTopology &topology_;
	VisibilityPageCapacity capacity_;
	VisibilityPageBudget budget_;
	LiveObserverQuery liveObservers_;
	HistoryPageQuery historyPages_;
	LeasePageQuery leasePages_;
	VisibilityRegionIndex regionIndex_;
	std::vector<LiveObserver> live_;
	std::vector<HistoryPageRef> historyPageRefs_;
	std::vector<LeasePageRef> leasePageRefs_;
	std::vector<HistoryRowRef> historyRows_;
	std::vector<HistorySlotRef> historyFreeSlots_;
	std::vector<LeaseSlotRef> leaseFreeSlots_;
	std::vector<CellRange> cellRanges_;
	std::size_t historyFreeCursor_{};
	std::size_t leaseFreeCursor_{};
	bool cellPagesValidated_{};
};
} // namespace engine::gameplay::rts::visibility

export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::visibility::VisibilitySystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
} // namespace ecs
