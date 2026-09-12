module;
export module engine.gameplay.rts.orders.inputs.order_input;
export import engine.gameplay.rts.orders.components.unit_order;
export namespace engine::gameplay::rts::orders
{
struct OrderInput {ecs::Entity actor{};UnitOrder order{};};
}
