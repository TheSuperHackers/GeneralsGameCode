module;
#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.combat.systems.poison_input_system;
export import games.generalszh.gameplay.combat.systems.poison_cleanse_system;
export import engine.ecs.system.system;
export import games.generalszh.gameplay.combat.inputs.poison_input;
export namespace generalszh {
struct PoisonInputSystem {
    using Query=ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<engine::gameplay::combat::PeriodicDamagePolicy>,ecs::Write<engine::gameplay::combat::PeriodicDamage>>;
    ecs::World &world;
    std::span<const PoisonInput> inputs;
    void SetInputs(std::span<const PoisonInput> value={}) {
        if(world.IsScheduledExecutionActive()) throw std::logic_error("Poison inputs require a joined boundary");
        inputs=value;
    }
    void Execute(ecs::SystemContext &context) {
            for(const auto &input:inputs)
            {
                using namespace engine::gameplay::combat;
                const auto *life=world.Get<LifeState>(input.target); const auto *policy=world.Get<PeriodicDamagePolicy>(input.target);auto *effect=world.Get<PeriodicDamage>(input.target);
                if(!life||!life->alive||!effect||!policy||!policy->interval||!policy->duration||!input.actualDamage) continue;
                const auto room=(std::numeric_limits<std::uint64_t>::max)()-policy->interval;
                if(policy->duration>room||context.Tick()>room-policy->duration) throw std::overflow_error("Poison deadline overflow");
                const auto next=context.Tick()+policy->interval;
                *effect={input.source,input.actualDamage,effect->active?std::min(effect->nextTick,next):next,context.Tick()+policy->duration,true};
            }

        inputs={};
    }
};
}
export namespace ecs {
template<> struct SystemTraits<generalszh::PoisonInputSystem> {
    static constexpr std::string_view StableName="games.generalszh.poison_inputs";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    static constexpr bool Batch=true;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}

export namespace generalszh {
inline void RegisterPoisonComponents(ecs::World &world) {
    world.RegisterComponent<engine::gameplay::combat::PeriodicDamagePolicy>(); world.RegisterComponent<engine::gameplay::combat::PeriodicDamage>();
}
inline void RegisterPoisonSystems(ecs::SystemRegistry &registry, PoisonInputSystem &input,
    engine::gameplay::combat::PeriodicDamageSystem &damage, combat::PoisonCleanseSystem &cleanse) {
    registry.Register(input,ecs::SystemPhase::Simulation); registry.Register(damage,ecs::SystemPhase::Simulation);
    registry.Register(cleanse,ecs::SystemPhase::Simulation);
    registry.OrderBefore<PoisonInputSystem,engine::gameplay::combat::PeriodicDamageSystem>();
    registry.OrderBefore<engine::gameplay::combat::PeriodicDamageSystem,combat::PoisonCleanseSystem>();
}
}
