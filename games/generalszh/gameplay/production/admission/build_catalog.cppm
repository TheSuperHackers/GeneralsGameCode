module;
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.admission.build_catalog;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import engine.gameplay.progression.definitions.progression_definition;
export import games.generalszh.gameplay.progression.definitions.veterancy_definition;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
export import engine.gameplay.combat.damage.definitions.armor_catalog;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import games.generalszh.gameplay.concealment.definitions.stealth_policy;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export namespace generalszh::production
{
class BuildCatalog
{
public:
    BuildCatalog(const ecs::World &world, std::span<const BuildDefinition> source, engine::time::FixedStep rate,
        const engine::gameplay::combat::regeneration::RegenerationDefinitions *regeneration = nullptr,
        const engine::gameplay::progression::ProgressionCatalog *progression = nullptr,
        const engine::gameplay::containment::TransportDefinitions *transport = nullptr,
        const capture::CaptureDefinitions *captureDefinitions = nullptr,
        const engine::gameplay::combat::damage::ArmorCatalog *armor = nullptr,
        const engine::gameplay::rts::repair::RepairDefinitions *repairDefinitions = nullptr,
        std::span<const engine::gameplay::rts::repair::RepairDefinition> authoredRepairs = {},
        const engine::gameplay::concealment::ConcealmentDefinitions *concealment = nullptr,
        const engine::gameplay::concealment::DetectionDefinitions *detection = nullptr,
        const generalszh::concealment::StealthPolicies *stealth = nullptr,
        const generalszh::progression::VeterancyCatalog *veterancy = nullptr,
        const engine::gameplay::rts::unlocks::UnlockCatalog *scienceCatalog = nullptr)
        : definitions(source.begin(), source.end()), step(rate)
    {
        if (!world.ComponentsFinalized() || world.IsScheduledExecutionActive())
            throw std::logic_error("Build catalog requires frozen components at startup");
        if (scienceCatalog && !scienceCatalog->IsFinalized())
            throw std::logic_error("Build catalog requires a finalized science unlock catalog");
        std::sort(definitions.begin(), definitions.end(), [](auto &a, auto &b) { return a.key < b.key; });
        std::optional<engine::gameplay::rts::unlocks::UnlockSchemaHash> scienceSchema;
        for (std::size_t i = 0; i != definitions.size(); ++i)
        {
            auto &definition = definitions[i];
            if (!definition.maximumHealth) definition.maximumHealth = definition.health;
            if ((i && definitions[i - 1].key == definition.key) || definition.quantity <= 0
                || definition.health == 0 || definition.health > definition.maximumHealth
                || (definition.kind != production::EntryKind::Unit && definition.kind != production::EntryKind::Upgrade)
                || (definition.kind == production::EntryKind::Upgrade &&
                    (definition.quantity != 1 || definition.regeneration || definition.progression || definition.veterancy || definition.lifetime || definition.transportDefinition || definition.passenger || definition.capture || definition.armor || definition.manualRepair || definition.initialPayload || definition.concealment || definition.detection || definition.stealthPolicy || definition.garrisonable)))
                throw std::invalid_argument("Invalid modern build definition");
            if (definition.visibility && definition.kind != production::EntryKind::Unit)
                throw std::invalid_argument("Visibility enrollment requires a unit build definition");
            if (definition.visibility)
            {
                const auto authored = *definition.visibility;
                definition.visibility = engine::gameplay::rts::visibility::CompileVisibilityDefinition(
                    authored.id, authored.shroudClearingRadiusCells,
                    authored.constructionRadiusCells, authored.unlookPersistDuration, rate);
            }
            if(definition.garrisonable && (definition.kind!=production::EntryKind::Unit || definition.builder || definition.harvest || definition.transportDefinition))
                throw std::invalid_argument("Garrisonable unit requires a ground infantry-compatible non-worker definition");
            if(definition.transportDefinition)
            {
                if(!transport || transport->Step()!=rate) throw std::invalid_argument("Transport requires a matching compiled catalog");
                (void)transport->Get(*definition.transportDefinition);
            }
            if(definition.passenger && (!definition.passenger->slots || definition.transportDefinition || definition.builder || definition.harvest))
                throw std::invalid_argument("Passenger requires positive slots and no nested carrier or unsupported work tasks");
            if(definition.initialPayload)
            {
                if(definition.kind!=production::EntryKind::Unit||!definition.transportDefinition||!transport||transport->Step()!=rate)
                    throw std::invalid_argument("Initial payload requires a matching transport carrier catalog");
                if(definition.initialPayload->count > static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()))
                    throw std::invalid_argument("Initial payload count is not representable by production quantity");
                const auto passenger=std::lower_bound(definitions.begin(),definitions.end(),definition.initialPayload->passengerDefinition,
                    [](const auto &candidate,const auto key) { return candidate.key<key; });
                if(passenger==definitions.end()||passenger->key!=definition.initialPayload->passengerDefinition||
                    passenger->kind!=production::EntryKind::Unit||!passenger->passenger||!passenger->passenger->slots||
                    passenger->transportDefinition||passenger->builder||passenger->harvest||passenger->initialPayload)
                    throw std::invalid_argument("Initial payload references an unsupported or nested passenger");
                const auto &carrier=transport->Get(*definition.transportDefinition);
                if(definition.initialPayload->count > carrier.slots / passenger->passenger->slots)
                    throw std::invalid_argument("Initial payload exceeds carrier capacity");
            }
            if (definition.concealment.has_value() != definition.stealthPolicy.has_value())
                throw std::invalid_argument("Stealth policy and concealment definition must be authored together");
            if (definition.concealment)
            {
                if (!concealment || !stealth || concealment->Step() != rate)
                    throw std::invalid_argument("Stealth requires matching immutable catalogs");
                const auto concealmentId = concealment->Id(*definition.concealment);
                const auto policyId = stealth->Id(*definition.stealthPolicy);
                (void)concealment->Get(concealmentId); (void)stealth->Get(policyId);
            }
            if (definition.detection)
            {
                if (!detection || detection->Step() != rate)
                    throw std::invalid_argument("Detection requires a matching immutable catalog");
                const auto detectionId = detection->Id(*definition.detection);
                (void)detection->Get(detectionId);
            }
            if(definition.capture)
            {
                if(!captureDefinitions || captureDefinitions->Step()!=rate || definition.builder || definition.harvest || definition.transportDefinition)
                    throw std::invalid_argument("Capture requires matching definitions and an actor without unsupported work/transport capabilities");
                const auto &capture = captureDefinitions->Get(definition.capture->definition);
                if(capture.activationUpgrade)
                {
                    const auto upgradeKey = capture.activationUpgrade->definition;
                    const auto upgrade = std::lower_bound(definitions.begin(),definitions.end(),upgradeKey,
                        [](const auto &candidate,const auto key) { return candidate.key<key; });
                    if(upgrade==definitions.end() || upgrade->key!=upgradeKey || upgrade->kind!=EntryKind::Upgrade)
                        throw std::invalid_argument("Capture activation must reference an authored upgrade definition");
                }
            }
            if(definition.manualRepair)
            {
                if(definition.kind!=EntryKind::Unit||!definition.builder||!repairDefinitions||
                    !engine::gameplay::rts::repair::IsValidRepairIntervalPolicy(definition.manualRepair->intervalPolicy)||
                    repairDefinitions->Step()!=rate||definition.manualRepair->rateNumerator==0||
                    !definition.manualRepair->rateDenominator||definition.manualRepair->interval.count()<=0)
                    throw std::invalid_argument("Manual repair requires a matching builder repair catalog");
                const auto same=[](const auto &left,const auto &right) {
                    return left.rateNumerator==right.rateNumerator&&left.rateDenominator==right.rateDenominator&&
                        left.interval==right.interval&&left.damageDelay==right.damageDelay&&left.initialDelay==right.initialDelay&&
                        left.intervalPolicy==right.intervalPolicy;
                };
                const auto found=std::find_if(authoredRepairs.begin(),authoredRepairs.end(),
                    [&](const auto &candidate){return same(*definition.manualRepair,candidate);});
                if(found==authoredRepairs.end()) throw std::invalid_argument("Manual repair is absent from the session repair catalog");
                const auto index=static_cast<std::size_t>(found-authoredRepairs.begin());
                if(index>static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)())||
                    index>=repairDefinitions->Count())
                    throw std::out_of_range("Manual repair catalog index is not representable");
            }
            if(definition.armor)
            {
                if(!armor) throw std::invalid_argument("Armor binding requires an explicit catalog");
                (void)armor->Get(definition.armor->definition);
            }
            if(definition.weapon.damage || definition.weapon.secondaryDamage)
            {
                if(!armor && definition.weapon.damageType.value!=0)
                    throw std::invalid_argument("Typed weapon damage requires an explicit catalog");
                if(armor && armor->Policy(definition.weapon.damageType).armor==engine::gameplay::combat::damage::ArmorApplication::Unsupported)
                    throw std::invalid_argument("Weapon damage semantics are not supported by ordinary health execution");
            }
            const bool hasScienceIds = !definition.sciencePrerequisiteIds.empty();
            if (definition.scienceSchemaHash.has_value() != hasScienceIds)
                throw std::invalid_argument("Science prerequisites require matching dense IDs and schema hash");
            if (hasScienceIds)
            {
                if (!scienceCatalog)
                    throw std::logic_error("Science prerequisites require an explicit unlock catalog dependency");
                if (*definition.scienceSchemaHash != scienceCatalog->SchemaHash())
                    throw std::invalid_argument("Science prerequisite schema does not match the unlock catalog");
                if (scienceSchema && *scienceSchema != *definition.scienceSchemaHash)
                    throw std::invalid_argument("Science prerequisite definitions use different schemas");
                scienceSchema = definition.scienceSchemaHash;
                for (const auto id : definition.sciencePrerequisiteIds)
                    if (static_cast<std::size_t>(id.value) >= scienceCatalog->Count())
                        throw std::out_of_range("Science prerequisite dense ID is absent from the unlock catalog");
            }
        }

        requirements.reserve(definitions.size()); weapons.reserve(definitions.size()); poisons.reserve(definitions.size());
        harvestPolicies.reserve(definitions.size());
        regenerationSpawns.reserve(definitions.size()); progressionSpawns.reserve(definitions.size()); veterancySpawns.reserve(definitions.size());
        lifetimes.reserve(definitions.size());
        captureTimings.reserve(definitions.size());
        manualRepairDefinitions.reserve(definitions.size());
        initialPayloads.reserve(definitions.size());
        stealthSpawns.reserve(definitions.size()); detectorSpawns.reserve(definitions.size());
        scienceRanges.reserve(definitions.size());
        for (const auto &definition : definitions)
        {
            std::vector<engine::gameplay::rts::unlocks::UnlockId> scienceIds =
                definition.sciencePrerequisiteIds;
            std::sort(scienceIds.begin(), scienceIds.end(),
                [](const auto left, const auto right) { return left.value < right.value; });
            scienceIds.erase(std::unique(scienceIds.begin(), scienceIds.end()), scienceIds.end());
            if (scienceIds.size() > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) ||
                sciencePrerequisiteIds.size() >
                    static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) - scienceIds.size())
                throw std::length_error("Science prerequisite storage exceeds addressable capacity");
            const auto scienceFirst = static_cast<std::uint32_t>(sciencePrerequisiteIds.size());
            sciencePrerequisiteIds.insert(sciencePrerequisiteIds.end(), scienceIds.begin(), scienceIds.end());
            scienceRanges.push_back({scienceFirst, static_cast<std::uint32_t>(scienceIds.size())});

            requirements.push_back(engine::gameplay::rts::production::AuthorBuild(definition.duration, rate));
            weapons.push_back(engine::gameplay::combat::AuthorWeapon(definition.weapon, rate));
            poisons.push_back(engine::gameplay::combat::AuthorPeriodicDamage(definition.poison, rate));
            harvestPolicies.push_back(definition.harvest
                ? std::optional{engine::gameplay::rts::harvesting::MakeHarvestPolicy(*definition.harvest, rate)}
                : std::nullopt);
            std::optional<RegenerationSpawn> regenerationSpawn;
            if (definition.regeneration)
            {
                if (!regeneration || regeneration->Step() != rate)
                    throw std::invalid_argument("Build regeneration requires a catalog compiled for the production step");
                const auto &config = *definition.regeneration;
                regenerationSpawn = RegenerationSpawn{regeneration->Id(config.definition), rate.TicksFor(config.phase), config.active};
            }
            regenerationSpawns.push_back(regenerationSpawn);
            std::optional<ProgressionSpawn> progressionSpawn;
            if (definition.progression)
            {
                if (!progression) throw std::invalid_argument("Build progression requires a catalog");
                const auto &config = *definition.progression;
                (void)progression->Get(config.definition);
                progressionSpawn = ProgressionSpawn{{config.definition}, config.trainable};
            }
            progressionSpawns.push_back(progressionSpawn);
            std::optional<VeterancySpawn> veterancySpawn;
            if (definition.veterancy)
            {
                if (!veterancy) throw std::invalid_argument("Build veterancy requires a catalog");
                const auto &config = *definition.veterancy;
                (void)veterancy->Get(config.definition);
                veterancySpawn = VeterancySpawn{config.definition, config.awardable};
            }
            veterancySpawns.push_back(veterancySpawn);
            lifetimes.push_back(definition.lifetime ? std::optional{rate.TicksFor(*definition.lifetime)} : std::nullopt);
            captureTimings.push_back(definition.capture ? std::optional{captureDefinitions->Get(definition.capture->definition)} : std::nullopt);
            if(definition.manualRepair)
            {
                const auto same=[](const auto &left,const auto &right) {
                    return left.rateNumerator==right.rateNumerator&&left.rateDenominator==right.rateDenominator&&
                        left.interval==right.interval&&left.damageDelay==right.damageDelay&&left.initialDelay==right.initialDelay&&
                        left.intervalPolicy==right.intervalPolicy;
                };
                const auto found=std::find_if(authoredRepairs.begin(),authoredRepairs.end(),
                    [&](const auto &candidate){return same(*definition.manualRepair,candidate);});
                manualRepairDefinitions.push_back(static_cast<std::uint32_t>(found-authoredRepairs.begin()));
            }
            else manualRepairDefinitions.push_back(std::nullopt);
            initialPayloads.push_back(definition.initialPayload);
            if (definition.concealment)
            {
                const auto concealmentId = concealment->Id(*definition.concealment);
                const auto policyId = stealth->Id(*definition.stealthPolicy);
                const auto &compiled = concealment->Get(concealmentId);
                stealthSpawns.push_back(StealthSpawn{concealmentId, policyId,
                    compiled.stealthDelayTicks, compiled.enabledByDefault});
            }
            else stealthSpawns.push_back(std::nullopt);
            if (definition.detection)
            {
                const auto detectionId = detection->Id(*definition.detection);
                detectorSpawns.push_back(DetectorSpawn{detectionId, detection->Get(detectionId).initiallyDisabled});
            }
            else detectorSpawns.push_back(std::nullopt);
        }
        hasPrerequisites = std::any_of(definitions.begin(), definitions.end(), [](const auto &definition) {
            return (definition.prerequisites && !definition.prerequisites->Empty()) ||
                !definition.sciencePrerequisiteIds.empty();
        });
        scienceSchemaHash = scienceSchema;

    }
    BuildCatalog(const BuildCatalog &) = delete;
    BuildCatalog &operator=(const BuildCatalog &) = delete;
    engine::time::FixedStep Step() const noexcept { return step; }
    std::span<const BuildDefinition> Definitions() const noexcept { return definitions; }
    const auto &Requirement(std::size_t i) const { return requirements[i]; }
    const auto &Weapon(std::size_t i) const { return weapons[i]; }
    const auto &Poison(std::size_t i) const { return poisons[i]; }
    const auto &Harvest(std::size_t i) const { return harvestPolicies[i]; }
    const auto &Regeneration(std::size_t i) const { return regenerationSpawns[i]; }
    const auto &Progression(std::size_t i) const { return progressionSpawns[i]; }
    const auto &Veterancy(std::size_t i) const { return veterancySpawns[i]; }
    const auto &Lifetime(std::size_t i) const { return lifetimes[i]; }
    const auto &CaptureTiming(std::size_t i) const { return captureTimings[i]; }
    const auto &ManualRepair(std::size_t i) const { return manualRepairDefinitions[i]; }
    const auto &InitialPayload(std::size_t i) const { return initialPayloads[i]; }
    const auto &Stealth(std::size_t i) const { return stealthSpawns[i]; }
    const auto &Detector(std::size_t i) const { return detectorSpawns[i]; }
    std::span<const engine::gameplay::rts::unlocks::UnlockId> SciencePrerequisites(
        std::size_t i) const noexcept
    {
        const auto range = scienceRanges[i];
        if (range.count == 0)
            return {};
        return {sciencePrerequisiteIds.data() + range.first, range.count};
    }
    const auto &ScienceSchemaHash() const noexcept { return scienceSchemaHash; }
    bool HasSciencePrerequisites() const noexcept { return scienceSchemaHash.has_value(); }
    bool HasPrerequisites() const noexcept
    { return hasPrerequisites; }
    std::size_t DefinitionIndex(std::uint32_t key) const
    {
        return static_cast<std::size_t>(std::lower_bound(definitions.begin(), definitions.end(), key,
            [](const auto &definition, auto value) { return definition.key < value; }) - definitions.begin());
    }

private:
    struct ScienceRequirementRange
    {
        std::uint32_t first{};
        std::uint32_t count{};
    };

    std::vector<BuildDefinition> definitions;
    std::vector<engine::gameplay::rts::production::BuildWork> requirements;
    std::vector<engine::gameplay::combat::WeaponDefinition> weapons;
    std::vector<engine::gameplay::combat::PeriodicDamagePolicy> poisons;
    std::vector<std::optional<engine::gameplay::rts::harvesting::HarvestPolicy>> harvestPolicies;
    std::vector<std::optional<RegenerationSpawn>> regenerationSpawns;
    std::vector<std::optional<ProgressionSpawn>> progressionSpawns;
    std::vector<std::optional<VeterancySpawn>> veterancySpawns;
    std::vector<std::optional<std::uint64_t>> lifetimes;
    std::vector<std::optional<capture::CompiledCaptureDefinition>> captureTimings;
    std::vector<std::optional<std::uint32_t>> manualRepairDefinitions;
    std::vector<std::optional<InitialPayloadDefinition>> initialPayloads;
    std::vector<std::optional<StealthSpawn>> stealthSpawns;
    std::vector<std::optional<DetectorSpawn>> detectorSpawns;
    std::vector<ScienceRequirementRange> scienceRanges;
    std::vector<engine::gameplay::rts::unlocks::UnlockId> sciencePrerequisiteIds;
    bool hasPrerequisites{};
    std::optional<engine::gameplay::rts::unlocks::UnlockSchemaHash> scienceSchemaHash;
    const engine::time::FixedStep step;
};
}
