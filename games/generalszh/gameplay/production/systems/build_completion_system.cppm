module;
#include <cstdint>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.production.systems.build_completion_system;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import engine.gameplay.containment.components.containment_task;
export import games.generalszh.gameplay.containment.components.initial_payload;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.progression.components.veterancy_award_binding;
export import engine.gameplay.rts.repair.components.manual_repair;
export import engine.gameplay.concealment.algorithms.detection_lease;
export import engine.gameplay.concealment.components.concealment_binding;
export import engine.gameplay.concealment.components.concealment_eligibility;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.concealment.components.detection_state;
export import engine.gameplay.navigation.components.movement_activity;
export import games.generalszh.gameplay.concealment.components.stealth_binding;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export namespace generalszh::production
{
// Records one order's remaining outputs; no system dispatch or world mutation.
inline void CompleteBuild(const BuildOrder &order,
    engine::gameplay::rts::production::ProductionQuantity &quantity, ecs::CommandBuffer &commands,
    std::uint64_t completionTick, engine::gameplay::rts::orders::UnitOrder initialOrder = {},
    ecs::Entity existingCarrier = {})
{
    // Check before recording any part of a pack. Horizon exhaustion remains a
    // fatal scheduler failure through the regeneration module's throw boundary.
    engine::gameplay::combat::regeneration::RegenerationState regenerationState;
    engine::gameplay::lifetime::Expiration expiration;
    capture::CaptureRecharge recharge;
    if(quantity.completed<quantity.total && order.kind==EntryKind::Unit && order.capture)
    {
        if(!order.captureTiming) throw std::invalid_argument("Capture output requires compiled recharge policy");
        recharge=capture::InitialCaptureRecharge(*order.captureTiming,completionTick);
    }
    if(quantity.completed<quantity.total && order.kind==EntryKind::Unit && order.lifetimeTicks)
        engine::gameplay::lifetime::ArmAfter(expiration,completionTick,*order.lifetimeTicks);
    if (quantity.completed < quantity.total && order.kind == EntryKind::Unit && order.regeneration)
    {
        const auto &spawn = *order.regeneration;
        regenerationState = {engine::gameplay::combat::regeneration::RegenerationDeadline(completionTick, spawn.phaseTicks),
            0, spawn.active, false, spawn.active};
    }
    for (; quantity.completed < quantity.total; ++quantity.completed)
    {
        const auto created = commands.Create();
        if (order.kind == EntryKind::Unit)
        {
            commands.Add<ProducedUnit>(created, ProducedUnit{order.producer, order.account, order.definition});
            if (order.cashBountyCost)
                commands.Add<generalszh::bounty::CashBountyCostBinding>(created, *order.cashBountyCost);
            if (order.visibility)
            {
                commands.Add<engine::gameplay::rts::visibility::VisibilityObserver>(created,
                    engine::gameplay::rts::visibility::VisibilityObserver{{}, *order.visibility});
                commands.Add<engine::gameplay::rts::visibility::VisibilityEligibility>(created);
            }
            if (order.veterancy)
                commands.Add<generalszh::progression::VeterancyAwardBinding>(created,
                    generalszh::progression::VeterancyAwardBinding{order.veterancy->definition, order.veterancy->awardable});
            commands.Add<engine::gameplay::combat::Health>(created, engine::gameplay::combat::Health{order.health});
            commands.Add<engine::gameplay::combat::HealthCapacity>(created,engine::gameplay::combat::HealthCapacity{order.maximumHealth});
            commands.Add<engine::gameplay::combat::PendingHealing>(created);
            commands.Add<engine::gameplay::combat::HealingResult>(created);
            commands.Add<engine::gameplay::combat::PeriodicDamagePolicy>(created,order.poison);
            commands.Add<engine::gameplay::combat::PeriodicDamage>(created);
            commands.Add<engine::gameplay::combat::LifeState>(created);
            commands.Add<engine::gameplay::combat::PendingDamage>(created);
            commands.Add<engine::gameplay::combat::DamageResult>(created);
            if(order.armor) commands.Add<engine::gameplay::combat::damage::ArmorBinding>(created,*order.armor);
            if(order.passenger || order.garrisonable)
            {
                if(order.passenger)
                    commands.Add<engine::gameplay::containment::PassengerSlots>(created,*order.passenger);
                auto membership = engine::gameplay::containment::PassengerMembership{};
                if(existingCarrier.IsValid())
                    membership = {existingCarrier,0,engine::gameplay::containment::ContainmentPhase::Inside,
                        engine::gameplay::containment::PassengerContainmentOwner::Transport,
                        engine::gameplay::containment::PassengerCombatPolicy::Blocked};
                commands.Add<engine::gameplay::containment::PassengerMembership>(created,membership);
                if(order.passenger)
                    commands.Add<engine::gameplay::containment::ContainmentTask>(created);
            }
            if(order.capture)
            {
                commands.Add<capture::CaptureCapability>(created,*order.capture);
                commands.Add<capture::CaptureActorState>(created);
                commands.Add<capture::CaptureRecharge>(created,recharge);
            }
            if(order.transport)
            {
                commands.Add<engine::gameplay::containment::TransportBinding>(created,*order.transport);
                commands.Add<engine::gameplay::containment::TransportState>(created);
            }
            if(order.initialPayload)
                commands.Add<containment::InitialPayloadBinding>(created,
                    containment::InitialPayloadBinding{order.initialPayload->passengerDefinition,
                        order.initialPayload->count,containment::InitialPayloadPhase::Pending});
            if(order.lifetimeTicks) commands.Add<engine::gameplay::lifetime::Expiration>(created,expiration);
            if (order.regeneration)
            {
                using namespace engine::gameplay::combat::regeneration;
                commands.Add<RegenerationBinding>(created, RegenerationBinding{order.regeneration->definition});
                commands.Add<RegenerationState>(created, regenerationState);
            }
            if (order.progression)
            {
                using namespace engine::gameplay::progression;
                commands.Add<ProgressionDefinitionRef>(created, order.progression->definition);
                commands.Add<ProgressionState>(created);
                commands.Add<ProgressionEligibility>(created, ProgressionEligibility{order.progression->trainable});
                commands.Add<ProgressionInbox>(created);
            }
            commands.Add<engine::gameplay::navigation::GridPosition>(created, engine::gameplay::navigation::GridPosition{order.spawn});
            commands.Add<engine::gameplay::navigation::MoveSpeed>(created, engine::gameplay::navigation::MoveSpeed{order.cellsPerSecond});
            commands.Add<engine::gameplay::navigation::MoveGoal>(created);
            commands.Add<engine::gameplay::navigation::MoveRangeGoal>(created);
            commands.Add<engine::gameplay::navigation::MoveCredit>(created);
            commands.Add<engine::gameplay::navigation::MoveField>(created);
            commands.Add<engine::gameplay::navigation::MoveEnabled>(created);
            commands.Add<engine::gameplay::navigation::MoveResult>(created);
            if (order.stealth)
            {
                const auto &spawn = *order.stealth;
                commands.Add<engine::gameplay::concealment::ConcealmentBinding>(created,
                    engine::gameplay::concealment::ConcealmentBinding{spawn.concealment});
                commands.Add<engine::gameplay::concealment::ConcealmentState>(created,
                    engine::gameplay::concealment::ConcealmentState{spawn.enabledByDefault, false, true,
                        engine::gameplay::concealment::ConcealmentDeadline(completionTick, spawn.concealDelayTicks), 0});
                commands.Add<engine::gameplay::concealment::ConcealmentEligibility>(created);
                commands.Add<engine::gameplay::concealment::ConcealmentGroup>(created);
                commands.Add<generalszh::concealment::StealthPolicyBinding>(created,
                    generalszh::concealment::StealthPolicyBinding{spawn.policy});
                commands.Add<engine::gameplay::navigation::MovementActivity>(created);
            }
            if (order.detector)
            {
                const auto &spawn = *order.detector;
                commands.Add<engine::gameplay::concealment::DetectionBinding>(created,
                    engine::gameplay::concealment::DetectionBinding{spawn.detection});
                commands.Add<engine::gameplay::concealment::DetectionState>(created,
                    engine::gameplay::concealment::DetectionState{!spawn.initiallyDisabled, completionTick});
                if (!order.stealth) commands.Add<engine::gameplay::concealment::ConcealmentGroup>(created);
            }
            commands.Add<engine::gameplay::rts::orders::UnitOrder>(created,initialOrder);
            commands.Add<engine::gameplay::rts::orders::EngagementObservation>(created);
            commands.Add<engine::gameplay::rts::orders::EngagementResult>(created);
            if (order.harvest)
            {
                using namespace engine::gameplay::rts::harvesting;
                commands.Add<HarvestPolicy>(created, *order.harvest);
                commands.Add<HarvestState>(created);
                commands.Add<HarvestCargo>(created);
                commands.Add<HarvestIntent>(created);
                commands.Add<HarvestDelivery>(created);
            }
            if (order.builder)
                commands.Add<construction::Builder>(created, construction::Builder{order.account});
            if (order.manualRepairDefinition)
            {
                commands.Add<engine::gameplay::rts::repair::ManualRepairRateBinding>(created,
                    engine::gameplay::rts::repair::ManualRepairRateBinding{*order.manualRepairDefinition});
                commands.Add<engine::gameplay::rts::repair::ManualRepairProgress>(created);
                commands.Add<engine::gameplay::rts::repair::ManualRepairAssignment>(created);
            }
            if (order.weapon.damage || order.weapon.secondaryDamage)
            {
                commands.Add<engine::gameplay::combat::WeaponDefinition>(created, order.weapon);
                commands.Add<engine::gameplay::combat::WeaponState>(created, engine::gameplay::combat::WeaponState{0, order.weapon.clipSize, false});
                commands.Add<engine::gameplay::combat::WeaponTarget>(created);
                commands.Add<engine::gameplay::combat::WeaponContact>(created);
                commands.Add<engine::gameplay::combat::DamageEmitter>(created);
                commands.Add<combat::AcquisitionPolicy>(created,order.acquisition);
                commands.Add<combat::TargetIntent>(created);
            }
        }
        else
        {
            commands.Add<ResearchedUpgrade>(created, ResearchedUpgrade{order.account, order.definition});
            commands.Add<upgrades::UpgradeInstanceStatus>(created,
                upgrades::UpgradeInstanceStatus{upgrades::Status::Complete});
        }
    }
}

struct BuildCompletionSystem
{
    using Quantity = engine::gameplay::rts::production::ProductionQuantity;
    using Ready = engine::gameplay::rts::production::BuildReady;
    using Query = ecs::Query<ecs::Read<BuildOrder>, ecs::Read<Ready>, ecs::Write<Quantity>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto orders = chunk.Get<BuildOrder>(); const auto ready = chunk.Get<Ready>();
        auto quantities = chunk.Get<Quantity>(); auto &commands = context.Commands();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            if (!ready[row].value) continue;
            const auto &order = orders[row]; auto &quantity = quantities[row];
            CompleteBuild(order, quantity, commands, context.Tick());
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::production::BuildCompletionSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.production.build_completion";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>;
    using After = SystemTypeList<engine::gameplay::rts::production::BuildSystem>;
};
}
