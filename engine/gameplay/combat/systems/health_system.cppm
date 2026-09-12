module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.systems.health_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export namespace engine::gameplay::combat
{
struct HealthSystem
{
    using Query = ecs::Query<ecs::Write<Health>, ecs::Write<LifeState>, ecs::Write<PendingDamage>, ecs::Write<DamageResult>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        auto health = chunk.Get<Health>(); auto life = chunk.Get<LifeState>();
        auto damage = chunk.Get<PendingDamage>(); auto results = chunk.Get<DamageResult>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            results[row] = {};
            if (life[row].alive)
            {
                const auto healthBefore = health[row].current;
                const auto pendingAfter = damage[row].quantity;
                results[row].applied = std::min(healthBefore, pendingAfter);
                const auto healthAfter = healthBefore - results[row].applied;
                results[row].killed = healthAfter == 0;
                if (results[row].killed && MatchesLethalTransition(damage[row].lethalSource,
                    chunk.Entities()[row], context.Tick(), healthBefore, healthAfter, pendingAfter))
                    results[row].lethalSource = damage[row].lethalSource;
                health[row].current = healthAfter;
                life[row].alive = !results[row].killed;
            }
            damage[row] = {};
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::combat::HealthSystem>
{
    static constexpr std::string_view StableName = "engine.gameplay.combat.apply_damage";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
