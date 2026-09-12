module;

#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>

export module games.generalszh.gameplay.production.progress.algorithms.production_progress;

export import games.generalszh.gameplay.production.progress.components.production_progress;
export import games.generalszh.gameplay.production.progress.definitions.production_requirement;

export namespace generalszh::production
{
// Each call represents one eligible fixed-step simulation update. The
// eligibility decision and any pause or time scaling belong to the simulation
// driver; this component stores no per-entity time rate and does not sample a
// wall clock.
inline void AdvanceOneStep(ProductionElapsed &elapsed) noexcept
{
	assert(elapsed.ticks != (std::numeric_limits<std::int64_t>::max)());
	const auto bits = std::bit_cast<std::uint64_t>(elapsed.ticks);
	elapsed.ticks = std::bit_cast<std::int64_t>(bits + std::uint64_t{1});
}

inline void RefreshProgress(const ProductionElapsed &elapsed,
	ProductionProgress &progress,
	const ProductionRequirement required) noexcept
{
	progress.percent = float(elapsed.ticks) / float(required.ticks) * 100.0f;
}
}
