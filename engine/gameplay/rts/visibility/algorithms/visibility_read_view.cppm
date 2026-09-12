module;

#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.visibility.algorithms.visibility_read_view;
export import engine.ecs.query.query;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;

export namespace engine::gameplay::rts::visibility
{
// Nonowning page/column cache used by target queries.  Visibility masks remain
// authoritative in ECS page components; these vectors only retain startup
// addresses so a candidate test never performs a World lookup.
class VisibilityReadView final
{
public:
	using Access = ecs::Query<ecs::Read<VisibilityCellIndex>, ecs::Read<VisibilityVisibleMaskPage>>;

	explicit VisibilityReadView(const VisibilityTopology &topology) : topology_(topology) {}

	void Bind(std::span<const VisibilityCellIndex * const> indexes,
		std::span<const VisibilityVisibleMaskPage * const> visiblePages)
	{
		if (bound_ || unrestricted_) throw std::logic_error("Visibility read view was already initialized");
		if (indexes.size() != visiblePages.size())
			throw std::invalid_argument("Visibility read view page columns have different counts");
		std::uint64_t expected{};
		for (std::size_t page = 0; page != indexes.size(); ++page)
		{
			if (!indexes[page] || !visiblePages[page] || indexes[page]->count > VisibilityPageLanes)
				throw std::invalid_argument("Visibility read view contains an invalid page reference");
			if (indexes[page]->count == 0 ||
				(page + 1 != indexes.size() && indexes[page]->count != VisibilityPageLanes))
				throw std::invalid_argument("Visibility read view requires full interior pages and a non-empty final page");
			if (indexes[page]->firstCell != expected)
				throw std::logic_error("Visibility read view pages are not a contiguous topology partition");
			expected += indexes[page]->count;
		}
		if (expected != topology_.Count())
			throw std::logic_error("Visibility read view pages do not cover the topology");
		indexes_.assign(indexes.begin(), indexes.end());
		visiblePages_.assign(visiblePages.begin(), visiblePages.end());
		bound_ = true;
	}

	void SetStartupUnrestricted()
	{
		if (bound_ || unrestricted_) throw std::logic_error("Visibility read view was already initialized");
		unrestricted_ = true;
	}

	bool IsBound() const noexcept { return bound_ || unrestricted_; }
	bool StartupUnrestricted() const noexcept { return unrestricted_; }

	bool Visible(const ParticipantHandle owner, const engine::gameplay::navigation::Cell cell) const noexcept
	{
		if (unrestricted_) return true;
		if (!bound_ || !owner.IsValid() || !topology_.Contains(cell)) return false;
		const auto page = static_cast<std::size_t>(cell / VisibilityPageLanes);
		if (page >= indexes_.size()) return false;
		const auto lane = static_cast<std::size_t>(cell % VisibilityPageLanes);
		if (lane >= indexes_[page]->count) return false;
		return (visiblePages_[page]->values[lane] & ObserverBit(owner.slot)) != 0;
	}

private:
	const VisibilityTopology &topology_;
	std::vector<const VisibilityCellIndex *> indexes_;
	std::vector<const VisibilityVisibleMaskPage *> visiblePages_;
	bool bound_{};
	bool unrestricted_{};
};
} // namespace engine::gameplay::rts::visibility
