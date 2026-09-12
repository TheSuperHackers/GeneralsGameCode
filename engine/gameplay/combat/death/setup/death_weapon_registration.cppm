module;
export module engine.gameplay.combat.death.setup.death_weapon_registration;
export import engine.ecs.core.world;
export import engine.gameplay.combat.death.components.death_weapon_state;

export namespace engine::gameplay::combat::death
{
inline void RegisterDeathWeaponComponents(ecs::World &world)
{
    world.RegisterComponent<DeathWeaponBinding>();
    world.RegisterComponent<DeathWeaponState>();
}
}
