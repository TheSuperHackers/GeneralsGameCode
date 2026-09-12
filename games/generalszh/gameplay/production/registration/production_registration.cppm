module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.registration.production_registration;
export import engine.gameplay.containment.components.containment_task;
export import games.generalszh.gameplay.containment.components.initial_payload;
export import games.generalszh.gameplay.production.systems.production_admission_system;
export import games.generalszh.gameplay.production.systems.production_system;
export import games.generalszh.gameplay.production.systems.production_exit_system;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import games.generalszh.gameplay.progression.components.veterancy_award_binding;
export import engine.gameplay.rts.repair.components.manual_repair;
export import engine.gameplay.concealment.components.concealment_binding;
export import engine.gameplay.concealment.components.concealment_eligibility;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.concealment.components.detection_state;
export import games.generalszh.gameplay.concealment.components.stealth_binding;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export namespace generalszh::production
{
inline void RegisterProductionComponents(ecs::World &world)
    {
        world.RegisterComponent<BuildAdmission>();
        // Admission's typed prerequisite join reads the authoritative structure
        // owner. The construction module remains the declaration owner.
        world.RegisterComponent<construction::Structure>();
        world.RegisterComponent<ProductionExitState>(); world.RegisterComponent<ProductionExitTiming>();
        world.RegisterComponent<rally::RallyPoint>();
        world.RegisterComponent<capture::CaptureCapability>(); world.RegisterComponent<capture::CaptureActorState>();
        world.RegisterComponent<capture::CaptureRecharge>();
        world.RegisterComponent<engine::gameplay::combat::damage::ArmorBinding>();
        world.RegisterComponent<engine::gameplay::combat::DamageEmitter>();
        world.RegisterComponent<engine::gameplay::containment::PassengerMembership>(); world.RegisterComponent<engine::gameplay::containment::PassengerSlots>();
        world.RegisterComponent<engine::gameplay::containment::ContainmentTask>();
        world.RegisterComponent<engine::gameplay::containment::TransportBinding>(); world.RegisterComponent<engine::gameplay::containment::TransportState>();
        world.RegisterComponent<generalszh::containment::InitialPayloadBinding>();
        world.RegisterComponent<engine::gameplay::lifetime::Expiration>();
        world.RegisterComponent<engine::gameplay::combat::regeneration::RegenerationBinding>();
        world.RegisterComponent<engine::gameplay::combat::regeneration::RegenerationState>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionState>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionDefinitionRef>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionEligibility>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionInbox>();
        world.RegisterComponent<generalszh::progression::VeterancyAwardBinding>();
        world.RegisterComponent<construction::Builder>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairRateBinding>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairProgress>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairAssignment>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairBenefactorLease>();
        world.RegisterComponent<engine::gameplay::rts::construction::BuilderAssignment>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestPolicy>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestState>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestCargo>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestIntent>();
        world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestDelivery>();
        world.RegisterComponent<Work>(); world.RegisterComponent<Enabled>(); world.RegisterComponent<Ready>();
        world.RegisterComponent<Quantity>(); world.RegisterComponent<Queue>(); world.RegisterComponent<Member>();
        world.RegisterComponent<Balance>(); world.RegisterComponent<production::Producer>();
        // Production admission observes account unlock state when a recipe has
        // science prerequisites; registration does not attach state to entities.
        world.RegisterComponent<engine::gameplay::rts::unlocks::UnlockState>();
        world.RegisterComponent<production::BuildOrder>(); world.RegisterComponent<production::ProducedUnit>();
        world.RegisterComponent<production::ResearchedUpgrade>(); world.RegisterComponent<upgrades::UpgradeInstanceStatus>();
        world.RegisterComponent<Health>(); world.RegisterComponent<Life>();
        world.RegisterComponent<engine::gameplay::combat::PendingDamage>(); world.RegisterComponent<engine::gameplay::combat::DamageResult>();
        world.RegisterComponent<engine::gameplay::combat::HealthCapacity>();world.RegisterComponent<engine::gameplay::combat::PendingHealing>();world.RegisterComponent<engine::gameplay::combat::HealingResult>();
        world.RegisterComponent<engine::gameplay::combat::PeriodicDamagePolicy>();world.RegisterComponent<engine::gameplay::combat::PeriodicDamage>();
        world.RegisterComponent<Position>(); world.RegisterComponent<engine::gameplay::navigation::MoveGoal>();
        world.RegisterComponent<engine::gameplay::navigation::MoveRangeGoal>();
        world.RegisterComponent<engine::gameplay::navigation::MoveSpeed>(); world.RegisterComponent<engine::gameplay::navigation::MoveCredit>();
        world.RegisterComponent<engine::gameplay::navigation::MoveField>(); world.RegisterComponent<engine::gameplay::navigation::MoveEnabled>(); world.RegisterComponent<engine::gameplay::navigation::MoveResult>();
        world.RegisterComponent<engine::gameplay::concealment::ConcealmentBinding>();
        world.RegisterComponent<engine::gameplay::concealment::ConcealmentState>();
        world.RegisterComponent<engine::gameplay::concealment::ConcealmentEligibility>();
        world.RegisterComponent<engine::gameplay::concealment::DetectionBinding>();
        world.RegisterComponent<engine::gameplay::concealment::DetectionState>();
        world.RegisterComponent<engine::gameplay::concealment::ConcealmentGroup>();
        world.RegisterComponent<generalszh::concealment::StealthPolicyBinding>();
        world.RegisterComponent<Weapon>(); world.RegisterComponent<engine::gameplay::combat::WeaponState>();
        world.RegisterComponent<engine::gameplay::combat::WeaponTarget>(); world.RegisterComponent<engine::gameplay::combat::WeaponContact>();
        world.RegisterComponent<engine::gameplay::combat::ProjectileImpact>(); world.RegisterComponent<engine::gameplay::combat::ImpactDue>();
        world.RegisterComponent<combat::AcquisitionPolicy>(); world.RegisterComponent<combat::TargetIntent>();
        world.RegisterComponent<engine::gameplay::rts::orders::UnitOrder>();
        world.RegisterComponent<engine::gameplay::rts::orders::EngagementObservation>(); world.RegisterComponent<engine::gameplay::rts::orders::EngagementResult>();
    }


// The root retains ownership. This is startup registration, never tick dispatch.
inline void RegisterProductionSystems(ecs::SystemRegistry &registry,
    ProductionAdmissionSystem &admission, ProductionSystem &production, ProductionExitSystem &exits)
{
    registry.Register(admission, ecs::SystemPhase::Simulation);
    registry.Register(production, ecs::SystemPhase::Simulation);
    registry.Register(exits, ecs::SystemPhase::Simulation);
    // Creation visibility plus request/receipt resource publication.
    registry.OrderBefore<ProductionAdmissionSystem, ProductionSystem>();
    registry.OrderBefore<ProductionSystem, ProductionExitSystem>();
}
}
