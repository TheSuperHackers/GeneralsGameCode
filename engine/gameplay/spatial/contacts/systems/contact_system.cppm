module;

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module engine.gameplay.spatial.contacts.systems.contact_system;
export import engine.ecs.system.system;
export import engine.events.storage.event_batch;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.spatial.grid.point_grid;
export import engine.gameplay.spatial.contacts.algorithms.contact_intersection;
export import engine.gameplay.spatial.contacts.algorithms.contact_participation;
export import engine.gameplay.spatial.contacts.components.contact_geometry;
export import engine.gameplay.spatial.contacts.events.contact_pair;

export namespace engine::gameplay::spatial::contacts
{
struct ContactLimits final
{
	std::size_t bodyCount{};
	// PointGrid::Capacity bounds both staged rows and direct entity-index lookup.
	// Keep the index requirement explicit instead of treating it as body storage.
	std::size_t entityIndexCapacity{};
	std::size_t logicalChunks{};
	std::size_t pairsPerChunk{};
	std::size_t totalPublishedPairs{};
};

class ContactSystem final
{
public:
	using Query = ecs::Query<ecs::Read<navigation::GridPosition>,
		ecs::Read<ContactGeometry>, ecs::Read<ContactParticipation>,
		ecs::Optional<combat::LifeState>>;

	ContactSystem(spatial::PointGrid &grid, const ContactLimits limits,
		engine::events::PublishedBatch<ContactPair> &published,
		std::pmr::memory_resource &memory) :
		limits_(limits), published_(published), grid_(grid),
		recorded_(limits.totalPublishedPairs, memory),
		bodies_(), pairs_(CheckedProduct(limits.logicalChunks, limits.pairsPerChunk)),
		pairCounts_(limits.logicalChunks), merged_()
	{
		ValidateLimits(grid, limits);
		bodies_.reserve(limits.bodyCount);
		// The injected grid is dedicated to this system for the lifetime of the
		// composed game instance; no other query may rebuild it concurrently.
		// Reduction scratch is bounded by the publication quota after the total
		// slice count is preflighted in AfterChunks.
		merged_.reserve(limits.totalPublishedPairs);
	}

	ContactSystem(const ContactSystem &) = delete;
	ContactSystem &operator=(const ContactSystem &) = delete;

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		if (published_.IsPublished())
			throw std::logic_error("Contact publication must be released before the next tick");

		bodies_.clear();
		merged_.clear();
		activeMaximumBoundCells_ = 0;
		std::fill(pairCounts_.begin(), pairCounts_.end(), 0);
		grid_.Clear();

		if (query.PreparedChunkCount() > limits_.logicalChunks)
			throw std::length_error("Contact query exceeds its logical chunk capacity");

		query.ForEachPreparedChunk([this](Query::Chunk chunk) {
			const auto positions = chunk.Get<navigation::GridPosition>();
			const auto geometries = chunk.Get<ContactGeometry>();
			const auto participation = chunk.Get<ContactParticipation>();
			const auto life = chunk.Get<combat::LifeState>();
			const auto entities = chunk.Entities();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto entity = entities[row];
				if (!participation[row].enabled || (!life.empty() && !life[row].alive))
					continue;
				const auto cell = positions[row].cell;
				const auto width = grid_.Width();
				assert(cell != navigation::InvalidCell &&
					std::uint64_t{cell} < std::uint64_t{width} * grid_.Height());
				assert(IsValidGeometry(geometries[row]));
				assert(IsValidParticipation(participation[row]));
				if (static_cast<std::size_t>(entity.index) >= limits_.entityIndexCapacity)
					throw std::length_error("Contact entity index exceeds declared lookup capacity");
				if (bodies_.size() == limits_.bodyCount)
					throw std::length_error("Contact body capacity exhausted");
				const auto x = static_cast<std::uint32_t>(cell % width);
				const auto y = static_cast<std::uint32_t>(cell / width);
				bodies_.push_back(Body{entity, x, y, geometries[row], participation[row]});
				activeMaximumBoundCells_ = (std::max)(activeMaximumBoundCells_,
					BroadphaseRadiusCells(geometries[row]));
			}
		});

		std::sort(bodies_.begin(), bodies_.end(), [](const Body &left, const Body &right) noexcept {
			return EntityLess(left.entity, right.entity);
		});
		for (std::size_t index = 1; index < bodies_.size(); ++index)
			if (bodies_[index - 1].entity.index == bodies_[index].entity.index)
				throw std::logic_error("Contact snapshot contains a duplicate entity index");

		for (const Body &body : bodies_)
			grid_.Add({body.entity, {}, body.x, body.y, body.participation.categoryMask});
		grid_.Publish();
		(void)context;
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
	{
		const auto chunkOrder = static_cast<std::size_t>(context.ChunkOrder());
		assert(chunkOrder < limits_.logicalChunks);
		const auto positions = chunk.Get<navigation::GridPosition>();
		const auto geometries = chunk.Get<ContactGeometry>();
		const auto participation = chunk.Get<ContactParticipation>();
		const auto life = chunk.Get<combat::LifeState>();
		const auto entities = chunk.Entities();
		const auto first = chunkOrder * limits_.pairsPerChunk;
		std::size_t &count = pairCounts_[chunkOrder];

		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			if (!participation[row].enabled || (!life.empty() && !life[row].alive))
				continue;
			const auto cell = positions[row].cell;
			const auto x = static_cast<std::uint32_t>(cell % grid_.Width());
			const auto y = static_cast<std::uint32_t>(cell / grid_.Width());
			const auto entity = entities[row];
			assert(IsValidGeometry(geometries[row]));
			assert(IsValidParticipation(participation[row]));
			const auto radius = BroadphaseRadiusCells(geometries[row]) + activeMaximumBoundCells_;
			assert(radius <= MaxBroadphaseRadiusCells);
			grid_.VisitWithinRadius(x, y, radius,
				[this, entity, x, y, &count, first, &geometry = geometries[row],
					&bodyParticipation = participation[row]](const spatial::SpatialPoint candidate) {
					if (!EntityLess(entity, candidate.entity))
						return;
					const Body *other = FindBody(candidate.entity);
					assert(other != nullptr);
					if (!CanCollide(bodyParticipation, other->participation))
						return;
					if (!Intersects(geometry, CellCoordinate(x), CellCoordinate(y),
						other->geometry, CellCoordinate(candidate.x), CellCoordinate(candidate.y)))
						return;
					if (count == limits_.pairsPerChunk)
						throw std::length_error("Contact pair scratch slice exhausted");
					pairs_[first + count] = CanonicalPair(entity, candidate.entity);
					++count;
				}, participation[row].collisionMask);
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &context)
	{
		std::size_t logicalTotal = 0;
		for (const auto count : pairCounts_)
		{
			if (count > limits_.totalPublishedPairs - logicalTotal)
				throw std::length_error("Contact publication capacity exhausted");
			logicalTotal += count;
		}
		merged_.clear();
		for (std::size_t chunk = 0; chunk != limits_.logicalChunks; ++chunk)
		{
			const auto count = pairCounts_[chunk];
			const auto first = chunk * limits_.pairsPerChunk;
			merged_.insert(merged_.end(), pairs_.begin() + first, pairs_.begin() + first + count);
		}
		std::sort(merged_.begin(), merged_.end(), ContactPairLess{});
		merged_.erase(std::unique(merged_.begin(), merged_.end()), merged_.end());
		assert(merged_.size() <= limits_.totalPublishedPairs);

		const auto boundary = engine::events::BatchBoundary{
			context.Tick(), static_cast<std::uint32_t>(context.Phase())};
		recorded_.Begin({boundary, context.Id(), 0, 0});
		for (const ContactPair pair : merged_)
			recorded_.Emplace(pair);
		recorded_.Seal();
		std::array<engine::events::RecordedBatch<ContactPair> *, 1> batches{&recorded_};
		published_.Publish(boundary, batches);
	}

	[[nodiscard]] const ContactLimits &Limits() const noexcept { return limits_; }

private:
	struct Body final
	{
		ecs::Entity entity{};
		std::uint32_t x{};
		std::uint32_t y{};
		ContactGeometry geometry{};
		ContactParticipation participation{};
	};

	static std::size_t CheckedProduct(const std::size_t left, const std::size_t right)
	{
		if (right != 0 && left > (std::numeric_limits<std::size_t>::max)() / right)
			throw std::length_error("Contact scratch size overflows size_t");
		return left * right;
	}

	static void ValidateLimits(const spatial::PointGrid &grid, const ContactLimits limits)
	{
		if (grid.Width() == 0 || grid.Height() == 0 ||
			grid.Width() > MaxMapDimensionCells || grid.Height() > MaxMapDimensionCells)
			throw std::invalid_argument("Contact map dimensions exceed the fixed contract");
		if (limits.bodyCount == 0 || limits.entityIndexCapacity == 0 || limits.logicalChunks == 0)
			throw std::invalid_argument("Contact body, entity-index and logical chunk capacities must be positive");
		if (limits.entityIndexCapacity < limits.bodyCount)
			throw std::invalid_argument("Contact entity-index capacity must cover body capacity");
		if (limits.logicalChunks > (std::numeric_limits<std::uint32_t>::max)())
			throw std::length_error("Contact logical chunk capacity exceeds scheduler order");
		if (grid.Capacity() < limits.bodyCount)
			throw std::invalid_argument("Contact grid row capacity is smaller than body capacity");
		if (grid.Capacity() < limits.entityIndexCapacity)
			throw std::invalid_argument("Contact grid entity-index capacity is smaller than the declared requirement");
		(void)CheckedProduct(limits.logicalChunks, limits.pairsPerChunk);
	}

	const Body *FindBody(const ecs::Entity entity) const noexcept
	{
		const auto found = std::lower_bound(bodies_.begin(), bodies_.end(), entity,
			[](const Body &body, const ecs::Entity value) noexcept { return EntityLess(body.entity, value); });
		assert(found != bodies_.end() && found->entity == entity);
		return &*found;
	}

	ContactLimits limits_;
	engine::events::PublishedBatch<ContactPair> &published_;
	spatial::PointGrid &grid_;
	engine::events::RecordedBatch<ContactPair> recorded_;
	std::vector<Body> bodies_;
	mutable std::vector<ContactPair> pairs_;
	mutable std::vector<std::size_t> pairCounts_;
	std::vector<ContactPair> merged_;
	std::uint32_t activeMaximumBoundCells_{};
};
} // namespace engine::gameplay::spatial::contacts

export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::spatial::contacts::ContactSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.spatial.contacts.contact_system";
	static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
