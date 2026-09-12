module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.visibility.algorithms.visibility_region_index;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;

export namespace engine::gameplay::rts::visibility
{
// Reusable per-tick scratch. It owns no authoritative state. All capacities
// are fixed at construction so publishing a tick cannot grow the heap.
class VisibilityRegionIndex final
{
public:
	VisibilityRegionIndex(const VisibilityTopology &topology, const std::size_t capacity,
		const std::uint32_t bucketSide = 8) :
		topology_(topology), side_(bucketSide), capacity_(capacity)
	{
		if (!capacity || !side_) throw std::invalid_argument("Visibility index capacity and bucket side must be positive");
		columns_ = (std::uint64_t{topology_.Width()} + side_ - 1) / side_;
		const auto rows = (std::uint64_t{topology_.Height()} + side_ - 1) / side_;
		const auto bucketCount = columns_ * rows;
		if (bucketCount >= (std::numeric_limits<std::size_t>::max)() - 1)
			throw std::length_error("Visibility index bucket count overflow");
		offsets_.resize(static_cast<std::size_t>(bucketCount) + 1);
		cursors_.resize(static_cast<std::size_t>(bucketCount));
		stagedOwners_.reserve(capacity_); stagedCenters_.reserve(capacity_); stagedRadii_.reserve(capacity_);
		owners_.resize(capacity_); centers_.resize(capacity_); radii_.resize(capacity_);
	}

	void Clear() noexcept
	{
		stagedOwners_.clear(); stagedCenters_.clear(); stagedRadii_.clear();
		published_ = false; maximumRadius_ = 0;
	}

	void Add(const ParticipantHandle owner, const VisibilityRegion region)
	{
		if (!owner.IsValid() || !region.valid || !topology_.Contains(region.center))
			throw std::invalid_argument("Visibility index received an invalid region");
		if (stagedOwners_.size() == capacity_)
			throw std::length_error("Visibility region index capacity exhausted");
		stagedOwners_.push_back(owner);
		stagedCenters_.push_back(region.center);
		stagedRadii_.push_back(region.radius);
		maximumRadius_ = (std::max)(maximumRadius_, region.radius);
	}

	void Publish()
	{
		if (published_) throw std::logic_error("Visibility index was already published");
		std::fill(offsets_.begin(), offsets_.end(), 0);
		for (const auto center : stagedCenters_)
			++offsets_[Bucket(topology_.X(center), topology_.Y(center)) + 1];
		for (std::size_t index = 1; index != offsets_.size(); ++index)
			offsets_[index] += offsets_[index - 1];
		std::copy(offsets_.begin(), offsets_.end() - 1, cursors_.begin());
		for (std::size_t index = 0; index != stagedCenters_.size(); ++index)
		{
			const auto bucket = Bucket(topology_.X(stagedCenters_[index]), topology_.Y(stagedCenters_[index]));
			const auto row = cursors_[bucket]++;
			owners_[row] = stagedOwners_[index];
			centers_[row] = stagedCenters_[index];
			radii_[row] = stagedRadii_[index];
		}
		published_ = true;
	}

	template<typename Visitor>
	void VisitCovering(const engine::gameplay::navigation::Cell cell, Visitor &&visitor) const
	{
		assert(published_ && topology_.Contains(cell));
		const auto x = topology_.X(cell), y = topology_.Y(cell);
		const auto minX = (x > maximumRadius_ ? x - maximumRadius_ : 0) / side_;
		const auto minY = (y > maximumRadius_ ? y - maximumRadius_ : 0) / side_;
		const auto maxX = (std::min<std::uint64_t>(topology_.Width() - 1,
			std::uint64_t{x} + maximumRadius_)) / side_;
		const auto maxY = (std::min<std::uint64_t>(topology_.Height() - 1,
			std::uint64_t{y} + maximumRadius_)) / side_;
		for (std::uint64_t bucketY = minY; bucketY <= maxY; ++bucketY)
			for (std::uint64_t bucketX = minX; bucketX <= maxX; ++bucketX)
			{
				const auto bucket = static_cast<std::size_t>(bucketY * columns_ + bucketX);
				for (auto row = offsets_[bucket]; row != offsets_[bucket + 1]; ++row)
				{
					const auto center = centers_[row];
					const auto dx = x > topology_.X(center) ? std::uint64_t{x - topology_.X(center)} :
						std::uint64_t{topology_.X(center) - x};
					const auto dy = y > topology_.Y(center) ? std::uint64_t{y - topology_.Y(center)} :
						std::uint64_t{topology_.Y(center) - y};
					const auto radiusSquared = std::uint64_t{radii_[row]} * radii_[row];
					const auto dxSquared = dx * dx;
					if (dxSquared > radiusSquared) continue;
					const auto dySquared = dy * dy;
					if (dySquared > radiusSquared - dxSquared) continue;
					visitor(owners_[row]);
				}
			}
	}

	std::size_t Size() const noexcept { return stagedCenters_.size(); }
	std::size_t Capacity() const noexcept { return capacity_; }

private:
	std::size_t Bucket(const std::uint32_t x, const std::uint32_t y) const noexcept
	{
		return static_cast<std::size_t>(std::uint64_t{y / side_} * columns_ + x / side_);
	}

	const VisibilityTopology &topology_;
	std::uint32_t side_{};
	std::uint64_t columns_{};
	std::size_t capacity_{};
	std::vector<ParticipantHandle> stagedOwners_;
	std::vector<engine::gameplay::navigation::Cell> stagedCenters_;
	std::vector<std::uint32_t> stagedRadii_;
	std::vector<ParticipantHandle> owners_;
	std::vector<engine::gameplay::navigation::Cell> centers_;
	std::vector<std::uint32_t> radii_;
	std::vector<std::size_t> offsets_;
	std::vector<std::size_t> cursors_;
	std::uint32_t maximumRadius_{};
	bool published_{};
};
} // namespace engine::gameplay::rts::visibility
