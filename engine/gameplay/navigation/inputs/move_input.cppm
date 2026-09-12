module;
export module engine.gameplay.navigation.inputs.move_input;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export namespace engine::gameplay::navigation
{
struct MoveInput { ecs::Entity actor{}; Cell destination{InvalidCell}; };
}
