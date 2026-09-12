module;
#include <cstdint>

export module games.generalszh.gameplay.combat.inputs.poison_input;
export import engine.ecs.core.entity;

export namespace generalszh
{
struct PoisonInput { ecs::Entity target{}, source{}; std::uint64_t actualDamage{}; };
}
