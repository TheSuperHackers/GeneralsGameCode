module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.bounty.systems.cash_bounty_award_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.rts.bounty.algorithms.bounty_amount;
export import engine.gameplay.rts.bounty.components.bounty_policy;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
export import games.generalszh.gameplay.bounty.systems.cash_bounty_policy_system;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.match.components.match_state;
export import games.generalszh.gameplay.production.components.production_state;

export namespace generalszh::bounty
{
class CashBountyAwardSystem final
{
    using DamageResult = engine::gameplay::combat::DamageResult;
    using LifeState = engine::gameplay::combat::LifeState;
    using BountyPolicy = engine::gameplay::rts::bounty::BountyPolicy;
    using ProducedUnit = generalszh::production::ProducedUnit;
    using Structure = generalszh::construction::Structure;
    using MatchMember = generalszh::match::MatchMember;

    // Victim work is ordinary contiguous chunk processing. A binding is a
    // decoded BuildCost snapshot; no catalog or account lookup is performed in
    // this hot path.
    using Victims = ecs::Query<ecs::Read<DamageResult>, ecs::Read<LifeState>,
        ecs::Read<CashBountyCostBinding>, ecs::Optional<ProducedUnit>, ecs::Optional<Structure>>;

    // Sources and their accounts are separate populations. Both joins are
    // cached typed queries; the source snapshot performs no per-entity lookup.
    using Sources = ecs::Query<ecs::Read<LifeState>, ecs::Optional<ProducedUnit>, ecs::Optional<Structure>>;
    // Match membership is authoritative for every participant, while bounty
    // policy presence is only the separate eligibility bit for a killer. A
    // victim's account must not need to carry bounty state just to be a valid
    // match participant.
    using Members = ecs::Query<ecs::Read<MatchMember>, ecs::Optional<BountyPolicy>>;

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

    CashBountyAwardSystem(ecs::World &world, const ecs::Entity &matchEntity,
        CashBountyBatch &batch, const std::size_t entityIndexCapacity,
        const std::size_t awardCapacity) :
        world_(world), matchEntity_(matchEntity), batch_(batch),
        sourceGenerations_(entityIndexCapacity), sourceRecords_(entityIndexCapacity),
        participantAccounts_(entityIndexCapacity), participantPolicies_(entityIndexCapacity),
        participantBountyEligible_(entityIndexCapacity), participantValid_(entityIndexCapacity),
        victimOwners_(entityIndexCapacity),
        victimOutputs_(entityIndexCapacity), awardCapacity_(awardCapacity)
    {
        if (entityIndexCapacity == 0 || awardCapacity == 0)
            throw std::invalid_argument("Cash bounty capacities must be positive");
        if (batch.Capacity() < awardCapacity)
            throw std::invalid_argument("Cash bounty batch is smaller than award capacity");
        std::fill(sourceGenerations_.begin(), sourceGenerations_.end(), ecs::Entity::InvalidGeneration);
        sourceTouched_.reserve(entityIndexCapacity);
        awards_.reserve(awardCapacity);
    }

    void Configure()
    {
        if (sources_ || members_)
            throw std::logic_error("Cash bounty source configuration is one-shot");
        sources_ = std::make_unique<Sources>(world_);
        members_ = std::make_unique<Members>(world_);
    }

    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        if (!sources_ || !members_)
            throw std::logic_error("Cash bounty source snapshot is not configured");

        batch_.BeginTick();
        for (const auto index : sourceTouched_)
        {
            sourceGenerations_[index] = ecs::Entity::InvalidGeneration;
            sourceRecords_[index] = {};
        }
        sourceTouched_.clear();
        std::fill(participantAccounts_.begin(), participantAccounts_.end(), ecs::Entity{});
        std::fill(participantPolicies_.begin(), participantPolicies_.end(), BountyPolicy{});
        std::fill(participantBountyEligible_.begin(), participantBountyEligible_.end(), false);
        std::fill(participantValid_.begin(), participantValid_.end(), false);
        std::fill(victimOwners_.begin(), victimOwners_.end(), VictimOwner{});
        std::fill(victimOutputs_.begin(), victimOutputs_.end(), AwardSlot{});

        members_->ForEachChunk([&](auto chunk) { SnapshotMembers(chunk); });
        sources_->ForEachChunk([&](auto chunk) { SnapshotSources(chunk); });
        query.ForEachPreparedChunk([&](auto chunk) { SnapshotVictimOwners(chunk); });
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context)
    {
        const auto results = chunk.Get<DamageResult>();
        const auto lives = chunk.Get<LifeState>();
        const auto costs = chunk.Get<CashBountyCostBinding>();
        const auto units = chunk.Get<ProducedUnit>();
        const auto structures = chunk.Get<Structure>();
        const auto tick = context.Tick();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto victim = chunk.Entities()[row];
            if (victim.index >= victimOutputs_.size())
                throw std::length_error("Cash bounty victim entity-index capacity exhausted");

            auto &slot = victimOutputs_[victim.index];
            slot = {};
            slot.victim = victim;
            slot.tick = tick;

            const auto &owner = victimOwners_[victim.index];
            if (owner.victim != victim || !owner.valid)
                continue;

            // HealthSystem has already validated the lethal transition and
            // source provenance. This consumer additionally validates the
            // current tick, target and source snapshot identity.
            const auto &proof = results[row].lethalSource;
            if (!results[row].killed || lives[row].alive || !proof.valid || proof.tick != tick
                || proof.target != victim || proof.source == victim || !proof.source.IsValid())
                continue;
            if (costs[row].basis != CashBountyCostBasis::DecodedBuildCost || !costs[row].eligible
                || !Completed(units, structures, row))
                continue;
            if (proof.source.index >= sourceGenerations_.size()
                || sourceGenerations_[proof.source.index] != proof.source.generation)
                continue;

            const auto &source = sourceRecords_[proof.source.index];
            if (source.source != proof.source || !source.valid
                || source.account != proof.sourceAccount || source.account == owner.account)
                continue;

            const auto amount = engine::gameplay::rts::bounty::CalculateBounty(
                costs[row].authoredBuildCost, source.policy.maximum);
            if (amount == 0)
                continue;

            slot.source = proof.source;
            slot.account = source.account;
            slot.amount = amount;
            slot.valid = true;
        }
    }

    void AfterChunks(Query &, ecs::SystemContext &context)
    {
        awards_.clear();
        for (const auto &slot : victimOutputs_)
        {
            if (!slot.valid || slot.tick != context.Tick())
                continue;
            if (awards_.size() == awardCapacity_)
                throw std::length_error("Cash bounty award capacity exhausted");
            awards_.push_back({slot.victim, slot.source, slot.account, slot.amount});
        }
        std::sort(awards_.begin(), awards_.end(), [](const auto &left, const auto &right) {
            return std::tie(left.victim.index, left.victim.generation,
                left.source.index, left.source.generation, left.account.index,
                left.account.generation)
                < std::tie(right.victim.index, right.victim.generation,
                    right.source.index, right.source.generation, right.account.index,
                    right.account.generation);
        });
        batch_.Publish(awards_);
    }

private:
    struct SourceRecord
    {
        ecs::Entity source{};
        ecs::Entity account{};
        BountyPolicy policy{};
        bool valid{};
    };

    struct VictimOwner
    {
        ecs::Entity victim{};
        ecs::Entity account{};
        bool valid{};
    };

    struct AwardSlot
    {
        ecs::Entity victim{};
        ecs::Entity source{};
        ecs::Entity account{};
        std::uint32_t amount{};
        std::uint64_t tick{};
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
        return !units.empty() || (!structures.empty() && structures[row].complete);
    }

    void SnapshotMembers(Members::Chunk chunk)
    {
        const auto memberships = chunk.template Get<MatchMember>();
        const auto policies = chunk.template Get<BountyPolicy>();
        if (!matchEntity_.IsValid())
            return;
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            if (memberships[row].match != matchEntity_)
                continue;
            const auto account = chunk.Entities()[row];
            if (account.index >= participantAccounts_.size())
                throw std::length_error("Cash bounty account entity-index capacity exhausted");
            if (participantValid_[account.index] && participantAccounts_[account.index] != account)
                throw std::logic_error("Cash bounty account generation changed during snapshot");
            participantAccounts_[account.index] = account;
            participantBountyEligible_[account.index] = !policies.empty();
            if (!policies.empty())
                participantPolicies_[account.index] = policies[row];
            participantValid_[account.index] = true;
        }
    }

    void SnapshotSources(Sources::Chunk chunk)
    {
        const auto lives = chunk.template Get<LifeState>();
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        if (units.empty() && structures.empty())
            return;

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            if (!lives[row].alive || !Completed(units, structures, row))
                continue;
            const auto source = chunk.Entities()[row];
            const auto account = Owner(units, structures, row);
            if (!account.IsValid() || account.index >= participantAccounts_.size()
                || !participantValid_[account.index] || participantAccounts_[account.index] != account
                || !participantBountyEligible_[account.index])
                continue;
            if (source.index >= sourceGenerations_.size())
                throw std::length_error("Cash bounty source entity-index capacity exhausted");
            if (sourceGenerations_[source.index] != ecs::Entity::InvalidGeneration
                && sourceGenerations_[source.index] != source.generation)
                throw std::logic_error("Cash bounty source generation changed during snapshot");
            if (sourceGenerations_[source.index] == ecs::Entity::InvalidGeneration)
            {
                sourceGenerations_[source.index] = source.generation;
                sourceTouched_.push_back(source.index);
            }
            sourceRecords_[source.index] = {source, account, participantPolicies_[account.index], true};
        }
    }

    void SnapshotVictimOwners(Victims::Chunk chunk)
    {
        const auto units = chunk.template Get<ProducedUnit>();
        const auto structures = chunk.template Get<Structure>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto victim = chunk.Entities()[row];
            if (victim.index >= victimOwners_.size())
                throw std::length_error("Cash bounty victim entity-index capacity exhausted");
            const auto account = Owner(units, structures, row);
            const bool participant = account.IsValid() && account.index < participantAccounts_.size()
                && participantValid_[account.index] && participantAccounts_[account.index] == account;
            victimOwners_[victim.index] = {victim, account, participant};
        }
    }

    ecs::World &world_;
    const ecs::Entity &matchEntity_;
    CashBountyBatch &batch_;
    std::unique_ptr<Sources> sources_;
    std::unique_ptr<Members> members_;
    std::vector<ecs::EntityGeneration> sourceGenerations_;
    std::vector<SourceRecord> sourceRecords_;
    std::vector<ecs::EntityIndex> sourceTouched_;
    std::vector<ecs::Entity> participantAccounts_;
    std::vector<BountyPolicy> participantPolicies_;
    std::vector<bool> participantBountyEligible_;
    std::vector<bool> participantValid_;
    std::vector<VictimOwner> victimOwners_;
    std::vector<AwardSlot> victimOutputs_;
    std::vector<CashBountyAward> awards_;
    std::size_t awardCapacity_{};
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::bounty::CashBountyAwardSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.bounty.award";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>;
    using After = SystemTypeList<
        engine::gameplay::combat::HealthSystem,
        generalszh::bounty::CashBountyPolicySystem>;
};
}
