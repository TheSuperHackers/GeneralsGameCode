module;
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
module games.generalszh.hosts.headless.content_match;

import games.generalszh.hosts.headless.headless_match;
import games.generalszh.composition.game_session;
import games.generalszh.gameplay.production.components.production_state;
import games.generalszh.gameplay.construction.components.structure;
import games.generalszh.gameplay.capture.components.capture_state;
import engine.gameplay.combat.components.health;
import engine.gameplay.combat.components.weapon;
import engine.gameplay.rts.repair.components.manual_repair;
import engine.gameplay.navigation.components.movement;
import engine.gameplay.rts.economy.components.resource_balance;
import games.generalszh.gameplay.economy.income_history;
import engine.gameplay.containment.components.passenger_membership;
import engine.gameplay.progression.components.progression_state;
import engine.gameplay.rts.construction.components.construction_components;
// Cash-bounty decoding is an implementation detail of RunContentMatch. Keep
// its production/construction binding dependencies out of this public host
// interface; callers that author decoded bounty fixtures import the adapter
// directly.
import games.generalszh.adapters.content.bounty.cash_bounty_definition;
import games.generalszh.adapters.content.units.unit_definition;
import games.generalszh.adapters.content.buildings.building_definition;
import engine.gameplay.rts.unlocks.definitions.unlock_definition;
import games.generalszh.adapters.content.science.science_definition;
import games.generalszh.adapters.content.science.science_prerequisite_binding;
import engine.gameplay.rts.unlocks.inputs.unlock_batch;
import engine.gameplay.concealment.components.concealment_binding;
import engine.gameplay.concealment.components.concealment_eligibility;
import engine.gameplay.concealment.components.concealment_state;
import engine.gameplay.concealment.components.detection_state;
import games.generalszh.gameplay.containment.components.initial_payload;
import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
import engine.gameplay.rts.bounty.components.bounty_policy;
namespace generalszh::headless
{
// Authored exercise using a decoded unit and a passive enemy base or produced unit. Not a map
// loader or an AI player: gameplay execution remains entirely GameSession.
DecodedMatchReport RunDecodedMatch(const content::DecodedUnit &unit,std::size_t workers,std::uint64_t tickLimit,
    ContentExercise exercise)
{
    using namespace engine::gameplay::navigation; using namespace engine::gameplay::combat;
    if(!workers||workers>64||!tickLimit) throw std::invalid_argument("Invalid content-host worker/tick limit");
    if (exercise.unlockCatalog && !exercise.unlockCatalog->IsFinalized())
        throw std::logic_error("Content host requires a finalized unlock catalog");
    if ((!exercise.purchaseUnlocks.empty() || exercise.unlockCredits) && !exercise.unlockCatalog)
        throw std::invalid_argument("Unlock exercise inputs require an injected unlock catalog");

    // The public API intentionally remains a const single-unit entry point.
    // Copy the complete supplied catalog at this boundary so name resolution
    // can never be performed against only the primary/passive exercise pair.
    const auto primaryDefinitionKey=unit.definition.key;
    const std::optional<std::uint32_t> passiveDefinitionKey=exercise.passiveTarget
        ? std::optional<std::uint32_t>{exercise.passiveTarget->definition.key}:std::nullopt;
    if (exercise.passiveTarget && exercise.passiveBuilding)
        throw std::invalid_argument("Content bounty exercise cannot use unit and building victims together");
    std::vector<content::DecodedUnit> decodedCatalog;
    decodedCatalog.reserve(1+(exercise.passiveTarget?1u:0u)+exercise.supportingCatalog.size());
    decodedCatalog.push_back(unit);
    if(exercise.passiveTarget) decodedCatalog.push_back(*exercise.passiveTarget);
    decodedCatalog.insert(decodedCatalog.end(),exercise.supportingCatalog.begin(),exercise.supportingCatalog.end());

    const auto validateUnbound=[](const content::DecodedUnit &decoded) {
        if(decoded.definition.regeneration||decoded.definition.progression||decoded.definition.veterancy||decoded.definition.transportDefinition||decoded.definition.capture
            ||decoded.definition.armor||decoded.definition.prerequisites||decoded.definition.initialPayload
            ||!decoded.definition.sciencePrerequisiteIds.empty()||decoded.definition.scienceSchemaHash)
            throw std::invalid_argument("Content host binds its own catalogs; prebound unit definitions are unsupported");
    };
    for(const auto &decoded:decodedCatalog) validateUnbound(decoded);

    const auto policies=content::DecodeDamagePolicies(content::ZeroHourDamageTypes);
    const auto validatePolicies=[&](const content::DecodedUnit &decoded) {
        if(decoded.damagePolicies.size()!=policies.size()) throw std::invalid_argument("Content damage policy schema width mismatch");
        for(std::size_t i=0;i<policies.size();++i)
            if(decoded.damagePolicies[i].armor!=policies[i].armor) throw std::invalid_argument("Content damage policy schema mismatch");
    };
    for(const auto &decoded:decodedCatalog) validatePolicies(decoded);

    // This is the publication barrier: every raw InitialPayload reference is
    // resolved against the complete catalog before any BuildDefinition copy or
    // ECS world is created. Binding stages all results and publishes none on a
    // missing passenger, collision, or capacity failure.
    content::BindDecodedUnitCatalog(std::span<content::DecodedUnit>{decodedCatalog.data(),decodedCatalog.size()});
    // Stable-key order owns every dense auxiliary catalog index below. The
    // caller's supporting span is an input set, not an ordering contract.
    std::sort(decodedCatalog.begin(),decodedCatalog.end(),[](const auto &left,const auto &right) {
        return left.definition.key<right.definition.key;
    });
    const auto findBound=[&](std::uint32_t key)->const content::DecodedUnit & {
        const auto found=std::find_if(decodedCatalog.begin(),decodedCatalog.end(),[&](const auto &candidate) {
            return candidate.definition.key==key;
        });
        if(found==decodedCatalog.end()) throw std::logic_error("Bound content catalog lost a requested unit identity");
        return *found;
    };
    const auto &boundUnit=findBound(primaryDefinitionKey);
    const auto *boundPassiveTarget=passiveDefinitionKey?&findBound(*passiveDefinitionKey):nullptr;
    const auto validateBound=[](const content::DecodedUnit &decoded) {
        if(decoded.initialPayload&&!decoded.definition.initialPayload)
            throw std::logic_error("Decoded InitialPayload remained unresolved at bound content execution");
        if(!decoded.initialPayload&&decoded.definition.initialPayload)
            throw std::logic_error("Content definition carried an unbound InitialPayload");
        if(decoded.definition.regeneration||decoded.definition.progression||decoded.definition.veterancy||decoded.definition.transportDefinition||decoded.definition.capture
            ||decoded.definition.armor||decoded.definition.prerequisites
            ||!decoded.definition.sciencePrerequisiteIds.empty()||decoded.definition.scienceSchemaHash)
            throw std::invalid_argument("Content host binds its own catalogs; prebound unit definitions are unsupported");
    };
    for(const auto &decoded:decodedCatalog) validateBound(decoded);

    const bool hasSciencePrerequisites=std::any_of(decodedCatalog.begin(),decodedCatalog.end(),[](const auto &decoded) {
        return !decoded.prerequisites.scienceNames.empty();
    });
    if (hasSciencePrerequisites && !exercise.unlockCatalog)
        throw std::invalid_argument("Decoded Science prerequisite requires an injected unlock catalog");

    std::vector<content::ContentDefinitionIdentity> identities;
    identities.reserve(decodedCatalog.size()+exercise.prerequisiteIdentities.size());
    const auto appendIdentity=[&](const content::ContentDefinitionIdentity identity) {
        if (identity.name.empty())
            throw std::invalid_argument("Content prerequisite identity name cannot be empty");
        for (const auto existing:identities)
        {
            if (existing.name==identity.name)
            {
                if (existing.key!=identity.key)
                    throw std::invalid_argument("Duplicate or colliding content prerequisite identity");
                return;
            }
            if (existing.key==identity.key)
                throw std::invalid_argument("Duplicate or colliding content prerequisite identity");
        }
        identities.push_back(identity);
    };
    for (const auto &decoded:decodedCatalog)
        appendIdentity({decoded.name,decoded.definition.key});
    for (const auto identity:exercise.prerequisiteIdentities)
        appendIdentity(identity);

    if(!boundUnit.definition.weapon.damage&&!boundUnit.definition.weapon.secondaryDamage)
        throw std::invalid_argument("Content combat scenario requires an armed unit");
    if(exercise.experience && !boundUnit.progression) throw std::invalid_argument("Accepted XP exercise requires authored progression");
    // Validate caller-supplied identities even when this exercise has no
    // prerequisite. Binding never infers names or keys from a legacy object.
    (void)content::BindPrerequisites(content::DecodedPrerequisites{}, identities);
    if(!boundUnit.prerequisites.objectGroups.empty()&&!exercise.initialBuilding)
        throw std::invalid_argument("Content prerequisite exercise requires an explicit initial building");
    if(boundPassiveTarget)
    {
        if(boundPassiveTarget->definition.weapon.damage||boundPassiveTarget->definition.weapon.secondaryDamage
            ||boundPassiveTarget->definition.acquisition.enabled)
            throw std::invalid_argument("Passive content target must be unarmed without automatic acquisition");
    }
    if(exercise.initialBuilding)
    {
        if(exercise.initialBuilding->definition.armor)
            throw std::invalid_argument("Content host expects initial building armor to be bound at composition");
        if(exercise.initialBuilding->name.empty()||!exercise.initialBuilding->definition.key)
            throw std::invalid_argument("Initial building has invalid content identity");
        for(const auto &decoded:decodedCatalog)
            if(exercise.initialBuilding->name==decoded.name||exercise.initialBuilding->definition.key==decoded.definition.key)
                throw std::invalid_argument("Initial building collides with a unit content identity");
    }
    if(exercise.passiveBuilding)
    {
        if(exercise.passiveBuilding->definition.armor)
            throw std::invalid_argument("Content host expects passive building armor to be bound at composition");
        if(exercise.passiveBuilding->name.empty()||!exercise.passiveBuilding->definition.key)
            throw std::invalid_argument("Passive building has invalid content identity");
        for(const auto &decoded:decodedCatalog)
            if(exercise.passiveBuilding->name==decoded.name||exercise.passiveBuilding->definition.key==decoded.definition.key)
                throw std::invalid_argument("Passive building collides with a unit content identity");
    }
    if(exercise.passiveBuildingUnderConstruction&&!exercise.passiveBuilding)
        throw std::invalid_argument("Under-construction content exercise requires a passive building");
    if (exercise.cashBountyFollowupDefinition && exercise.cashBountyDefinitions.empty())
        throw std::invalid_argument("Cash bounty follow-up production requires an enabled bounty catalog");
    std::array<std::uint8_t,33> terrain; terrain.fill(1); NavigationGrid grid(33,1,terrain);
    ecs::World world; GameSession::RegisterComponents(world); world.FinalizeComponents(); engine::jobs::JobSystem jobs({workers});
    std::vector<production::BuildDefinition> definitions;
    definitions.reserve(decodedCatalog.size());
    for(const auto &decoded:decodedCatalog) definitions.push_back(decoded.definition);
    std::vector<regeneration::RegenerationDefinition> regenerationDefinitions;
    std::vector<engine::gameplay::progression::ProgressionDefinition> progressionDefinitions;
    std::vector<generalszh::progression::VeterancyDefinition> veterancyDefinitions;
    std::vector<engine::gameplay::containment::TransportDefinition> transportDefinitions;
    std::vector<capture::CaptureDefinition> captureDefinitions;
    std::vector<damage::ArmorDefinition> armorDefinitions;
    std::vector<engine::gameplay::rts::repair::RepairDefinition> repairDefinitions;
    std::vector<construction::BuildingDefinition> buildingDefinitions;
    const auto bind=[&](const content::DecodedUnit &decoded,production::BuildDefinition &definition) {
    if(!exercise.cashBountyDefinitions.empty())
        definition.cashBountyCost=content::BindCashBountyCostBinding(decoded.definition,decoded.ignoredInGui);
    if(decoded.definition.manualRepair)
    {
        const auto same=[](const auto &left,const auto &right) {
            return left.rateNumerator==right.rateNumerator&&left.rateDenominator==right.rateDenominator&&
                left.interval==right.interval&&left.damageDelay==right.damageDelay&&left.initialDelay==right.initialDelay&&
                left.intervalPolicy==right.intervalPolicy;
        };
        if(std::find_if(repairDefinitions.begin(),repairDefinitions.end(),
            [&](const auto &candidate){return same(*decoded.definition.manualRepair,candidate);})==repairDefinitions.end())
            repairDefinitions.push_back(*decoded.definition.manualRepair);
    }
    if(decoded.armor)
    {
        definition.armor=damage::ArmorBinding{{static_cast<std::uint32_t>(armorDefinitions.size())}};
        armorDefinitions.push_back(*decoded.armor);
    }
    if(decoded.capture)
    {
        definition.capture=capture::CaptureCapability{static_cast<std::uint32_t>(captureDefinitions.size()),decoded.captureEnabled};
        captureDefinitions.push_back(*decoded.capture);
    }
    if(decoded.transport)
    {
        definition.transportDefinition=static_cast<std::uint32_t>(transportDefinitions.size());
        transportDefinitions.push_back(*decoded.transport);
    }
    if(decoded.regeneration)
    {
        // Deliberate scenario phase, not the reference random startup phase.
        definition.regeneration=production::RegenerationSpawnConfig{static_cast<std::uint32_t>(regenerationDefinitions.size()),exercise.regenerationPhase,decoded.regeneration->startsActive};
        regenerationDefinitions.push_back(decoded.regeneration->definition);
    }
    if(decoded.progression)
    {
        definition.progression=production::ProgressionSpawnConfig{static_cast<std::uint32_t>(progressionDefinitions.size()),decoded.progression->trainable};
        progressionDefinitions.push_back(decoded.progression->Progression());
        definition.veterancy=production::VeterancySpawnConfig{static_cast<std::uint32_t>(veterancyDefinitions.size()),!decoded.ignoredInGui};
        veterancyDefinitions.push_back(*decoded.progression);
    }
    if(!decoded.prerequisites.Empty())
    {
        if (decoded.prerequisites.scienceNames.empty())
            definition.prerequisites=content::BindObjectPrerequisites(decoded.prerequisites,identities);
        else
        {
            const auto bound=content::BindProductionPrerequisites(decoded.prerequisites,identities,*exercise.unlockCatalog);
            if (!bound.object.Empty()) definition.prerequisites=bound.object;
            definition.sciencePrerequisiteIds=bound.scienceIds;
            definition.scienceSchemaHash=bound.scienceSchemaHash;
        }
    }
    };
    for(std::size_t i=0;i<decodedCatalog.size();++i) bind(decodedCatalog[i],definitions[i]);
    const auto appendBuilding=[&](const content::DecodedBuilding &decoded) {
        if(std::find_if(buildingDefinitions.begin(),buildingDefinitions.end(),[&](const auto &candidate) {
            return candidate.key==decoded.definition.key;
        })!=buildingDefinitions.end())
            return;
        {
            auto definition=decoded.definition;
            if(decoded.armor)
            {
                definition.armor=damage::ArmorBinding{{static_cast<std::uint32_t>(armorDefinitions.size())}};
                armorDefinitions.push_back(*decoded.armor);
            }
            if(!exercise.cashBountyDefinitions.empty())
                definition.cashBountyCost=content::BindCashBountyCostBinding(decoded.definition,decoded.ignoredInGui);
            buildingDefinitions.push_back(definition);
        }
    };
    if(exercise.initialBuilding)
        appendBuilding(*exercise.initialBuilding);
    if(exercise.passiveBuilding)
        appendBuilding(*exercise.passiveBuilding);
    GameSession game(world,jobs,definitions,grid,4096,regenerationDefinitions,progressionDefinitions,buildingDefinitions,{},repairDefinitions,
        transportDefinitions,captureDefinitions,policies,armorDefinitions,ZeroHourManualRepairRetention,0,0,veterancyDefinitions,{},exercise.unlockCatalog,
        nullptr,4096,1,exercise.cashBountyDefinitions);
    const engine::time::FixedStep step{20}; game.Finalize(step);
    const auto blueStartingBalance=boundPassiveTarget ? boundPassiveTarget->definition.cost
        : exercise.passiveBuildingUnderConstruction ? exercise.passiveBuilding->definition.cost : 0u;
    const auto red=game.CreateAccount(boundUnit.definition.cost,exercise.unlockCredits),blue=game.CreateAccount(blueStartingBalance);
    const auto redBase=exercise.initialBuilding
        ? game.CreateCompletedStructure(red,exercise.initialBuilding->definition.key,exercise.initialBuildingCell)
        : game.CreateProducer(red,9,boundUnit.definition.health,0);
    const auto blueBase=exercise.passiveBuilding
        ? (exercise.passiveBuildingUnderConstruction
            ? world.Create<construction::Builder,engine::gameplay::rts::construction::BuilderAssignment,
                LifeState,Health,PendingDamage,DamageResult,GridPosition,MoveGoal,MoveCredit>()
            : ecs::Entity{})
        : game.CreateProducer(blue,9,boundUnit.definition.health,32);
    if(exercise.passiveBuildingUnderConstruction)
    {
        *world.Get<construction::Builder>(blueBase)={blue};
        *world.Get<Health>(blueBase)={boundUnit.definition.health};
        *world.Get<GridPosition>(blueBase)={exercise.passiveBuildingCell};
        const auto survivorCell=exercise.passiveBuildingCell==31 ? 30u : 31u;
        (void)game.CreateProducer(blue,9,boundUnit.definition.health,survivorCell);
    }
    const auto passiveBuilding=exercise.passiveBuilding&&!exercise.passiveBuildingUnderConstruction
        ? game.CreateCompletedStructure(blue,exercise.passiveBuilding->definition.key,exercise.passiveBuildingCell)
        : ecs::Entity{};
    const std::array players{red,blue}; game.BeginMatch(players);
    std::vector builds{BuildInput{redBase,primaryDefinitionKey}};
    if(boundPassiveTarget) builds.push_back({blueBase,*passiveDefinitionKey});
    if (exercise.purchaseUnlocks.size()>4096)
        throw std::length_error("Content unlock exercise exceeds input capacity");
    std::vector<engine::gameplay::rts::unlocks::UnlockRequest> purchaseRequests;
    purchaseRequests.reserve(exercise.purchaseUnlocks.size());
    for (const auto key:exercise.purchaseUnlocks)
        purchaseRequests.push_back({red,key,engine::gameplay::rts::unlocks::UnlockOperation::Purchase});
    ecs::Query<ecs::Read<production::ProducedUnit>> units(world);
    ecs::Query<ecs::Read<production::ProducedUnit>,ecs::Read<engine::gameplay::containment::PassengerMembership>,
        ecs::Read<engine::gameplay::containment::PassengerSlots>> payloads(world);
    ecs::Entity actor{}; bool ordered=false;
    ecs::Entity target=exercise.passiveBuilding ? passiveBuilding : boundPassiveTarget ? ecs::Entity{} : blueBase;
    const std::array constructionInputs{construction::ConstructionInput{
        blueBase,exercise.passiveBuilding?exercise.passiveBuilding->definition.key:0,exercise.passiveBuildingCell}};
    std::array followupBuilds{BuildInput{redBase,exercise.cashBountyFollowupDefinition.value_or(0)}};
    bool followupRequested=false;
    DecodedMatchReport report;report.attackerAccount=red;report.targetAccount=blue;
    report.acceptedHits.reserve(exercise.hitCapacity);
    const auto observePayload=[&](std::uint64_t tick) {
        if(!actor.IsValid()||!boundUnit.definition.initialPayload) return;
        const auto *binding=world.Get<generalszh::containment::InitialPayloadBinding>(actor);
        if(!binding) throw std::logic_error("Produced payload carrier is missing its runtime binding");
        report.initialPayloadPresent=true;report.initialPayloadCarrier=actor;
        report.initialPayloadConfiguredCount=binding->count;
        report.initialPayloadConsumed=binding->phase==generalszh::containment::InitialPayloadPhase::Consumed;
        std::vector<std::array<std::uint64_t,8>> records;
        payloads.ForEachChunk([&](auto chunk) {
            const auto values=chunk.template Get<production::ProducedUnit>();
            const auto memberships=chunk.template Get<engine::gameplay::containment::PassengerMembership>();
            for(std::size_t row=0;row!=chunk.Count();++row)
                if(values[row].account==red&&values[row].definition==binding->passengerDefinition
                    &&engine::gameplay::containment::IsContained(memberships[row])
                    &&memberships[row].carrier==actor)
                    records.push_back({chunk.Entities()[row].index,chunk.Entities()[row].generation,
                        values[row].account.index,values[row].account.generation,memberships[row].carrier.index,
                        memberships[row].carrier.generation,values[row].definition,
                        static_cast<std::uint64_t>(memberships[row].phase)});
        });
        std::sort(records.begin(),records.end(),[](const auto &left,const auto &right) {
            return std::tie(left[0],left[1])<std::tie(right[0],right[1]);
        });
        report.initialPayloadSpawnCount=static_cast<std::uint32_t>(records.size());
        report.initialPayloadState.clear();
        for(const auto &record:records) report.initialPayloadState.insert(report.initialPayloadState.end(),record.begin(),record.end());
        if(!records.empty()&&!report.initialPayloadSpawned) {report.initialPayloadSpawned=true;report.initialPayloadSpawnTick=tick;}
    };
    std::uint64_t lastTick{};
    for(std::uint64_t tick=0;tick<tickLimit&&game.Outcome().status==MatchStatus::Running;++tick)
    {
        const std::array attacks{AttackInput{actor,target}}; GameInputs inputs;
        const std::array experience{engine::gameplay::progression::AcceptedExperience{actor,exercise.experience.value_or(0)}};
        if(!tick)
        {
            inputs.builds=builds;
            inputs.unlocks=std::span<const engine::gameplay::rts::unlocks::UnlockRequest>{purchaseRequests};
            if(exercise.passiveBuildingUnderConstruction)
                inputs.construction=std::span<const construction::ConstructionInput>{constructionInputs};
        }
        if(exercise.cashBountyFollowupDefinition && target.IsValid() && !world.Get<LifeState>(target)->alive
            && !followupRequested)
        {
            followupBuilds[0].definition=*exercise.cashBountyFollowupDefinition;
            inputs.builds=followupBuilds;
            followupRequested=true;
        }
        if(actor.IsValid()&&target.IsValid()&&!ordered)
        {
            inputs.attacks=attacks;
            if(exercise.experience) inputs.experience=experience;
            ordered=true;
        }
        const auto buildResults=game.Execute({tick,step},inputs);
        report.buildReceipts.insert(report.buildReceipts.end(),buildResults.begin(),buildResults.end());
        if(exercise.passiveBuildingUnderConstruction&&!target.IsValid())
            for(const auto &receipt:game.ConstructionResults())
                if(receipt.status==construction::ConstructionAcceptance::Accepted)
                {
                    target=receipt.site;
                    break;
                }
        if(followupRequested)
            for(const auto &receipt:buildResults)
                if(receipt.status==BuildAcceptance::Accepted)
                    report.bountyFollowupAccepted=true;
        const auto unlockResults=game.UnlockReceipts();
        report.unlockReceipts.insert(report.unlockReceipts.end(),unlockResults.begin(),unlockResults.end());
        for(const auto &award:game.CashBountyAwards()) report.bountyAwards.push_back(award);
        lastTick=tick;
        const auto hits=game.AcceptedHits();
        if(hits.size()>exercise.hitCapacity-report.acceptedHits.size()) throw std::length_error("Content diagnostic hit capacity exhausted");
        report.acceptedHits.insert(report.acceptedHits.end(),hits.begin(),hits.end());
        for(const auto &result:game.ExperienceResults()) report.experienceResults.push_back(result);
        if(!actor.IsValid()||!target.IsValid())
        {
            const bool hadActor=actor.IsValid();
            units.ForEachChunk([&](auto chunk) {
                const auto values=chunk.template Get<production::ProducedUnit>();
                for(std::size_t i=0;i<chunk.Count();++i) {
                    const auto candidate=chunk.Entities()[i];
                    if(values[i].account==red&&values[i].definition==primaryDefinitionKey&&
                        (!actor.IsValid()||candidate.index<actor.index||
                            (candidate.index==actor.index&&candidate.generation<actor.generation))) actor=candidate;
                    if(boundPassiveTarget&&values[i].account==blue&&values[i].definition==*passiveDefinitionKey&&
                        (!target.IsValid()||candidate.index<target.index||
                            (candidate.index==target.index&&candidate.generation<target.generation))) target=candidate;
                }
            });
            if(actor.IsValid()&&!hadActor)
            {
                // Observe the actual production commit. No post-spawn enrollment
                // or gameplay mutation: first capability processing is next tick.
                report.spawnTick=tick; report.spawnHealth=world.Get<Health>(actor)->current;
                report.armorPresent=world.Has<damage::ArmorBinding>(actor);
                report.progressionPresent=world.Has<engine::gameplay::progression::ProgressionState>(actor)
                    &&world.Has<engine::gameplay::progression::ProgressionDefinitionRef>(actor)
                    &&world.Has<engine::gameplay::progression::ProgressionEligibility>(actor)
                    &&world.Has<engine::gameplay::progression::ProgressionInbox>(actor);
                report.regenerationPresent=world.Has<regeneration::RegenerationBinding>(actor)
                    &&world.Has<regeneration::RegenerationState>(actor);
                report.transportPresent=world.Has<engine::gameplay::containment::TransportBinding>(actor)
                    &&world.Has<engine::gameplay::containment::TransportState>(actor);
                report.passengerPresent=world.Has<engine::gameplay::containment::PassengerSlots>(actor)
                    &&world.Has<engine::gameplay::containment::PassengerMembership>(actor);
                report.manualRepairPresent=world.Has<engine::gameplay::rts::repair::ManualRepairRateBinding>(actor)
                    &&world.Has<engine::gameplay::rts::repair::ManualRepairProgress>(actor)
                    &&world.Has<engine::gameplay::rts::repair::ManualRepairAssignment>(actor);
                report.capturePresent=world.Has<capture::CaptureCapability>(actor)&&world.Has<capture::CaptureActorState>(actor)
                    &&world.Has<capture::CaptureRecharge>(actor);
                if(report.capturePresent) report.captureEnabled=world.Get<capture::CaptureCapability>(actor)->enabled;
                report.concealmentPresent=world.Has<engine::gameplay::concealment::ConcealmentBinding>(actor)
                    &&world.Has<engine::gameplay::concealment::ConcealmentState>(actor)
                    &&world.Has<engine::gameplay::concealment::ConcealmentEligibility>(actor);
                report.detectorPresent=world.Has<engine::gameplay::concealment::DetectionBinding>(actor)
                    &&world.Has<engine::gameplay::concealment::DetectionState>(actor);
            }
            if(boundPassiveTarget&&target.IsValid())
            {
                report.targetArmorPresent=world.Has<damage::ArmorBinding>(target);
                report.targetConcealmentPresent=world.Has<engine::gameplay::concealment::ConcealmentBinding>(target)
                    &&world.Has<engine::gameplay::concealment::ConcealmentState>(target);
                report.targetDetectorPresent=world.Has<engine::gameplay::concealment::DetectionBinding>(target)
                    &&world.Has<engine::gameplay::concealment::DetectionState>(target);
            }
        }
        if(exercise.cashBountyFollowupDefinition)
            units.ForEachChunk([&](auto chunk) {
                const auto values=chunk.template Get<production::ProducedUnit>();
                for(std::size_t row=0;row!=chunk.Count();++row)
                    if(followupRequested&&!report.bountyFollowupEntity.IsValid()&&chunk.Entities()[row]!=actor
                        &&values[row].account==red&&values[row].definition==*exercise.cashBountyFollowupDefinition)
                    {
                        report.bountyFollowupEntity=chunk.Entities()[row];
                        report.bountyFollowupAccount=values[row].account;
                        report.bountyFollowupDefinition=values[row].definition;
                        report.bountyFollowupSpawned=true;
                    }
            });
        if(target.IsValid()&&!world.Get<LifeState>(target)->alive
            && (!exercise.cashBountyFollowupDefinition || report.bountyFollowupSpawned)) break;
        observePayload(tick);
    }
    report.outcome=game.Outcome(); report.attacker=actor; report.target=target;
    report.targetHealth=target.IsValid()?world.Get<Health>(target)->current:0;
    if(target.IsValid()&&world.Has<construction::Structure>(target))
        report.targetStructureComplete=world.Get<construction::Structure>(target)->complete;
    if (target.IsValid()&&world.Has<engine::gameplay::concealment::ConcealmentState>(target))
        report.targetDetectedAtEnd=world.Get<engine::gameplay::concealment::ConcealmentState>(target)->detectedUntilTick!=0 &&
            lastTick<world.Get<engine::gameplay::concealment::ConcealmentState>(target)->detectedUntilTick;
    if(actor.IsValid())
    {
        report.survivingUnits=world.Get<LifeState>(actor)->alive;
        report.state={actor.index,actor.generation,primaryDefinitionKey,world.Get<Health>(actor)->current,
            world.Get<GridPosition>(actor)->cell,world.Get<WeaponState>(actor)->ammo,
            world.Get<engine::gameplay::rts::economy::ResourceBalance>(red)->quantity};
        report.state.insert(report.state.end(),{report.progressionPresent,report.regenerationPresent,report.spawnTick,report.spawnHealth});
        report.state.insert(report.state.end(),{report.transportPresent,report.passengerPresent});
        report.state.insert(report.state.end(),{report.capturePresent,report.captureEnabled});
        report.state.insert(report.state.end(),{report.armorPresent,report.targetArmorPresent,target.index,target.generation,report.targetHealth});
        report.state.push_back(report.manualRepairPresent);
        if(report.progressionPresent)
        {
            const auto &state=*world.Get<engine::gameplay::progression::ProgressionState>(actor);
            report.experience=state.experience; report.level=state.level;
            report.state.insert(report.state.end(),{state.experience,state.level,
                world.Get<engine::gameplay::progression::ProgressionDefinitionRef>(actor)->index,
                world.Get<engine::gameplay::progression::ProgressionEligibility>(actor)->trainable});
        }
        if(report.regenerationPresent)
        {
            const auto &state=*world.Get<regeneration::RegenerationState>(actor);
            report.state.insert(report.state.end(),{state.nextPulse,state.soonestDamageWake,state.active,state.stopped,state.armed});
        }
    }
    if(const auto *policy=world.Get<engine::gameplay::rts::bounty::BountyPolicy>(red))
    {
        report.bountyRateNumerator=policy->maximum.numerator;
        report.bountyRateDenominator=policy->maximum.denominator;
    }
    report.attackerBalance=world.Get<engine::gameplay::rts::economy::ResourceBalance>(red)->quantity;
    report.attackerIncome=world.Get<economy::IncomeHistory>(red)->total;
    return report;
}
ContentReport RunContentMatch(const std::filesystem::path &iniRoot,std::size_t workers)
{
    const auto object=content::ReadIniFile(iniRoot/"Object"/"AmericaInfantry.ini");
    const auto buildingObject=content::ReadIniFile(iniRoot/"Object"/"FactionBuilding.ini");
    const auto weapons=content::ReadIniFile(iniRoot/"Weapon.ini"); const auto locomotors=content::ReadIniFile(iniRoot/"Locomotor.ini");
    const auto powers=content::ReadIniFile(iniRoot/"SpecialPower.ini");
    const auto decodedCashBounty=content::DecodeCashBountyDefinitions(buildingObject,powers,"AmericaCommandCenter");
    const auto cashBountyDefinitions=content::BindCashBountyDefinitions(decodedCashBounty);
    const auto armor=content::ReadIniFile(iniRoot/"Armor.ini");
    const auto decodedScience=content::DecodeScienceFile(iniRoot/"Science.ini");
    const auto scienceDefinitions=content::BindScienceDefinitions(decodedScience);
    engine::gameplay::rts::unlocks::UnlockCatalog scienceCatalog;
    for (const auto &definition:scienceDefinitions) scienceCatalog.Register(definition);
    scienceCatalog.Finalize();
    const auto rules=content::DecodeGameRules(content::ReadIniFile(iniRoot/"GameData.ini"));
    const auto unit=content::DecodeUnit(object,weapons,locomotors,"AmericaInfantryRanger",{},powers,content::CapturePermission::Allowed,armor);
    const auto barracks=content::DecodeBuilding(buildingObject,"AmericaBarracks",rules,1000,armor);
    const std::array identities{
        content::ContentDefinitionIdentity{unit.name,unit.definition.key},
        content::ContentDefinitionIdentity{barracks.name,barracks.definition.key}};
    ContentExercise exercise;
    exercise.initialBuilding=&barracks;
    exercise.initialBuildingCell=1;
    exercise.prerequisiteIdentities=identities;
    exercise.unlockCatalog=&scienceCatalog;
    exercise.cashBountyDefinitions=cashBountyDefinitions;
    return {RunDecodedMatch(unit,workers,4096,exercise),unit.name,unit.omittedBehaviors};
}
}
