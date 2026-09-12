module;
#include <string_view>
export module engine.gameplay.concealment.inputs.uncloak_input;
export import engine.ecs.core.entity;

export namespace engine::gameplay::concealment
{
struct UncloakInput { ecs::Entity actor{}; };
}
