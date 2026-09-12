module;
#include <algorithm>
#include <string_view>
export module engine.gameplay.combat.systems.timed_life_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.lifetime.components.expiration;
export namespace engine::gameplay::combat
{
// A timed living actor expires through the health pipeline, not immediate entity
// destruction. The deadline and one-shot state are the existing persistent ECS
// Expiration component; no companion clock or feature executor is needed.
struct TimedLifeSystem
{
    using Query=ecs::Query<ecs::Write<engine::gameplay::lifetime::Expiration>,
        ecs::Read<Health>,ecs::Read<LifeState>,ecs::Write<PendingDamage>>;
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const noexcept
    {
        using namespace engine::gameplay::lifetime;
        auto expires=chunk.Get<Expiration>(); const auto health=chunk.Get<Health>();
        const auto life=chunk.Get<LifeState>(); auto damage=chunk.Get<PendingDamage>();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            auto &expiry=expires[row];
            if(expiry.state!=ExpirationState::Armed || context.Tick()<expiry.deadline) continue;
            expiry.state=ExpirationState::Elapsed;
            if(life[row].alive) damage[row].quantity=std::max(damage[row].quantity,health[row].current);
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::combat::TimedLifeSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.timed_life";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<engine::gameplay::combat::HealthSystem>;
    using After=SystemTypeList<>;
};
}
