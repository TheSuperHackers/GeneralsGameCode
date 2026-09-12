module;
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.construction.setup.construction_registration;
export import games.generalszh.gameplay.construction.systems.construction_placement_system;
export import games.generalszh.gameplay.construction.systems.builder_task_system;
export namespace generalszh::construction {
inline void RegisterConstructionComponents(ecs::World &world) {
    using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
    using Site=engine::gameplay::rts::construction::ConstructionSite;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Life=engine::gameplay::combat::LifeState;
    using Health=engine::gameplay::combat::Health;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    using Work=engine::gameplay::rts::production::BuildWork;
    using Enabled=engine::gameplay::rts::production::BuildEnabled;
    using Ready=engine::gameplay::rts::production::BuildReady;
        world.RegisterComponent<Builder>(); world.RegisterComponent<Structure>(); world.RegisterComponent<Assignment>();
        world.RegisterComponent<selling::SaleState>(); world.RegisterComponent<selling::SaleEligibility>();
        world.RegisterComponent<engine::gameplay::rts::repair::RepairBinding>(); world.RegisterComponent<engine::gameplay::rts::repair::RepairState>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairBenefactorLease>();
        world.RegisterComponent<Site>(); world.RegisterComponent<ConstructionAdmission>(); world.RegisterComponent<ConstructionHealth>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityObserver>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityEligibility>();
        world.RegisterComponent<Work>(); world.RegisterComponent<Enabled>(); world.RegisterComponent<Ready>();
        world.RegisterComponent<Balance>(); world.RegisterComponent<Life>(); world.RegisterComponent<Health>();
        world.RegisterComponent<engine::gameplay::combat::HealthCapacity>();
        world.RegisterComponent<engine::gameplay::combat::PendingDamage>(); world.RegisterComponent<engine::gameplay::combat::DamageResult>();
        world.RegisterComponent<engine::gameplay::combat::PendingHealing>(); world.RegisterComponent<engine::gameplay::combat::HealingResult>();
        world.RegisterComponent<Position>(); world.RegisterComponent<Goal>(); world.RegisterComponent<Range>(); world.RegisterComponent<Credit>();
        world.RegisterComponent<production::Producer>(); world.RegisterComponent<engine::gameplay::rts::production::ProductionQueue>();
        world.RegisterComponent<production::rally::RallyPoint>();
        world.RegisterComponent<capture::Capturable>(); world.RegisterComponent<capture::CaptureState>();
        world.RegisterComponent<engine::gameplay::combat::damage::ArmorBinding>();
        world.RegisterComponent<match::VictoryAsset>();
        world.RegisterComponent<engine::gameplay::rts::power::PowerSource>();
        world.RegisterComponent<engine::gameplay::rts::power::PowerContribution>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::SupplyDropoff>();
        world.RegisterComponent<harvesting::SupplyDropoffOwner>();
    }
// Startup declarations only; the game root owns every argument.
inline void RegisterConstructionSystems(ecs::SystemRegistry &registry,
    ConstructionPlacementSystem &placement,BuilderTaskSystem &builders,
    ConstructionSystem &construction)
{
    registry.Register(placement,ecs::SystemPhase::Simulation);
    registry.Register(builders,ecs::SystemPhase::Simulation);
    registry.Register(construction,ecs::SystemPhase::Simulation);
    registry.OrderBefore<ConstructionPlacementSystem,BuilderTaskSystem>();
    registry.OrderBefore<BuilderTaskSystem,ConstructionSystem>();
}
}
