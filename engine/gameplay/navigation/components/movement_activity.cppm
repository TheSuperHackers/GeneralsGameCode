module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.navigation.components.movement_activity;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::navigation
{
// Transient output owned by MovementSystem. A non-zero value means that the
// actor actually crossed cells during this fixed tick, including a crossing
// that leaves MoveResult::Arrived as the final status.
struct MovementActivity { std::uint32_t cellsMovedThisTick{}; };
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::navigation::MovementActivity>
{
    static constexpr std::string_view StableName="engine.gameplay.navigation.movement_activity";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
}
