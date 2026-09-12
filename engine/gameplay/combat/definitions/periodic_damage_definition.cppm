module;
#include <stdexcept>
export module engine.gameplay.combat.definitions.periodic_damage_definition;
export import engine.gameplay.combat.components.periodic_damage;
export import engine.time.simulation_time;
export namespace engine::gameplay::combat
{
struct PeriodicDamageConfig { engine::time::Duration interval{},duration{}; };
inline PeriodicDamagePolicy AuthorPeriodicDamage(PeriodicDamageConfig config,engine::time::FixedStep step)
{
    const auto interval=step.TicksFor(config.interval),duration=step.TicksFor(config.duration);
    if((interval==0)!=(duration==0)) throw std::invalid_argument("Periodic damage needs both positive interval and duration, or neither");
    return {interval,duration};
}
}
