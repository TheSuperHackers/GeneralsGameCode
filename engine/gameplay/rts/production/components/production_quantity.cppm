module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.production.components.production_quantity;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::production
{
struct ProductionQuantity
{
	std::int32_t total{0};
	std::int32_t completed{0};
};

static_assert(std::is_standard_layout_v<ProductionQuantity>);
static_assert(std::is_trivially_copyable_v<ProductionQuantity>);
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::production::ProductionQuantity>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.production.production_quantity";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
