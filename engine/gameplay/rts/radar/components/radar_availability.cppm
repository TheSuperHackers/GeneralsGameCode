module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.radar.components.radar_availability;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::radar
{
struct RadarAvailability
{
	std::int32_t producers{0};
	std::int32_t resistantProducers{0};
	bool suppressed{false};
};

static_assert(std::is_standard_layout_v<RadarAvailability>);
static_assert(std::is_trivially_copyable_v<RadarAvailability>);
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::radar::RadarAvailability>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.radar.availability";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
