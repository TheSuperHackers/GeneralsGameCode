module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.crates.systems.crate_reward_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.components.healing;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.combat.systems.healing_system;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.progression.definitions.progression_definition;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.gameplay.progression.systems.progression_system;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.crates.algorithms.crate_veterancy_projection;
export import games.generalszh.gameplay.crates.components.crate_state;
export import games.generalszh.gameplay.crates.definitions.crate_definition;
export import games.generalszh.gameplay.crates.inputs.crate_pickup_batch;
export import games.generalszh.gameplay.crates.inputs.crate_reward_batch;
export import games.generalszh.gameplay.crates.systems.crate_claim_system;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.upgrades.components.upgrade_status;

export namespace generalszh::crates
{
struct CrateRewardLimits final
{
    std::size_t referenceCount{};
    std::size_t outputAwards{};
};

class CrateRewardSystem final
{
public:
    using LifeState = engine::gameplay::combat::LifeState;
    using Health = engine::gameplay::combat::Health;
    using HealthCapacity = engine::gameplay::combat::HealthCapacity;
    using PendingHealing = engine::gameplay::combat::PendingHealing;
    using GridPosition = engine::gameplay::navigation::GridPosition;
    using ProgressionState = engine::gameplay::progression::ProgressionState;
    using ProgressionDefinitionRef = engine::gameplay::progression::ProgressionDefinitionRef;
    using ProgressionEligibility = engine::gameplay::progression::ProgressionEligibility;
    using ProducedUnit = generalszh::production::ProducedUnit;
    using Structure = generalszh::construction::Structure;
    using ResearchedUpgrade = generalszh::production::ResearchedUpgrade;
    using UpgradeInstanceStatus = generalszh::upgrades::UpgradeInstanceStatus;

    // Health/healing/progression are optional because money and veterancy
    // pickup must match collectors that do not carry those columns.  Only a
    // heal effect uses all three optional health columns.
    using Query = ecs::Query<ecs::Read<LifeState>, ecs::Optional<Health>,
        ecs::Optional<HealthCapacity>, ecs::OptionalWrite<PendingHealing>,
        ecs::Optional<ProducedUnit>, ecs::Optional<Structure>, ecs::Optional<GridPosition>,
        ecs::Optional<ProgressionState>, ecs::Optional<ProgressionDefinitionRef>,
        ecs::Optional<ProgressionEligibility>>;
    using CrateRows = ecs::Query<ecs::Read<CrateState>>;
    using UpgradeRows = ecs::Query<ecs::Read<ResearchedUpgrade>,
        ecs::Read<UpgradeInstanceStatus>>;

    struct JoinedAccess final
    {
        static std::vector<ecs::AccessDescriptor> ResolveAccesses(
            const ecs::ComponentRegistry &components)
        {
            const auto main = Query::ResolveAccesses(components);
            const auto crates = CrateRows::ResolveAccesses(components);
            const auto upgrades = UpgradeRows::ResolveAccesses(components);
            std::vector<ecs::AccessDescriptor> result;
            result.reserve(main.size() + crates.size() + upgrades.size());
            result.insert(result.end(), main.begin(), main.end());
            result.insert(result.end(), crates.begin(), crates.end());
            result.insert(result.end(), upgrades.begin(), upgrades.end());
            return result;
        }
    };

    using AuxiliaryAccess = JoinedAccess;

private:
    template<typename T>
    static void AppendBounded(std::vector<T> &values, const std::size_t limit,
        const T &value, const char *message)
    {
        if (values.size() >= limit)
            throw std::length_error(message);
        values.push_back(value);
    }

    template<typename T>
    static void ResizeBounded(std::vector<T> &values, const std::size_t size,
        const std::size_t limit, const char *message)
    {
        if (size > limit)
            throw std::length_error(message);
        values.resize(size);
    }

    template<typename T>
    static void CopyBounded(std::vector<T> &values, const std::span<const T> source,
        const std::size_t limit, const char *message)
    {
        if (source.size() > limit)
            throw std::length_error(message);
        values.resize(source.size());
        std::copy(source.begin(), source.end(), values.begin());
    }

public:

    CrateRewardSystem(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        const CrateDefinitionCatalog &crateDefinitions,
        const engine::gameplay::progression::ProgressionCatalog &progressionDefinitions,
        engine::gameplay::progression::ProgressionBatch &progressionBatch,
        CratePickupBatch &pickupBatch, CrateRewardBatch &rewardBatch,
        const std::size_t entityIndexCapacity, const CrateRewardLimits limits) :
        world_(world), grid_(grid), crateDefinitions_(crateDefinitions),
        progressionDefinitions_(progressionDefinitions), progressionBatch_(progressionBatch),
        pickupBatch_(pickupBatch), rewardBatch_(rewardBatch), entityLimit_(entityIndexCapacity),
        claimLimit_(pickupBatch.Capacity()), prefixLimit_(progressionBatch.Capacity()),
        referenceLimit_(limits.referenceCount), outputAwardLimit_(limits.outputAwards)
    {
        if (!entityIndexCapacity || !pickupBatch.Capacity())
            throw std::invalid_argument("Crate reward capacities must be positive");
        if (progressionBatch.Capacity() > pickupBatch.ProgressionPrefixCapacity())
            throw std::invalid_argument("Crate progression prefix storage is too small");
        if (rewardBatch.Capacity() < pickupBatch.Capacity())
            throw std::invalid_argument("Crate reward publication capacity is smaller than pickup capacity");
        crates_.reserve(entityLimit_);
        upgrades_.reserve(entityLimit_);
        prefix_.reserve(prefixLimit_);
        sortedPrefix_.reserve(prefixLimit_);
        prefixIndices_.reserve(prefixLimit_);
        prefixRanges_.reserve(prefixLimit_);
        experienceEvents_.reserve(referenceLimit_);
        moneyEvents_.reserve(claimLimit_);
        successful_.reserve(claimLimit_);
        xpAwards_.reserve(outputAwardLimit_);
        requests_.reserve(claimLimit_);
        consumedCrates_.reserve(claimLimit_);
        indexedClaims_.reserve(claimLimit_);
        moneyReferences_.reserve(claimLimit_);
        healReferences_.reserve(claimLimit_);
        veterancyReferences_.reserve(referenceLimit_);
        targetRows_.reserve(entityLimit_);
        targetRanges_.reserve(entityLimit_);
        moneyRanges_.reserve(claimLimit_);
        healRanges_.reserve(claimLimit_);
        veterancyRanges_.reserve(entityLimit_);
        claimObserved_.reserve(claimLimit_);
        moneyAmounts_.reserve(claimLimit_);
        experienceAmounts_.reserve(referenceLimit_);
    }

    void Configure()
    {
        if (crateRows_ || upgradeRows_)
            throw std::logic_error("Crate reward configuration is one-shot");
        if (!world_.ComponentsFinalized())
            throw std::logic_error("Crate reward queries require finalized components");
        crateRows_ = std::make_unique<CrateRows>(world_);
        upgradeRows_ = std::make_unique<UpgradeRows>(world_);
    }

    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        if (!crateRows_ || !upgradeRows_)
            throw std::logic_error("Crate reward system was not configured");
        if (rewardBatch_.IsPublished())
            throw std::logic_error("Release crate rewards before the next tick");

        // This prefix was captured by CrateClaimSystem before this
        // PreSimulation publication.  Same-tick kill experience is appended
        // later by KillExperienceSystem; it is not part of crate projection.
        const auto pendingPrefix = pickupBatch_.ProgressionPrefix();
        CopyBounded(prefix_, pendingPrefix, prefixLimit_,
            "Crate progression prefix capacity exhausted");
        ResizeBounded(prefixIndices_, prefix_.size(), prefixLimit_,
            "Crate progression prefix index capacity exhausted");
        std::iota(prefixIndices_.begin(), prefixIndices_.end(), std::size_t{});
        std::sort(prefixIndices_.begin(), prefixIndices_.end(), [&](const auto left, const auto right) {
            return std::tie(prefix_[left].entity.index, prefix_[left].entity.generation, left) <
                std::tie(prefix_[right].entity.index, prefix_[right].entity.generation, right);
        });
        ResizeBounded(sortedPrefix_, prefix_.size(), prefixLimit_,
            "Crate sorted progression prefix capacity exhausted");
        for (std::size_t index = 0; index != prefixIndices_.size(); ++index)
            sortedPrefix_[index] = prefix_[prefixIndices_[index]];
        BuildPrefixRanges();

        crates_.clear();
        crateRows_->ForEachChunk([&](CrateRows::Chunk chunk) {
            const auto states = chunk.template Get<CrateState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                AppendBounded(crates_, entityLimit_, CrateObservation{chunk.Entities()[row], states[row]},
                    "Crate reward observation capacity exhausted");
        });
        std::sort(crates_.begin(), crates_.end(), [](const auto &left, const auto &right) {
            return EntityLess(left.entity, right.entity);
        });

        upgrades_.clear();
        upgradeRows_->ForEachChunk([&](UpgradeRows::Chunk chunk) {
            const auto grants = chunk.template Get<ResearchedUpgrade>();
            const auto statuses = chunk.template Get<UpgradeInstanceStatus>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                if (statuses[row].value == generalszh::upgrades::Status::Complete)
                    AppendBounded(upgrades_, entityLimit_, UpgradeObservation{
                        grants[row].account, grants[row].definition},
                        "Crate upgrade observation capacity exhausted");
        });
        std::sort(upgrades_.begin(), upgrades_.end(), [](const auto &left, const auto &right) {
            return std::tie(left.owner.index, left.owner.generation, left.definition) <
                std::tie(right.owner.index, right.owner.generation, right.definition);
        });

        BuildRewardIndexes(query);
        if (prefix_.size() > progressionBatch_.Capacity())
            throw std::length_error("Crate progression publication capacity exhausted");
        ResizeBounded(claimObserved_, pickupBatch_.Claims().size(), claimLimit_,
            "Crate reward observation capacity exhausted");
        std::fill(claimObserved_.begin(), claimObserved_.end(), std::uint8_t{});
        ResizeBounded(moneyAmounts_, pickupBatch_.Claims().size(), claimLimit_,
            "Crate money amount capacity exhausted");
        std::fill(moneyAmounts_.begin(), moneyAmounts_.end(), std::uint64_t{});
        ResizeBounded(experienceAmounts_, veterancyReferences_.size(), referenceLimit_,
            "Crate experience amount capacity exhausted");
        std::fill(experienceAmounts_.begin(), experienceAmounts_.end(), std::uint64_t{});
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context)
    {
        (void)context;
        const auto lives = chunk.template Get<LifeState>();
        const auto health = chunk.template Get<Health>();
        const auto capacities = chunk.template Get<HealthCapacity>();
        auto pending = chunk.template Get<PendingHealing>();
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        const auto positions = chunk.template Get<GridPosition>();
        const auto states = chunk.template Get<ProgressionState>();
        const auto refs = chunk.template Get<ProgressionDefinitionRef>();
        const auto eligibility = chunk.template Get<ProgressionEligibility>();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto entity = chunk.Entities()[row];

            // Money is a collector-local effect.  The index contains only
            // claims for this entity, so unrelated accounts/crates never enter
            // the chunk loop.
            if (const auto range = FindRange(moneyRanges_, entity))
            {
                for (std::size_t offset = 0; offset != range->count; ++offset)
                {
                    const auto &reference = moneyReferences_[range->first + offset];
                    const auto &indexed = indexedClaims_[reference.claimIndex];
                    claimObserved_[reference.claimIndex] = 1;
                    moneyAmounts_[reference.claimIndex] = MoneyAmount(
                        indexed.crate->state, *indexed.definition, indexed.claim.account);
                }
            }

            // HealCrateCollide calls healAllObjects on the collector's account,
            // so this account join intentionally visits every owned live row.
            // The optional columns keep money/veterancy-only archetypes in the
            // same typed query; absent health merely omits this row's heal
            // mutation, not the collector's successful pickup.
            if (const auto range = FindRange(healRanges_, Owner(units, structures, row)))
            {
                for (std::size_t offset = 0; offset != range->count; ++offset)
                {
                    const auto &reference = healReferences_[range->first + offset];
                    const auto &claim = indexedClaims_[reference.claimIndex].claim;
                    if (!lives[row].alive)
                        continue;
                    if (claim.collector == entity)
                        claimObserved_[reference.claimIndex] = 1;
                    if (!health.empty() && !capacities.empty() && !pending.empty() &&
                        health[row].current < capacities[row].maximum)
                    {
                        const auto missing = capacities[row].maximum - health[row].current;
                        if (pending[row].quantity < missing)
                            pending[row].quantity = missing;
                    }
                }
            }

            // Veterancy references are pre-expanded by account/radius in the
            // joined hook.  This chunk owns the ordinary sequential projection
            // and writes one bounded result slot per (target, canonical claim).
            if (lives[row].alive && !states.empty() && !refs.empty() && !eligibility.empty() &&
                refs[row].index < progressionDefinitions_.Size())
            {
                if (const auto range = FindRange(veterancyRanges_, entity))
                {
                    const auto &definition = progressionDefinitions_.Get(refs[row].index);
                    auto projection = MakeProjection(states[row].experience, states[row].level,
                        eligibility[row].trainable, &definition);
                    ApplyPrefix(projection, definition, PrefixFor(entity));
                    if (!projection.valid)
                        continue;
                    for (std::size_t offset = 0; offset != range->count; ++offset)
                    {
                        const auto referenceIndex = range->first + offset;
                        const auto &reference = veterancyReferences_[referenceIndex];
                        const auto &indexed = indexedClaims_[reference.claimIndex];
                        const auto levels = indexed.definition->addsOwnerVeterancy
                            ? indexed.crate->state.veterancyLevel : 1u;
                        experienceAmounts_[referenceIndex] = ApplyLevelGain(
                            projection, definition, levels);
                    }
                }
            }
        }
    }

    void AfterChunks(Query &, ecs::SystemContext &context)
    {
        experienceEvents_.clear();
        moneyEvents_.clear();
        const auto claims = pickupBatch_.Claims();
        ResizeBounded(successful_, claims.size(), claimLimit_,
            "Crate reward success capacity exhausted");
        std::fill(successful_.begin(), successful_.end(), std::uint8_t{});

        for (std::size_t claimIndex = 0; claimIndex != claims.size(); ++claimIndex)
        {
            const auto &indexed = indexedClaims_[claimIndex];
            if (indexed.definition->reward == RewardKind::Money)
            {
                if (claimObserved_[claimIndex])
                {
                    successful_[claimIndex] = 1;
                    if (moneyAmounts_[claimIndex])
                        AppendBounded(moneyEvents_, claimLimit_, RewardEvent{claims[claimIndex].ordinal,
                            EventKind::Money, claims[claimIndex].account,
                            moneyAmounts_[claimIndex], 0},
                            "Crate money event capacity exhausted");
                }
            }
            else if (indexed.definition->reward == RewardKind::Heal && claimObserved_[claimIndex])
                successful_[claimIndex] = 1;
        }

        for (std::size_t referenceIndex = 0; referenceIndex != veterancyReferences_.size();
            ++referenceIndex)
        {
            const auto amount = experienceAmounts_[referenceIndex];
            if (!amount)
                continue;
            const auto &reference = veterancyReferences_[referenceIndex];
            const auto &claim = indexedClaims_[reference.claimIndex].claim;
            successful_[reference.claimIndex] = 1;
            AppendBounded(experienceEvents_, referenceLimit_, RewardEvent{claim.ordinal, EventKind::Experience,
                reference.key, amount, referenceIndex}, "Crate experience event capacity exhausted");
        }
        std::sort(experienceEvents_.begin(), experienceEvents_.end(),
            [](const auto &left, const auto &right) {
                return std::tie(left.claimOrdinal, left.entity.index, left.entity.generation,
                    left.referenceOrdinal) <
                    std::tie(right.claimOrdinal, right.entity.index, right.entity.generation,
                        right.referenceOrdinal);
            });
        std::sort(moneyEvents_.begin(), moneyEvents_.end(),
            [](const auto &left, const auto &right) {
                return std::tie(left.claimOrdinal, left.entity.index, left.entity.generation,
                    left.referenceOrdinal) <
                    std::tie(right.claimOrdinal, right.entity.index, right.entity.generation,
                        right.referenceOrdinal);
            });

        xpAwards_.clear();
        for (const auto &event : experienceEvents_)
            AppendBounded(xpAwards_, outputAwardLimit_, engine::gameplay::progression::AcceptedExperience{
                event.entity, event.amount}, "Crate XP award capacity exhausted");
        requests_.clear();
        for (const auto &event : moneyEvents_)
        {
            if (event.amount == 0) continue;
            assert(event.amount <= static_cast<std::uint64_t>(
                (std::numeric_limits<std::uint32_t>::max)()));
            const auto amount = static_cast<std::uint32_t>(event.amount);
            AppendBounded(requests_, claimLimit_, economy::AccountRequest{event.entity,
                {economy::AccountOperation::Deposit, amount, true, true}},
                "Crate account request capacity exhausted");
        }

        const auto current = progressionBatch_.PendingInputs();
        if (current.size() != prefix_.size() ||
            !std::equal(current.begin(), current.end(), prefix_.begin(),
                [](const auto &left, const auto &right) {
                    return left.entity == right.entity && left.amount == right.amount;
                }))
            throw std::logic_error("Progression prefix changed before crate publication");
        if (prefix_.size() > progressionBatch_.Capacity() ||
            xpAwards_.size() > progressionBatch_.Capacity() - prefix_.size())
            throw std::length_error("Crate progression publication capacity exhausted");
        if (!xpAwards_.empty())
            progressionBatch_.Append(xpAwards_);

        consumedCrates_.clear();
        for (std::size_t index = 0; index != claims.size(); ++index)
            if (successful_[index])
                AppendBounded(consumedCrates_, claimLimit_, claims[index].crate,
                    "Crate consumed-entity capacity exhausted");
        std::sort(consumedCrates_.begin(), consumedCrates_.end(), EntityLess);
        consumedCrates_.erase(std::unique(consumedCrates_.begin(), consumedCrates_.end()),
            consumedCrates_.end());
        rewardBatch_.Publish(requests_, consumedCrates_);
        for (const auto crate : consumedCrates_)
            context.Commands().Destroy(crate);
    }

private:
    enum class EventKind : std::uint8_t { Money, Experience };
    struct RewardEvent final
    {
        std::size_t claimOrdinal{};
        EventKind kind{EventKind::Money};
        ecs::Entity entity{};
        std::uint64_t amount{};
        std::size_t referenceOrdinal{};
    };
    struct CrateObservation final { ecs::Entity entity{}; CrateState state{}; };
    struct UpgradeObservation final
    {
        ecs::Entity owner{};
        std::uint32_t definition{};
    };
    struct PrefixRange final { ecs::Entity entity{}; std::size_t first{}, count{}; };
    struct IndexedClaim final
    {
        CrateClaim claim{};
        const CrateObservation *crate{};
        const CrateDefinition *definition{};
    };
    struct ClaimReference final
    {
        ecs::Entity key{};
        std::size_t claimIndex{};
    };
    struct EntityRange final { ecs::Entity key{}; std::size_t first{}, count{}; };
    struct TargetObservation final
    {
        ecs::Entity entity{}, account{};
        engine::gameplay::navigation::Cell cell{engine::gameplay::navigation::InvalidCell};
        bool alive{}, hasPosition{};
    };

    static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
    {
        return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
    }

    static ecs::Entity Owner(const std::span<const ProducedUnit> units,
        const std::span<const Structure> structures, const std::size_t row) noexcept
    {
        return !structures.empty() ? structures[row].account :
            !units.empty() ? units[row].account : ecs::Entity{};
    }

    const CrateObservation *FindCrate(const ecs::Entity entity) const noexcept
    {
        const auto it = std::lower_bound(crates_.begin(), crates_.end(), entity,
            [](const CrateObservation &row, const ecs::Entity value) { return EntityLess(row.entity, value); });
        return it != crates_.end() && it->entity == entity ? &*it : nullptr;
    }

    static bool AccountTargetLess(const TargetObservation &left,
        const TargetObservation &right) noexcept
    {
        return std::tie(left.account.index, left.account.generation, left.entity.index,
            left.entity.generation) < std::tie(right.account.index, right.account.generation,
            right.entity.index, right.entity.generation);
    }

    static bool ReferenceLess(const ClaimReference &left, const ClaimReference &right,
        const std::vector<IndexedClaim> &claims) noexcept
    {
        return std::tie(left.key.index, left.key.generation,
            claims[left.claimIndex].claim.ordinal, left.claimIndex) <
            std::tie(right.key.index, right.key.generation,
                claims[right.claimIndex].claim.ordinal, right.claimIndex);
    }

    const EntityRange *FindRange(const std::vector<EntityRange> &ranges,
        const ecs::Entity key) const noexcept
    {
        const auto it = std::lower_bound(ranges.begin(), ranges.end(), key,
            [](const EntityRange &range, const ecs::Entity value) {
                return EntityLess(range.key, value);
            });
        return it != ranges.end() && it->key == key ? &*it : nullptr;
    }

    static void BuildTargetRanges(const std::vector<TargetObservation> &targets,
        std::vector<EntityRange> &ranges, const std::size_t limit)
    {
        ranges.clear();
        for (std::size_t first = 0; first < targets.size();)
        {
            const auto key = targets[first].account;
            std::size_t last = first + 1;
            while (last < targets.size() && targets[last].account == key)
                ++last;
            AppendBounded(ranges, limit, EntityRange{key, first, last - first},
                "Crate target range capacity exhausted");
            first = last;
        }
    }

    static void BuildReferenceRanges(const std::vector<ClaimReference> &references,
        std::vector<EntityRange> &ranges, const std::size_t limit)
    {
        ranges.clear();
        for (std::size_t first = 0; first < references.size();)
        {
            const auto key = references[first].key;
            std::size_t last = first + 1;
            while (last < references.size() && references[last].key == key)
                ++last;
            AppendBounded(ranges, limit, EntityRange{key, first, last - first},
                "Crate reference range capacity exhausted");
            first = last;
        }
    }

    static void AppendReference(std::vector<ClaimReference> &references,
        const std::size_t limit, const ClaimReference reference)
    {
        AppendBounded(references, limit, reference, "Crate reward reference capacity exhausted");
    }

    void BuildRewardIndexes(Query &query)
    {
        targetRows_.clear();
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            const auto lives = chunk.template Get<LifeState>();
            const auto units = chunk.template Get<ProducedUnit>();
            const auto structures = chunk.template Get<Structure>();
            const auto positions = chunk.template Get<GridPosition>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto account = Owner(units, structures, row);
                if (!account.IsValid())
                    continue;
                AppendBounded(targetRows_, entityLimit_, TargetObservation{chunk.Entities()[row], account,
                    !positions.empty() ? positions[row].cell : engine::gameplay::navigation::InvalidCell,
                    lives[row].alive, !positions.empty()},
                    "Crate reward target capacity exhausted");
            }
        });
        std::sort(targetRows_.begin(), targetRows_.end(), AccountTargetLess);
        BuildTargetRanges(targetRows_, targetRanges_, entityLimit_);

        const auto claims = pickupBatch_.Claims();
        indexedClaims_.clear();
        for (std::size_t claimIndex = 0; claimIndex != claims.size(); ++claimIndex)
        {
            const auto &claim = claims[claimIndex];
            const auto *crate = FindCrate(claim.crate);
            if (!crate)
                throw std::logic_error("Crate reward claim references a missing crate");
            if (claim.definition != crate->state.definition)
                throw std::logic_error("Crate reward claim definition changed before reward");
            const auto &definition = crateDefinitions_.Get(claim.definition);
            if (definition.reward != RewardKind::Money && definition.reward != RewardKind::Heal &&
                definition.reward != RewardKind::Veterancy)
                throw std::logic_error("Crate reward claim contains an unsupported reward family");
            AppendBounded(indexedClaims_, claimLimit_, IndexedClaim{claim, crate, &definition},
                "Crate indexed claim capacity exhausted");
        }

        moneyReferences_.clear();
        healReferences_.clear();
        veterancyReferences_.clear();
        for (std::size_t claimIndex = 0; claimIndex != indexedClaims_.size(); ++claimIndex)
        {
            const auto &indexed = indexedClaims_[claimIndex];
            if (indexed.definition->reward == RewardKind::Money)
                AppendReference(moneyReferences_, claimLimit_, {indexed.claim.collector, claimIndex});
            else if (indexed.definition->reward == RewardKind::Heal)
                AppendReference(healReferences_, claimLimit_, {indexed.claim.account, claimIndex});
            else if (indexed.definition->effectRange == 0)
                AppendReference(veterancyReferences_, referenceLimit_,
                    {indexed.claim.collector, claimIndex});
            else if (indexed.claim.hasCollectorCell)
            {
                const auto *targets = FindRange(targetRanges_, indexed.claim.account);
                if (!targets)
                    continue;
                for (std::size_t offset = 0; offset != targets->count; ++offset)
                {
                    const auto &target = targetRows_[targets->first + offset];
                    if (target.alive && target.hasPosition && WithinRange(
                        indexed.claim.collectorCell, target.cell, indexed.definition->effectRange))
                        AppendReference(veterancyReferences_, referenceLimit_,
                            {target.entity, claimIndex});
                }
            }
        }

        std::sort(moneyReferences_.begin(), moneyReferences_.end(),
            [&](const auto &left, const auto &right) {
                return ReferenceLess(left, right, indexedClaims_);
            });
        std::sort(healReferences_.begin(), healReferences_.end(),
            [&](const auto &left, const auto &right) {
                return ReferenceLess(left, right, indexedClaims_);
            });
        std::sort(veterancyReferences_.begin(), veterancyReferences_.end(),
            [&](const auto &left, const auto &right) {
                return ReferenceLess(left, right, indexedClaims_);
            });
        BuildReferenceRanges(moneyReferences_, moneyRanges_, claimLimit_);
        BuildReferenceRanges(healReferences_, healRanges_, claimLimit_);
        BuildReferenceRanges(veterancyReferences_, veterancyRanges_, entityLimit_);
    }

    std::span<const engine::gameplay::progression::AcceptedExperience> PrefixFor(
        const ecs::Entity entity) const noexcept
    {
        const auto it = std::lower_bound(prefixRanges_.begin(), prefixRanges_.end(), entity,
            [](const PrefixRange &range, const ecs::Entity value) { return EntityLess(range.entity, value); });
        if (it == prefixRanges_.end() || it->entity != entity)
            return {};
        return {sortedPrefix_.data() + it->first, it->count};
    }

    void BuildPrefixRanges()
    {
        prefixRanges_.clear();
        for (std::size_t first = 0; first != sortedPrefix_.size();)
        {
            const auto entity = sortedPrefix_[first].entity;
            std::size_t last = first + 1;
            while (last != sortedPrefix_.size() && sortedPrefix_[last].entity == entity)
                ++last;
            AppendBounded(prefixRanges_, prefixLimit_, PrefixRange{entity, first, last - first},
                "Crate progression prefix range capacity exhausted");
            first = last;
        }
    }

    bool HasUpgrade(const ecs::Entity owner, const MoneyUpgradeBoost &boost) const noexcept
    {
        const auto it = std::lower_bound(upgrades_.begin(), upgrades_.end(),
            std::tuple{owner.index, owner.generation, boost.definition},
            [](const UpgradeObservation &row, const auto &key) {
                return std::tie(row.owner.index, row.owner.generation, row.definition) < key;
            });
        return it != upgrades_.end() && it->owner == owner && it->definition == boost.definition;
    }

    std::uint64_t MoneyAmount(const CrateState &, const CrateDefinition &definition,
        const ecs::Entity account) const
    {
        std::uint64_t result = definition.moneyProvided;
        // Legacy returns the first matching boost.  The runtime grant is
        // matched by the complete account entity and canonical definition key.
        for (const auto &boost : definition.moneyUpgradeBoosts)
            if (HasUpgrade(account, boost))
            {
                result += boost.amount;
                break;
            }
        assert(result <= static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
        return result;
    }

    bool WithinRange(const engine::gameplay::navigation::Cell first,
        const engine::gameplay::navigation::Cell second, const std::uint32_t radius) const noexcept
    {
        const auto width = static_cast<std::uint64_t>(grid_.Width());
        if (!width) return false;
        const auto firstX = static_cast<std::uint64_t>(first) % width;
        const auto firstY = static_cast<std::uint64_t>(first) / width;
        const auto secondX = static_cast<std::uint64_t>(second) % width;
        const auto secondY = static_cast<std::uint64_t>(second) / width;
        const auto dx = firstX > secondX ? firstX - secondX : secondX - firstX;
        const auto dy = firstY > secondY ? firstY - secondY : secondY - firstY;
        return dx * dx + dy * dy <= static_cast<std::uint64_t>(radius) * radius;
    }

    ecs::World &world_;
    const engine::gameplay::navigation::NavigationGrid &grid_;
    const CrateDefinitionCatalog &crateDefinitions_;
    const engine::gameplay::progression::ProgressionCatalog &progressionDefinitions_;
    engine::gameplay::progression::ProgressionBatch &progressionBatch_;
    CratePickupBatch &pickupBatch_;
    CrateRewardBatch &rewardBatch_;
    const std::size_t entityLimit_;
    const std::size_t claimLimit_;
    const std::size_t prefixLimit_;
    const std::size_t referenceLimit_;
    const std::size_t outputAwardLimit_;
    std::unique_ptr<CrateRows> crateRows_;
    std::unique_ptr<UpgradeRows> upgradeRows_;
    std::vector<CrateObservation> crates_;
    std::vector<UpgradeObservation> upgrades_;
    std::vector<engine::gameplay::progression::AcceptedExperience> prefix_, sortedPrefix_;
    std::vector<std::size_t> prefixIndices_;
    std::vector<PrefixRange> prefixRanges_;
    std::vector<IndexedClaim> indexedClaims_;
    std::vector<ClaimReference> moneyReferences_, healReferences_, veterancyReferences_;
    std::vector<EntityRange> targetRanges_, moneyRanges_, healRanges_, veterancyRanges_;
    std::vector<TargetObservation> targetRows_;
    std::vector<std::uint8_t> claimObserved_;
    std::vector<std::uint64_t> moneyAmounts_, experienceAmounts_;
    std::vector<RewardEvent> experienceEvents_, moneyEvents_;
    std::vector<std::uint8_t> successful_;
    std::vector<engine::gameplay::progression::AcceptedExperience> xpAwards_;
    std::vector<economy::AccountRequest> requests_;
    std::vector<ecs::Entity> consumedCrates_;
};
} // namespace generalszh::crates

export namespace ecs
{
template<> struct SystemTraits<generalszh::crates::CrateRewardSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.crates.reward";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<engine::gameplay::combat::HealingSystem,
        engine::gameplay::progression::ProgressionSystem>;
    using After = SystemTypeList<generalszh::crates::CrateClaimSystem,
        engine::gameplay::combat::HealthSystem>;
};
} // namespace ecs
