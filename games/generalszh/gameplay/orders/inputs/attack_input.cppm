export module games.generalszh.gameplay.orders.inputs.attack_input;
export import engine.ecs.core.entity;

export namespace generalszh
{
struct AttackInput { ecs::Entity actor{}, target{}; };
}
