module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.radar.components.radar_provider_contribution;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::radar
{
// Derived per-provider output. RadarAvailability remains the only aggregate
// authority; this component is rebuilt by the joined availability system.
struct RadarProviderContribution final
{
	std::int32_t producers{};
	std::int32_t resistantProducers{};
};

static_assert(std::is_standard_layout_v<RadarProviderContribution>);
static_assert(std::is_trivially_copyable_v<RadarProviderContribution>);
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::radar::RadarProviderContribution>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.radar.provider_contribution";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
