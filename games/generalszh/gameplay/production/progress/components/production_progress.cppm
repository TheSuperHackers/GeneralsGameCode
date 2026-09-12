module;

#include <cstdint>
#include <limits>
#include <string_view>

export module games.generalszh.gameplay.production.progress.components.production_progress;

export import engine.ecs.core.component_registry;

export namespace generalszh::production
{
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);

struct ProductionElapsed
{
	std::int64_t ticks{0};
};

struct ProductionProgress
{
	float percent{0};
};
}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::production::ProductionElapsed>
{
	static constexpr std::string_view StableName = "games.generalszh.production.elapsed";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<generalszh::production::ProductionProgress>
{
	static constexpr std::string_view StableName = "games.generalszh.production.progress";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
