module;

#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.radar.components.radar_batch_range;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::radar
{
// A row owns a disjoint slice of the injected batch, including its output
// column. Write access to this component declares exclusive slice ownership.
// Indices are transient; they are neither pointers nor serialized state.
struct RadarBatchRange { std::size_t first{}, count{}; };
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::radar::RadarBatchRange>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.radar.batch_range";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
