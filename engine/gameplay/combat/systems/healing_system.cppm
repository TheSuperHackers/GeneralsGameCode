module;
#include <algorithm>
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.systems.healing_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.combat.components.healing;
export namespace engine::gameplay::combat
{
struct HealingSystem
{
    using Query=ecs::Query<ecs::Write<Health>,ecs::Read<HealthCapacity>,ecs::Read<LifeState>,ecs::Write<PendingHealing>,ecs::Write<HealingResult>>;
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept
    {
        auto health=chunk.Get<Health>(); const auto capacities=chunk.Get<HealthCapacity>(); const auto lives=chunk.Get<LifeState>();
        auto pending=chunk.Get<PendingHealing>(); auto results=chunk.Get<HealingResult>();
        for(std::size_t row=0;row!=chunk.Count();++row)
        {
            results[row]={};
            if(lives[row].alive&&health[row].current<capacities[row].maximum)
            {
                results[row].applied=std::min(pending[row].quantity,capacities[row].maximum-health[row].current);
                health[row].current+=results[row].applied;
            }
            pending[row].quantity=0;
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::combat::HealingSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.healing";
    static constexpr SystemPhase Phase=SystemPhase::PreSimulation;
    using Before=SystemTypeList<>;using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
