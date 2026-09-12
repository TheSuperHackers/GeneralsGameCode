module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.containment.components.containment_task;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::containment
{
// A pending request to board a live carrier. PassengerMembership remains the
// sole authoritative relationship once the request reaches the carrier.
struct ContainmentTask
{
    ecs::Entity carrier{};
};
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::containment::ContainmentTask>
{
    static constexpr std::string_view StableName="engine.gameplay.containment.task";
    static constexpr std::uint32_t Version=1;
    // This is the ECS persistence policy marker. Serialization is provided by
    // a separate persistence slice and is intentionally not implemented here.
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
