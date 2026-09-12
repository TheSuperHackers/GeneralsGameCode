module;
#include <algorithm>
#include <cstddef>
#include <limits>
#include <string_view>
#include <stdexcept>
export module engine.gameplay.combat.regeneration.systems.regeneration_system;
export import engine.gameplay.combat.regeneration.components.regeneration;
export import engine.gameplay.combat.systems.healing_system;

export namespace engine::gameplay::combat::regeneration
{
class RegenerationSystem
{
public:
    using Query=ecs::Query<ecs::Read<Health>, ecs::Read<HealthCapacity>, ecs::Read<LifeState>,
        ecs::Read<DamageResult>, ecs::Read<RegenerationBinding>, ecs::Write<RegenerationState>,
        ecs::Write<PendingHealing>>;
    explicit RegenerationSystem(const RegenerationDefinitions &definitions) : definitions(definitions) {}
    void BeforeChunks(Query &, ecs::SystemContext &context) const
    {
        if (context.Time().Step() != definitions.Step())
            throw std::invalid_argument("Regeneration catalog differs from finalized simulation step");
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto health=chunk.Get<Health>(); const auto capacity=chunk.Get<HealthCapacity>();
        const auto life=chunk.Get<LifeState>(); const auto damage=chunk.Get<DamageResult>();
        const auto bindings=chunk.Get<RegenerationBinding>(); auto states=chunk.Get<RegenerationState>();
        auto pending=chunk.Get<PendingHealing>(); const auto tick=context.Time().Tick();
        for (std::size_t row=0; row<chunk.Count(); ++row)
        {
            auto &state=states[row];
            if (state.stopped || !state.active || !life[row].alive) continue;
            const auto &definition=definitions.Get(bindings[row].definition);
            if (damage[row].applied != 0)
            {
                if (definition.damageDelay != 0)
                {
                    state.nextPulse=RegenerationDeadline(tick,definition.damageDelay);
                    state.armed=true;
                }
                else if (definition.strictDamageWakeCooldown ? tick>state.soonestDamageWake : tick>=state.soonestDamageWake)
                {
                    state.nextPulse=RegenerationDeadline(tick,definition.damageWakeLatency);
                    state.armed=true;
                }
            }
            if (!state.armed || tick<state.nextPulse) continue;
            if (health[row].current>=capacity[row].maximum)
            {
                state.armed=false;
                continue;
            }
            // Validate the next deadline before producing this pulse. Scheduler
            // failure is fatal; no partially executed tick may be resumed.
            const auto next=RegenerationDeadline(tick,definition.interval);
            const auto room=(std::numeric_limits<std::uint64_t>::max)()-pending[row].quantity;
            pending[row].quantity+=std::min(room,definition.quantity);
            state.nextPulse=next;
            state.soonestDamageWake=next;
        }
    }
private:
    const RegenerationDefinitions &definitions;
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::combat::regeneration::RegenerationSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.regeneration";
    static constexpr SystemPhase Phase=SystemPhase::PreSimulation;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
    using Before=SystemTypeList<engine::gameplay::combat::HealingSystem>;
};
}
