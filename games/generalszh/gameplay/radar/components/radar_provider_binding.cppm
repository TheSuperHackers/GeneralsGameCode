module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module games.generalszh.gameplay.radar.components.radar_provider_binding;
export import engine.ecs.core.component_registry;

export namespace generalszh::radar
{
// Immutable authored enrollment. The definition ID resolves through the
// startup-owned RadarProviderDefinitions catalog.
struct RadarProviderBinding final
{
	std::uint32_t definitionId{};
};

// Persistent effect latch. This is intentionally separate from the raw object
// upgrade word: removing that word resets upgrade eligibility, but does not
// undo an already-applied RadarUpgrade effect.
struct RadarProviderGrant final
{
	bool granted{};
};

static_assert(std::is_standard_layout_v<RadarProviderBinding>);
static_assert(std::is_trivially_copyable_v<RadarProviderBinding>);
static_assert(std::is_standard_layout_v<RadarProviderGrant>);
static_assert(std::is_trivially_copyable_v<RadarProviderGrant>);
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::radar::RadarProviderBinding>
{
	static constexpr std::string_view StableName =
		"games.generalszh.radar.provider_binding";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<generalszh::radar::RadarProviderGrant>
{
	static constexpr std::string_view StableName =
		"games.generalszh.radar.provider_grant";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
