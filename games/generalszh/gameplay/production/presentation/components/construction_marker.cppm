module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.production.presentation.components.construction_marker;

export import engine.ecs.core.component_registry;

export namespace generalszh::production
{
struct ConstructionMarker
{
	std::uint64_t tick{0};
};
}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::production::ConstructionMarker>
{
	static constexpr std::string_view StableName =
		"games.generalszh.production.presentation.construction_marker";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
