module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.navigation.components.movement;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export namespace engine::gameplay::navigation
{
struct GridPosition { Cell cell{}; };
struct MoveGoal { Cell cell{InvalidCell}; };
struct MoveRangeGoal
{
    Cell center{InvalidCell};
    std::uint32_t radius{};

    constexpr bool Active() const noexcept { return center != InvalidCell; }
    friend constexpr bool operator==(const MoveRangeGoal &, const MoveRangeGoal &) noexcept = default;
};
struct MoveSpeed { std::uint32_t cellsPerSecond{1}; };
struct MoveCredit { std::uint32_t remainder{}; };
struct MoveField { Cell index{InvalidCell}; };
struct MoveEnabled { bool value{true}; };
enum class MoveStatus : std::uint8_t { Idle, Moving, Arrived, Unreachable };
struct MoveResult { MoveStatus status{MoveStatus::Idle}; };

inline void ClearMoveRangeGoal(MoveRangeGoal &range, MoveCredit &credit) noexcept
{
    if (!range.Active()) return;
    range = {};
    credit = {};
}

inline void SetMoveRangeGoal(MoveRangeGoal &range, MoveCredit &credit, const Cell center,
    const std::uint32_t radius) noexcept
{
    if (range.center != center || range.radius != radius) credit = {};
    range = {center, radius};
}
}
export namespace ecs
{
#define MOVEMENT_COMPONENT(Type, Key, Policy) template<> struct ComponentTraits<engine::gameplay::navigation::Type> { \
 static constexpr std::string_view StableName = Key; static constexpr std::uint32_t Version = 1; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; };
MOVEMENT_COMPONENT(GridPosition, "engine.gameplay.navigation.grid_position", Serializable)
MOVEMENT_COMPONENT(MoveGoal, "engine.gameplay.navigation.move_goal", Serializable)
MOVEMENT_COMPONENT(MoveRangeGoal, "engine.gameplay.navigation.move_range_goal", Transient)
MOVEMENT_COMPONENT(MoveSpeed, "engine.gameplay.navigation.move_speed", Serializable)
MOVEMENT_COMPONENT(MoveCredit, "engine.gameplay.navigation.move_credit", Serializable)
MOVEMENT_COMPONENT(MoveField, "engine.gameplay.navigation.move_field", Transient)
MOVEMENT_COMPONENT(MoveEnabled, "engine.gameplay.navigation.move_enabled", Transient)
MOVEMENT_COMPONENT(MoveResult, "engine.gameplay.navigation.move_result", Transient)
#undef MOVEMENT_COMPONENT
}
