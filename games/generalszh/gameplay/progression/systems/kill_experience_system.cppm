module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.progression.systems.kill_experience_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.gameplay.progression.systems.progression_system;
export import engine.gameplay.rts.rank.algorithms.rank_skill_award;
export import engine.gameplay.rts.rank.components.rank_skill_award_modifier;
export import engine.gameplay.rts.rank.inputs.rank_batch;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.match.components.match_state;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.progression.components.veterancy_award_binding;
export import games.generalszh.gameplay.progression.definitions.veterancy_definition;

export namespace generalszh::progression
{
class KillExperienceSystem
{
    using LifeState = engine::gameplay::combat::LifeState;
    using DamageResult = engine::gameplay::combat::DamageResult;
    using ProgressionState = engine::gameplay::progression::ProgressionState;
    using ProgressionDefinitionRef = engine::gameplay::progression::ProgressionDefinitionRef;
    using ProgressionEligibility = engine::gameplay::progression::ProgressionEligibility;
    using ProgressionCatalog = engine::gameplay::progression::ProgressionCatalog;
    using RankBatch = engine::gameplay::rts::rank::RankBatch;
    using RankSkillAwardModifier = engine::gameplay::rts::rank::RankSkillAwardModifier;
    using MatchMember = generalszh::match::MatchMember;
    using ProducedUnit = generalszh::production::ProducedUnit;
    using Structure = generalszh::construction::Structure;

    // Victims are joined in typed chunks. No progression component is
    // required on the victim: binding plus optional state is sufficient to
    // calculate the value of a regular/non-trainable victim.
    using Victims = ecs::Query<ecs::Read<DamageResult>, ecs::Read<LifeState>,
        ecs::Read<VeterancyAwardBinding>, ecs::Optional<ProgressionState>,
        ecs::Optional<ProducedUnit>, ecs::Optional<Structure>>;

    // The source snapshot is rebuilt once before victim chunks. It is the
    // bounded joined lookup used by Execute; kill reduction never calls
    // World::Get once per victim.
    // Progression is optional on the source. Player rank attribution is a
    // source-account reward and must survive a non-trainable killer or a
    // source with no unit-XP components.
    using Sources = ecs::Query<ecs::Optional<ProgressionState>, ecs::Optional<ProgressionDefinitionRef>,
        ecs::Optional<ProgressionEligibility>, ecs::Optional<engine::gameplay::progression::ProgressionInbox>,
        ecs::Optional<LifeState>, ecs::Optional<ProducedUnit>, ecs::Optional<Structure>>;
    using Members = ecs::Query<ecs::Read<MatchMember>, ecs::Optional<RankSkillAwardModifier>>;

    // Sources and accounts are separate entity populations. The scheduler
    // metadata declares both prepared-query accesses without pretending that a
    // source and its owning account share an archetype.
    struct JoinedAccess
    {
        static std::vector<ecs::AccessDescriptor> ResolveAccesses(const ecs::ComponentRegistry &components)
        {
            const auto sourceAccess = Sources::ResolveAccesses(components);
            const auto memberAccess = Members::ResolveAccesses(components);
            std::vector<ecs::AccessDescriptor> result;
            result.reserve(sourceAccess.size() + memberAccess.size());
            result.insert(result.end(), sourceAccess.begin(), sourceAccess.end());
            result.insert(result.end(), memberAccess.begin(), memberAccess.end());
            return result;
        }
    };

public:
    using Query = Victims;
    using AuxiliaryAccess = JoinedAccess;

    KillExperienceSystem(ecs::World &world, const ecs::Entity &matchEntity,
        const ProgressionCatalog &progressionCatalog,
        const VeterancyCatalog &veterancyCatalog,
        engine::gameplay::progression::ProgressionBatch &batch,
        const std::size_t entityIndexCapacity, const std::size_t awardCapacity,
        RankBatch *rankBatch = nullptr) :
        world_(world), matchEntity_(matchEntity), progressionCatalog_(progressionCatalog),
        veterancyCatalog_(veterancyCatalog), rankBatch_(rankBatch),
        batch_(batch), sourceGenerations_(entityIndexCapacity), sourceRecords_(entityIndexCapacity),
        participantAccounts_(entityIndexCapacity), victimOwners_(entityIndexCapacity),
        participantModifiers_(entityIndexCapacity), participantModifierValid_(entityIndexCapacity),
        victimOutputs_(entityIndexCapacity), awardCapacity_(awardCapacity)
    {
        if (!entityIndexCapacity || !awardCapacity)
            throw std::invalid_argument("Kill experience capacities must be positive");
        for (auto &generation : sourceGenerations_)
            generation = ecs::Entity::InvalidGeneration;
        sourceTouched_.reserve(entityIndexCapacity);
        awards_.reserve(awardCapacity);
        automatic_.reserve(awardCapacity);
        rankAutomatic_.reserve(awardCapacity);
    }

    // Query construction must happen after the composition has finalized its
    // component registry. This mirrors the existing cached-join systems.
    void Configure()
    {
        if (sources_)
            throw std::logic_error("Kill experience configuration is one-shot");
        sources_ = std::make_unique<Sources>(world_);
        members_ = std::make_unique<Members>(world_);
    }

    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        if (!sources_ || !members_)
            throw std::logic_error("Kill experience source snapshot is not configured");
        for (const auto index : sourceTouched_)
        {
            sourceGenerations_[index] = ecs::Entity::InvalidGeneration;
            sourceRecords_[index] = {};
        }
        sourceTouched_.clear();
        std::fill(participantAccounts_.begin(), participantAccounts_.end(), ecs::Entity{});
        std::fill(participantModifiers_.begin(), participantModifiers_.end(), RankSkillAwardModifier{});
        std::fill(participantModifierValid_.begin(), participantModifierValid_.end(), false);
        std::fill(victimOwners_.begin(), victimOwners_.end(), VictimOwner{});
        std::fill(victimOutputs_.begin(), victimOutputs_.end(), AwardSlot{});
        members_->ForEachChunk([&](auto chunk) { SnapshotMembers(chunk); });
        sources_->ForEachChunk([&](auto chunk) { SnapshotSources(chunk); });
        query.ForEachPreparedChunk([&](auto chunk) { SnapshotVictimOwners(chunk); });
    }

    // Execute performs the victim-side observation in the actual chunk job.
    // Each live victim owns one dense slot keyed by its entity index, so jobs
    // never append to shared storage. The generation and tick markers make a
    // stale slot harmless when an entity index is reused or a tick advances.
    void Execute(Query::Chunk chunk, ecs::SystemContext &context)
    {
        const auto results = chunk.template Get<DamageResult>();
        const auto lives = chunk.template Get<LifeState>();
        const auto bindings = chunk.template Get<VeterancyAwardBinding>();
        const auto states = chunk.template Get<ProgressionState>();
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        const auto tick = context.Tick();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto victim = chunk.Entities()[row];
            if (victim.index >= victimOwners_.size())
                throw std::length_error("Kill experience victim entity-index capacity exhausted");

            auto &slot = victimOutputs_[victim.index];
            slot = {};
            slot.victim = victim;
            slot.tick = tick;

            const auto &owner = victimOwners_[victim.index];
            if (owner.victim != victim || !owner.valid)
                continue;

            const auto &proof = results[row].lethalSource;
            if (!results[row].killed || lives[row].alive || !proof.valid || proof.tick != tick
                || !bindings[row].awardable || !Completed(units, structures, row))
                continue;
            if (proof.target != victim || proof.source == victim || !proof.source.IsValid())
                continue;
            if (proof.source.index >= sourceGenerations_.size()
                || sourceGenerations_[proof.source.index] != proof.source.generation)
                continue;

            const auto &source = sourceRecords_[proof.source.index];
            if (source.source != proof.source || !source.accountValid ||
                (!source.experienceEligible && !source.rankEligible))
                continue;

            // The modern targeting path has account identity but no separate
            // diplomacy service. Preserve the inspected same-owner rejection;
            // distinct valid accounts are the existing conservative enemy rule.
            if (!proof.sourceAccount.IsValid() || source.account != proof.sourceAccount
                || source.account == owner.account)
                continue;

            if (bindings[row].definition >= veterancyCatalog_.Size())
                continue;
            const auto &victimDefinition = veterancyCatalog_.Get(bindings[row].definition);
            const auto level = states.empty() ? VeterancyLevel::Regular
                : static_cast<VeterancyLevel>(states[row].level);
            if (static_cast<std::uint32_t>(level) > static_cast<std::uint32_t>(VeterancyLevel::Heroic))
                continue;
            const auto experienceAmount = victimDefinition.AwardValue(level);
            const auto skillPointValue = victimDefinition.SkillPointValue(level);
            if ((!source.experienceEligible || !experienceAmount) &&
                (!source.rankEligible || !skillPointValue))
                continue;

            slot.source = proof.source;
            if (source.experienceEligible && experienceAmount)
            {
                slot.experience = {proof.source, experienceAmount};
                slot.experienceValid = true;
            }
            if (source.rankEligible && skillPointValue)
            {
                const auto resolved = engine::gameplay::rts::rank::ApplyRankSkillAward(
                    skillPointValue, source.rankModifier);
                if (resolved)
                {
                    slot.rankPoints = {proof.sourceAccount, resolved};
                    slot.rankValid = true;
                }
            }
            slot.valid = slot.experienceValid || slot.rankValid;
        }
    }

    // AfterChunks only compacts the disjoint job output, applies canonical
    // ordering, and appends the bounded automatic input set.
    void AfterChunks(Query &, ecs::SystemContext &context)
    {
        awards_.clear(); automatic_.clear(); rankAutomatic_.clear();
        for (const auto &slot : victimOutputs_)
        {
            if (!slot.valid || slot.tick != context.Tick())
                continue;
            if (slot.experienceValid)
            {
                if (awards_.size() == awardCapacity_)
                    throw std::length_error("Kill experience award capacity exhausted");
                awards_.push_back({slot.victim, slot.source, slot.experience});
            }
        }
        std::sort(awards_.begin(), awards_.end(), [](const auto &left, const auto &right) {
            return std::tie(left.victim.index, left.victim.generation,
                left.source.index, left.source.generation)
                < std::tie(right.victim.index, right.victim.generation,
                    right.source.index, right.source.generation);
        });
        for (const auto &slot : victimOutputs_)
        {
            if (slot.valid && slot.tick == context.Tick() && slot.rankValid)
            {
                if (rankAutomatic_.size() == awardCapacity_)
                    throw std::length_error("Kill rank-award capacity exhausted");
                rankAutomatic_.push_back(slot.rankPoints);
            }
        }
        for (const auto &award : awards_) automatic_.push_back(award.experience);
        if (!automatic_.empty()) batch_.Append(automatic_);
        if (rankBatch_ && !rankAutomatic_.empty()) rankBatch_->Append(rankAutomatic_);
    }

private:
    struct SourceRecord
    {
        ecs::Entity source{};
        ecs::Entity account{};
        std::uint32_t progressionDefinition{};
        bool accountValid{};
        bool experienceEligible{};
        bool rankEligible{};
        RankSkillAwardModifier rankModifier{};
    };
    struct VictimOwner
    {
        ecs::Entity victim{};
        ecs::Entity account{};
        bool valid{};
    };
    struct Award
    {
        ecs::Entity victim{};
        ecs::Entity source{};
        engine::gameplay::progression::AcceptedExperience experience{};
    };
    struct AwardSlot
    {
        ecs::Entity victim{};
        ecs::Entity source{};
        engine::gameplay::progression::AcceptedExperience experience{};
        engine::gameplay::rts::rank::AcceptedRankPoints rankPoints{};
        std::uint64_t tick{};
        bool experienceValid{};
        bool rankValid{};
        bool valid{};
    };

    static ecs::Entity Owner(const std::span<const ProducedUnit> units,
        const std::span<const Structure> structures, const std::size_t row) noexcept
    {
        return !units.empty() ? units[row].account
            : !structures.empty() ? structures[row].account : ecs::Entity{};
    }

    static bool Completed(const std::span<const ProducedUnit> units,
        const std::span<const Structure> structures, const std::size_t row) noexcept
    {
        // ProducedUnit is created by build completion. Structures carry their
        // explicit completion bit; no inferred progress enrollment is used.
        return !units.empty() || (!structures.empty() && structures[row].complete);
    }

    void SnapshotVictimOwners(Victims::Chunk chunk)
    {
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto victim = chunk.Entities()[row];
            if (victim.index >= victimOwners_.size())
                throw std::length_error("Kill experience victim entity-index capacity exhausted");
            const auto account = Owner(units, structures, row);
            const bool participant = account.index < participantAccounts_.size()
                && participantAccounts_[account.index] == account;
            victimOwners_[victim.index] = {victim, account, participant};
        }
    }

    void SnapshotMembers(Members::Chunk chunk)
    {
        const auto memberships = chunk.template Get<MatchMember>();
        const auto modifiers = chunk.template Get<RankSkillAwardModifier>();
        if (!matchEntity_.IsValid()) return;
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto account = chunk.Entities()[row];
            if (account.index >= participantAccounts_.size())
                throw std::length_error("Kill experience participant entity-index capacity exhausted");
            if (memberships[row].match != matchEntity_) continue;
            auto &participant = participantAccounts_[account.index];
            if (participant.IsValid() && participant != account)
                throw std::logic_error("Kill experience participant generation changed during snapshot");
            participant = account;
            if (rankBatch_ && !modifiers.empty())
            {
                assert(engine::gameplay::rts::rank::IsValidRankSkillAwardModifier(modifiers[row]));
                participantModifiers_[account.index] = modifiers[row];
                participantModifierValid_[account.index] = true;
            }
        }
    }

    void SnapshotSources(Sources::Chunk chunk)
    {
        const auto states = chunk.template Get<ProgressionState>();
        const auto refs = chunk.template Get<ProgressionDefinitionRef>();
        const auto eligibility = chunk.template Get<ProgressionEligibility>();
        const auto inboxes = chunk.template Get<engine::gameplay::progression::ProgressionInbox>();
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        const auto lives = chunk.template Get<LifeState>();
        if (units.empty() && structures.empty()) return;

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto entity = chunk.Entities()[row];
            if (!Completed(units,structures,row)) continue;
            // Retained source-lifecycle limitation: the current producer only
            // accepts a live, complete source in this tick's snapshot. A
            // stale/dead source proof is rejected even when its entity handle
            // and account fields are otherwise valid; attribution is never
            // recovered through a per-victim World lookup.
            if (!lives.empty() && !lives[row].alive) continue;
            const auto account = Owner(units,structures,row);
            const bool participant = account.index < participantAccounts_.size()
                && participantAccounts_[account.index] == account;
            if (!account.IsValid() || !participant || !world_.IsAlive(account)) continue;

            bool experienceEligible = false;
            if (!states.empty() && !refs.empty() && !eligibility.empty() && !inboxes.empty() &&
                eligibility[row].trainable && refs[row].index < progressionCatalog_.Size())
            {
                const auto &definition = progressionCatalog_.Get(refs[row].index);
                experienceEligible = states[row].experience <= definition.maximumExperience &&
                    states[row].level < definition.thresholds.size();
            }
            const bool rankEligible = rankBatch_ && account.index < participantModifierValid_.size() &&
                participantModifierValid_[account.index];
            if (entity.index >= sourceGenerations_.size())
                throw std::length_error("Kill experience source entity-index capacity exhausted");
            if (sourceGenerations_[entity.index] != ecs::Entity::InvalidGeneration
                && sourceGenerations_[entity.index] != entity.generation)
                throw std::logic_error("Kill experience source generation changed during snapshot");
            if (sourceGenerations_[entity.index] == ecs::Entity::InvalidGeneration)
            {
                sourceGenerations_[entity.index] = entity.generation;
                sourceTouched_.push_back(entity.index);
            }
            sourceRecords_[entity.index] = {entity, account,
                refs.empty() ? 0u : refs[row].index, true, experienceEligible, rankEligible,
                rankEligible ? participantModifiers_[account.index] : RankSkillAwardModifier{}};
        }
    }

    ecs::World &world_;
    const ecs::Entity &matchEntity_;
    const ProgressionCatalog &progressionCatalog_;
    const VeterancyCatalog &veterancyCatalog_;
    RankBatch *rankBatch_{};
    engine::gameplay::progression::ProgressionBatch &batch_;
    std::unique_ptr<Sources> sources_;
    std::unique_ptr<Members> members_;
    std::vector<ecs::EntityGeneration> sourceGenerations_;
    std::vector<SourceRecord> sourceRecords_;
    std::vector<ecs::EntityIndex> sourceTouched_;
    std::vector<ecs::Entity> participantAccounts_;
    std::vector<RankSkillAwardModifier> participantModifiers_;
    std::vector<bool> participantModifierValid_;
    std::vector<VictimOwner> victimOwners_;
    std::vector<AwardSlot> victimOutputs_;
    std::vector<Award> awards_;
    std::vector<engine::gameplay::progression::AcceptedExperience> automatic_;
    std::vector<engine::gameplay::rts::rank::AcceptedRankPoints> rankAutomatic_;
    std::size_t awardCapacity_{};
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::progression::KillExperienceSystem>
{
    static constexpr std::string_view StableName="games.generalszh.progression.kill_experience";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<engine::gameplay::progression::ProgressionSystem>;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
