module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.combat.systems.periodic_damage_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.combat.definitions.periodic_damage_definition;
export namespace engine::gameplay::combat
{
struct PeriodicDamageSystem
{
    using Query=ecs::Query<ecs::Read<LifeState>,ecs::Read<PeriodicDamagePolicy>,ecs::Write<PeriodicDamage>,ecs::Write<PendingDamage>>;
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const noexcept
    {
        const auto lives=chunk.Get<LifeState>(); const auto policies=chunk.Get<PeriodicDamagePolicy>();
        auto effects=chunk.Get<PeriodicDamage>(); auto pending=chunk.Get<PendingDamage>(); const auto tick=context.Tick();
        for(std::size_t row=0;row!=chunk.Count();++row)
        {
            auto &effect=effects[row]; if(!effect.active) continue;
            if(!lives[row].alive||tick>effect.stopTick) {effect={};continue;}
            assert(policies[row].interval>0);
            if(tick>=effect.nextTick)
            {
                pending[row].quantity+=std::min(effect.quantity,(std::numeric_limits<std::uint64_t>::max)()-pending[row].quantity);
                assert(tick<=(std::numeric_limits<std::uint64_t>::max)()-policies[row].interval);
                effect.nextTick=tick+policies[row].interval;
            }
            if(tick==effect.stopTick) effect={};
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::combat::PeriodicDamageSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.periodic_damage";
    static constexpr SystemPhase Phase=SystemPhase::PreSimulation;
    using Before=SystemTypeList<>;using After=SystemTypeList<>;
};
}
