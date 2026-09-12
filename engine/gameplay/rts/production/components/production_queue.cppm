module;

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.production.components.production_queue;

export import engine.ecs.core.component_registry;
export import engine.ecs.core.world;

export namespace engine::gameplay::rts::production
{

struct ProductionQueue
{
	std::uint32_t offset{(std::numeric_limits<std::uint32_t>::max)()};
	std::uint32_t count{0};
	std::uint32_t capacity{0};
};

struct ProductionQueueMember
{
	ecs::Entity queue{};
	std::uint32_t position{0};
};

static_assert(std::is_standard_layout_v<ProductionQueue>);
static_assert(std::is_trivially_copyable_v<ProductionQueue>);
static_assert(std::is_standard_layout_v<ProductionQueueMember>);
static_assert(std::is_trivially_copyable_v<ProductionQueueMember>);

} // namespace engine::gameplay::rts::production

export namespace ecs
{

template<>
struct ComponentTraits<engine::gameplay::rts::production::ProductionQueue>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.production.queue";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<engine::gameplay::rts::production::ProductionQueueMember>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.production.queue_member";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

} // namespace ecs
