module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.radar.events.radar_availability_transition;
export import engine.ecs.core.entity;
export import engine.events.schema.message_registry;

export namespace engine::gameplay::rts::radar
{
struct RadarAvailabilityTransition final
{
	ecs::Entity account{};
	bool before{};
	bool after{};

	friend constexpr bool operator==(const RadarAvailabilityTransition &,
		const RadarAvailabilityTransition &) noexcept = default;
};

static_assert(std::is_standard_layout_v<RadarAvailabilityTransition>);
static_assert(std::is_trivially_copyable_v<RadarAvailabilityTransition>);
}

export namespace engine::events
{
template<> struct MessageTraits<engine::gameplay::rts::radar::RadarAvailabilityTransition>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.radar.availability_transition";
	static constexpr std::uint32_t Version = 1;
	static constexpr MessageKind Kind = MessageKind::Fact;
	static constexpr RecordPolicy Recording = RecordPolicy::Transient;
};
}
