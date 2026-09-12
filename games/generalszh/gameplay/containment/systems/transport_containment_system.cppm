module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.containment.systems.transport_containment_system;
export import engine.gameplay.containment.components.containment_task;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.containment.definitions.transport_definition;
export import engine.gameplay.containment.inputs.containment_batch;
export import engine.gameplay.navigation.components.movement;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export namespace generalszh::containment
{
// Same-owner ground infantry transport. Main enrollment supplies PassengerSlots
// and ContainmentTask only for infantry; builders/harvesters/nested containers
// remain unsupported. Membership is authoritative. Occupancy below is a bounded
// derived reduction, and all pending passenger effects are planned from values
// before the typed query chunks apply them.
class TransportContainmentSystem
{
    using Entity=ecs::Entity;
    using Member=engine::gameplay::containment::PassengerMembership;
    using Slots=engine::gameplay::containment::PassengerSlots;
    using Task=engine::gameplay::containment::ContainmentTask;
    using Binding=engine::gameplay::containment::TransportBinding;
    using State=engine::gameplay::containment::TransportState;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Life=engine::gameplay::combat::LifeState;
    using Health=engine::gameplay::combat::Health;
    using Capacity=engine::gameplay::combat::HealthCapacity;
    using Damage=engine::gameplay::combat::PendingDamage;
    using Owner=production::ProducedUnit;
    using Builder=construction::Builder;
    using Harvest=engine::gameplay::rts::harvesting::HarvestPolicy;
    using Capture=generalszh::capture::CaptureActorState;
    using Definitions=engine::gameplay::containment::TransportDefinitions;
    using Batch=engine::gameplay::containment::ContainmentBatch;
    using Outcome=engine::gameplay::containment::ContainmentOutcome;
    using Cell=engine::gameplay::navigation::Cell;

    using Carriers=ecs::Query<ecs::Read<Binding>,ecs::Write<State>,ecs::Read<Position>,ecs::Read<Life>,
        ecs::Read<Owner>,ecs::Optional<Member>>;

    struct Carrier
    {
        Entity entity{},owner{};
        Cell cell{engine::gameplay::navigation::InvalidCell};
        const engine::gameplay::containment::CompiledTransportDefinition *definition{};
        State state{};
        std::uint64_t occupied{};
        bool alive{},nested{};
    };

    struct Passenger
    {
        Entity entity{},owner{};
        Cell cell{engine::gameplay::navigation::InvalidCell};
        std::uint32_t slots{};
        Member membership{};
        Task task{};
        Goal goal{};
        Credit credit{};
        Range range{};
        Capture capture{};
        bool alive{},ownerAlive{};
        bool hasTask{},hasGoal{},hasCredit{},hasRange{};
        bool hasBinding{},hasBuilder{},hasHarvest{},hasCapture{};
        std::size_t decisionIndex{};
    };

    struct Decision
    {
        Entity passenger{};
        std::size_t passengerIndex{};
        Member membership{};
        Task task{};
        Goal goal{};
        Credit credit{};
        Range range{};
        Cell position{engine::gameplay::navigation::InvalidCell};
        bool hasTask{},hasGoal{},hasCredit{},hasRange{};
        bool directHandled{};
    };

    struct ChunkSlot { std::size_t first{},count{}; };

public:
    using Query=ecs::Query<ecs::Write<Member>,ecs::Read<Slots>,ecs::Write<Position>,ecs::Read<Life>,
        ecs::Read<Health>,ecs::Read<Capacity>,ecs::Write<Damage>,ecs::Read<Owner>,ecs::OptionalWrite<Task>,
        ecs::OptionalWrite<Goal>,ecs::OptionalWrite<Credit>,ecs::OptionalWrite<Range>,ecs::Optional<Binding>,ecs::Optional<Builder>,
        ecs::Optional<Harvest>,ecs::Optional<Capture>>;

    // This metadata covers the carrier join and every account/marker read used
    // by the value snapshots. It does not create a second execution node.
    using AuxiliaryAccess=ecs::Query<ecs::Read<Binding>,ecs::Write<State>,ecs::Read<Position>,ecs::Read<Life>,
        ecs::Read<Owner>,ecs::Optional<Member>,ecs::Optional<Builder>,ecs::Optional<Harvest>,ecs::Optional<Capture>>;

    TransportContainmentSystem(ecs::World &world,const Definitions &definitions,Batch &batch,std::size_t entityCapacity=65536)
        :world_(world),definitions_(definitions),batch_(batch),carriers_(world),snapshot_(entityCapacity)
    {
        passengers_.reserve(entityCapacity);passengerOrder_.reserve(entityCapacity);decisions_.reserve(entityCapacity);
        rowPassengers_.reserve(entityCapacity);chunkSlots_.reserve(entityCapacity);touched_.reserve(entityCapacity);
    }

    // Joined input staging supplies TaskSelection's sorted ordinary/capture
    // overrides. The span is borrowed only through this system's BeforeChunks.
    void SetInterruptions(std::span<const Entity> actors)
    {
        if(world_.IsScheduledExecutionActive()) throw std::logic_error("Transport interruptions require a joined boundary");
        if(!std::is_sorted(actors.begin(),actors.end(),Less))
            throw std::invalid_argument("Transport interruptions must be sorted");
        boardingOverrides_=actors;
    }

    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        using namespace engine::gameplay::containment;
        assert(context.Time().Step()==definitions_.Step());
        currentTick_=context.Tick();
        batch_.Prepare(currentTick_);
        for(const auto index:touched_) snapshot_[index]={};
        touched_.clear();
        carriers_.ForEachChunk([&](Carriers::Chunk chunk) {
            const auto bindings=chunk.Get<Binding>();const auto states=chunk.Get<State>();const auto positions=chunk.Get<Position>();
            const auto lives=chunk.Get<Life>();const auto owners=chunk.Get<Owner>();const auto members=chunk.Get<Member>();
            for(std::size_t row=0;row<chunk.Count();++row) {
                const auto entity=chunk.Entities()[row];
                if(entity.index>=snapshot_.size()) throw std::length_error("Transport snapshot entity-index capacity exceeded");
                touched_.push_back(entity.index);
                snapshot_[entity.index]={entity,owners[row].account,positions[row].cell,
                    &definitions_.GetUnchecked(bindings[row].definition),states[row],0,lives[row].alive,
                    !members.empty()&&IsContained(members[row])};
            }
        });

        passengers_.clear();passengerOrder_.clear();rowPassengers_.clear();chunkSlots_.clear();
        std::size_t total=0;
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            if(total>(std::numeric_limits<std::size_t>::max)()-chunk.Count())
                throw std::length_error("Transport passenger snapshot size overflow");
            chunkSlots_.push_back({total,chunk.Count()});
            const auto members=chunk.Get<Member>();const auto slots=chunk.Get<Slots>();const auto positions=chunk.Get<Position>();
            const auto lives=chunk.Get<Life>();const auto owners=chunk.Get<Owner>();
            const auto tasks=chunk.Get<Task>();const auto goals=chunk.Get<Goal>();const auto credits=chunk.Get<Credit>();
            const auto ranges=chunk.Get<Range>();
            const auto bindings=chunk.Get<Binding>();const auto builders=chunk.Get<Builder>();const auto harvest=chunk.Get<Harvest>();
            const auto captures=chunk.Get<Capture>();
            for(std::size_t row=0;row<chunk.Count();++row) {
                if(passengers_.size()==snapshot_.size()) throw std::length_error("Transport passenger snapshot capacity exceeded");
                rowPassengers_.push_back(passengers_.size());
                const auto entity=chunk.Entities()[row];
                auto &passenger=passengers_.emplace_back();
                passenger.entity=entity;passenger.owner=owners[row].account;passenger.cell=positions[row].cell;
                passenger.slots=slots[row].slots;passenger.membership=members[row];passenger.alive=lives[row].alive;
                passenger.ownerAlive=world_.IsAlive(passenger.owner);passenger.hasTask=!tasks.empty();
                passenger.hasGoal=!goals.empty();passenger.hasCredit=!credits.empty();passenger.hasRange=!ranges.empty();
                passenger.hasBinding=!bindings.empty();passenger.hasBuilder=!builders.empty();passenger.hasHarvest=!harvest.empty();
                passenger.hasCapture=!captures.empty();if(passenger.hasTask) passenger.task=tasks[row];
                if(passenger.hasGoal) passenger.goal=goals[row];if(passenger.hasCredit) passenger.credit=credits[row];
                if(passenger.hasRange) passenger.range=ranges[row];
                if(passenger.hasCapture) passenger.capture=captures[row];
                if(IsTransportOwned(passenger.membership)&&passenger.alive)
                    if(auto *carrier=Find(passenger.membership.carrier)) carrier->occupied+=passenger.slots;
            }
            total+=chunk.Count();
        });
        for(std::size_t index=0;index<passengers_.size();++index) passengerOrder_.push_back(index);
        std::sort(passengerOrder_.begin(),passengerOrder_.end(),[&](std::size_t left,std::size_t right) {
            return Less(passengers_[left].entity,passengers_[right].entity);
        });
        decisions_.clear();decisions_.reserve(passengers_.size());
        for(const auto passengerIndex:passengerOrder_) {
            auto &passenger=passengers_[passengerIndex];
            passenger.decisionIndex=decisions_.size();
            decisions_.push_back({passenger.entity,passengerIndex,passenger.membership,passenger.task,passenger.goal,
                passenger.credit,passenger.range,passenger.cell,passenger.hasTask,passenger.hasGoal,
                passenger.hasCredit,passenger.hasRange,false});
        }

        // Interruptions preclear active boarding before any direct input,
        // including malformed input that must retain its existing receipt.
        for(auto &passenger:passengers_)
            if((!IsContained(passenger.membership)||IsTransportOwned(passenger.membership)) &&
                (IsOverride(passenger.entity)||IsCaptureBusy(passenger))) {
                CancelTask(passenger);SyncDecision(passenger);
            }

        const auto inputs=batch_.Inputs();
        // Direct input order is the first canonical decision boundary. Active
        // tasks are considered only after every direct actor has been marked.
        for(std::size_t index=0;index<inputs.size();++index)
            batch_.Result(index).outcome=AcceptDirect(inputs[index]);
        for(const auto passengerIndex:passengerOrder_) {
            auto &decision=decisions_[passengers_[passengerIndex].decisionIndex];
            if(decision.directHandled) continue;
            ProcessTask(passengers_[passengerIndex]);
        }
    }

    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        using namespace engine::gameplay::containment;
        const auto slot=chunkSlots_[context.ChunkOrder()];
        assert(slot.count==chunk.Count());
        auto members=chunk.Get<Member>();auto positions=chunk.Get<Position>();const auto lives=chunk.Get<Life>();
        const auto health=chunk.Get<Health>();const auto capacities=chunk.Get<Capacity>();auto damage=chunk.Get<Damage>();
        auto tasks=chunk.Get<Task>();auto goals=chunk.Get<Goal>();auto credits=chunk.Get<Credit>();auto ranges=chunk.Get<Range>();
        const auto tick=context.Tick();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            const auto passengerIndex=rowPassengers_[slot.first+row];
            const auto &decision=decisions_[passengers_[passengerIndex].decisionIndex];
            auto &member=members[row];
            // Another containment system owns this row. Guard before any
            // decision, position, task, or orphan/death write.
            if(IsGarrisonOwned(member)) continue;
            member=decision.membership;positions[row].cell=decision.position;
            if(!tasks.empty()&&decision.hasTask) tasks[row]=decision.task;
            if(!goals.empty()&&decision.hasGoal) goals[row]=decision.goal;
            if(!credits.empty()&&decision.hasCredit) credits[row]=decision.credit;
            if(!ranges.empty()&&decision.hasRange) ranges[row]=decision.range;

            if(!IsTransportOwned(member)) continue;
            if(!lives[row].alive) {member={};continue;}
            if(member.phase==ContainmentPhase::AwaitingEvacuation)
            {
                // Required next-health-pass barrier: input Exit cannot bypass it.
                if(tick>=member.evacuationTick) member={};
                continue;
            }
            const auto *carrier=Find(member.carrier);
            if(!carrier) {member={};continue;} // Orphan: keep last observed carrier cell, no inferred death damage.
            positions[row].cell=carrier->cell;
            if(carrier->alive) continue;
            const auto release=ContainmentDeadline(tick,1);
            const auto amount=PassengerDeathDamage(capacities[row].maximum,*carrier->definition);
            // Clamp to still-unclaimed lethal damage; never overflow a pending
            // sum. Dead carriers damage once, then membership holds until health.
            if(damage[row].quantity<health[row].current)
                damage[row].quantity+=std::min(amount,health[row].current-damage[row].quantity);
            member.phase=ContainmentPhase::AwaitingEvacuation;member.evacuationTick=release;
        }
    }

    void AfterChunks(Query &,ecs::SystemContext &) noexcept
    {
        carriers_.ForEachChunk([&](Carriers::Chunk chunk) {
            auto states=chunk.Get<State>();
            for(std::size_t row=0;row<chunk.Count();++row)
                if(auto *carrier=Find(chunk.Entities()[row])) states[row]=carrier->state;
        });
        batch_.Publish();boardingOverrides_={};
    }

private:
    static bool Less(Entity left,Entity right) noexcept
    { return std::tie(left.index,left.generation)<std::tie(right.index,right.generation); }

    Carrier *Find(Entity entity) noexcept
    { return entity.IsValid()&&entity.index<snapshot_.size()&&snapshot_[entity.index].entity==entity?
        &snapshot_[entity.index]:nullptr; }
    const Carrier *Find(Entity entity) const noexcept
    { return entity.IsValid()&&entity.index<snapshot_.size()&&snapshot_[entity.index].entity==entity?
        &snapshot_[entity.index]:nullptr; }
    Passenger *FindPassenger(Entity entity) noexcept
    {
        const auto iterator=std::lower_bound(passengerOrder_.begin(),passengerOrder_.end(),entity,
            [&](std::size_t index,Entity key){return Less(passengers_[index].entity,key);});
        return iterator!=passengerOrder_.end()&&passengers_[*iterator].entity==entity?&passengers_[*iterator]:nullptr;
    }
    Decision *FindDecision(Entity entity) noexcept
    {
        const auto iterator=std::lower_bound(decisions_.begin(),decisions_.end(),entity,
            [](const Decision &value,Entity key){return Less(value.passenger,key);});
        return iterator!=decisions_.end()&&iterator->passenger==entity?&*iterator:nullptr;
    }
    bool IsOverride(Entity entity) const noexcept
    { return std::binary_search(boardingOverrides_.begin(),boardingOverrides_.end(),entity,Less); }
    static bool HasTask(const Passenger &passenger) noexcept
    { return passenger.hasTask&&passenger.task.carrier.IsValid(); }
    static bool HasTaskComponent(const Passenger &passenger) noexcept
    { return passenger.hasTask; }
    static bool IsCaptureBusy(const Passenger &passenger) noexcept
    { return passenger.hasCapture&&generalszh::capture::IsCaptureBusy(passenger.capture); }

    void SyncDecision(const Passenger &passenger) noexcept
    {
        auto &decision=decisions_[passenger.decisionIndex];
        decision.membership=passenger.membership;decision.position=passenger.cell;
        if(passenger.hasTask) decision.task=passenger.task;
        if(passenger.hasGoal) decision.goal=passenger.goal;
        if(passenger.hasCredit) decision.credit=passenger.credit;
        if(passenger.hasRange) decision.range=passenger.range;
    }
    void MarkDirect(Passenger &passenger) noexcept
    {
        decisions_[passenger.decisionIndex].directHandled=true;
    }
    void ClearMovement(Passenger &passenger) const noexcept
    {
        if(passenger.hasGoal) passenger.goal={};
        if(passenger.hasRange&&passenger.hasCredit)
            engine::gameplay::navigation::ClearMoveRangeGoal(passenger.range,passenger.credit);
        if(passenger.hasRange) passenger.range={};
        if(passenger.hasCredit) passenger.credit={};
    }
    bool CancelTask(Passenger &passenger) const noexcept
    {
        if(!HasTaskComponent(passenger)) return false;
        const bool active=HasTask(passenger);
        passenger.task={};
        if(active) ClearMovement(passenger);
        return true;
    }
    void SetFollowGoal(Passenger &passenger,Cell cell) const noexcept
    {
        if(!passenger.hasGoal) return;
        if(passenger.goal.cell!=cell&&passenger.hasCredit) passenger.credit={};
        passenger.goal.cell=cell;
    }
    void StartTask(Passenger &passenger,const Carrier &carrier) const noexcept
    {
        if(!passenger.hasTask) return;
        passenger.task={carrier.entity};
        // A newly accepted boarding task owns the movement actuator from this
        // boundary, then follows the carrier's current live cell.
        ClearMovement(passenger);SetFollowGoal(passenger,carrier.cell);
    }

    Outcome ValidateEnter(const Passenger &passenger,Entity requestedCarrier,const Carrier *carrier) const noexcept
    {
        using namespace engine::gameplay::containment;
        if(!passenger.alive) return Outcome::NotAlive;
        if(!passenger.slots||passenger.hasBinding||passenger.hasBuilder||passenger.hasHarvest||
            passenger.entity==requestedCarrier) return Outcome::UnsupportedActor;
        if(!carrier) return Outcome::InvalidCarrier;
        if(!carrier->alive) return Outcome::NotAlive;
        if(carrier->nested) return Outcome::UnsupportedActor;
        if(!passenger.ownerAlive||passenger.owner!=carrier->owner) return Outcome::WrongOwner;
        if(carrier->occupied>carrier->definition->slots||passenger.slots>carrier->definition->slots-carrier->occupied)
            return Outcome::Full;
        if(passenger.cell!=carrier->cell) return Outcome::NotArrived;
        return Outcome::Entered;
    }

    Outcome AcceptExit(Passenger &passenger,const engine::gameplay::containment::ContainmentInput &input)
    {
        using namespace engine::gameplay::containment;
        if(!IsTransportOwned(passenger.membership)) return Outcome::InvalidCarrier;
        // An explicit Exit cancels only an active pending route. An unrelated
        // movement goal remains untouched when there was no boarding task.
        CancelTask(passenger);
        if(!passenger.alive) return Outcome::NotAlive;
        auto &member=passenger.membership;
        if(!IsContained(member)) return Outcome::NotContained;
        if(member.phase==ContainmentPhase::AwaitingEvacuation) return Outcome::AwaitingEvacuation;
        if(member.carrier!=input.carrier) return Outcome::InvalidCarrier;
        auto *carrier=Find(member.carrier);
        if(!carrier||!carrier->alive) return Outcome::AwaitingEvacuation;
        if(currentTick_<carrier->state.nextExitTick) return Outcome::ExitBusy;
        const auto next=ContainmentDeadline(currentTick_,carrier->definition->exitTicks);
        passenger.cell=carrier->cell;
        member={};carrier->state.nextExitTick=next;
        assert(carrier->occupied>=passenger.slots);carrier->occupied-=passenger.slots;
        return Outcome::Exited;
    }

    Outcome AcceptDirect(const engine::gameplay::containment::ContainmentInput &input)
    {
        using namespace engine::gameplay::containment;
        auto *passenger=FindPassenger(input.passenger);
        if(!passenger) return Outcome::InvalidPassenger;
        MarkDirect(*passenger);
        // Garrison owns this relationship. Return a deterministic receipt
        // without cancelling a task or synchronizing a transport decision.
        if(IsGarrisonOwned(passenger->membership))
            return input.action==ContainmentAction::Enter ? Outcome::AlreadyContained :
                input.action==ContainmentAction::Exit ? Outcome::InvalidCarrier : Outcome::InvalidAction;
        if(!passenger->alive) {CancelTask(*passenger);SyncDecision(*passenger);return Outcome::NotAlive;}
        if(input.action==ContainmentAction::Exit) {
            const auto outcome=AcceptExit(*passenger,input);SyncDecision(*passenger);return outcome;
        }
        if(input.action!=ContainmentAction::Enter) return Outcome::InvalidAction;
        if(IsOverride(input.passenger)||IsCaptureBusy(*passenger)) {
            // The task was normally precleared before direct input; retaining
            // this guard makes the direct receipt deterministic in isolation.
            CancelTask(*passenger);SyncDecision(*passenger);return Outcome::Interrupted;
        }
        if(IsContained(passenger->membership)) {CancelTask(*passenger);SyncDecision(*passenger);return Outcome::AlreadyContained;}
        auto *carrier=Find(input.carrier);
        const auto outcome=ValidateEnter(*passenger,input.carrier,carrier);
        if(outcome==Outcome::NotArrived) {
            if(passenger->hasTask) {
                StartTask(*passenger,*carrier);SyncDecision(*passenger);return Outcome::BoardingQueued;
            }
            return Outcome::NotArrived;
        }
        if(outcome!=Outcome::Entered) {CancelTask(*passenger);SyncDecision(*passenger);return outcome;}
        passenger->membership={input.carrier,0,ContainmentPhase::Inside,
            PassengerContainmentOwner::Transport,PassengerCombatPolicy::Blocked};
        carrier->occupied+=passenger->slots;
        if(passenger->hasTask) passenger->task={};
        ClearMovement(*passenger);SyncDecision(*passenger);
        return Outcome::Entered;
    }

    void ProcessTask(Passenger &passenger)
    {
        using namespace engine::gameplay::containment;
        if(IsContained(passenger.membership)&&!IsTransportOwned(passenger.membership)) return;
        if(!HasTaskComponent(passenger)) return;
        if(!HasTask(passenger)) {CancelTask(passenger);SyncDecision(passenger);return;}
        if(IsOverride(passenger.entity)||IsCaptureBusy(passenger)||!passenger.alive||
            IsContained(passenger.membership)) {CancelTask(passenger);SyncDecision(passenger);return;}
        auto *carrier=Find(passenger.task.carrier);
        const auto outcome=ValidateEnter(passenger,passenger.task.carrier,carrier);
        if(outcome==Outcome::NotArrived) {SetFollowGoal(passenger,carrier->cell);SyncDecision(passenger);return;}
        if(outcome!=Outcome::Entered) {CancelTask(passenger);SyncDecision(passenger);return;}
        passenger.membership={carrier->entity,0,ContainmentPhase::Inside,
            PassengerContainmentOwner::Transport,PassengerCombatPolicy::Blocked};
        passenger.cell=carrier->cell;carrier->occupied+=passenger.slots;
        passenger.task={};ClearMovement(passenger);SyncDecision(passenger);
    }

    ecs::World &world_;
    const Definitions &definitions_;
    Batch &batch_;
    Carriers carriers_;
    std::vector<Carrier> snapshot_;
    std::vector<Passenger> passengers_;
    std::vector<std::size_t> passengerOrder_,rowPassengers_;
    std::vector<Decision> decisions_;
    std::vector<ChunkSlot> chunkSlots_;
    std::vector<std::uint32_t> touched_;
    std::span<const Entity> boardingOverrides_;
    std::uint64_t currentTick_{};
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::containment::TransportContainmentSystem>
{
    static constexpr std::string_view StableName="games.generalszh.containment.transport";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
