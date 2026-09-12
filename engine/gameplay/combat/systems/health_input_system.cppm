module;
#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.combat.systems.health_input_system;
export import engine.gameplay.combat.systems.healing_system;
export import engine.ecs.system.system;
export namespace engine::gameplay::combat {
struct DamageInput { ecs::Entity target{}; std::uint64_t quantity{}; };
struct HealingInput { ecs::Entity target{}; std::uint64_t quantity{}; };
struct HealthInputSystem {
    using Query=ecs::Query<ecs::Read<LifeState>,ecs::Read<Health>,ecs::Read<DamageResult>,
        ecs::Write<PendingDamage>,ecs::Read<HealthCapacity>,ecs::Read<HealingResult>,ecs::Write<PendingHealing>>;
    ecs::World &world;
    std::span<const DamageInput> inputs;
    std::span<const HealingInput> heals;
    void SetInputs(std::span<const DamageInput> damage={}, std::span<const HealingInput> healing={}) {
        if(world.IsScheduledExecutionActive()) throw std::logic_error("Health inputs require a joined boundary");
        inputs=damage; heals=healing;
    }
    void Execute(ecs::SystemContext &) {
            for (const auto &input : inputs)
            {
                const auto *life = world.Get<LifeState>(input.target);
                if (!life || !life->alive || !world.Get<Health>(input.target) || !world.Get<DamageResult>(input.target)) continue;
                if (auto *pending = world.Get<PendingDamage>(input.target))
                {
                    const auto room = (std::numeric_limits<std::uint64_t>::max)() - pending->quantity;
                    pending->quantity += input.quantity < room ? input.quantity : room;
                }
            }
            for(const auto &input:heals)
            {
                const auto *life=world.Get<LifeState>(input.target);
                if(!life||!life->alive||!world.Get<Health>(input.target)||!world.Get<HealthCapacity>(input.target)||!world.Get<HealingResult>(input.target)) continue;
                if(auto *pending=world.Get<PendingHealing>(input.target))
                    pending->quantity+=std::min(input.quantity,(std::numeric_limits<std::uint64_t>::max)()-pending->quantity);
            }

        inputs={}; heals={};
    }
};
}
export namespace ecs {
template<> struct SystemTraits<engine::gameplay::combat::HealthInputSystem> {
    static constexpr std::string_view StableName="engine.gameplay.combat.health_inputs";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    static constexpr bool Batch=true;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}

export namespace engine::gameplay::combat {
inline void RegisterHealthComponents(ecs::World &world) {
    world.RegisterComponent<Health>(); world.RegisterComponent<LifeState>();
    world.RegisterComponent<PendingDamage>(); world.RegisterComponent<DamageResult>();
    world.RegisterComponent<HealthCapacity>(); world.RegisterComponent<PendingHealing>(); world.RegisterComponent<HealingResult>();
}
inline void RegisterHealthSystems(ecs::SystemRegistry &registry, HealthInputSystem &input, HealthSystem &health, HealingSystem &healing) {
    registry.Register(input,ecs::SystemPhase::Simulation); registry.Register(health,ecs::SystemPhase::Simulation);
    registry.Register(healing,ecs::SystemPhase::Simulation);
    registry.OrderBefore<HealthInputSystem,HealthSystem>(); registry.OrderBefore<HealthSystem,HealingSystem>();
}
}
