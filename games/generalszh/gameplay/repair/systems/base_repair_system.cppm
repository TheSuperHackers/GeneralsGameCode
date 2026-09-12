module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.repair.systems.base_repair_system;
export import engine.gameplay.rts.repair.components.repair_state;
export import engine.gameplay.rts.repair.algorithms.repair_progress;
export import engine.gameplay.combat.systems.healing_system;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import games.generalszh.gameplay.selling.components.sale_state;
export namespace generalszh::repair
{
class BaseRepairSystem
{
    using Binding=engine::gameplay::rts::repair::RepairBinding;
    using State=engine::gameplay::rts::repair::RepairState;
public:
    using Query=ecs::Query<ecs::Read<engine::gameplay::combat::Health>,ecs::Read<engine::gameplay::combat::HealthCapacity>,
        ecs::Read<engine::gameplay::combat::LifeState>,ecs::Read<engine::gameplay::combat::DamageResult>,
        ecs::Read<construction::Structure>,ecs::Optional<selling::SaleState>,ecs::Read<Binding>,ecs::Write<State>,
        ecs::Write<engine::gameplay::combat::PendingHealing>>;
    explicit BaseRepairSystem(const engine::gameplay::rts::repair::RepairDefinitions &definitions):definitions_(definitions) {}
    void BeforeChunks(Query &,ecs::SystemContext &context) const
    { assert(context.Time().Step()==definitions_.Step()); }
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        using namespace engine::gameplay::combat;
        using namespace engine::gameplay::rts::repair;
        const auto health=chunk.Get<Health>(); const auto capacity=chunk.Get<HealthCapacity>();
        const auto life=chunk.Get<LifeState>(); const auto damage=chunk.Get<DamageResult>();
        const auto structures=chunk.Get<construction::Structure>(); const auto sales=chunk.Get<selling::SaleState>();
        const auto bindings=chunk.Get<Binding>(); auto states=chunk.Get<State>(); auto healing=chunk.Get<PendingHealing>();
        const auto tick=context.Tick();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            auto &state=states[row];
            if(state.stopped) continue;
            if(!life[row].alive || (!sales.empty()&&sales[row].phase!=selling::SalePhase::Idle))
            { state.stopped=true; state.armed=false; continue; }
            const auto &definition=definitions_.GetUnchecked(bindings[row].definition);
            if(!definition.numerator) {state.armed=false;continue;}
            // DamageResult cannot observe zero-effective-damage callbacks.
            if(damage[row].applied) {state.nextPulse=RepairDeadline(tick,definition.damageDelay);state.armed=true;}
            if(!structures[row].complete || !state.armed || tick<state.nextPulse) continue;
            if(health[row].current>=capacity[row].maximum)
            {state.armed=false;state.remainder=0;continue;}
            const auto next=RepairDeadline(tick,definition.interval);
            const auto amount=RepairPulse(capacity[row].maximum,definition,state.remainder);
            const auto missing=capacity[row].maximum-health[row].current;
            // Existing pending healing already owns that part of the deficit;
            // cap this repair contribution without overflowing the shared sum.
            if(healing[row].quantity<missing)
                healing[row].quantity+=std::min(amount,missing-healing[row].quantity);
            state.nextPulse=next;
        }
    }
private:
    const engine::gameplay::rts::repair::RepairDefinitions &definitions_;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::repair::BaseRepairSystem>
{
    static constexpr std::string_view StableName="games.generalszh.repair.base_repair";
    static constexpr SystemPhase Phase=SystemPhase::PreSimulation;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
    using Before=SystemTypeList<engine::gameplay::combat::HealingSystem>;
};
}
