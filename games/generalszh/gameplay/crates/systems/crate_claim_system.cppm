module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.crates.systems.crate_claim_system;
export import engine.ecs.system.system;
export import engine.events.storage.event_batch;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.progression.definitions.progression_definition;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.gameplay.progression.systems.progression_system;
export import engine.gameplay.pickups.algorithms.pickup_arbitration;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import engine.gameplay.spatial.contacts.events.contact_pair;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.crates.algorithms.crate_veterancy_projection;
export import games.generalszh.gameplay.crates.components.crate_state;
export import games.generalszh.gameplay.crates.definitions.account_pickup_policy;
export import games.generalszh.gameplay.crates.definitions.collector_definition;
export import games.generalszh.gameplay.crates.definitions.crate_definition;
export import games.generalszh.gameplay.crates.inputs.crate_pickup_batch;
export import games.generalszh.gameplay.production.components.production_state;

export namespace generalszh::crates
{
class CrateClaimSystem final
{
public:
    using LifeState = engine::gameplay::combat::LifeState;
    using GridPosition = engine::gameplay::navigation::GridPosition;
    using ProgressionState = engine::gameplay::progression::ProgressionState;
    using ProgressionDefinitionRef = engine::gameplay::progression::ProgressionDefinitionRef;
    using ProgressionEligibility = engine::gameplay::progression::ProgressionEligibility;
    using UnlockState = engine::gameplay::rts::unlocks::UnlockState;
    using ProducedUnit = generalszh::production::ProducedUnit;
    using Structure = generalszh::construction::Structure;

    // The main query is the collector/target population.  All ordinary
    // eligibility work runs over these typed chunks; auxiliary queries are
    // only the distinct crate and account joins needed by the hook.
    using Query = ecs::Query<ecs::Read<LifeState>, ecs::Optional<ProducedUnit>,
        ecs::Optional<Structure>, ecs::Optional<GridPosition>, ecs::Optional<ProgressionState>,
        ecs::Optional<ProgressionDefinitionRef>, ecs::Optional<ProgressionEligibility>>;
    using CrateRows = ecs::Query<ecs::Read<CrateState>>;
    using AccountRows = ecs::Query<ecs::Read<UnlockState>>;

    struct JoinedAccess final
    {
        static std::vector<ecs::AccessDescriptor> ResolveAccesses(
            const ecs::ComponentRegistry &components)
        {
            const auto main = Query::ResolveAccesses(components);
            const auto crates = CrateRows::ResolveAccesses(components);
            const auto accounts = AccountRows::ResolveAccesses(components);
            std::vector<ecs::AccessDescriptor> result;
            result.reserve(main.size() + crates.size() + accounts.size());
            result.insert(result.end(), main.begin(), main.end());
            result.insert(result.end(), crates.begin(), crates.end());
            result.insert(result.end(), accounts.begin(), accounts.end());
            return result;
        }
    };

    using AuxiliaryAccess = JoinedAccess;

    CrateClaimSystem(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        const CrateDefinitionCatalog &crateDefinitions,
        const CollectorDefinitionCatalog &collectorDefinitions,
        const AccountPickupPolicies &accountPolicies,
        const engine::gameplay::progression::ProgressionCatalog &progressionDefinitions,
        engine::gameplay::progression::ProgressionBatch &progressionBatch,
        CratePickupBatch &pickupBatch,
        engine::events::PublishedBatch<engine::gameplay::spatial::contacts::ContactPair> &publishedContacts,
        const std::size_t entityIndexCapacity) :
        world_(world), grid_(grid), crateDefinitions_(crateDefinitions),
        collectorDefinitions_(collectorDefinitions), accountPolicies_(accountPolicies),
        progressionDefinitions_(progressionDefinitions), progressionBatch_(progressionBatch),
        pickupBatch_(pickupBatch), publishedContacts_(publishedContacts), entityLimit_(entityIndexCapacity),
        claimLimit_(pickupBatch.Capacity()), prefixLimit_(progressionBatch.Capacity())
    {
        if (entityIndexCapacity == 0 || pickupBatch.Capacity() == 0)
            throw std::invalid_argument("Crate claim capacities must be positive");
        if (progressionBatch.Capacity() > pickupBatch.ProgressionPrefixCapacity())
            throw std::invalid_argument("Crate progression prefix storage is too small");
        contacts_.reserve(claimLimit_);
        crates_.reserve(entityLimit_);
        unlocks_.reserve(entityLimit_);
        actors_.reserve(entityLimit_);
        prefix_.reserve(prefixLimit_);
        sortedPrefix_.reserve(prefixLimit_);
        prefixIndices_.reserve(prefixLimit_);
        prefixRanges_.reserve(prefixLimit_);
        candidateSlots_.reserve(claimLimit_);
        candidateValid_.reserve(claimLimit_);
        candidates_.reserve(claimLimit_);
        arbitration_.reserve(claimLimit_);
        claims_.reserve(claimLimit_);
        projections_.reserve(entityLimit_);
        candidatePlans_.reserve(entityLimit_);
        selected_.reserve(claimLimit_);
    }

    void Configure()
    {
        if (crateRows_ || accountRows_)
            throw std::logic_error("Crate claim configuration is one-shot");
        if (!world_.ComponentsFinalized())
            throw std::logic_error("Crate claim queries require finalized components");
        crateRows_ = std::make_unique<CrateRows>(world_);
        accountRows_ = std::make_unique<AccountRows>(world_);
    }

    void SetInputs(std::span<const CratePickupInput> inputs)
    {
        if (world_.IsScheduledExecutionActive())
            throw std::logic_error("Crate pickup inputs require a joined boundary");
        pickupBatch_.SetInputs(inputs);
    }

    void BeforeChunks(Query &query, ecs::SystemContext &context)
    {
        if (!crateRows_ || !accountRows_)
            throw std::logic_error("Crate claim system was not configured");
        if (!accountPolicies_.IsFrozen())
            throw std::logic_error("Crate account pickup policies are not frozen");

        // This is the only ProgressionBatch read before crate awards.  The
        // root registers the crate path in its ordered Simulation prefix, so
        // this is the ordered host prefix only:
        // KillExperienceSystem runs later in Simulation and its same-tick
        // awards are intentionally not visible to this projection.  The root
        // graph must keep CrateRewardSystem before KillExperienceSystem so the
        // later producer sees the crate publication order.
        const auto pendingPrefix = progressionBatch_.PendingInputs();
        if (pendingPrefix.size() > pickupBatch_.ProgressionPrefixCapacity())
            throw std::length_error("Crate progression prefix handoff capacity exhausted");
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
        pickupBatch_.SetProgressionPrefix(prefix_);

        crates_.clear();
        crateRows_->ForEachChunk([&](CrateRows::Chunk chunk) {
            const auto states = chunk.template Get<CrateState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                AppendBounded(crates_, entityLimit_, CrateObservation{chunk.Entities()[row], states[row]},
                    "Crate observation capacity exhausted");
        });
        std::sort(crates_.begin(), crates_.end(), [](const auto &left, const auto &right) {
            return EntityLess(left.entity, right.entity);
        });

        const auto inputs = pickupBatch_.Inputs();
        if (inputs.size() > claimLimit_)
            throw std::length_error("Crate pickup contact capacity exhausted");

        if (!publishedContacts_.IsPublished())
            throw std::logic_error("Crate claim requires a contact publication");
        const auto boundary = publishedContacts_.Boundary();
        if (boundary.tick != context.Tick() ||
            boundary.phase != static_cast<std::uint32_t>(context.Phase()))
            throw std::logic_error("Crate contact publication belongs to another tick or phase");
        const auto publishedPairs = publishedContacts_.Values();
        std::size_t generatedCount = 0;
        for (const auto pair : publishedPairs)
        {
            const bool firstIsCrate = FindCrate(pair.first) != nullptr;
            const bool secondIsCrate = FindCrate(pair.second) != nullptr;
            if (firstIsCrate == secondIsCrate)
                continue;
            if (generatedCount >= (std::numeric_limits<std::uint32_t>::max)())
                throw std::length_error("Generated crate contact sequence capacity exhausted");
            ++generatedCount;
        }
        if (generatedCount > claimLimit_ - inputs.size())
            throw std::length_error("Combined crate contact capacity exhausted");

        contacts_.clear();
        for (std::size_t index = 0; index != inputs.size(); ++index)
            AppendBounded(contacts_, claimLimit_, Contact{inputs[index], index, ContactSource::Host, 0},
                "Crate pickup contact capacity exhausted");
        std::size_t generatedOrdinal = 0;
        for (const auto pair : publishedPairs)
        {
            const bool firstIsCrate = FindCrate(pair.first) != nullptr;
            const bool secondIsCrate = FindCrate(pair.second) != nullptr;
            if (firstIsCrate == secondIsCrate)
                continue;
            const auto crate = firstIsCrate ? pair.first : pair.second;
            const auto collector = firstIsCrate ? pair.second : pair.first;
            AppendBounded(contacts_, claimLimit_, Contact{
                CratePickupInput{crate, collector, static_cast<std::uint32_t>(generatedOrdinal)},
                generatedOrdinal, ContactSource::Published, 0},
                "Crate pickup contact capacity exhausted");
            ++generatedOrdinal;
        }
        std::sort(contacts_.begin(), contacts_.end(), [](const Contact &left, const Contact &right) {
            return std::tie(left.input.collector.index, left.input.collector.generation,
                    left.input.crate.index, left.input.crate.generation, left.input.sequence,
                    left.source, left.sourceOrdinal) <
                std::tie(right.input.collector.index, right.input.collector.generation,
                    right.input.crate.index, right.input.crate.generation, right.input.sequence,
                    right.source, right.sourceOrdinal);
        });
        for (std::size_t ordinal = 0; ordinal != contacts_.size(); ++ordinal)
            contacts_[ordinal].slotOrdinal = ordinal;
        // The claim system has copied and bounded the complete joined contact
        // set.  ContactSystem -> CrateClaimSystem is an explicit scheduler
        // wave edge; this joined BeforeChunks consumer releases the consumed
        // publication exactly once before the next tick can publish another
        // contact snapshot.
        publishedContacts_.Release();

        unlocks_.clear();
        accountRows_->ForEachChunk([&](AccountRows::Chunk chunk) {
            const auto states = chunk.template Get<UnlockState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                AppendBounded(unlocks_, entityLimit_, UnlockObservation{chunk.Entities()[row], states[row]},
                    "Crate unlock observation capacity exhausted");
        });
        std::sort(unlocks_.begin(), unlocks_.end(), [](const auto &left, const auto &right) {
            return EntityLess(left.entity, right.entity);
        });

        actors_.clear();
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            const auto lives = chunk.template Get<LifeState>();
            const auto units = chunk.template Get<ProducedUnit>();
            const auto structures = chunk.template Get<Structure>();
            const auto positions = chunk.template Get<GridPosition>();
            const auto states = chunk.template Get<ProgressionState>();
            const auto refs = chunk.template Get<ProgressionDefinitionRef>();
            const auto eligibility = chunk.template Get<ProgressionEligibility>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                AppendBounded(actors_, entityLimit_, MakeActor(chunk.Entities()[row], lives[row], units,
                    structures, positions, states, refs, eligibility, row),
                    "Crate collector observation capacity exhausted");
        });
        std::sort(actors_.begin(), actors_.end(), [](const auto &left, const auto &right) {
            return EntityLess(left.entity, right.entity);
        });

        ResizeBounded(candidateSlots_, contacts_.size(), claimLimit_,
            "Crate candidate slot capacity exhausted");
        ResizeBounded(candidateValid_, contacts_.size(), claimLimit_,
            "Crate candidate validity capacity exhausted");
        std::fill(candidateValid_.begin(), candidateValid_.end(), std::uint8_t{0});
        ResizeBounded(projections_, actors_.size(), entityLimit_,
            "Crate projection capacity exhausted");
        std::fill(projections_.begin(), projections_.end(), ProjectedProgression{});
        candidatePlans_.clear();
        (void)context;
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context)
    {
        (void)context;
        const auto lives = chunk.template Get<LifeState>();
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        const auto positions = chunk.template Get<GridPosition>();
        const auto states = chunk.template Get<ProgressionState>();
        const auto refs = chunk.template Get<ProgressionDefinitionRef>();
        const auto eligibility = chunk.template Get<ProgressionEligibility>();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            if (!lives[row].alive)
                continue;
            const auto collector = chunk.Entities()[row];
            const auto first = std::lower_bound(contacts_.begin(), contacts_.end(), collector,
                [](const Contact &contact, const ecs::Entity entity) {
                    return EntityLess(contact.input.collector, entity);
                });
            const auto last = std::upper_bound(first, contacts_.end(), collector,
                [](const ecs::Entity entity, const Contact &contact) {
                    return EntityLess(entity, contact.input.collector);
                });
            const auto actor = MakeActor(collector, lives[row], units, structures, positions,
                states, refs, eligibility, row);
            for (auto contact = first; contact != last; ++contact)
            {
                CrateCandidate candidate{};
                if (!BuildCandidate(actor, contact->input, contact->slotOrdinal, candidate))
                    continue;
                if (contact->slotOrdinal >= candidateSlots_.size())
                    throw std::logic_error("Crate contact slot was not prepared");
                candidateSlots_[contact->slotOrdinal] = candidate;
                candidateValid_[contact->slotOrdinal] = 1;
            }
        }
    }

    void AfterChunks(Query &, ecs::SystemContext &)
    {
        candidates_.clear();
        arbitration_.clear();
        for (std::size_t ordinal = 0; ordinal != candidateSlots_.size(); ++ordinal)
            if (candidateValid_[ordinal] != 0)
                AppendBounded(candidates_, claimLimit_, candidateSlots_[ordinal],
                    "Crate candidate capacity exhausted");
        std::sort(candidates_.begin(), candidates_.end(), [](const auto &left, const auto &right) {
            const auto &a = left.arbitration;
            const auto &b = right.arbitration;
            return std::tie(a.crate.index, a.crate.generation, a.collector.index,
                    a.collector.generation, a.sequence, a.ordinal) <
                std::tie(b.crate.index, b.crate.generation, b.collector.index,
                    b.collector.generation, b.sequence, b.ordinal);
        });
        for (std::size_t ordinal = 0; ordinal != candidates_.size(); ++ordinal)
        {
            candidates_[ordinal].arbitration.ordinal = ordinal;
            AppendBounded(arbitration_, claimLimit_, candidates_[ordinal].arbitration,
                "Crate arbitration capacity exhausted");
        }

        for (std::size_t index = 0; index != actors_.size(); ++index)
        {
            const auto &actor = actors_[index];
            const auto *definition = actor.definition < progressionDefinitions_.Size()
                ? &progressionDefinitions_.Get(actor.definition) : nullptr;
            projections_[index] = MakeProjection(actor.experience, actor.level,
                actor.hasProgression && actor.trainable, definition);
            if (definition)
                ApplyPrefix(projections_[index], *definition, PrefixFor(actor.entity));
        }

        claims_.clear();
        ResizeBounded(selected_, arbitration_.size(), claimLimit_,
            "Crate selection capacity exhausted");
        const auto selectedCount = engine::gameplay::pickups::SelectSuccessAware(
            arbitration_, selected_, [this](const auto &candidate) {
                return TryReserveReward(candidate);
            });
        if (selectedCount > claimLimit_)
            throw std::length_error("Crate claim capacity exhausted");
        for (std::size_t index = 0; index != selectedCount; ++index)
        {
            const auto &selected = selected_[index];
            const auto &candidate = candidates_[selected.candidateOrdinal];
            AppendBounded(claims_, claimLimit_, CrateClaim{candidate.arbitration.crate,
                candidate.arbitration.collector, candidate.account, candidate.definition,
                candidate.arbitration.sequence, index, candidate.collectorCell,
                candidate.hasCollectorCell}, "Crate claim capacity exhausted");
        }
        pickupBatch_.SetClaims(claims_);
    }

private:
    enum class ContactSource : std::uint8_t { Host, Published };
    struct Contact final
    {
        CratePickupInput input{};
        std::size_t sourceOrdinal{};
        ContactSource source{ContactSource::Host};
        std::size_t slotOrdinal{};
    };
    struct CrateObservation final { ecs::Entity entity{}; CrateState state{}; };
    struct UnlockObservation final { ecs::Entity entity{}; UnlockState state{}; };
    struct ActorObservation final
    {
        ecs::Entity entity{}, account{};
        std::uint32_t definition{};
        KindOfMask kindOf{};
        engine::gameplay::navigation::Cell cell{engine::gameplay::navigation::InvalidCell};
        std::uint64_t experience{};
        std::uint32_t level{};
        bool structure{}, alive{}, hasDefinition{}, hasPosition{}, hasProgression{}, trainable{};
        bool supportsGroundVeterancy{};
    };
    struct ProjectionPlan final { std::size_t actorIndex{}; ProjectedProgression projection{}; };

    static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
    {
        return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
    }

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

    static ecs::Entity Owner(const std::span<const ProducedUnit> units,
        const std::span<const Structure> structures, const std::size_t row) noexcept
    {
        return !structures.empty() ? structures[row].account :
            !units.empty() ? units[row].account : ecs::Entity{};
    }

    ActorObservation MakeActor(const ecs::Entity entity, const LifeState &life,
        const std::span<const ProducedUnit> units, const std::span<const Structure> structures,
        const std::span<const GridPosition> positions,
        const std::span<const ProgressionState> states,
        const std::span<const ProgressionDefinitionRef> refs,
        const std::span<const ProgressionEligibility> eligibility,
        const std::size_t row) const
    {
        ActorObservation result{};
        result.entity = entity; result.account = Owner(units, structures, row);
        result.structure = !structures.empty(); result.alive = life.alive;
        result.hasPosition = !positions.empty();
        if (result.hasPosition) result.cell = positions[row].cell;
        result.hasProgression = !states.empty() && !refs.empty() && !eligibility.empty();
        if (result.hasProgression)
        {
            result.experience = states[row].experience; result.level = states[row].level;
            result.definition = refs[row].index; result.trainable = eligibility[row].trainable;
        }
        const auto *definition = collectorDefinitions_.Find(result.structure,
            !structures.empty() ? structures[row].definition : units.empty() ? 0u : units[row].definition);
        if (definition)
        {
            result.kindOf = definition->kindOf;
            result.supportsGroundVeterancy = definition->supportsGroundVeterancy;
            result.hasDefinition = true;
        }
        return result;
    }

    const CrateObservation *FindCrate(const ecs::Entity entity) const noexcept
    {
        const auto it = std::lower_bound(crates_.begin(), crates_.end(), entity,
            [](const CrateObservation &row, const ecs::Entity value) { return EntityLess(row.entity, value); });
        return it != crates_.end() && it->entity == entity ? &*it : nullptr;
    }

    const UnlockState *FindUnlock(const ecs::Entity entity) const noexcept
    {
        const auto it = std::lower_bound(unlocks_.begin(), unlocks_.end(), entity,
            [](const UnlockObservation &row, const ecs::Entity value) { return EntityLess(row.entity, value); });
        return it != unlocks_.end() && it->entity == entity ? &it->state : nullptr;
    }

    const ActorObservation *FindActor(const ecs::Entity entity) const noexcept
    {
        const auto it = std::lower_bound(actors_.begin(), actors_.end(), entity,
            [](const ActorObservation &row, const ecs::Entity value) { return EntityLess(row.entity, value); });
        return it != actors_.end() && it->entity == entity ? &*it : nullptr;
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

    bool BuildCandidate(const ActorObservation &actor, const CratePickupInput &input,
        const std::size_t slotOrdinal, CrateCandidate &result) const
    {
        const auto *accountPolicy = actor.account.IsValid()
            ? accountPolicies_.Find(actor.account) : nullptr;
        if (!actor.alive || !actor.account.IsValid() || accountPolicy == nullptr ||
            accountPolicy->neutral || !actor.hasDefinition)
            return false;
        const auto *crate = FindCrate(input.crate);
        if (!crate || crate->state.definition >= crateDefinitions_.Size())
            return false;
        const auto &definition = crateDefinitions_.Get(crate->state.definition);
        const bool validBuildingAttempt = definition.buildingPickup && actor.structure;
        if (actor.structure && !definition.buildingPickup)
            return false;
        if (!validBuildingAttempt && actor.structure)
            return false;
        if ((actor.kindOf & definition.requiredKindOf) != definition.requiredKindOf ||
            (actor.kindOf & definition.forbiddenKindOf) != 0 ||
            (actor.kindOf & KindOfParachute) != 0)
            return false;
        if (crate->state.placement == CratePlacement::Airborne && !validBuildingAttempt)
            return false;
        if (definition.forbidOwner && crate->state.ownerAccount.IsValid() &&
            crate->state.ownerAccount == actor.account)
            return false;
        if (definition.humanOnly && !accountPolicy->human)
            return false;
        if (definition.pickupScience != engine::gameplay::rts::unlocks::InvalidUnlockId)
        {
            const auto *unlock = FindUnlock(actor.account);
            if (!unlock || !unlock->IsOwned(definition.pickupScience))
                return false;
        }

        // VeterancyCrateCollide rejects significantly-above-terrain collectors.
        // The modern bounded slice has no runtime locomotor/height component;
        // only an explicit startup collector classification may enter it.
        if (definition.reward == RewardKind::Veterancy && !actor.supportsGroundVeterancy)
            return false;

        // VeterancyCrateCollide checks the crate's own AI goal object even for
        // non-pilot crates.  The port therefore validates the spawn/order-owned
        // full recipient identity in CrateState; the contact cannot assert it.
        if (definition.reward == RewardKind::Veterancy &&
            crate->state.intendedRecipient != actor.entity)
            return false;

        result.arbitration = {input.crate, actor.entity, input.sequence, slotOrdinal,
            definition.allowMultiPickup, true};
        result.definition = crate->state.definition;
        result.account = actor.account;
        result.collectorCell = actor.cell;
        result.hasCollectorCell = actor.hasPosition;
        return true;
    }

    bool WithinRange(const engine::gameplay::navigation::Cell first,
        const engine::gameplay::navigation::Cell second, const std::uint32_t radius) const noexcept
    {
        if (!grid_.Width()) return false;
        const auto width = static_cast<std::uint64_t>(grid_.Width());
        const auto firstX = static_cast<std::uint64_t>(first) % width;
        const auto firstY = static_cast<std::uint64_t>(first) / width;
        const auto secondX = static_cast<std::uint64_t>(second) % width;
        const auto secondY = static_cast<std::uint64_t>(second) / width;
        const auto dx = firstX > secondX ? firstX - secondX : secondX - firstX;
        const auto dy = firstY > secondY ? firstY - secondY : secondY - firstY;
        return dx * dx + dy * dy <= static_cast<std::uint64_t>(radius) * radius;
    }

    bool TryReserveReward(const engine::gameplay::pickups::PickupCandidate &candidate)
    {
        assert(candidate.ordinal < candidates_.size());
        const auto &extra = candidates_[candidate.ordinal];
        const auto *crate = FindCrate(candidate.crate);
        if (!crate) return false;
        const auto &definition = crateDefinitions_.Get(extra.definition);
        if (definition.reward == RewardKind::Money || definition.reward == RewardKind::Heal)
            return true;
        if (definition.reward != RewardKind::Veterancy)
            return false;
        const auto levels = definition.addsOwnerVeterancy ? crate->state.veterancyLevel : 1u;
        if (!levels) return false;
        const auto *collector = FindActor(candidate.collector);
        if (!collector) return false;

        candidatePlans_.clear();
        for (std::size_t index = 0; index != actors_.size(); ++index)
        {
            const auto &target = actors_[index];
            if (target.account != extra.account || !target.alive)
                continue;
            if (definition.effectRange == 0)
            {
                if (target.entity != candidate.collector) continue;
            }
            else if (!extra.hasCollectorCell || !target.hasPosition ||
                !WithinRange(extra.collectorCell, target.cell, definition.effectRange))
                continue; // Map-status filtering is a documented one-map gap.
            if (!target.hasProgression || !target.trainable ||
                target.definition >= progressionDefinitions_.Size())
                continue;
            const auto &progression = progressionDefinitions_.Get(target.definition);
            auto projection = projections_[index];
            const auto amount = ApplyLevelGain(projection, progression, levels);
            if (!amount) continue;
            AppendBounded(candidatePlans_, entityLimit_, ProjectionPlan{index, projection},
                "Crate veterancy plan capacity exhausted");
        }
        if (candidatePlans_.empty())
            return false;
        for (const auto &plan : candidatePlans_)
            projections_[plan.actorIndex] = plan.projection;
        return true;
    }

    struct PrefixRange final { ecs::Entity entity{}; std::size_t first{}, count{}; };

    ecs::World &world_;
    const engine::gameplay::navigation::NavigationGrid &grid_;
    const CrateDefinitionCatalog &crateDefinitions_;
    const CollectorDefinitionCatalog &collectorDefinitions_;
    const AccountPickupPolicies &accountPolicies_;
    const engine::gameplay::progression::ProgressionCatalog &progressionDefinitions_;
    engine::gameplay::progression::ProgressionBatch &progressionBatch_;
    CratePickupBatch &pickupBatch_;
    engine::events::PublishedBatch<engine::gameplay::spatial::contacts::ContactPair> &publishedContacts_;
    const std::size_t entityLimit_;
    const std::size_t claimLimit_;
    const std::size_t prefixLimit_;
    std::unique_ptr<CrateRows> crateRows_;
    std::unique_ptr<AccountRows> accountRows_;
    std::vector<Contact> contacts_;
    std::vector<CrateObservation> crates_;
    std::vector<UnlockObservation> unlocks_;
    std::vector<ActorObservation> actors_;
    std::vector<engine::gameplay::progression::AcceptedExperience> prefix_, sortedPrefix_;
    std::vector<PrefixRange> prefixRanges_;
    std::vector<CrateCandidate> candidateSlots_;
    // This is written by parallel chunk jobs at distinct combined contact
    // slot ordinals.  A byte per slot is required: packed boolean storage
    // combines unrelated ordinals into shared words and would race when
    // neighboring chunks write them.
    std::vector<std::uint8_t> candidateValid_;
    std::vector<std::size_t> prefixIndices_;
    std::vector<CrateCandidate> candidates_;
    std::vector<engine::gameplay::pickups::PickupCandidate> arbitration_;
    std::vector<engine::gameplay::pickups::PickupClaim> selected_;
    std::vector<CrateClaim> claims_;
    std::vector<ProjectedProgression> projections_;
    std::vector<ProjectionPlan> candidatePlans_;
};
} // namespace generalszh::crates

export namespace ecs
{
template<> struct SystemTraits<generalszh::crates::CrateClaimSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.crates.claim";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<engine::gameplay::progression::ProgressionSystem>;
    using After = SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
} // namespace ecs
