module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <memory_resource>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.repair.systems.manual_repair_system;
export import engine.ecs.system.system;
export import engine.events.storage.event_batch;
export import engine.gameplay.combat.components.healing;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.systems.healing_system;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.economy.components.resource_balance;
export import engine.gameplay.rts.repair.algorithms.repair_progress;
export import engine.gameplay.rts.repair.components.manual_repair;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.repair.inputs.manual_repair_batches;
export import games.generalszh.gameplay.selling.components.sale_state;

export namespace generalszh::repair
{
class ManualRepairSystem
{
    using ManualBinding=engine::gameplay::rts::repair::ManualRepairRateBinding;
    using ManualProgress=engine::gameplay::rts::repair::ManualRepairProgress;
    using ManualAssignment=engine::gameplay::rts::repair::ManualRepairAssignment;
    using Lease=engine::gameplay::rts::repair::ManualRepairBenefactorLease;
    using RepairState=engine::gameplay::rts::repair::RepairState;
    using RepairDefinitions=engine::gameplay::rts::repair::RepairDefinitions;
    using Health=engine::gameplay::combat::Health;
    using Capacity=engine::gameplay::combat::HealthCapacity;
    using Life=engine::gameplay::combat::LifeState;
    using Pending=engine::gameplay::combat::PendingHealing;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Cell=engine::gameplay::navigation::Cell;
    using Entity=ecs::Entity;
    using Passenger=engine::gameplay::containment::PassengerMembership;
    using Capture=generalszh::capture::CaptureActorState;
    using Structure=generalszh::construction::Structure;
    using Builder=generalszh::construction::Builder;
    using Sale=generalszh::selling::SaleState;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    using AcceptedBatch=engine::events::PublishedBatch<Entity>;

    using TargetAccess=ecs::Query<ecs::Write<Lease>,ecs::Read<Structure>,ecs::Read<Health>,
        ecs::Read<Capacity>,ecs::Read<Life>,ecs::Read<Position>,ecs::Write<Pending>,ecs::Optional<Sale>>;
    using AccountAccess=ecs::Query<ecs::Read<Balance>>;

public:
    // SystemRegistry accepts one AuxiliaryAccess type.  The target and
    // account snapshots are genuinely separate typed queries, so this small
    // metadata combiner declares both without changing scheduler/query
    // infrastructure or pretending that accounts live on structures.
    struct AuxiliaryAccess
    {
        static std::vector<ecs::AccessDescriptor> ResolveAccesses(const ecs::ComponentRegistry &components)
        {
            const auto targets=TargetAccess::ResolveAccesses(components);
            const auto accounts=AccountAccess::ResolveAccesses(components);
            std::vector<ecs::AccessDescriptor> result;
            result.reserve(targets.size()+accounts.size());
            result.insert(result.end(),targets.begin(),targets.end());
            result.insert(result.end(),accounts.begin(),accounts.end());
            return result;
        }
    };

public:
    using Query=ecs::Query<ecs::Read<ManualBinding>,ecs::Write<ManualAssignment>,ecs::Write<ManualProgress>,
        ecs::Read<Builder>,ecs::Read<Life>,ecs::Read<Position>,ecs::Write<Goal>,ecs::Write<Credit>,ecs::OptionalWrite<Range>,
        ecs::Optional<Passenger>,ecs::Optional<Capture>>;

    ManualRepairSystem(ecs::World &world,const engine::gameplay::navigation::NavigationGrid &grid,
        const RepairDefinitions &definitions,AcceptedBatch &accepted,std::size_t capacity,
        engine::time::Duration retention,
        std::pmr::memory_resource &recordingMemory=*std::pmr::get_default_resource()) :
        world_(world),grid_(grid),definitions_(definitions),accepted_(accepted),capacity_(capacity),
        retention_(retention),recorded_(capacity,recordingMemory)
    {
        if(capacity>(std::numeric_limits<std::size_t>::max)()/5)
            throw std::length_error("Manual repair override capacity overflow");
        ordinaryCapacity_=5*capacity;
        requests_.reserve(capacity); ordinary_.reserve(ordinaryCapacity_); cancellations_.reserve(capacity);
        actors_.reserve(capacity); targets_.reserve(capacity); accounts_.reserve(capacity);
        decisions_.reserve(capacity); slots_.reserve(capacity); candidates_.reserve(capacity);
        groups_.reserve(capacity); acceptedActors_.reserve(capacity);
        if(accepted.Capacity()<capacity) throw std::invalid_argument("Accepted repair batch is too small");
    }

    // Configuration is startup-only and must happen after component
    // finalization, before the system is registered/finalized.  The injected
    // typed retention duration is rounded up by FixedStep::TicksFor so a
    // lease never expires early; expiryTick is the exclusive endpoint.
    void Configure(engine::time::FixedStep step)
    {
        if(step_) throw std::logic_error("Manual repair configuration is one-shot");
        if(step!=definitions_.Step()) throw std::invalid_argument("Manual repair definition step mismatch");
        const auto ticks=step.TicksFor(retention_);
        if(!ticks) throw std::invalid_argument("Manual repair retention must span at least one tick");
        if(!world_.ComponentsFinalized()) throw std::logic_error("Manual repair queries require finalized components");
        step_=step; retentionTicks_=ticks;
        targetsQuery_=std::make_unique<TargetAccess>(world_);
        accountsQuery_=std::make_unique<AccountAccess>(world_);
    }

    // This is the joined input boundary.  It copies only bounded transient
    // input; all ECS mutation remains in BeforeChunks/Execute/AfterChunks.
    void SetInputs(std::span<const ManualRepairInput> inputs,
        std::span<const Entity> ordinaryOverrides={},std::span<const Entity> cancellations={})
    {
        if(world_.IsScheduledExecutionActive()) throw std::logic_error("Manual repair inputs require a joined boundary");
        if(inputs.size()>capacity_||ordinaryOverrides.size()>ordinaryCapacity_||cancellations.size()>capacity_)
            throw std::length_error("Manual repair input capacity exhausted");
        requests_.assign(inputs.begin(),inputs.end());
        ordinary_.assign(ordinaryOverrides.begin(),ordinaryOverrides.end());
        cancellations_.assign(cancellations.begin(),cancellations.end());
        std::sort(requests_.begin(),requests_.end(),ManualRepairInputLess);
        std::sort(ordinary_.begin(),ordinary_.end(),Less);
        std::sort(cancellations_.begin(),cancellations_.end(),Less);
        ordinary_.erase(std::unique(ordinary_.begin(),ordinary_.end()),ordinary_.end());
        cancellations_.erase(std::unique(cancellations_.begin(),cancellations_.end()),cancellations_.end());
    }

    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        if(!step_||*step_!=context.Time().Step()) throw std::logic_error("Manual repair step mismatch");
        if(accepted_.IsPublished()) throw std::logic_error("Release accepted repair identities before the next tick");
        if(!targetsQuery_||!accountsQuery_) throw std::logic_error("Manual repair was not configured");

        const auto tick=context.Tick();
        accounts_.clear();
        accountsQuery_->ForEachChunk([&](auto chunk) {
            if(chunk.Count()>capacity_-accounts_.size())
                throw std::length_error("Manual repair account snapshot capacity exhausted");
            for(const auto entity:chunk.Entities()) accounts_.push_back(entity);
        });
        std::sort(accounts_.begin(),accounts_.end(),Less);
        accounts_.erase(std::unique(accounts_.begin(),accounts_.end()),accounts_.end());

        targets_.clear();
        targetsQuery_->ForEachChunk([&](auto chunk) {
            const auto leases=chunk.template Get<Lease>(); const auto structures=chunk.template Get<Structure>();
            const auto health=chunk.template Get<Health>(); const auto capacities=chunk.template Get<Capacity>();
            const auto lives=chunk.template Get<Life>(); const auto positions=chunk.template Get<Position>();
            const auto pending=chunk.template Get<Pending>(); const auto sales=chunk.template Get<Sale>();
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                if(targets_.size()==capacity_) throw std::length_error("Manual repair target snapshot capacity exhausted");
                const auto &structure=structures[row];
                const bool account=structure.account.IsValid()&&HasAccount(structure.account);
                targets_.push_back({chunk.Entities()[row],positions[row].cell,structure.account,
                    health[row].current,capacities[row].maximum,pending[row].quantity,lives[row].alive,
                    structure.complete,(!sales.empty()&&sales[row].phase!=generalszh::selling::SalePhase::Idle),
                    account&&lives[row].alive&&structure.complete&&health[row].current<capacities[row].maximum&&
                        (sales.empty()||sales[row].phase==generalszh::selling::SalePhase::Idle)&&
                        positions[row].cell!=engine::gameplay::navigation::InvalidCell&&grid_.Walkable(positions[row].cell),
                    leases[row],false,false,{}});
            }
        });
        std::sort(targets_.begin(),targets_.end(),[](const auto &left,const auto &right){return Less(left.entity,right.entity);});

        const auto chunkCount=query.PreparedChunkCount();
        slots_.assign(chunkCount,{});
        std::size_t total=0;
        query.ForEachPreparedChunk([&](auto chunk) {
            if(chunk.Count()>(std::numeric_limits<std::size_t>::max)()-total)
                throw std::length_error("Manual repair scratch size overflow");
            total+=chunk.Count();
        });
        if(total>capacity_) throw std::length_error("Manual repair actor capacity exhausted");
        decisions_.assign(total,{});
        actors_.clear(); actors_.reserve(total);
        std::size_t offset=0,chunkIndex=0;
        query.ForEachPreparedChunk([&](auto chunk) {
            slots_[chunkIndex++]={offset,chunk.Count()};
            const auto bindings=chunk.template Get<ManualBinding>(); const auto assignments=chunk.template Get<ManualAssignment>();
            const auto progress=chunk.template Get<ManualProgress>(); const auto builders=chunk.template Get<Builder>();
            const auto lives=chunk.template Get<Life>(); const auto positions=chunk.template Get<Position>();
            const auto passengers=chunk.template Get<Passenger>(); const auto captures=chunk.template Get<Capture>();
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                const auto entity=chunk.Entities()[row];
                const bool contained=!passengers.empty()&&engine::gameplay::containment::IsContained(passengers[row]);
                const bool captured=!captures.empty()&&generalszh::capture::IsCaptureBusy(captures[row]);
                actors_.push_back({entity,builders[row].account,positions[row].cell,assignments[row],progress[row],bindings[row],
                    lives[row].alive,!contained&&!captured,IsOverride(entity),IsCancellation(entity),offset+row});
                decisions_[offset+row]=PlanInitial(actors_.back(),tick);
            }
            offset+=chunk.Count();
        });
        std::sort(actors_.begin(),actors_.end(),[](const auto &left,const auto &right){return Less(left.entity,right.entity);});
        ResolveLeasesAndContests(tick);
    }

    void Execute(Query::Chunk chunk,ecs::SystemContext &context) noexcept
    {
        const auto bindings=chunk.Get<ManualBinding>(); const auto lives=chunk.Get<Life>();
        const auto positions=chunk.Get<Position>(); auto assignments=chunk.Get<ManualAssignment>();
        auto progress=chunk.Get<ManualProgress>(); auto goals=chunk.Get<Goal>(); auto credits=chunk.Get<Credit>();
        auto ranges=chunk.Get<Range>();
        const auto slot=slots_[context.ChunkOrder()];
        assert(slot.count==chunk.Count());
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            auto &decision=decisions_[slot.first+row];
            if(!lives[row].alive||decision.stop)
            {
                assignments[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                goals[row]={}; credits[row]={};
                progress[row].state={0,0,false,true}; decision.executed=true; continue;
            }
            if(!decision.hasAssignment) { decision.executed=true; continue; }
            if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
            if(decision.reset) progress[row].state=decision.state;
            if(assignments[row].target!=decision.target||assignments[row].cell!=decision.cell) credits[row]={};
            assignments[row]={decision.target,decision.cell}; goals[row].cell=decision.cell;
            if(decision.advancePulse)
            {
                const auto &definition=definitions_.GetUnchecked(bindings[row].definition);
                decision.amount=engine::gameplay::rts::repair::RepairPulse(
                    decision.capacity,definition,progress[row].state.remainder);
                progress[row].state.nextPulse=decision.nextPulse;
            }
            decision.executed=true;
        }
    }

    void AfterChunks(Query &,ecs::SystemContext &context)
    {
        const auto tick=context.Tick();
        targetsQuery_->ForEachChunk([&](auto chunk) {
            auto leases=chunk.template Get<Lease>(); auto pending=chunk.template Get<Pending>();
            const auto health=chunk.template Get<Health>(); const auto capacities=chunk.template Get<Capacity>();
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                auto *target=FindTarget(chunk.Entities()[row]);
                if(!target) continue;
                if(target->clearLease||!engine::gameplay::rts::repair::IsActive(leases[row],tick)) leases[row]={};
                if(!target->contributor.IsValid()) continue;
                auto *decision=FindDecision(target->contributor);
                if(!decision||!decision->executed||!decision->advancePulse||!decision->amount) continue;
                const auto missing=capacities[row].maximum>health[row].current?
                    capacities[row].maximum-health[row].current:0;
                if(pending[row].quantity>=missing) continue;
                const auto applied=(std::min)(decision->amount,missing-pending[row].quantity);
                pending[row].quantity+=applied;
                if(applied) leases[row]={target->contributor,engine::gameplay::rts::repair::RepairDeadline(tick,retentionTicks_)};
            }
        });

        acceptedActors_.clear();
        for(const auto &decision:decisions_)
            if(decision.acceptedInput&&decision.hasAssignment&&!decision.stop&&decision.executed)
                acceptedActors_.push_back(decision.actor);
        std::sort(acceptedActors_.begin(),acceptedActors_.end(),Less);
        acceptedActors_.erase(std::unique(acceptedActors_.begin(),acceptedActors_.end()),acceptedActors_.end());

        const engine::events::BatchBoundary boundary{tick,static_cast<std::uint32_t>(ecs::SystemPhaseIndex(context.Phase()))};
        recorded_.Begin({boundary,context.Id(),0,0});
        for(const auto actor:acceptedActors_) recorded_.Emplace(actor);
        recorded_.Seal();
        std::array<engine::events::RecordedBatch<Entity> *,1> batches{&recorded_};
        accepted_.Publish(boundary,batches);
    }

private:
    struct TargetObservation
    {
        Entity entity{}; Cell cell{engine::gameplay::navigation::InvalidCell}; Entity account{};
        std::uint64_t health{},maximum{},pending{}; bool alive{},complete{},selling{},eligible{};
        Lease lease{}; bool clearLease{}; bool leaseValid{}; Entity contributor{};
    };
    struct ActorObservation
    {
        Entity entity{},account{}; Cell cell{engine::gameplay::navigation::InvalidCell};
        ManualAssignment assignment{}; ManualProgress progress{}; ManualBinding binding{};
        bool alive{},eligible{},ordinary{},cancelled{}; std::size_t decisionIndex{};
    };
    struct Decision
    {
        Entity actor{},target{}; Cell cell{engine::gameplay::navigation::InvalidCell};
        RepairState state{}; std::uint64_t capacity{}; std::uint64_t nextPulse{}; std::uint64_t potential{}; std::uint64_t amount{};
        bool hasAssignment{},acceptedInput{},reset{},atTarget{},pulseDue{},advancePulse{},stop{},executed{},deferred{};
    };
    struct Slot { std::size_t first{},count{}; };
    struct Candidate
    {
        std::size_t targetIndex{},decisionIndex{};
        Entity target{},actor{};
    };
    struct TargetGroup { std::size_t targetIndex{},first{},count{}; };

    static bool Less(Entity left,Entity right) noexcept
    { return std::tie(left.index,left.generation)<std::tie(right.index,right.generation); }

    bool HasAccount(Entity account) const noexcept
    {
        return std::binary_search(accounts_.begin(),accounts_.end(),account,Less);
    }
    bool IsOverride(Entity actor) const noexcept
    { return std::binary_search(ordinary_.begin(),ordinary_.end(),actor,Less); }
    bool IsCancellation(Entity actor) const noexcept
    { return std::binary_search(cancellations_.begin(),cancellations_.end(),actor,Less); }

    TargetObservation *FindTarget(Entity entity) noexcept
    {
        auto it=std::lower_bound(targets_.begin(),targets_.end(),entity,
            [](const TargetObservation &value,Entity key){return Less(value.entity,key);});
        return it!=targets_.end()&&it->entity==entity?&*it:nullptr;
    }
    const TargetObservation *FindTarget(Entity entity) const noexcept
    { return const_cast<ManualRepairSystem *>(this)->FindTarget(entity); }
    Decision *FindDecision(Entity actor) noexcept
    {
        auto it=std::lower_bound(actors_.begin(),actors_.end(),actor,
            [](const ActorObservation &value,Entity key){return Less(value.entity,key);});
        return it!=actors_.end()&&it->entity==actor?&decisions_[it->decisionIndex]:nullptr;
    }

    bool CanPreserveExistingAssignment(const ActorObservation &actor) const noexcept
    {
        if(!actor.assignment.target.IsValid()||!ActorEligible(actor)||actor.progress.state.stopped) return false;
        const auto *target=FindTarget(actor.assignment.target);
        return target&&TargetEligible(actor,*target);
    }

    void DeferContendedRequest(Decision &decision,const ActorObservation &actor)
    {
        // A rejected new request with no existing repair assignment is inert.
        // It must not cancel an unrelated movement/harvest task merely because
        // the request lost arbitration.
        if(decision.acceptedInput&&decision.reset&&!actor.assignment.target.IsValid())
        {
            const auto actorEntity=decision.actor;
            decision={}; decision.actor=actorEntity; decision.deferred=true;
            return;
        }
        // A request that replaced a different valid repair task is rejected at
        // the canonicalization boundary. The authoritative old assignment and
        // route remain untouched, but this decision contributes no healing for
        // this tick. This explicit one-tick deferral prevents a post-grouping
        // target rewrite and lets the old task resume on the next input tick.
        if(decision.acceptedInput&&decision.reset&&actor.assignment.target!=decision.target&&
            CanPreserveExistingAssignment(actor))
        {
            const auto actorEntity=decision.actor;
            decision={}; decision.actor=actorEntity; decision.deferred=true;
            return;
        }
        // A same-target loser, a stale old assignment, and an ordinary existing
        // contender are stopped consistently. No candidate target is restored
        // after grouping, so the sorted join remains canonical.
        decision.stop=true; decision.hasAssignment=false; decision.acceptedInput=false;
        decision.reset=false; decision.pulseDue=false; decision.advancePulse=false;
        decision.potential=0; decision.amount=0;
    }

    std::pair<std::vector<ManualRepairInput>::const_iterator,std::vector<ManualRepairInput>::const_iterator>
    RequestRange(Entity actor) const
    {
        const auto first=std::lower_bound(requests_.begin(),requests_.end(),actor,
            [](const ManualRepairInput &value,Entity key){return Less(value.actor,key);});
        auto last=first;
        while(last!=requests_.end()&&last->actor==actor) ++last;
        return {first,last};
    }

    bool ActorEligible(const ActorObservation &actor) const noexcept
    {
        if(!actor.alive||!actor.eligible||!actor.account.IsValid()||!HasAccount(actor.account)||
            actor.binding.definition>=definitions_.Count()) return false;
        return definitions_.GetUnchecked(actor.binding.definition).numerator!=0;
    }
    bool TargetEligible(const ActorObservation &actor,const TargetObservation &target) const noexcept
    { return target.eligible&&!target.selling&&target.account==actor.account; }

    std::size_t FindTargetIndex(Entity entity) const noexcept
    {
        const auto it=std::lower_bound(targets_.begin(),targets_.end(),entity,
            [](const TargetObservation &value,Entity key){return Less(value.entity,key);});
        return it==targets_.end()||it->entity!=entity?
            (std::numeric_limits<std::size_t>::max)():static_cast<std::size_t>(it-targets_.begin());
    }

    Decision PlanInitial(const ActorObservation &actor,std::uint64_t tick)
    {
        Decision decision; decision.actor=actor.entity;
        const bool actorEligible=ActorEligible(actor);
        const auto range=RequestRange(actor.entity);
        const bool ordinary=actor.ordinary||actor.cancelled;
        if(ordinary) { decision.stop=true; return decision; }

        bool useRequest=false;
        if(range.second-range.first==1)
        {
            const auto &request=*range.first;
            auto *target=FindTarget(request.target);
            if(actorEligible&&request.actor.IsValid()&&request.target.IsValid()&&target&&TargetEligible(actor,*target))
            {
                decision.target=request.target; decision.cell=target->cell; decision.hasAssignment=true;
                decision.acceptedInput=true; decision.reset=true; useRequest=true;
            }
        }
        if(!useRequest&&actorEligible&&actor.assignment.target.IsValid())
        {
            auto *target=FindTarget(actor.assignment.target);
            if(target&&TargetEligible(actor,*target)&&!actor.progress.state.stopped)
            {
                decision.target=actor.assignment.target; decision.cell=target->cell; decision.hasAssignment=true;
            }
        }
        if(!decision.hasAssignment)
        {
            decision.stop=actor.assignment.target.IsValid()||ordinary;
            return decision;
        }
        const auto *target=FindTarget(decision.target);
        decision.atTarget=actor.cell==target->cell;
        decision.capacity=target->maximum;
        decision.state=decision.reset?
            engine::gameplay::rts::repair::StartRepair(definitions_,actor.binding.definition,{tick,*step_}):actor.progress.state;
        decision.nextPulse=decision.state.nextPulse;
        if(decision.atTarget&&decision.state.armed&&!decision.state.stopped&&tick>=decision.state.nextPulse)
        {
            auto remainder=decision.state.remainder;
            const auto &definition=definitions_.GetUnchecked(actor.binding.definition);
            const auto pulse=engine::gameplay::rts::repair::RepairPulse(target->maximum,definition,remainder);
            const auto missing=target->maximum>target->health?target->maximum-target->health:0;
            decision.potential=target->pending<missing?(std::min)(pulse,missing-target->pending):0;
            decision.pulseDue=true; decision.nextPulse=engine::gameplay::rts::repair::RepairDeadline(tick,definition.interval);
        }
        return decision;
    }

    void ResolveLeasesAndContests(std::uint64_t tick)
    {
        ResolveImmutableLeaseValidity(tick);
        PrevalidateRequestsAgainstLeases();
        BuildCandidateGroups();
        ResolveLeasesAndContestsPass();
    }

    void ResolveImmutableLeaseValidity(std::uint64_t tick)
    {
        // Lease validity is derived from the pre-input ECS assignment snapshot.
        // A holder's speculative new request cannot release its finite old
        // lease before this tick's canonical grouping boundary.
        for(auto &target:targets_)
        {
            target.clearLease=false; target.leaseValid=false; target.contributor={};
            if(!engine::gameplay::rts::repair::IsActive(target.lease,tick))
            { target.clearLease=target.lease.benefactor.IsValid(); target.leaseValid=false; continue; }
            auto *holder=FindActor(target.lease.benefactor);
            if(!target.eligible||!holder||!ActorEligible(*holder)||holder->ordinary||holder->cancelled||
                !holder->assignment.target.IsValid()||holder->assignment.target!=target.entity||
                holder->progress.state.stopped)
            { target.clearLease=true; target.leaseValid=false; }
            else target.leaseValid=true;
        }
    }

    void PrevalidateRequestsAgainstLeases()
    {
        // Reject requests defeated by immutable active leases before building
        // sorted candidate groups. Rejected retargets are deferred for this
        // tick; their existing ECS assignment remains authoritative.
        for(const auto &actor:actors_)
        {
            auto &decision=decisions_[actor.decisionIndex];
            if(!decision.acceptedInput||!decision.reset||!decision.hasAssignment) continue;
            const auto *target=FindTarget(decision.target);
            if(target&&target->leaseValid&&target->lease.benefactor!=actor.entity)
                DeferContendedRequest(decision,actor);
        }
    }

    void ResolveLeasesAndContestsPass()
    {
        // Candidate targets are canonical and immutable for this pass. Any
        // rejection below removes or stops a candidate; it never rewrites it
        // to a different target after grouping.

        for(const auto &candidate:candidates_)
        {
            auto &decision=decisions_[candidate.decisionIndex];
            if(!decision.hasAssignment||decision.stop) continue;
            auto &target=targets_[candidate.targetIndex];
            if(target.leaseValid&&target.lease.benefactor!=candidate.actor)
            {
                if(const auto *actor=FindActor(candidate.actor)) DeferContendedRequest(decision,*actor);
                else { decision.stop=true; decision.hasAssignment=false; decision.acceptedInput=false; }
            }
        }

        for(const auto &group:groups_)
        {
            auto &target=targets_[group.targetIndex];
            Entity winner{};
            if(target.leaseValid)
            {
                auto *holder=FindDecision(target.lease.benefactor);
                if(holder&&!holder->stop&&holder->hasAssignment&&holder->target==target.entity)
                {
                    target.contributor=holder->potential?holder->actor:Entity{};
                    // RepairPulse also advances the fractional remainder when
                    // its integer result is zero.  The due holder therefore
                    // consumes that pulse/deadline even without renewing the
                    // target lease; renewal remains conditional on positive
                    // PendingHealing applied in AfterChunks.
                    if(holder->pulseDue) holder->advancePulse=true;
                }
                continue;
            }
            for(std::size_t offset=0;offset<group.count;++offset)
            {
                const auto &candidate=candidates_[group.first+offset];
                auto &decision=decisions_[candidate.decisionIndex];
                if(decision.hasAssignment&&!decision.stop&&decision.atTarget&&decision.potential)
                    if(!winner.IsValid()||Less(candidate.actor,winner)) winner=candidate.actor;
            }
            if(!winner.IsValid())
            {
                for(std::size_t offset=0;offset<group.count;++offset)
                {
                    auto &decision=decisions_[candidates_[group.first+offset].decisionIndex];
                    if(decision.hasAssignment&&!decision.stop&&decision.atTarget&&decision.pulseDue)
                        decision.advancePulse=true;
                }
                continue;
            }
            target.contributor=winner;
            for(std::size_t offset=0;offset<group.count;++offset)
            {
                const auto &candidate=candidates_[group.first+offset];
                auto &decision=decisions_[candidate.decisionIndex];
                if(decision.hasAssignment&&decision.atTarget&&decision.pulseDue)
                {
                    if(candidate.actor==winner) decision.advancePulse=true;
                    else if(const auto *actor=FindActor(candidate.actor))
                        DeferContendedRequest(decision,*actor);
                    else { decision.stop=true; decision.hasAssignment=false; decision.acceptedInput=false; }
                }
            }
        }
    }

    ActorObservation *FindActor(Entity entity) noexcept
    {
        auto it=std::lower_bound(actors_.begin(),actors_.end(),entity,
            [](const ActorObservation &value,Entity key){return Less(value.entity,key);});
        return it!=actors_.end()&&it->entity==entity?&*it:nullptr;
    }
    void BuildCandidateGroups()
    {
        candidates_.clear(); groups_.clear();
        for(const auto &actor:actors_)
        {
            const auto &decision=decisions_[actor.decisionIndex];
            if(!decision.hasAssignment||decision.stop) continue;
            const auto targetIndex=FindTargetIndex(decision.target);
            if(targetIndex==(std::numeric_limits<std::size_t>::max)()) continue;
            candidates_.push_back({targetIndex,actor.decisionIndex,decision.target,actor.entity});
        }
        std::sort(candidates_.begin(),candidates_.end(),[](const auto &left,const auto &right) {
            return std::tie(left.target.index,left.target.generation,left.actor.index,left.actor.generation)<
                std::tie(right.target.index,right.target.generation,right.actor.index,right.actor.generation);
        });
        std::size_t first=0;
        while(first<candidates_.size())
        {
            std::size_t last=first+1;
            while(last<candidates_.size()&&candidates_[last].target==candidates_[first].target) ++last;
            groups_.push_back({candidates_[first].targetIndex,first,last-first});
            first=last;
        }
    }

    ecs::World &world_;
    const engine::gameplay::navigation::NavigationGrid &grid_;
    const RepairDefinitions &definitions_;
    AcceptedBatch &accepted_;
    std::size_t capacity_;
    std::size_t ordinaryCapacity_{};
    engine::time::Duration retention_;
    std::optional<engine::time::FixedStep> step_;
    std::uint64_t retentionTicks_{};
    std::unique_ptr<TargetAccess> targetsQuery_;
    std::unique_ptr<AccountAccess> accountsQuery_;
    std::vector<ManualRepairInput> requests_;
    std::vector<Entity> ordinary_,cancellations_;
    std::vector<Entity> accounts_;
    std::vector<TargetObservation> targets_;
    std::vector<ActorObservation> actors_;
    std::vector<Decision> decisions_;
    std::vector<Slot> slots_;
    std::vector<Candidate> candidates_;
    std::vector<TargetGroup> groups_;
    std::vector<Entity> acceptedActors_;
    engine::events::RecordedBatch<Entity> recorded_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::repair::ManualRepairSystem>
{
    static constexpr std::string_view StableName="games.generalszh.repair.manual";
    static constexpr SystemPhase Phase=SystemPhase::PreSimulation;
    using Before=SystemTypeList<engine::gameplay::combat::HealingSystem>;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
