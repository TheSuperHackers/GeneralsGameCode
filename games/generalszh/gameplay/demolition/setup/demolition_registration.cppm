module;
export module games.generalszh.gameplay.demolition.setup.demolition_registration;
export import games.generalszh.gameplay.demolition.components.demolition_trap;
export import engine.ecs.core.world;
export import engine.gameplay.combat.death.setup.death_weapon_registration;

export namespace generalszh::demolition
{
inline void RegisterDemolitionComponents(ecs::World &world)
{
    engine::gameplay::combat::death::RegisterDeathWeaponComponents(world);
    world.RegisterComponent<DemolitionTrapState>();
    world.RegisterComponent<DemolitionTargetClassification>();
}
}
