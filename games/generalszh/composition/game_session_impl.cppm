module;
#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <memory>
#include <memory_resource>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>
module games.generalszh.composition.game_session;
import games.generalszh.gameplay.economy.systems.economy_system;
import games.generalszh.gameplay.crates.components.crate_state;
import games.generalszh.gameplay.crates.definitions.collector_definition;
import games.generalszh.gameplay.crates.definitions.crate_definition;
import games.generalszh.gameplay.crates.inputs.crate_pickup_batch;
import games.generalszh.gameplay.crates.definitions.account_pickup_policy;
import games.generalszh.gameplay.crates.systems.crate_claim_system;
import games.generalszh.gameplay.crates.systems.crate_reward_system;
import engine.events.storage.event_batch;
import engine.gameplay.spatial.contacts.components.contact_geometry;
import engine.gameplay.spatial.contacts.components.contact_participation;
import engine.gameplay.spatial.contacts.events.contact_pair;
import engine.gameplay.spatial.contacts.systems.contact_system;
import engine.gameplay.spatial.grid.point_grid;
import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
import games.generalszh.gameplay.bounty.definitions.cash_bounty_catalog;
import games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;
import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
import games.generalszh.gameplay.bounty.systems.cash_bounty_award_system;
import games.generalszh.gameplay.bounty.systems.cash_bounty_policy_system;
import games.generalszh.gameplay.ai.systems.force_production_system;
import games.generalszh.gameplay.production.registration.production_registration;
import engine.gameplay.rts.unlocks.components.unlock_inbox;
import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
import engine.gameplay.rts.unlocks.inputs.unlock_batch;
import engine.gameplay.rts.unlocks.systems.unlock_system;
import engine.gameplay.rts.rank.algorithms.rank_initialization;
import engine.gameplay.rts.rank.components.rank_inbox;
import engine.gameplay.rts.rank.components.rank_state;
import engine.gameplay.rts.rank.components.rank_skill_award_modifier;
import engine.gameplay.rts.rank.definitions.rank_definition;
import engine.gameplay.rts.rank.inputs.rank_batch;
import engine.gameplay.rts.rank.systems.rank_system;
import games.generalszh.gameplay.power.systems.power_system;
import engine.gameplay.combat.inputs.health_input_system;
import engine.gameplay.navigation.systems.movement_system;
import engine.gameplay.concealment.definitions.concealment_definition;
import engine.gameplay.concealment.definitions.detection_definition;
import engine.gameplay.concealment.inputs.uncloak_input;
import engine.gameplay.concealment.algorithms.detection_index;
import engine.gameplay.concealment.systems.concealment_system;
import games.generalszh.gameplay.concealment.definitions.stealth_policy;
import games.generalszh.gameplay.concealment.systems.stealth_policy_system;
import games.generalszh.gameplay.movement.systems.movement_eligibility_system;
import engine.gameplay.combat.systems.projectile_system;
import games.generalszh.gameplay.combat.targeting.target_index;
import games.generalszh.gameplay.combat.targeting.impact_target_projection;
import games.generalszh.gameplay.combat.targeting.launch_snapshot;
import games.generalszh.gameplay.match.systems.victory_system;
import games.generalszh.gameplay.orders.systems.order_system;
import games.generalszh.gameplay.combat.poison.poison_input_system;
import engine.ecs.scheduler.scheduler;
import engine.gameplay.combat.regeneration.systems.regeneration_system;
import engine.gameplay.progression.systems.progression_system;
import games.generalszh.gameplay.progression.systems.kill_experience_system;
import games.generalszh.gameplay.construction.setup.construction_registration;
import games.generalszh.gameplay.construction.algorithms.completed_structure;
import games.generalszh.gameplay.demolition.setup.demolition_registration;
import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
import games.generalszh.gameplay.demolition.inputs.demolition_trap_inputs;
import games.generalszh.gameplay.demolition.algorithms.demolition_source_projection;
import games.generalszh.gameplay.demolition.algorithms.demolition_target_index;
import games.generalszh.gameplay.demolition.systems.demolition_trap_system;
import engine.gameplay.combat.death.definitions.death_weapon_definition;
import engine.gameplay.combat.death.systems.death_weapon_system;
import games.generalszh.gameplay.tasks.systems.task_selection_system;
import engine.gameplay.rts.repair.components.manual_repair;
import engine.gameplay.rts.harvesting.systems.harvest_system;
import games.generalszh.gameplay.selling.systems.sell_system;
import games.generalszh.gameplay.repair.systems.base_repair_system;
import games.generalszh.gameplay.repair.systems.manual_repair_system;
import games.generalszh.gameplay.containment.systems.transport_containment_system;
import games.generalszh.gameplay.containment.systems.garrison_containment_system;
import games.generalszh.gameplay.containment.systems.initial_payload_production_system;
import games.generalszh.gameplay.production.rally.systems.rally_system;
import games.generalszh.gameplay.capture.systems.capture_system;
import games.generalszh.gameplay.capture.systems.capture_upgrade_activation_system;
import engine.gameplay.rts.visibility.algorithms.visibility_page_capacity;
import engine.gameplay.rts.visibility.algorithms.visibility_read_view;
import engine.gameplay.rts.visibility.components.visibility_cell_page;
import engine.gameplay.rts.visibility.components.visibility_history_page;
import engine.gameplay.rts.visibility.components.visibility_lease_page;
import engine.gameplay.rts.visibility.components.visibility_observer;
import engine.gameplay.rts.visibility.definitions.visibility_definition;
import engine.gameplay.rts.visibility.definitions.visibility_participant;
import engine.gameplay.rts.visibility.systems.visibility_system;
import games.generalszh.gameplay.visibility.systems.visibility_policy_system;
namespace generalszh
{
class GameSession::Impl
{
public:
    using DemolitionDeathWeaponSystem = engine::gameplay::combat::death::DeathWeaponSystemT<demolition::DemolitionSourceProjection>;

    Impl(ecs::World &world, engine::jobs::JobSystem &jobs,
        std::span<const production::BuildDefinition> definitions,
        const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t inputCapacity = 65536,
        std::span<const engine::gameplay::combat::regeneration::RegenerationDefinition> regenerationDefinitions = {},
        std::span<const engine::gameplay::progression::ProgressionDefinition> progressionDefinitions = {},
        std::span<const construction::BuildingDefinition> buildingDefinitions = {},
        std::span<const selling::SellDefinition> saleDefinitions = {},
        std::span<const engine::gameplay::rts::repair::RepairDefinition> repairDefinitions = {},
        std::span<const engine::gameplay::containment::TransportDefinition> transportDefinitions = {},
        std::span<const capture::CaptureDefinition> captureDefinitions = {},
        std::span<const engine::gameplay::combat::damage::DamageTypePolicy> damagePolicies = DefaultDirectDamagePolicies,
        std::span<const engine::gameplay::combat::damage::ArmorDefinition> armorDefinitions = {},
        engine::time::Duration manualRepairRetention = ZeroHourManualRepairRetention,
        std::size_t splashResultCapacity = 0, std::size_t acceptedHitCapacity = 0,
         std::span<const generalszh::progression::VeterancyDefinition> veterancyDefinitions = {},
         std::span<const demolition::DemolitionTrapDefinition> demolitionDefinitions = {},
         const engine::gameplay::rts::unlocks::UnlockCatalog *unlockCatalog = nullptr,
         const engine::gameplay::rts::rank::RankCatalog *rankCatalog = nullptr,
         std::size_t maximumVisibilityObservers = 4096,
         std::size_t maximumVisibilityRegionTransitions = 1,
         std::span<const generalszh::bounty::CashBountyDefinition> cashBountyDefinitions = {}) :
          world(world), grid(grid), contactBatchMemory(),
          contactGrid(grid.Width(), grid.Count() / grid.Width(), inputCapacity),
          publishedContacts(inputCapacity, 1, contactBatchMemory),
          visibilityTopology(grid.Width(), grid.Count() / grid.Width()),
         visibilityView(visibilityTopology), definitions(definitions.begin(), definitions.end()),
         authoredCashBountyDefinitions(cashBountyDefinitions.begin(), cashBountyDefinitions.end()),
         unlockCatalog_(unlockCatalog), rankCatalog_(rankCatalog),
        healthInput{world}, authoredRegeneration(regenerationDefinitions.begin(),regenerationDefinitions.end()),
        progressionCatalog(progressionDefinitions), veterancyCatalog(veterancyDefinitions),
        progressionBatch(inputCapacity), rankBatch(inputCapacity),
        progression(progressionCatalog,progressionBatch),
        killExperience(world,matchEntity,progressionCatalog,veterancyCatalog,progressionBatch,inputCapacity,inputCapacity,
            rankCatalog ? &rankBatch : nullptr),
        repairBatchMemory(),acceptedRepairActors(inputCapacity,1,repairBatchMemory),
         harvestSnapshots(inputCapacity),harvest(world,grid,harvestSnapshots,&acceptedRepairActors),supplyRevenue(world,inputCapacity),
         cratePickupBatch(inputCapacity,inputCapacity), crateRewardBatch(inputCapacity),
         crateAccountPolicies(inputCapacity), bountyBatch(inputCapacity),
         accountBatch(EconomySystem::Capacity({inputCapacity,inputCapacity,inputCapacity,inputCapacity,inputCapacity})),
         accounts(accountBatch), economy(world,accountBatch,
             {inputCapacity,inputCapacity,inputCapacity,inputCapacity,inputCapacity},&supplyRevenue,
             cashBountyDefinitions.empty() ? nullptr : &bountyBatch,nullptr), buildBatch(inputCapacity),
        unlockBatch(inputCapacity),
        powerFrame(inputCapacity), power(world,powerFrame),
        fields(grid), movement(world,jobs,grid,fields,inputCapacity),
        targets(world,grid,inputCapacity,visibilityView,participants),
        impactTargets(world,grid,inputCapacity,inputCapacity),
        demolitionTargets(world,grid,inputCapacity),
        demolitionSources(world,grid,inputCapacity),
        demolitionInputs(inputCapacity),
        armorCatalog(damagePolicies,armorDefinitions),
        acceptedHits(acceptedHitCapacity ? acceptedHitCapacity : inputCapacity),
        projectiles(world,armorCatalog,impactTargets,acceptedHits,inputCapacity,
            splashResultCapacity ? splashResultCapacity : inputCapacity),
        acquisition(targets), targeting(targets.Frame(),grid),
        weapon(combat::LaunchSnapshotView(targets.Frame())),
        defeated(inputCapacity), victory(world,matchEntity,defeated,inputCapacity), defeat(defeated),
        orders(world,grid,targets,inputCapacity), poisonInput{world},
        authoredBuildings(buildingDefinitions.begin(),buildingDefinitions.end()),
        authoredDemolitionTraps(demolitionDefinitions.begin(),demolitionDefinitions.end()),
            constructionRequests(inputCapacity),constructionReceipts(inputCapacity),builderFrame(inputCapacity),
        builderTasks(world,constructionReceipts),construction(world,builderFrame),taskSelection(inputCapacity,&acceptedRepairActors),
        authoredSales(saleDefinitions.begin(),saleDefinitions.end()),saleBatch(inputCapacity,inputCapacity),
        authoredRepairs(repairDefinitions.begin(),repairDefinitions.end()),
        authoredTransports(transportDefinitions.begin(),transportDefinitions.end()),containmentBatch(inputCapacity),
        garrisonBatch(inputCapacity),
        rallyFields(grid),rallyBatch(inputCapacity),rally(grid,jobs,rallyFields,rallyBatch,inputCapacity),
            authoredCapture(captureDefinitions.begin(),captureDefinitions.end()),
        captureBatch(inputCapacity,capture::CaptureBatch::RequiredResultCapacity(inputCapacity,inputCapacity)),
        scheduler(world,registry,jobs), inputCapacity(inputCapacity), manualRepairRetention(manualRepairRetention),
         maximumVisibilityObservers_(maximumVisibilityObservers),
         maximumVisibilityRegionTransitions_(maximumVisibilityRegionTransitions)
    {
        if(inputCapacity>(std::numeric_limits<std::size_t>::max)()/5)
            throw std::length_error("Manual repair interruption capacity overflow");
        repairOverrideCapacity=5*inputCapacity;
        repairOverrides.reserve(repairOverrideCapacity);
        cratePolicyAccounts.reserve(inputCapacity);
    }

    // This boundary takes ownership of the nested authoring graph. Caller
    // spans need only live through this call; Finalize rebuilds the immutable
    // runtime catalog against its fixed step.
    void ConfigureForceProduction(std::span<const ForceProductionPolicyInput> policies,
        ForceProductionPlanLimits limits)
    {
        if (step || failed || forceProductionConfigured)
            throw std::logic_error("Force production configuration is a pre-finalization one-shot");
        if (!policies.empty() && (limits.maxPolicies == 0 || limits.maxCandidates == 0 || limits.maxEntries == 0))
            throw std::invalid_argument("Force production configuration limits must be positive");
        try
        {
            CopyForceProductionPolicies(policies, limits);
            forceProductionLimits=limits;
            forceProductionConfigured=true;
        }
        catch (...)
        {
            authoredForceProductionPolicies.clear();
            authoredForceProductionCandidates.clear();
            authoredForceProductionEntries.clear();
            throw;
        }
    }

    void ConfigureCrates(CrateConfiguration configuration)
    {
        if (step || failed || cratesConfigured)
            throw std::logic_error("Crate configuration is a pre-finalization one-shot");
        if (configuration.definitions.empty())
        {
            if (!configuration.unitCollectors.empty() || !configuration.structureCollectors.empty()
                || configuration.referenceCount != 0 || configuration.outputAwards != 0)
                throw std::invalid_argument("Empty crate configuration cannot carry collector or reward data");
            return;
        }
        try
        {
            authoredCrateDefinitions.assign(configuration.definitions.begin(), configuration.definitions.end());
            authoredCrateUnitCollectors.assign(configuration.unitCollectors.begin(), configuration.unitCollectors.end());
            authoredCrateStructureCollectors.assign(configuration.structureCollectors.begin(), configuration.structureCollectors.end());
            const auto byKey=[](const auto &left, const auto &right) { return left.key < right.key; };
            std::stable_sort(authoredCrateUnitCollectors.begin(), authoredCrateUnitCollectors.end(), byKey);
            std::stable_sort(authoredCrateStructureCollectors.begin(), authoredCrateStructureCollectors.end(), byKey);
            crateRewardLimits={configuration.referenceCount,configuration.outputAwards};
            cratesConfigured=true;
        }
        catch (...)
        {
            authoredCrateDefinitions.clear();
            authoredCrateUnitCollectors.clear();
            authoredCrateStructureCollectors.clear();
            crateRewardLimits={};
            throw;
        }
    }

    static void RegisterComponents(ecs::World &world)
    {
        RegisterEconomyComponents(world); production::RegisterProductionComponents(world);
        world.RegisterComponent<engine::gameplay::rts::unlocks::UnlockInbox>();
        world.RegisterComponent<engine::gameplay::rts::rank::RankState>();
        world.RegisterComponent<engine::gameplay::rts::rank::RankInbox>();
        world.RegisterComponent<engine::gameplay::rts::rank::RankSkillAwardModifier>();
        engine::gameplay::combat::RegisterHealthComponents(world);
        engine::gameplay::navigation::RegisterMovementComponents(world);
        RegisterPoisonComponents(world);
        construction::RegisterConstructionComponents(world);
        demolition::RegisterDemolitionComponents(world);
        // Garrison uses the already registered optional movement columns and
        // the shared PassengerMembership component; no second relationship type
        // or container-specific ECS payload is introduced.
        selling::RegisterSellComponents(world);
        engine::gameplay::rts::harvesting::RegisterHarvestComponents(world);
        world.RegisterComponent<match::VictoryAsset>(); world.RegisterComponent<match::MatchState>();
        world.RegisterComponent<match::MatchMember>();
        world.RegisterComponent<engine::gameplay::rts::match::Contender>();
        world.RegisterComponent<engine::gameplay::rts::match::SurvivalCount>();
        world.RegisterComponent<construction::Structure>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairRateBinding>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairProgress>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairAssignment>();
        world.RegisterComponent<engine::gameplay::rts::repair::ManualRepairBenefactorLease>();
        world.RegisterComponent<power::PowerLedger>(); world.RegisterComponent<power::PowerState>();
        world.RegisterComponent<power::PowerPolicy>(); world.RegisterComponent<power::PowerSource>();
        world.RegisterComponent<power::PowerContribution>();
        world.RegisterComponent<engine::gameplay::combat::regeneration::RegenerationBinding>();
        world.RegisterComponent<engine::gameplay::combat::regeneration::RegenerationState>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionState>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionDefinitionRef>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionEligibility>();
        world.RegisterComponent<engine::gameplay::progression::ProgressionInbox>();
        world.RegisterComponent<crates::CrateState>();
        world.RegisterComponent<engine::gameplay::spatial::contacts::ContactGeometry>();
        world.RegisterComponent<engine::gameplay::spatial::contacts::ContactParticipation>();
        world.RegisterComponent<engine::gameplay::rts::ai::ForceProductionController>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityObserver>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityEligibility>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityCellIndex>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityExploredMaskPage>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityHistoryPage>();
        world.RegisterComponent<engine::gameplay::rts::visibility::VisibilityLeasePage>();
        world.RegisterComponent<engine::gameplay::rts::bounty::BountyPolicy>();
        world.RegisterComponent<generalszh::bounty::CashBountyCostBinding>();
    }
    void Finalize(engine::time::FixedStep rate)
    {
        if (step || failed) throw std::logic_error("Game finalization is one-shot");
        try
        {
            if (unlockCatalog_ && !unlockCatalog_->IsFinalized())
                throw std::logic_error("Game unlock catalog must be finalized before session finalization");
            if (rankCatalog_)
            {
                if (!unlockCatalog_)
                    throw std::logic_error("Game rank catalog requires an unlock catalog");
                if (rankCatalog_->UnlockSchemaHash() != unlockCatalog_->SchemaHash())
                    throw std::invalid_argument("Game rank and unlock catalog schemas do not match");
            }
            if (!authoredCashBountyDefinitions.empty() && !unlockCatalog_)
                throw std::invalid_argument("Cash bounty definitions require an injected unlock catalog");
            if (cratesConfigured)
            {
                contactSystem = std::make_unique<engine::gameplay::spatial::contacts::ContactSystem>(
                    contactGrid,
                    engine::gameplay::spatial::contacts::ContactLimits{
                        inputCapacity, inputCapacity, inputCapacity,
                        (std::min)(inputCapacity, std::size_t{64}), inputCapacity},
                    publishedContacts, contactBatchMemory);
                crateDefinitionCatalog=std::make_unique<crates::CrateDefinitionCatalog>(
                    std::span<const crates::CrateDefinition>(authoredCrateDefinitions));
                crateCollectorCatalog=std::make_unique<crates::CollectorDefinitionCatalog>(
                    std::span<const crates::CollectorDefinition>(authoredCrateUnitCollectors),
                    std::span<const crates::CollectorDefinition>(authoredCrateStructureCollectors));
                crateClaim=std::make_unique<crates::CrateClaimSystem>(world,grid,*crateDefinitionCatalog,
                    *crateCollectorCatalog,crateAccountPolicies,progressionCatalog,progressionBatch,
                    cratePickupBatch,publishedContacts,inputCapacity);
                crateClaim->Configure();
                crateReward=std::make_unique<crates::CrateRewardSystem>(world,grid,*crateDefinitionCatalog,
                    progressionCatalog,progressionBatch,cratePickupBatch,crateRewardBatch,inputCapacity,
                    crateRewardLimits);
                crateReward->Configure();
            }
            economy.Configure(rate,cratesConfigured ? &crateRewardBatch : nullptr);
            harvest.Configure(rate); supplyRevenue.Configure();
            regenerationDefinitions=std::make_unique<engine::gameplay::combat::regeneration::RegenerationDefinitions>(rate,authoredRegeneration);
            regeneration=std::make_unique<engine::gameplay::combat::regeneration::RegenerationSystem>(*regenerationDefinitions);
            transportDefinitions=std::make_unique<engine::gameplay::containment::TransportDefinitions>(rate,authoredTransports);
            transport=std::make_unique<containment::TransportContainmentSystem>(world,*transportDefinitions,containmentBatch,inputCapacity);
            captureDefinitions=std::make_unique<capture::CaptureDefinitions>(rate,authoredCapture);
            captureSystem=std::make_unique<capture::CaptureSystem>(world,*captureDefinitions,captureBatch,inputCapacity);
            captureUpgradeActivation=std::make_unique<capture::CaptureUpgradeActivationSystem>(world,*captureDefinitions,inputCapacity);
            authoredConcealment.clear(); authoredDetection.clear(); authoredStealth.clear();
            const auto appendConcealment=[&](const auto &value) {
                if (std::find(authoredConcealment.begin(),authoredConcealment.end(),value)==authoredConcealment.end())
                    authoredConcealment.push_back(value);
            };
            const auto appendDetection=[&](const auto &value) {
                if (std::find(authoredDetection.begin(),authoredDetection.end(),value)==authoredDetection.end())
                    authoredDetection.push_back(value);
            };
            const auto appendStealth=[&](const auto &value) {
                if (std::find(authoredStealth.begin(),authoredStealth.end(),value)==authoredStealth.end())
                    authoredStealth.push_back(value);
            };
            // Runtime definition IDs are an adapter-internal schema. Derive them
            // from the stable authored key order, while BuildCatalog retains its
            // existing key-sorted producer index contract independently.
            std::vector<std::size_t> canonicalDefinitionOrder;
            canonicalDefinitionOrder.reserve(definitions.size());
            for (std::size_t index=0; index!=definitions.size(); ++index)
                canonicalDefinitionOrder.push_back(index);
            std::stable_sort(canonicalDefinitionOrder.begin(),canonicalDefinitionOrder.end(),
                [&](const auto left,const auto right) { return definitions[left].key < definitions[right].key; });
            for (const auto index : canonicalDefinitionOrder)
            {
                const auto &definition=definitions[index];
                if (definition.concealment) appendConcealment(*definition.concealment);
                if (definition.detection) appendDetection(*definition.detection);
                if (definition.stealthPolicy) appendStealth(*definition.stealthPolicy);
            }
            concealmentDefinitions=std::make_unique<engine::gameplay::concealment::ConcealmentDefinitions>(rate,authoredConcealment);
            detectionDefinitions=std::make_unique<engine::gameplay::concealment::DetectionDefinitions>(rate,authoredDetection);
            stealthPolicies=std::make_unique<generalszh::concealment::StealthPolicies>(authoredStealth);
            detectionIndex=std::make_unique<engine::gameplay::concealment::DetectionIndex>(world,grid,inputCapacity);
            stealthPolicy=std::make_unique<generalszh::concealment::StealthPolicySystem>(*stealthPolicies,world);
            concealment=std::make_unique<engine::gameplay::concealment::ConcealmentSystem>(world,
                *concealmentDefinitions,*detectionDefinitions,*detectionIndex,inputCapacity);
            repairDefinitions=std::make_unique<engine::gameplay::rts::repair::RepairDefinitions>(rate,authoredRepairs);
            manualRepair=std::make_unique<repair::ManualRepairSystem>(world,grid,*repairDefinitions,
                acceptedRepairActors,inputCapacity,manualRepairRetention);
            manualRepair->Configure(rate);
            ConfigureVisibility(rate);
            targets.ConfigureVisibilityPolicy();
            catalog=std::make_unique<production::BuildCatalog>(world,definitions,rate,regenerationDefinitions.get(),&progressionCatalog,
                transportDefinitions.get(),captureDefinitions.get(),&armorCatalog,repairDefinitions.get(),authoredRepairs,
                concealmentDefinitions.get(),detectionDefinitions.get(),stealthPolicies.get(),&veterancyCatalog,unlockCatalog_);
            initialPayload=std::make_unique<containment::InitialPayloadProductionSystem>(*catalog);
            admission=std::make_unique<production::ProductionAdmissionSystem>(world,*catalog,buildBatch);
            production=std::make_unique<production::ProductionSystem>(world,buildBatch,powerFrame);
            productionExits=std::make_unique<production::ProductionExitSystem>(world,buildBatch);
            if (unlockCatalog_)
                unlockSystem=std::make_unique<engine::gameplay::rts::unlocks::UnlockSystem>(*unlockCatalog_,unlockBatch);
            if (rankCatalog_)
                rankSystem=std::make_unique<engine::gameplay::rts::rank::RankSystem>(*rankCatalog_,rankBatch);
            if (!authoredCashBountyDefinitions.empty())
            {
                cashBountyCatalog=std::make_unique<generalszh::bounty::CashBountyCatalog>(
                    std::span<const generalszh::bounty::CashBountyDefinition>(authoredCashBountyDefinitions),*unlockCatalog_);
                cashBountyPolicy=std::make_unique<generalszh::bounty::CashBountyPolicySystem>(*cashBountyCatalog);
                cashBountyAwards=std::make_unique<generalszh::bounty::CashBountyAwardSystem>(
                    world,matchEntity,bountyBatch,inputCapacity,inputCapacity);
                cashBountyAwards->Configure();
            }
            if (!authoredForceProductionPolicies.empty())
            {
                forceProductionPlans=std::make_unique<engine::gameplay::rts::ai::ForceProductionPlanCatalog>(
                    std::span<const ForceProductionPolicyInput>(authoredForceProductionPolicies),rate,forceProductionLimits);
                forceProduction=std::make_unique<generalszh::ai::ForceProductionSystem>(
                    world,*forceProductionPlans,*catalog,buildBatch,inputCapacity,inputCapacity);
                forceProduction->ValidateConfiguredControllers();
            }
            baseRepair=std::make_unique<repair::BaseRepairSystem>(*repairDefinitions);
            buildings=std::make_unique<construction::BuildingCatalog>(authoredBuildings,rate,repairDefinitions.get(),&armorCatalog);
            demolitionCatalog=std::make_unique<demolition::DemolitionTrapCatalog>(authoredDemolitionTraps,rate);
            deathWeaponCatalog=std::make_unique<engine::gameplay::combat::death::DeathWeaponCatalog>(
                demolitionCatalog->DeathWeapons(),rate);
            demolitionTrap=std::make_unique<demolition::DemolitionTrapSystem>(
                *demolitionCatalog,demolitionTargets,demolitionInputs);
            deathWeapons=std::make_unique<DemolitionDeathWeaponSystem>(*deathWeaponCatalog,demolitionSources);
            garrison=std::make_unique<containment::GarrisonContainmentSystem>(world,*catalog,*buildings,garrisonBatch,inputCapacity);
            saleCatalog=std::make_unique<selling::SellCatalog>(*buildings,authoredSales);
            sellingSystem=std::make_unique<selling::SellSystem>(*saleCatalog,saleBatch,inputCapacity);
            placement=std::make_unique<construction::ConstructionPlacementSystem>(world,grid,*buildings,
                constructionRequests,constructionReceipts,inputCapacity,demolitionCatalog.get());
            placement->Configure(); builderTasks.Configure(); construction.Configure(repairDefinitions.get());
            killExperience.Configure();
            RegisterSystems();
            registry.Finalize(world.Components()); scheduler.Finalize(rate); step=rate;
        }
        catch (...) { failed=true; throw; }
    }

    void RegisterController(ecs::Entity account, std::uint32_t policyKey, bool enabled=true)
    {
        RequireBoundary();
        if (!forceProduction || !forceProductionPlans)
            throw std::logic_error("Force production is not configured for this session");
        if (!world.IsAlive(account) || !world.Get<engine::gameplay::rts::economy::ResourceBalance>(account))
            throw std::invalid_argument("Force production controller requires a live account balance");
        if (world.Has<engine::gameplay::rts::ai::ForceProductionController>(account))
            throw std::logic_error("Force production controller is already registered");
        if (!forceProductionPlans->FindPolicy(policyKey))
            throw std::invalid_argument("Force production controller references unknown policy");
        if (!world.Add<engine::gameplay::rts::ai::ForceProductionController>(account))
            throw std::logic_error("Force production controller could not be attached");
        auto *controller=world.Get<engine::gameplay::rts::ai::ForceProductionController>(account);
        if (!controller)
            throw std::logic_error("Force production controller attachment was not visible");
        *controller={policyKey,0,enabled};
    }
    ecs::Entity CreateAccount(std::uint32_t balance, std::uint64_t unlockCredits = 0,
        std::uint64_t intrinsicScienceCredits = 0,
        engine::gameplay::rts::rank::RankSkillAwardModifier skillAwardModifier = {})
    {
        RequireBoundary();
        if (cratesConfigured && (crateAccountPoliciesFrozen || lastTick))
            throw std::logic_error("Crate-enabled accounts cannot be created after the first simulation tick");
        if (!unlockCatalog_ && unlockCredits)
            throw std::invalid_argument("Unlock credits require an injected unlock catalog");
        if (!rankCatalog_ && intrinsicScienceCredits)
            throw std::invalid_argument("Intrinsic rank credits require an injected rank catalog");
        engine::gameplay::rts::rank::ValidateRankSkillAwardModifier(skillAwardModifier);
        if (!rankCatalog_ && (skillAwardModifier.numerator != 1 || skillAwardModifier.denominator != 1))
            throw std::invalid_argument("Rank skill-award modifiers require an injected rank catalog");
        if (visibilityDefinitions && !participants.CanEnroll())
            throw std::length_error("Visibility participant slot capacity exhausted");
        ecs::Entity entity{};
        if (cashBountyCatalog)
            entity=unlockCatalog_
                ? (rankCatalog_
                    ? world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                        power::PowerLedger,power::PowerState,engine::gameplay::rts::unlocks::UnlockState,
                        engine::gameplay::rts::unlocks::UnlockInbox,engine::gameplay::rts::rank::RankState,
                        engine::gameplay::rts::rank::RankInbox,engine::gameplay::rts::rank::RankSkillAwardModifier,
                        engine::gameplay::rts::bounty::BountyPolicy>()
                    : world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                    power::PowerLedger,power::PowerState,engine::gameplay::rts::unlocks::UnlockState,
                    engine::gameplay::rts::unlocks::UnlockInbox,engine::gameplay::rts::bounty::BountyPolicy>())
                : world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                    power::PowerLedger,power::PowerState,engine::gameplay::rts::bounty::BountyPolicy>();
        else
            entity=unlockCatalog_
                ? (rankCatalog_
                    ? world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                        power::PowerLedger,power::PowerState,engine::gameplay::rts::unlocks::UnlockState,
                        engine::gameplay::rts::unlocks::UnlockInbox,engine::gameplay::rts::rank::RankState,
                        engine::gameplay::rts::rank::RankInbox,engine::gameplay::rts::rank::RankSkillAwardModifier>()
                    : world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                    power::PowerLedger,power::PowerState,engine::gameplay::rts::unlocks::UnlockState,
                    engine::gameplay::rts::unlocks::UnlockInbox>())
                : world.Create<engine::gameplay::rts::economy::ResourceBalance,economy::IncomeHistory,economy::AccountRange,
                    power::PowerLedger,power::PowerState>();
        world.Get<engine::gameplay::rts::economy::ResourceBalance>(entity)->quantity=balance;
        if (unlockCatalog_)
            world.Get<engine::gameplay::rts::unlocks::UnlockState>(entity)->credits=unlockCredits;
        if (rankCatalog_)
        {
            *world.Get<engine::gameplay::rts::rank::RankSkillAwardModifier>(entity) = skillAwardModifier;
            engine::gameplay::rts::rank::InitializeRankAccount(
                *world.Get<engine::gameplay::rts::rank::RankState>(entity),
                *world.Get<engine::gameplay::rts::unlocks::UnlockState>(entity),
                *rankCatalog_, intrinsicScienceCredits);
        }
        if (visibilityDefinitions) (void)participants.Enroll(entity);
        return entity;
    }

    void RegisterCrateAccountPolicy(ecs::Entity account, const bool human, const bool neutral)
    {
        RequireBoundary();
        if (!cratesConfigured)
            throw std::logic_error("Crate account policies require configured crates");
        if (crateAccountPoliciesFrozen || lastTick)
            throw std::logic_error("Crate account policies are closed after the first simulation tick");
        if (!world.IsAlive(account) || !world.Get<economy::ResourceBalance>(account))
            throw std::invalid_argument("Crate account policy requires a live account balance");
        crateAccountPolicies.Register(account,human,neutral);
        cratePolicyAccounts.push_back(account);
    }

    ecs::Entity CreateCrate(crates::CrateState state)
    {
        RequireBoundary();
        if (lastTick || crateAccountPoliciesFrozen)
            throw std::logic_error("Crates can only be created during startup before the first simulation tick");
        if (!cratesConfigured || !crateDefinitionCatalog)
            throw std::logic_error("Crates are not configured");
        (void)crateDefinitionCatalog->Get(state.definition);
        if (state.ownerAccount.IsValid()
            && (!world.IsAlive(state.ownerAccount) || !world.Get<economy::ResourceBalance>(state.ownerAccount)))
            throw std::invalid_argument("Crate owner must be a live account balance");
        if (state.placement != crates::CratePlacement::Grounded
            && state.placement != crates::CratePlacement::Airborne)
            throw std::invalid_argument("Crate placement is invalid");
        try
        {
            const auto entity=world.Create<crates::CrateState>();
            *world.Get<crates::CrateState>(entity)=state;
            return entity;
        }
        catch (...) { failed=true; throw; }
    }
    ecs::Entity CreateProducer(ecs::Entity account, std::uint32_t limit=9, std::uint64_t health=1000,
        engine::gameplay::navigation::Cell spawn=0,
        std::optional<production::ProductionExitDefinition> exit=std::nullopt)
    {
        RequireBoundary();
        if (!grid.Walkable(spawn)) throw std::invalid_argument("Producer spawn must be on traversable terrain");
        const auto timing=exit?std::optional{production::AuthorProductionExit(*exit,*step)}:std::nullopt;
        try
        {
            const auto entity=production::CreateProducer(world,ProductionBoundaryState::Ready,inputCapacity,account,limit,health,spawn);
            world.Add<match::VictoryAsset>(entity); world.Get<match::VictoryAsset>(entity)->account=account;
            if(timing)
            {
                world.Add<production::ProductionExitState>(entity);
                world.Add<production::ProductionExitTiming>(entity);
                *world.Get<production::ProductionExitTiming>(entity)=*timing;
            }
            return entity;
        }
        catch (...) { failed=true; throw; }
    }
    ecs::Entity CreateCompletedStructure(ecs::Entity account, std::uint32_t definition,
        engine::gameplay::navigation::Cell spawn=0)
    {
        RequireBoundary();
        if (lastTick)
            throw std::logic_error("Initial structures must be created before the first simulation tick");
        try
        {
            return construction::CreateCompletedStructure(world, grid, *buildings, repairDefinitions.get(),
                account, definition, spawn, engine::time::SimulationTime{0, *step},demolitionCatalog.get());
        }
        catch (...) { failed=true; throw; }
    }
    ecs::Entity CreateSupplySource(std::uint32_t boxes,engine::gameplay::navigation::Cell cell,std::uint64_t health=1000)
    {
        RequireBoundary();
        if(!health || !grid.Walkable(cell)) throw std::invalid_argument("Supply source requires positive health and traversable position");
        try
        {
            using namespace engine::gameplay::combat;
            const auto entity=world.Create<engine::gameplay::rts::harvesting::SupplySource,
                engine::gameplay::navigation::GridPosition,Health,HealthCapacity,LifeState,PendingDamage,DamageResult,PendingHealing,HealingResult>();
            *world.Get<engine::gameplay::rts::harvesting::SupplySource>(entity)={boxes,true};
            world.Get<engine::gameplay::navigation::GridPosition>(entity)->cell=cell;
            world.Get<Health>(entity)->current=health; world.Get<HealthCapacity>(entity)->maximum=health;
            return entity;
        }
        catch (...) { failed=true; throw; }
    }
    ecs::Entity CreateSupplyDropoff(ecs::Entity account,engine::gameplay::navigation::Cell cell,
        std::uint32_t valuePerBox,std::uint64_t health=1000)
    {
        RequireBoundary();
        if(!health || !grid.Walkable(cell) || !world.Has<engine::gameplay::rts::economy::ResourceBalance>(account))
            throw std::invalid_argument("Supply dropoff requires a live account, positive health and traversable position");
        try
        {
            using namespace engine::gameplay::combat;
            const auto entity=world.Create<engine::gameplay::rts::harvesting::SupplyDropoff,harvesting::SupplyDropoffOwner,construction::Structure,match::VictoryAsset,
                engine::gameplay::rts::repair::ManualRepairBenefactorLease,
                engine::gameplay::navigation::GridPosition,Health,HealthCapacity,LifeState,PendingDamage,DamageResult,PendingHealing,HealingResult>();
            *world.Get<harvesting::SupplyDropoffOwner>(entity)={account,valuePerBox};
            *world.Get<construction::Structure>(entity)={account,0,0,0,true,true,valuePerBox};
            *world.Get<match::VictoryAsset>(entity)={account};
            world.Get<engine::gameplay::navigation::GridPosition>(entity)->cell=cell;
            world.Get<Health>(entity)->current=health; world.Get<HealthCapacity>(entity)->maximum=health;
            return entity;
        }
        catch (...) { failed=true; throw; }
    }
    void BeginMatch(std::span<const ecs::Entity> roster)
    {
        RequireBoundary();
        if (matchEntity.IsValid()) throw std::logic_error("Match start is one-shot");
        try { matchEntity=match::BeginMatch(world,roster,inputCapacity); }
        // BeginMatch validates the complete external roster before any writes.
        // Rejected input is recoverable; structural/allocation failure is not.
        catch (const std::invalid_argument &) { throw; }
        catch (...) { failed=true; throw; }
    }
    MatchOutcome Outcome() const { return match::ReadMatchOutcome(world,matchEntity); }
    bool CancelBuild(ecs::Entity producer, ecs::Entity entry)
    {
        RequireBoundary();
        try { return production::CancelBuild(world,ProductionBoundaryState::Ready,inputCapacity,producer,entry); }
        catch (...) { failed=true; throw; }
    }
    const ecs::SystemRegistry &Systems() const noexcept { return registry; }
    void EnableRegeneration(ecs::Entity entity, std::uint32_t definition,
        engine::time::SimulationTime time, engine::time::Duration phase, bool active=true)
    {
        RequireBoundary();
        using namespace engine::gameplay::combat;
        using namespace engine::gameplay::combat::regeneration;
        if (!world.Has<Health>(entity) || !world.Has<HealthCapacity>(entity) || !world.Has<LifeState>(entity)
            || !world.Has<DamageResult>(entity) || !world.Has<PendingHealing>(entity) || !world.Has<HealingResult>(entity))
            throw std::invalid_argument("Regeneration requires a complete health entity");
        const auto id=regenerationDefinitions->Id(definition);
        const auto state=StartRegeneration(*regenerationDefinitions,id,time,phase,active);
        try
        {
            world.Add<RegenerationBinding>(entity); world.Add<RegenerationState>(entity);
            *world.Get<RegenerationBinding>(entity)={id}; *world.Get<RegenerationState>(entity)=state;
        }
        catch (...) { failed=true; throw; }
    }
    void EnrollProgression(ecs::Entity entity, std::uint32_t definition, bool trainable)
    {
        RequireBoundary();
        using namespace engine::gameplay::progression;
        (void)progressionCatalog.Get(definition);
        if (!world.IsAlive(entity) || world.Has<ProgressionState>(entity))
            throw std::invalid_argument("Progression enrollment requires a live unassigned entity");
        try
        {
            world.Add<ProgressionState>(entity); world.Add<ProgressionDefinitionRef>(entity);
            world.Add<ProgressionEligibility>(entity); world.Add<ProgressionInbox>(entity);
            *world.Get<ProgressionDefinitionRef>(entity)={definition};
            *world.Get<ProgressionEligibility>(entity)={trainable};
        }
        catch (...) { failed=true; throw; }
    }
    std::span<const engine::gameplay::progression::ExperienceResult> ExperienceResults() const noexcept
    { return progressionBatch.Results(); }
    std::span<const construction::ConstructionReceipt> ConstructionResults() const noexcept
    { return constructionReceipts.Receipts(); }
    std::span<const selling::SellReceipt> SaleReceipts() const noexcept { return saleBatch.Receipts(); }
    std::span<const selling::SellResult> SaleResults() const noexcept { return saleBatch.Results(); }
    std::span<const engine::gameplay::containment::ContainmentResult> ContainmentResults() const noexcept { return containmentBatch.Results(); }
    std::span<const engine::gameplay::containment::ContainmentResult> GarrisonResults() const noexcept { return garrisonBatch.Results(); }
    std::span<const production::rally::RallyReceipt> RallyReceipts() const noexcept { return rallyBatch.Receipts(); }
    std::span<const capture::CaptureResult> CaptureResults() const noexcept { return captureBatch.Results(); }
    std::span<const engine::gameplay::combat::damage::ResolvedHit> AcceptedHits() const noexcept { return acceptedHits.Results(); }
    std::span<const engine::gameplay::rts::unlocks::UnlockReceipt> UnlockReceipts() const noexcept
    { return unlockBatch.Results(); }
    std::span<const engine::gameplay::rts::rank::RankResult> RankResults() const noexcept
    { return rankBatch.Results(); }
    std::span<const generalszh::bounty::CashBountyAward> CashBountyAwards() const noexcept
    { return bountyBatch.SettledAwards(); }
    const ecs::DependencyGraph &Graph() const noexcept { return scheduler.Graph(); }
    const ecs::ExecutionPlan &Plan() const noexcept { return scheduler.Plan(); }
    std::span<const BuildReceipt> Execute(engine::time::SimulationTime time, GameInputs inputs={})
    {
        RequireBoundary();
        try
        {
            if (time.Step()!=*step || (lastTick && (time.Tick()<=*lastTick || time.Tick()-*lastTick!=1)))
                throw std::invalid_argument("Game requires consecutive fixed simulation ticks");
            if (inputs.damage.size()>inputCapacity || inputs.healing.size()>inputCapacity || inputs.poison.size()>inputCapacity)
                throw std::length_error("Game health input capacity exhausted");
            if (inputs.manualRepair.size()>inputCapacity)
                throw std::length_error("Manual repair input capacity exhausted");
            if (inputs.uncloak.size()>inputCapacity)
                throw std::length_error("Uncloak input capacity exhausted");
            if (inputs.unlocks.size()>inputCapacity)
                throw std::length_error("Unlock input capacity exhausted");
            if (inputs.rankAwards.size()>inputCapacity)
                throw std::length_error("Rank input capacity exhausted");
            if (inputs.crateContacts.size()>inputCapacity)
                throw std::length_error("Crate contact capacity exhausted");
            if (!inputs.unlocks.empty() && !unlockSystem)
                throw std::logic_error("Unlock requests require an injected unlock catalog");
            if (!inputs.rankAwards.empty() && !rankSystem)
                throw std::logic_error("Rank awards require an injected rank catalog");
            if (cratesConfigured)
            {
                FreezeCrateAccountPolicies();
                crateClaim->SetInputs(inputs.crateContacts);
            }
            else if (!inputs.crateContacts.empty())
                throw std::logic_error("Crate contacts require configured crates");
            // Consumers from the previous tick have joined at this boundary;
            // publication ownership stays in the root and is released exactly
            // once before the next ManualRepairSystem recording.
            acceptedRepairActors.Release();
            healthInput.SetInputs(inputs.damage,inputs.healing); poisonInput.SetInputs(inputs.poison);
            if (unlockSystem) unlockBatch.SetInputs(inputs.unlocks);
            economy.SetInputs(inputs.accounts); buildBatch.StageInputs(world,ProductionBoundaryState::Ready,inputs.builds);
            orders.SetInputs(inputs.moves,inputs.attacks,inputs.orders);
            concealment->SetInputs(inputs.uncloak);
            progressionBatch.SetInputs(inputs.experience);
            if (rankSystem) rankBatch.SetInputs(inputs.rankAwards);
            constructionRequests.SetInputs(inputs.construction,inputs.constructionCancellations);
            taskSelection.SetInputs(inputs.moves,inputs.attacks,inputs.orders,inputs.constructionCancellations,inputs.capture);
            StageRepairOverrides(inputs);
            manualRepair->SetInputs(inputs.manualRepair,repairOverrides,inputs.constructionCancellations);
            harvest.SetInputs(inputs.harvest,taskSelection.ManualActors());
            saleBatch.SetInputs(inputs.sales);
            containmentBatch.SetInputs(inputs.containment);
            garrisonBatch.SetInputs(inputs.garrison);
            transport->SetInterruptions(taskSelection.BoardingOverrides());
            rallyBatch.SetInputs(inputs.rally);
            captureBatch.SetInputs(inputs.capture);
            captureSystem->SetInterruptions(taskSelection.ManualActors());
            demolitionInputs.SetInputs(inputs.demolition);
            scheduler.Execute(time); lastTick=time.Tick();
            return buildBatch.Receipts();
        }
        catch (...)
        {
            if (publishedContacts.IsPublished())
                publishedContacts.Release();
            failed=true;
            throw;
        }
    }
private:
    void StageRepairOverrides(const GameInputs &inputs)
    {
        repairOverrides.clear();
        const auto append=[&](ecs::Entity actor) {
            if(!actor.IsValid()) return;
            if(repairOverrides.size()==repairOverrideCapacity)
                throw std::length_error("Manual repair interruption capacity exhausted");
            repairOverrides.push_back(actor);
        };
        for(const auto actor:taskSelection.ManualActors()) append(actor);
        for(const auto &input:inputs.construction) append(input.builder);
        for(const auto &input:inputs.harvest) append(input.actor);
        std::sort(repairOverrides.begin(),repairOverrides.end(),[](ecs::Entity left,ecs::Entity right) {
            return std::tie(left.index,left.generation)<std::tie(right.index,right.generation);
        });
        repairOverrides.erase(std::unique(repairOverrides.begin(),repairOverrides.end()),repairOverrides.end());
    }

    void FreezeCrateAccountPolicies()
    {
        if (!cratesConfigured || crateAccountPoliciesFrozen)
            return;
        bool missing=false;
        ecs::Query<ecs::Read<economy::ResourceBalance>> query(world);
        query.ForEachChunk([&](auto chunk) {
            for (std::size_t row=0; row!=chunk.Count(); ++row)
            {
                const auto account=chunk.Entities()[row];
                if (std::find(cratePolicyAccounts.begin(),cratePolicyAccounts.end(),account)==cratePolicyAccounts.end())
                {
                    missing=true;
                }
            }
        });
        if (missing)
            throw std::invalid_argument("Every live crate account requires an explicit pickup policy");
        crateAccountPolicies.Freeze();
        crateAccountPoliciesFrozen=true;
    }

    void CopyForceProductionPolicies(std::span<const ForceProductionPolicyInput> source,
        ForceProductionPlanLimits limits)
    {
        std::size_t candidateCount{};
        std::size_t entryCount{};
        const auto addCount=[](std::size_t &total, const std::size_t value,
            const std::size_t limit, const char *message)
        {
            if (value > limit - total)
                throw std::length_error(message);
            total += value;
        };
        if (source.size() > limits.maxPolicies)
            throw std::length_error("Force production policy capacity exhausted");
        for (const auto &policy : source)
        {
            addCount(candidateCount, policy.candidates.size(), limits.maxCandidates,
                "Force production candidate capacity exhausted");
            for (const auto &candidate : policy.candidates)
                addCount(entryCount, candidate.entries.size(), limits.maxEntries,
                    "Force production entry capacity exhausted");
        }

        authoredForceProductionPolicies.clear();
        authoredForceProductionCandidates.clear();
        authoredForceProductionEntries.clear();
        authoredForceProductionPolicies.reserve(source.size());
        authoredForceProductionCandidates.reserve(candidateCount);
        authoredForceProductionEntries.reserve(entryCount);

        for (const auto &policy : source)
        {
            const auto candidateOffset=authoredForceProductionCandidates.size();
            for (const auto &candidate : policy.candidates)
            {
                const auto entryOffset=authoredForceProductionEntries.size();
                authoredForceProductionEntries.insert(authoredForceProductionEntries.end(),
                    candidate.entries.begin(),candidate.entries.end());
                authoredForceProductionCandidates.push_back({candidate.stableKey,
                    candidate.productionPriority,candidate.retryCooldown,{}});
                auto &ownedCandidate=authoredForceProductionCandidates.back();
                ownedCandidate.entries=candidate.entries.empty()
                    ? std::span<const engine::gameplay::rts::ai::ForceProductionEntry>{}
                    : std::span<const engine::gameplay::rts::ai::ForceProductionEntry>(
                        authoredForceProductionEntries.data()+entryOffset,candidate.entries.size());
            }
            authoredForceProductionPolicies.push_back({policy.stableKey,policy.version,
                policy.fallbackCooldown,{}});
            auto &ownedPolicy=authoredForceProductionPolicies.back();
            ownedPolicy.candidates=policy.candidates.empty()
                ? std::span<const engine::gameplay::rts::ai::ForceProductionCandidateInput>{}
                : std::span<const engine::gameplay::rts::ai::ForceProductionCandidateInput>(
                    authoredForceProductionCandidates.data()+candidateOffset,policy.candidates.size());
        }
    }

    void ConfigureVisibility(const engine::time::FixedStep rate)
    {
        struct Source final
        {
            std::uint32_t key{};
            bool building{};
            std::size_t index{};
        };
        std::vector<Source> sources;
        sources.reserve(definitions.size() + authoredBuildings.size());
        for (std::size_t index = 0; index != definitions.size(); ++index)
            if (definitions[index].visibility) sources.push_back({definitions[index].key, false, index});
        for (std::size_t index = 0; index != authoredBuildings.size(); ++index)
            if (authoredBuildings[index].visibility) sources.push_back({authoredBuildings[index].key, true, index});

        authoredVisibility.clear();
        if (sources.empty())
        {
            // Fog-disabled fixtures must choose this policy at startup.  A
            // target query never infers unrestricted visibility from an
            // unbound or first-use page view.
            visibilityView.SetStartupUnrestricted();
            return;
        }
        if (sources.size() > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
            throw std::length_error("Visibility definition ID capacity exhausted");
        std::stable_sort(sources.begin(), sources.end(), [](const Source left, const Source right) {
            if (left.key != right.key) return left.key < right.key;
            if (left.building != right.building) return left.building < right.building;
            return left.index < right.index;
        });
        authoredVisibility.reserve(sources.size());
        for (std::size_t index = 0; index != sources.size(); ++index)
        {
            auto *binding = sources[index].building
                ? &authoredBuildings[sources[index].index].visibility
                : &definitions[sources[index].index].visibility;
            const auto authored = **binding;
            **binding = engine::gameplay::rts::visibility::CompileVisibilityDefinition(
                static_cast<std::uint32_t>(index), authored.shroudClearingRadiusCells,
                authored.constructionRadiusCells, authored.unlookPersistDuration, rate);
            authoredVisibility.push_back(**binding);
        }
        visibilityDefinitions = std::make_unique<engine::gameplay::rts::visibility::VisibilityDefinitions>(
            std::span<const engine::gameplay::rts::visibility::VisibilityDefinition>(authoredVisibility.data(), authoredVisibility.size()), rate);
        constexpr std::size_t reservedRootEntityCapacity = 1; // BeginMatch's match entity.
        visibilitySessionBudget = engine::gameplay::rts::visibility::FitVisibilitySessionBudget(
            inputCapacity, visibilityTopology.Count(), maximumVisibilityObservers_, reservedRootEntityCapacity,
            visibilityDefinitions->MaximumGraceTicks(), maximumVisibilityRegionTransitions_);
        CreateVisibilityPagePool();
        visibilityPolicy = std::make_unique<generalszh::visibility::VisibilityPolicySystem>(
            world, *visibilityDefinitions, participants, inputCapacity);
        visibilitySystem = std::make_unique<engine::gameplay::rts::visibility::VisibilitySystem>(
            world, *visibilityDefinitions, visibilityTopology, visibilitySessionBudget->capacity);
    }

    void CreateVisibilityPagePool()
    {
        const auto &pages = visibilitySessionBudget->pages;
        if (world.EntityCount() != 0)
            throw std::logic_error("Visibility page pool must be created before gameplay entities");
        if (pages.visibilityPageEntityCount > inputCapacity)
            throw std::length_error("Visibility page pool exceeds the root entity-index budget");

        // Preallocate the complete root index range once.  Destroying the
        // high suffix first lets the page entities reuse only those indices;
        // the remaining free prefix is the bounded gameplay-entity range.
        std::vector<ecs::Entity> placeholders;
        placeholders.reserve(inputCapacity);
        for (std::size_t index = 0; index != inputCapacity; ++index)
            placeholders.push_back(world.Create<>());
        for (std::size_t index = 0; index != pages.visibilityPageEntityCount; ++index)
            if (!world.Destroy(placeholders[inputCapacity - 1 - index]))
                throw std::logic_error("Visibility page placeholder retirement failed");

        visibilityPageEntities.clear();
        visibilityPageEntities.reserve(pages.visibilityPageEntityCount);
        visibilityCellIndexes.clear();
        visibilityCellIndexes.reserve(pages.cellPageCount);
        visibilityVisiblePages.clear();
        visibilityVisiblePages.reserve(pages.cellPageCount);

        for (std::size_t page = 0; page != pages.cellPageCount; ++page)
        {
            const auto first = page * engine::gameplay::rts::visibility::VisibilityPageLanes;
            const auto remaining = static_cast<std::size_t>(visibilityTopology.Count()) - first;
            const auto count = (std::min)(remaining,
                static_cast<std::size_t>(engine::gameplay::rts::visibility::VisibilityPageLanes));
            const auto entity = world.Create<
                engine::gameplay::rts::visibility::VisibilityCellIndex,
                engine::gameplay::rts::visibility::VisibilityExploredMaskPage,
                engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>();
            visibilityPageEntities.push_back(entity);
            auto *index = world.Get<engine::gameplay::rts::visibility::VisibilityCellIndex>(entity);
            auto *visible = world.Get<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>(entity);
            if (!index || !visible) throw std::logic_error("Visibility cell page creation was incomplete");
            *index = {static_cast<std::uint32_t>(first), static_cast<std::uint16_t>(count), 0};
            visibilityCellIndexes.push_back(index);
            visibilityVisiblePages.push_back(visible);
        }
        for (std::size_t page = 0; page != pages.historyPageCount; ++page)
        {
            const auto entity = world.Create<engine::gameplay::rts::visibility::VisibilityHistoryPage>();
            visibilityPageEntities.push_back(entity);
            auto *value = world.Get<engine::gameplay::rts::visibility::VisibilityHistoryPage>(entity);
            if (!value) throw std::logic_error("Visibility history page creation was incomplete");
            value->pageIndex = static_cast<std::uint32_t>(page);
        }
        for (std::size_t page = 0; page != pages.leasePageCount; ++page)
        {
            const auto entity = world.Create<engine::gameplay::rts::visibility::VisibilityLeasePage>();
            visibilityPageEntities.push_back(entity);
            auto *value = world.Get<engine::gameplay::rts::visibility::VisibilityLeasePage>(entity);
            if (!value) throw std::logic_error("Visibility lease page creation was incomplete");
            value->pageIndex = static_cast<std::uint32_t>(page);
        }
        for (std::size_t index = 0; index != inputCapacity - pages.visibilityPageEntityCount; ++index)
            if (!world.Destroy(placeholders[index]))
                throw std::logic_error("Visibility gameplay placeholder retirement failed");

        // The root retains these page entities and never adds/removes
        // components or destroys them after this bind.  The read view is only
        // a nonowning cache of these stable column addresses; ECS page
        // components remain the sole authoritative masks/history/leases.
        visibilityView.Bind(
            std::span<const engine::gameplay::rts::visibility::VisibilityCellIndex * const>(
                visibilityCellIndexes.data(), visibilityCellIndexes.size()),
            std::span<const engine::gameplay::rts::visibility::VisibilityVisibleMaskPage * const>(
                visibilityVisiblePages.data(), visibilityVisiblePages.size()));
    }

    void RegisterSystems()
    {
        using namespace engine::gameplay::combat;
        RegisterHealthSystems(registry,healthInput,health,healing);
        if (contactSystem)
            registry.Register(*contactSystem,ecs::SystemPhase::Simulation);
        if (crateClaim)
            registry.Register(*crateClaim,ecs::SystemPhase::Simulation);
        if (crateReward)
            registry.Register(*crateReward,ecs::SystemPhase::Simulation);
        registry.Register(timedLife,ecs::SystemPhase::Simulation);
        registry.Register(*regeneration,ecs::SystemPhase::Simulation);
        registry.Register(*baseRepair,ecs::SystemPhase::Simulation);
        registry.Register(*manualRepair,ecs::SystemPhase::Simulation);
        registry.Register(killExperience,ecs::SystemPhase::Simulation);
        registry.Register(progression,ecs::SystemPhase::Simulation);
        registry.Register(harvest,ecs::SystemPhase::Simulation);
        if (cashBountyPolicy)
            registry.Register(*cashBountyPolicy,ecs::SystemPhase::Simulation);
        if (cashBountyAwards)
            registry.Register(*cashBountyAwards,ecs::SystemPhase::Simulation);
        RegisterPoisonSystems(registry,poisonInput,periodic,cleanse);
        if (rankSystem)
            registry.Register(*rankSystem,ecs::SystemPhase::Simulation);
        if (unlockSystem)
            registry.Register(*unlockSystem,ecs::SystemPhase::Simulation);
        production::RegisterProductionSystems(registry,*admission,*production,*productionExits);
        registry.Register(*initialPayload,ecs::SystemPhase::Simulation);
        construction::RegisterConstructionSystems(registry,*placement,builderTasks,construction);
        registry.Register(*demolitionTrap,ecs::SystemPhase::Simulation);
        registry.Register(taskSelection,ecs::SystemPhase::Simulation);
        registry.Register(*sellingSystem,ecs::SystemPhase::Simulation);
        registry.Register(*transport,ecs::SystemPhase::Simulation);
        registry.Register(rally,ecs::SystemPhase::Simulation);
        registry.Register(*captureUpgradeActivation,ecs::SystemPhase::Simulation);
        registry.Register(*captureSystem,ecs::SystemPhase::Simulation);
        registry.Register(*garrison,ecs::SystemPhase::Simulation);
        if (forceProduction)
            registry.Register(*forceProduction,ecs::SystemPhase::Simulation);
        registry.Register(movement,ecs::SystemPhase::Simulation);
        if (visibilityPolicy)
        {
            registry.Register(*visibilityPolicy,ecs::SystemPhase::Simulation);
            registry.Register(*visibilitySystem,ecs::SystemPhase::Simulation);
        }
        registry.Register(*stealthPolicy,ecs::SystemPhase::Simulation);
        registry.Register(*concealment,ecs::SystemPhase::Simulation);
        registry.Register(accounts,ecs::SystemPhase::Simulation); registry.Register(economy,ecs::SystemPhase::Simulation);
        registry.Register(power,ecs::SystemPhase::Simulation);
        registry.Register(projectiles,ecs::SystemPhase::PreSimulation);
        registry.Register(acquisition,ecs::SystemPhase::Simulation); registry.Register(targeting,ecs::SystemPhase::Simulation); registry.Register(weapon,ecs::SystemPhase::Simulation);
        registry.Register(victory,ecs::SystemPhase::Simulation); registry.Register(defeat,ecs::SystemPhase::Simulation);
        registry.Register(orders,ecs::SystemPhase::Simulation); registry.Register(eligibility,ecs::SystemPhase::Simulation);
        registry.Register(*deathWeapons,ecs::SystemPhase::PostSimulation);
        // Actual semantic/resource dependencies. No registration-order semantics.
        using ImpactProjectileSystem = engine::gameplay::combat::ProjectileSystemT<combat::ImpactTargetProjection>;
        using LaunchWeaponSystem = engine::gameplay::combat::WeaponSystemT<combat::LaunchSnapshotView>;
        if (crateClaim && crateReward)
        {
            // RegisterHealthSystems places the health input/result pipeline in
            // this root's Simulation phase. Crate claim/reward therefore form
            // the early post-health, pre-kill/progression prefix of the same
            // phase; their publication remains visible through explicit edges
            // without moving the rest of the health pipeline.
            if (contactSystem)
            {
                registry.OrderBefore<HealthSystem,
                    engine::gameplay::spatial::contacts::ContactSystem>();
                registry.OrderBefore<engine::gameplay::spatial::contacts::ContactSystem,
                    crates::CrateClaimSystem>();
            }
            registry.OrderBefore<crates::CrateClaimSystem,crates::CrateRewardSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,HealingSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,
                engine::gameplay::combat::regeneration::RegenerationSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,repair::BaseRepairSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,repair::ManualRepairSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,progression::KillExperienceSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,
                engine::gameplay::progression::ProgressionSystem>();
            registry.OrderBefore<crates::CrateRewardSystem,EconomySystem>();
            if (unlockSystem)
                registry.OrderBefore<crates::CrateClaimSystem,
                    engine::gameplay::rts::unlocks::UnlockSystem>();
        }
        registry.OrderBefore<ImpactProjectileSystem,HealthInputSystem>();
        registry.OrderBefore<HealthSystem,progression::KillExperienceSystem>();
        if (cashBountyAwards)
        {
            registry.OrderBefore<HealthSystem,generalszh::bounty::CashBountyAwardSystem>();
            registry.OrderBefore<generalszh::bounty::CashBountyAwardSystem,match::DefeatSystem>();
            registry.OrderBefore<generalszh::bounty::CashBountyAwardSystem,capture::CaptureSystem>();
            registry.OrderBefore<generalszh::bounty::CashBountyAwardSystem,construction::ConstructionSystem>();
            registry.OrderBefore<generalszh::bounty::CashBountyAwardSystem,EconomySystem>();
        }
        if (cashBountyPolicy)
        {
            if (rankSystem)
                registry.OrderBefore<generalszh::bounty::CashBountyPolicySystem,
                    engine::gameplay::rts::rank::RankSystem>();
            if (unlockSystem)
                registry.OrderBefore<generalszh::bounty::CashBountyPolicySystem,
                    engine::gameplay::rts::unlocks::UnlockSystem>();
        }
        if (rankSystem)
            registry.OrderBefore<progression::KillExperienceSystem,
                engine::gameplay::rts::rank::RankSystem>();
        // Defeat writes Health/LifeState for defeated accounts; lethal XP must observe
        // the canonical HealthSystem result and provenance before that cleanup masks it.
        registry.OrderBefore<progression::KillExperienceSystem,match::DefeatSystem>();
        // TimedLifeSystem is already Before<HealthSystem>, while RegenerationSystem is
        // already After<HealthSystem> and only reads the published damage result.
        // Capture and construction write the joined Structure owner/completion state;
        // keep both downstream so XP uses the pre-transfer/pre-completion observation.
        registry.OrderBefore<progression::KillExperienceSystem,capture::CaptureSystem>();
        registry.OrderBefore<progression::KillExperienceSystem,construction::ConstructionSystem>();
        registry.OrderBefore<progression::KillExperienceSystem,engine::gameplay::progression::ProgressionSystem>();
        registry.OrderBefore<HealthInputSystem,PoisonInputSystem>();
        registry.OrderBefore<PeriodicDamageSystem,HealthSystem>();
        registry.OrderBefore<PeriodicDamageSystem,TimedLifeSystem>();
        registry.OrderBefore<engine::gameplay::combat::regeneration::RegenerationSystem,repair::BaseRepairSystem>();
        registry.OrderBefore<repair::BaseRepairSystem,repair::ManualRepairSystem>();
        registry.OrderBefore<repair::ManualRepairSystem,engine::gameplay::rts::harvesting::HarvestSystem>();
        registry.OrderBefore<repair::ManualRepairSystem,tasks::TaskSelectionSystem>();
        registry.OrderBefore<engine::gameplay::rts::harvesting::HarvestSystem,tasks::TaskSelectionSystem>();
        registry.OrderBefore<HealingSystem,combat::PoisonCleanseSystem>();
        registry.OrderBefore<combat::PoisonCleanseSystem,match::VictorySystem>();
        registry.OrderBefore<match::VictorySystem,match::DefeatSystem>();
        registry.OrderBefore<match::DefeatSystem,power::PowerSystem>();
        registry.OrderBefore<match::DefeatSystem,engine::gameplay::rts::harvesting::HarvestSystem>();
        registry.OrderBefore<match::DefeatSystem,containment::InitialPayloadProductionSystem>();
        registry.OrderBefore<containment::InitialPayloadProductionSystem,containment::TransportContainmentSystem>();
        registry.OrderBefore<containment::InitialPayloadProductionSystem,production::ProductionExitSystem>();
        registry.OrderBefore<match::DefeatSystem,containment::TransportContainmentSystem>();
        registry.OrderBefore<containment::TransportContainmentSystem,engine::gameplay::rts::harvesting::HarvestSystem>();
        // Harvest writes the same movement actuators and position column as
        // garrison.  Retained garrison rows must be the final lifecycle
        // writer so ordinary harvest work cannot reintroduce movement.
        registry.OrderBefore<engine::gameplay::rts::harvesting::HarvestSystem,containment::GarrisonContainmentSystem>();
        registry.OrderBefore<containment::TransportContainmentSystem,tasks::TaskSelectionSystem>();
        registry.OrderBefore<containment::TransportContainmentSystem,containment::GarrisonContainmentSystem>();
        // Capture reads the pre-action actor position, transfers all supported
        // capability routes, then later waves observe the new owner this tick.
        // Research records and newborn units are deliberately downstream of
        // activation; their structural visibility is therefore next tick.
        registry.OrderBefore<capture::CaptureUpgradeActivationSystem,capture::CaptureSystem>();
        registry.OrderBefore<containment::TransportContainmentSystem,capture::CaptureUpgradeActivationSystem>();
        registry.OrderBefore<containment::TransportContainmentSystem,capture::CaptureSystem>();
        registry.OrderBefore<capture::CaptureSystem,containment::GarrisonContainmentSystem>();
        registry.OrderBefore<capture::CaptureSystem,power::PowerSystem>();
        registry.OrderBefore<capture::CaptureSystem,engine::gameplay::rts::harvesting::HarvestSystem>();
        registry.OrderBefore<capture::CaptureSystem,production::ProductionAdmissionSystem>();
        registry.OrderBefore<engine::gameplay::rts::harvesting::HarvestSystem,EconomySystem>();
        registry.OrderBefore<match::DefeatSystem,production::ProductionAdmissionSystem>();
        if (unlockSystem)
        {
            if (rankSystem)
                registry.OrderBefore<engine::gameplay::rts::rank::RankSystem,
                    engine::gameplay::rts::unlocks::UnlockSystem>();
            registry.OrderBefore<engine::gameplay::rts::unlocks::UnlockSystem,
                production::ProductionAdmissionSystem>();
            if (forceProduction)
                registry.OrderBefore<engine::gameplay::rts::unlocks::UnlockSystem,
                    generalszh::ai::ForceProductionSystem>();
        }
        registry.OrderBefore<power::PowerSystem,production::ProductionSystem>();
        registry.OrderBefore<economy::AccountSystem,production::ProductionAdmissionSystem>();
        registry.OrderBefore<economy::AccountSystem,selling::SellSystem>();
        registry.OrderBefore<selling::SellSystem,production::ProductionAdmissionSystem>();
        registry.OrderBefore<selling::SellSystem,production::rally::RallySystem>();
        registry.OrderBefore<production::rally::RallySystem,production::ProductionAdmissionSystem>();
        registry.OrderBefore<production::rally::RallySystem,production::ProductionSystem>();
        registry.OrderBefore<production::ProductionExitSystem,movement::MovementEligibilitySystem>();
        registry.OrderBefore<production::ProductionExitSystem,construction::ConstructionPlacementSystem>();
        // ProductionExit reads/writes the shared production membership
        // aggregate; garrison observes the completed exit projection.
        registry.OrderBefore<production::ProductionExitSystem,containment::GarrisonContainmentSystem>();
        if (forceProduction)
        {
            generalszh::ai::RegisterForceProductionOrdering(registry);
            registry.OrderBefore<generalszh::ai::ForceProductionSystem,
                containment::GarrisonContainmentSystem>();
        }
        registry.OrderBefore<construction::BuilderTaskSystem,tasks::TaskSelectionSystem>();
        // BuilderTask writes garrison's movement actuators before the retained
        // lifecycle pass.  Garrison then precedes ConstructionSystem: its
        // carrier snapshot observes existing buildings, while a completion
        // becomes eligible to garrison on the next tick.
        registry.OrderBefore<construction::BuilderTaskSystem,containment::GarrisonContainmentSystem>();
        registry.OrderBefore<containment::GarrisonContainmentSystem,construction::ConstructionSystem>();
        registry.OrderBefore<tasks::TaskSelectionSystem,OrderSystem>();
        registry.OrderBefore<movement::MovementEligibilitySystem,OrderSystem>();
        registry.OrderBefore<containment::GarrisonContainmentSystem,movement::MovementEligibilitySystem>();
        registry.OrderBefore<containment::GarrisonContainmentSystem,tasks::TaskSelectionSystem>();
        registry.OrderBefore<containment::GarrisonContainmentSystem,OrderSystem>();
        registry.OrderBefore<OrderSystem,engine::gameplay::navigation::MovementSystem>();
        if (visibilityPolicy)
        {
            using VisibilityPolicySystem = generalszh::visibility::VisibilityPolicySystem;
            using VisibilitySystem = engine::gameplay::rts::visibility::VisibilitySystem;
            // OrderSystem deliberately remains before MovementSystem.  Its
            // index join reads the previous committed visibility pages; on
            // tick zero those pages are explicitly zeroed.  Movement, life,
            // capture, construction and containment then feed this policy,
            // which publishes the current map before acquisition joins it.
            registry.OrderBefore<engine::gameplay::navigation::MovementSystem,VisibilityPolicySystem>();
            registry.OrderBefore<HealthSystem,VisibilityPolicySystem>();
            registry.OrderBefore<TimedLifeSystem,VisibilityPolicySystem>();
            registry.OrderBefore<production::ProductionExitSystem,VisibilityPolicySystem>();
            registry.OrderBefore<containment::TransportContainmentSystem,VisibilityPolicySystem>();
            registry.OrderBefore<containment::GarrisonContainmentSystem,VisibilityPolicySystem>();
            registry.OrderBefore<capture::CaptureSystem,VisibilityPolicySystem>();
            registry.OrderBefore<construction::ConstructionSystem,VisibilityPolicySystem>();
            registry.OrderBefore<containment::InitialPayloadProductionSystem,VisibilityPolicySystem>();
            registry.OrderBefore<demolition::DemolitionTrapSystem,VisibilityPolicySystem>();
            registry.OrderBefore<VisibilityPolicySystem,VisibilitySystem>();
            registry.OrderBefore<VisibilitySystem,combat::AcquisitionSystem>();
        }
        registry.OrderBefore<HealthSystem,generalszh::concealment::StealthPolicySystem>();
        registry.OrderBefore<TimedLifeSystem,generalszh::concealment::StealthPolicySystem>();
        registry.OrderBefore<capture::CaptureSystem,generalszh::concealment::StealthPolicySystem>();
        registry.OrderBefore<engine::gameplay::navigation::MovementSystem,generalszh::concealment::StealthPolicySystem>();
        registry.OrderBefore<generalszh::concealment::StealthPolicySystem,engine::gameplay::concealment::ConcealmentSystem>();
        registry.OrderBefore<engine::gameplay::concealment::ConcealmentSystem,combat::AcquisitionSystem>();
        registry.OrderBefore<engine::gameplay::concealment::ConcealmentSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<containment::InitialPayloadProductionSystem,generalszh::concealment::StealthPolicySystem>();
        registry.OrderBefore<engine::gameplay::navigation::MovementSystem,combat::AcquisitionSystem>();
        registry.OrderBefore<containment::GarrisonContainmentSystem,combat::AcquisitionSystem>();
        registry.OrderBefore<engine::gameplay::navigation::MovementSystem,construction::ConstructionSystem>();
        registry.OrderBefore<construction::ConstructionSystem,combat::AcquisitionSystem>();
        registry.OrderBefore<HealthSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<HealingSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<match::DefeatSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<capture::CaptureSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<selling::SellSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<engine::gameplay::navigation::MovementSystem,demolition::DemolitionTrapSystem>();
        // This explicit edge places the late trap wave between construction
        // completion and ordinary target publication/firing.
        registry.OrderBefore<construction::ConstructionSystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<demolition::DemolitionTrapSystem,combat::AcquisitionSystem>();
        // Victory remains before the late trap wave.  A trap killed here is
        // therefore visible to victory on the next tick, without a cycle.
        registry.OrderBefore<match::VictorySystem,demolition::DemolitionTrapSystem>();
        registry.OrderBefore<demolition::DemolitionTrapSystem,DemolitionDeathWeaponSystem>();
        registry.OrderBefore<combat::AcquisitionSystem,combat::TargetingSystem>();
        registry.OrderBefore<combat::TargetingSystem,LaunchWeaponSystem>();
    }
    void RequireBoundary() const
    {
        const auto status=Outcome().status;
        if (!step || failed || scheduler.IsFailed() || world.IsScheduledExecutionActive() || status==MatchStatus::Won || status==MatchStatus::Draw)
            throw std::logic_error("Game is not at a runnable boundary");
    }
    ecs::World &world;
    const engine::gameplay::navigation::NavigationGrid &grid;
    std::pmr::unsynchronized_pool_resource contactBatchMemory;
    engine::gameplay::spatial::PointGrid contactGrid;
    engine::events::PublishedBatch<engine::gameplay::spatial::contacts::ContactPair> publishedContacts;
    engine::gameplay::rts::visibility::VisibilityTopology visibilityTopology;
    engine::gameplay::rts::visibility::VisibilityReadView visibilityView;
    engine::gameplay::rts::visibility::VisibilityParticipantTable participants;
    std::vector<production::BuildDefinition> definitions;
    std::vector<generalszh::bounty::CashBountyDefinition> authoredCashBountyDefinitions;
    std::vector<engine::gameplay::rts::visibility::VisibilityDefinition> authoredVisibility;
    std::unique_ptr<engine::gameplay::rts::visibility::VisibilityDefinitions> visibilityDefinitions;
    std::optional<engine::gameplay::rts::visibility::VisibilitySessionBudget> visibilitySessionBudget;
    std::vector<ecs::Entity> visibilityPageEntities;
    std::vector<const engine::gameplay::rts::visibility::VisibilityCellIndex *> visibilityCellIndexes;
    std::vector<const engine::gameplay::rts::visibility::VisibilityVisibleMaskPage *> visibilityVisiblePages;
    const engine::gameplay::rts::unlocks::UnlockCatalog *unlockCatalog_{};
    const engine::gameplay::rts::rank::RankCatalog *rankCatalog_{};
    std::vector<engine::gameplay::concealment::ConcealmentDefinition> authoredConcealment;
    std::vector<engine::gameplay::concealment::DetectionDefinition> authoredDetection;
    std::vector<generalszh::concealment::StealthPolicy> authoredStealth;
    engine::gameplay::combat::HealthInputSystem healthInput;
    engine::gameplay::combat::HealthSystem health;
    engine::gameplay::combat::TimedLifeSystem timedLife;
    engine::gameplay::combat::HealingSystem healing;
    std::vector<engine::gameplay::combat::regeneration::RegenerationDefinition> authoredRegeneration;
    std::unique_ptr<engine::gameplay::combat::regeneration::RegenerationDefinitions> regenerationDefinitions;
    std::unique_ptr<engine::gameplay::combat::regeneration::RegenerationSystem> regeneration;
    engine::gameplay::progression::ProgressionCatalog progressionCatalog;
    generalszh::progression::VeterancyCatalog veterancyCatalog;
    engine::gameplay::progression::ProgressionBatch progressionBatch;
    engine::gameplay::rts::rank::RankBatch rankBatch;
    engine::gameplay::progression::ProgressionSystem progression;
    ecs::Entity matchEntity{};
    generalszh::progression::KillExperienceSystem killExperience;
    std::pmr::unsynchronized_pool_resource repairBatchMemory;
    engine::events::PublishedBatch<ecs::Entity> acceptedRepairActors;
    engine::gameplay::rts::harvesting::HarvestSnapshots harvestSnapshots;
    engine::gameplay::rts::harvesting::HarvestSystem harvest;
    harvesting::HarvestRevenue supplyRevenue;
    crates::CratePickupBatch cratePickupBatch;
    crates::CrateRewardBatch crateRewardBatch;
    crates::AccountPickupPolicies crateAccountPolicies;
    std::vector<ecs::Entity> cratePolicyAccounts;
    std::vector<crates::CrateDefinition> authoredCrateDefinitions;
    std::vector<crates::CollectorDefinition> authoredCrateUnitCollectors;
    std::vector<crates::CollectorDefinition> authoredCrateStructureCollectors;
    crates::CrateRewardLimits crateRewardLimits{};
    std::unique_ptr<crates::CrateDefinitionCatalog> crateDefinitionCatalog;
    std::unique_ptr<crates::CollectorDefinitionCatalog> crateCollectorCatalog;
    std::unique_ptr<engine::gameplay::spatial::contacts::ContactSystem> contactSystem;
    std::unique_ptr<crates::CrateClaimSystem> crateClaim;
    std::unique_ptr<crates::CrateRewardSystem> crateReward;
    bool cratesConfigured{};
    bool crateAccountPoliciesFrozen{};
    generalszh::bounty::CashBountyBatch bountyBatch;
    economy::AccountBatch accountBatch;
    economy::AccountSystem accounts;
    EconomySystem economy;
    BuildBatch buildBatch;
    engine::gameplay::rts::unlocks::UnlockBatch unlockBatch;
    // These rows are root-owned authoring storage. Their nested spans are
    // rebuilt only after the flat vectors have reserved their final sizes.
    std::vector<ForceProductionPolicyInput> authoredForceProductionPolicies;
    std::vector<engine::gameplay::rts::ai::ForceProductionCandidateInput> authoredForceProductionCandidates;
    std::vector<engine::gameplay::rts::ai::ForceProductionEntry> authoredForceProductionEntries;
    ForceProductionPlanLimits forceProductionLimits{};
    std::unique_ptr<engine::gameplay::rts::ai::ForceProductionPlanCatalog> forceProductionPlans;
    std::unique_ptr<generalszh::ai::ForceProductionSystem> forceProduction;
    bool forceProductionConfigured{};
    power::PowerFrame powerFrame;
    power::PowerSystem power;
    std::unique_ptr<production::BuildCatalog> catalog;
    std::unique_ptr<generalszh::bounty::CashBountyCatalog> cashBountyCatalog;
    std::unique_ptr<generalszh::bounty::CashBountyPolicySystem> cashBountyPolicy;
    std::unique_ptr<generalszh::bounty::CashBountyAwardSystem> cashBountyAwards;
    std::unique_ptr<engine::gameplay::rts::unlocks::UnlockSystem> unlockSystem;
    std::unique_ptr<engine::gameplay::rts::rank::RankSystem> rankSystem;
    std::unique_ptr<production::ProductionAdmissionSystem> admission;
    std::unique_ptr<production::ProductionSystem> production;
    std::unique_ptr<production::ProductionExitSystem> productionExits;
    std::unique_ptr<containment::InitialPayloadProductionSystem> initialPayload;
    engine::gameplay::navigation::FlowFields fields;
    engine::gameplay::navigation::MovementSystem movement;
    movement::MovementEligibilitySystem eligibility;
    std::unique_ptr<generalszh::visibility::VisibilityPolicySystem> visibilityPolicy;
    std::unique_ptr<engine::gameplay::rts::visibility::VisibilitySystem> visibilitySystem;
    std::unique_ptr<engine::gameplay::concealment::ConcealmentDefinitions> concealmentDefinitions;
    std::unique_ptr<engine::gameplay::concealment::DetectionDefinitions> detectionDefinitions;
    std::unique_ptr<generalszh::concealment::StealthPolicies> stealthPolicies;
    std::unique_ptr<engine::gameplay::concealment::DetectionIndex> detectionIndex;
    std::unique_ptr<generalszh::concealment::StealthPolicySystem> stealthPolicy;
    std::unique_ptr<engine::gameplay::concealment::ConcealmentSystem> concealment;
    combat::TargetIndex targets;
    combat::ImpactTargetProjection impactTargets;
    demolition::DemolitionTargetIndex demolitionTargets;
    demolition::DemolitionSourceProjection demolitionSources;
    demolition::DemolitionTrapInputBatch demolitionInputs;
    engine::gameplay::combat::damage::ArmorCatalog armorCatalog;
    engine::gameplay::combat::damage::AcceptedHitBatch acceptedHits;
    engine::gameplay::combat::ProjectileSystemT<combat::ImpactTargetProjection> projectiles;
    combat::AcquisitionSystem acquisition;
    combat::TargetingSystem targeting;
    engine::gameplay::combat::WeaponSystemT<combat::LaunchSnapshotView> weapon;
    match::DefeatFrame defeated;
    match::VictorySystem victory;
    match::DefeatSystem defeat;
    OrderSystem orders;
    PoisonInputSystem poisonInput;
    engine::gameplay::combat::PeriodicDamageSystem periodic;
    combat::PoisonCleanseSystem cleanse;
    std::vector<construction::BuildingDefinition> authoredBuildings;
    std::vector<demolition::DemolitionTrapDefinition> authoredDemolitionTraps;
    std::unique_ptr<construction::BuildingCatalog> buildings;
    std::unique_ptr<demolition::DemolitionTrapCatalog> demolitionCatalog;
    std::unique_ptr<engine::gameplay::combat::death::DeathWeaponCatalog> deathWeaponCatalog;
    construction::ConstructionRequests constructionRequests;
    construction::ConstructionReceipts constructionReceipts;
    engine::gameplay::rts::construction::BuilderFrame builderFrame;
    std::unique_ptr<construction::ConstructionPlacementSystem> placement;
    construction::BuilderTaskSystem builderTasks;
    construction::ConstructionSystem construction;
    std::unique_ptr<demolition::DemolitionTrapSystem> demolitionTrap;
    std::unique_ptr<DemolitionDeathWeaponSystem> deathWeapons;
    tasks::TaskSelectionSystem taskSelection;
    std::size_t repairOverrideCapacity{};
    std::vector<ecs::Entity> repairOverrides;
    std::vector<selling::SellDefinition> authoredSales;
    std::unique_ptr<selling::SellCatalog> saleCatalog;
    selling::SellBatch saleBatch;
    std::unique_ptr<selling::SellSystem> sellingSystem;
    std::vector<engine::gameplay::rts::repair::RepairDefinition> authoredRepairs;
    std::unique_ptr<engine::gameplay::rts::repair::RepairDefinitions> repairDefinitions;
    std::unique_ptr<repair::BaseRepairSystem> baseRepair;
    std::unique_ptr<repair::ManualRepairSystem> manualRepair;
    std::vector<engine::gameplay::containment::TransportDefinition> authoredTransports;
    std::unique_ptr<engine::gameplay::containment::TransportDefinitions> transportDefinitions;
    engine::gameplay::containment::ContainmentBatch containmentBatch;
    std::unique_ptr<containment::TransportContainmentSystem> transport;
    engine::gameplay::containment::ContainmentBatch garrisonBatch;
    std::unique_ptr<containment::GarrisonContainmentSystem> garrison;
    engine::gameplay::navigation::FlowFields rallyFields;
    production::rally::RallyBatch rallyBatch;
    production::rally::RallySystem rally;
    std::vector<capture::CaptureDefinition> authoredCapture;
    std::unique_ptr<capture::CaptureDefinitions> captureDefinitions;
    capture::CaptureBatch captureBatch;
    std::unique_ptr<capture::CaptureUpgradeActivationSystem> captureUpgradeActivation;
    std::unique_ptr<capture::CaptureSystem> captureSystem;
    ecs::SystemRegistry registry;
    ecs::Scheduler scheduler;
    std::size_t inputCapacity;
    engine::time::Duration manualRepairRetention;
    std::size_t maximumVisibilityObservers_{};
    std::size_t maximumVisibilityRegionTransitions_{};
    std::optional<engine::time::FixedStep> step;
    std::optional<std::uint64_t> lastTick;
    bool failed{};
};
GameSession::GameSession(ecs::World &world, engine::jobs::JobSystem &jobs,
    std::span<const production::BuildDefinition> definitions,
    const engine::gameplay::navigation::NavigationGrid &grid,
    std::size_t inputCapacity,
    std::span<const engine::gameplay::combat::regeneration::RegenerationDefinition> regenerationDefinitions,
    std::span<const engine::gameplay::progression::ProgressionDefinition> progressionDefinitions,
    std::span<const construction::BuildingDefinition> buildingDefinitions,
    std::span<const selling::SellDefinition> saleDefinitions,
    std::span<const engine::gameplay::rts::repair::RepairDefinition> repairDefinitions,
    std::span<const engine::gameplay::containment::TransportDefinition> transportDefinitions,
    std::span<const capture::CaptureDefinition> captureDefinitions,
    std::span<const engine::gameplay::combat::damage::DamageTypePolicy> damagePolicies,
    std::span<const engine::gameplay::combat::damage::ArmorDefinition> armorDefinitions,
    engine::time::Duration manualRepairRetention,
    std::size_t splashResultCapacity, std::size_t acceptedHitCapacity,
    std::span<const generalszh::progression::VeterancyDefinition> veterancyDefinitions,
    std::span<const demolition::DemolitionTrapDefinition> demolitionDefinitions,
    const engine::gameplay::rts::unlocks::UnlockCatalog *unlockCatalog,
    const engine::gameplay::rts::rank::RankCatalog *rankCatalog,
    std::size_t maximumVisibilityObservers,
    std::size_t maximumVisibilityRegionTransitions,
    std::span<const generalszh::bounty::CashBountyDefinition> cashBountyDefinitions)
    : impl_(std::make_unique<Impl>(world, jobs, definitions, grid, inputCapacity,
        regenerationDefinitions, progressionDefinitions, buildingDefinitions, saleDefinitions,
        repairDefinitions, transportDefinitions, captureDefinitions, damagePolicies, armorDefinitions,
        manualRepairRetention, splashResultCapacity, acceptedHitCapacity, veterancyDefinitions,
        demolitionDefinitions, unlockCatalog, rankCatalog, maximumVisibilityObservers,
        maximumVisibilityRegionTransitions, cashBountyDefinitions))
{
}

GameSession::~GameSession() noexcept = default;

void GameSession::ConfigureForceProduction(std::span<const ForceProductionPolicyInput> policies,
    ForceProductionPlanLimits limits)
{
    impl_->ConfigureForceProduction(policies, limits);
}

void GameSession::ConfigureCrates(CrateConfiguration configuration)
{
    impl_->ConfigureCrates(configuration);
}

void GameSession::RegisterComponents(ecs::World &world)
{
    Impl::RegisterComponents(world);
}

void GameSession::Finalize(engine::time::FixedStep rate)
{
    impl_->Finalize(rate);
}

void GameSession::RegisterController(ecs::Entity account, std::uint32_t policyKey, bool enabled)
{
    impl_->RegisterController(account, policyKey, enabled);
}

ecs::Entity GameSession::CreateAccount(std::uint32_t balance, std::uint64_t unlockCredits,
    std::uint64_t intrinsicScienceCredits,
    engine::gameplay::rts::rank::RankSkillAwardModifier skillAwardModifier)
{
    return impl_->CreateAccount(balance, unlockCredits, intrinsicScienceCredits, skillAwardModifier);
}

void GameSession::RegisterCrateAccountPolicy(ecs::Entity account, const bool human, const bool neutral)
{
    impl_->RegisterCrateAccountPolicy(account, human, neutral);
}

ecs::Entity GameSession::CreateCrate(crates::CrateState state)
{
    return impl_->CreateCrate(state);
}

ecs::Entity GameSession::CreateProducer(ecs::Entity account, std::uint32_t limit,
    std::uint64_t health, engine::gameplay::navigation::Cell spawn,
    std::optional<production::ProductionExitDefinition> exit)
{
    return impl_->CreateProducer(account, limit, health, spawn, exit);
}

ecs::Entity GameSession::CreateCompletedStructure(ecs::Entity account, std::uint32_t definition,
    engine::gameplay::navigation::Cell spawn)
{
    return impl_->CreateCompletedStructure(account, definition, spawn);
}

ecs::Entity GameSession::CreateSupplySource(std::uint32_t boxes, engine::gameplay::navigation::Cell cell,
    std::uint64_t health)
{
    return impl_->CreateSupplySource(boxes, cell, health);
}

ecs::Entity GameSession::CreateSupplyDropoff(ecs::Entity account, engine::gameplay::navigation::Cell cell,
    std::uint32_t valuePerBox, std::uint64_t health)
{
    return impl_->CreateSupplyDropoff(account, cell, valuePerBox, health);
}

void GameSession::BeginMatch(std::span<const ecs::Entity> roster)
{
    impl_->BeginMatch(roster);
}

MatchOutcome GameSession::Outcome() const
{
    return impl_->Outcome();
}

bool GameSession::CancelBuild(ecs::Entity producer, ecs::Entity entry)
{
    return impl_->CancelBuild(producer, entry);
}

const ecs::SystemRegistry &GameSession::Systems() const noexcept
{
    return impl_->Systems();
}

void GameSession::EnableRegeneration(ecs::Entity entity, std::uint32_t definition,
    engine::time::SimulationTime time, engine::time::Duration phase, bool active)
{
    impl_->EnableRegeneration(entity, definition, time, phase, active);
}

void GameSession::EnrollProgression(ecs::Entity entity, std::uint32_t definition, bool trainable)
{
    impl_->EnrollProgression(entity, definition, trainable);
}

std::span<const engine::gameplay::progression::ExperienceResult> GameSession::ExperienceResults() const noexcept
{
    return impl_->ExperienceResults();
}

std::span<const construction::ConstructionReceipt> GameSession::ConstructionResults() const noexcept
{
    return impl_->ConstructionResults();
}

std::span<const selling::SellReceipt> GameSession::SaleReceipts() const noexcept
{
    return impl_->SaleReceipts();
}

std::span<const selling::SellResult> GameSession::SaleResults() const noexcept
{
    return impl_->SaleResults();
}

std::span<const engine::gameplay::containment::ContainmentResult> GameSession::ContainmentResults() const noexcept
{
    return impl_->ContainmentResults();
}

std::span<const engine::gameplay::containment::ContainmentResult> GameSession::GarrisonResults() const noexcept
{
    return impl_->GarrisonResults();
}

std::span<const production::rally::RallyReceipt> GameSession::RallyReceipts() const noexcept
{
    return impl_->RallyReceipts();
}

std::span<const capture::CaptureResult> GameSession::CaptureResults() const noexcept
{
    return impl_->CaptureResults();
}

std::span<const engine::gameplay::combat::damage::ResolvedHit> GameSession::AcceptedHits() const noexcept
{
    return impl_->AcceptedHits();
}

std::span<const engine::gameplay::rts::unlocks::UnlockReceipt> GameSession::UnlockReceipts() const noexcept
{
    return impl_->UnlockReceipts();
}

std::span<const engine::gameplay::rts::rank::RankResult> GameSession::RankResults() const noexcept
{
    return impl_->RankResults();
}

std::span<const generalszh::bounty::CashBountyAward> GameSession::CashBountyAwards() const noexcept
{
    return impl_->CashBountyAwards();
}

const ecs::DependencyGraph &GameSession::Graph() const noexcept
{
    return impl_->Graph();
}

const ecs::ExecutionPlan &GameSession::Plan() const noexcept
{
    return impl_->Plan();
}

std::span<const BuildReceipt> GameSession::Execute(engine::time::SimulationTime time, GameInputs inputs)
{
    return impl_->Execute(time, inputs);
}

}
