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
export module games.generalszh.gameplay.containment.systems.garrison_containment_system;
export import engine.gameplay.containment.components.containment_task;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.containment.inputs.containment_batch;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.navigation.components.movement;
export import games.generalszh.gameplay.construction.definitions.building_catalog;
export import games.generalszh.gameplay.production.admission.build_catalog;
export import games.generalszh.gameplay.production.runtime.production_runtime;

export namespace generalszh::containment
{
// Bounded first slice of GarrisonContain. A building's immutable catalog policy
// controls capacity; PassengerMembership is the only runtime relationship. The
// supported health policy is exactly LifeState::alive plus Health::current != 0
// for both carrier and passenger. There is no typed disabled/subdued state.
// Entry is same-owner/same-cell and direct: authored exit bones, doors, routing,
// initial rosters, healing, damage transfer, body damage states and subdued
// suppression are deliberately outside this slice and remain adapter-visible.
class GarrisonContainmentSystem
{
    using Entity=ecs::Entity;
    using Member=engine::gameplay::containment::PassengerMembership;
    using Task=engine::gameplay::containment::ContainmentTask;
    using Batch=engine::gameplay::containment::ContainmentBatch;
    using Outcome=engine::gameplay::containment::ContainmentOutcome;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Health=engine::gameplay::combat::Health;
    using Life=engine::gameplay::combat::LifeState;
    using Owner=production::ProducedUnit;
    using Structure=construction::Structure;
    using GarrisonDefinition=generalszh::containment::GarrisonDefinition;
    using Cell=engine::gameplay::navigation::Cell;

    using Carriers=ecs::Query<ecs::Read<Structure>,ecs::Read<Position>,ecs::Read<Health>,ecs::Read<Life>,
        ecs::Optional<Member>>;

    struct Carrier
    {
        Entity entity{},owner{};
        Cell cell{engine::gameplay::navigation::InvalidCell};
        const GarrisonDefinition *definition{};
        std::uint32_t occupied{};
        bool complete{},alive{},ownerAlive{},nested{};
    };

    struct Passenger
    {
        Entity entity{},owner{};
        Cell cell{engine::gameplay::navigation::InvalidCell};
        Member membership{};
        Task task{};
        bool garrisonable{},alive{},ownerAlive{};
        bool hasTask{},hasGoal{},hasCredit{},hasRange{};
        Goal goal{};
        Credit credit{};
        Range range{};
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
    using Query=ecs::Query<ecs::Write<Member>,ecs::Read<Owner>,ecs::Write<Position>,ecs::Read<Health>,ecs::Read<Life>,
        ecs::OptionalWrite<Goal>,ecs::OptionalWrite<Credit>,ecs::OptionalWrite<Range>,ecs::OptionalWrite<Task>>;

    // The carrier join is a complete declaration of the non-query reads used
    // to prepare capacity, ownership, health and catalog-bound policy.
    using AuxiliaryAccess=Carriers;

    GarrisonContainmentSystem(ecs::World &world,const production::BuildCatalog &units,
        const construction::BuildingCatalog &buildings,Batch &batch,std::size_t entityCapacity=65536)
        :world_(world),units_(units),buildings_(buildings),batch_(batch),carriers_(world),snapshot_(entityCapacity)
    {
        passengers_.reserve(entityCapacity);passengerOrder_.reserve(entityCapacity);decisions_.reserve(entityCapacity);
        rowPassengers_.reserve(entityCapacity);chunkSlots_.reserve(entityCapacity);touched_.reserve(entityCapacity);
    }

    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        using namespace engine::gameplay::containment;
        assert(context.Time().Step()==units_.Step());
        currentTick_=context.Tick();
        batch_.Prepare(currentTick_);
        for(const auto index:touched_) snapshot_[index]={};
        touched_.clear();
        carriers_.ForEachChunk([&](Carriers::Chunk chunk) {
            const auto structures=chunk.Get<Structure>();const auto positions=chunk.Get<Position>();
            const auto health=chunk.Get<Health>();const auto lives=chunk.Get<Life>();const auto members=chunk.Get<Member>();
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                const auto entity=chunk.Entities()[row];
                if(entity.index>=snapshot_.size()) throw std::length_error("Garrison snapshot entity-index capacity exceeded");
                touched_.push_back(entity.index);
                const auto *entry=buildings_.Find(structures[row].definition);
                snapshot_[entity.index]={entity,structures[row].account,positions[row].cell,
                    entry&&entry->definition.garrison ? &*entry->definition.garrison : nullptr,0,
                    structures[row].complete,lives[row].alive&&health[row].current!=0,
                    world_.IsAlive(structures[row].account),
                    !members.empty()&&IsContained(members[row])};
            }
        });

        passengers_.clear();passengerOrder_.clear();rowPassengers_.clear();chunkSlots_.clear();
        std::size_t total=0;
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            if(total>(std::numeric_limits<std::size_t>::max)()-chunk.Count())
                throw std::length_error("Garrison passenger snapshot size overflow");
            chunkSlots_.push_back({total,chunk.Count()});
            const auto members=chunk.Get<Member>();const auto owners=chunk.Get<Owner>();
            const auto positions=chunk.Get<Position>();const auto health=chunk.Get<Health>();const auto lives=chunk.Get<Life>();
            const auto goals=chunk.Get<Goal>();const auto credits=chunk.Get<Credit>();const auto ranges=chunk.Get<Range>();
            const auto tasks=chunk.Get<Task>();
            const auto definitions=units_.Definitions();
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                if(passengers_.size()==snapshot_.size()) throw std::length_error("Garrison passenger snapshot capacity exceeded");
                rowPassengers_.push_back(passengers_.size());
                const auto entity=chunk.Entities()[row];
                auto &passenger=passengers_.emplace_back();
                passenger.entity=entity;passenger.owner=owners[row].account;passenger.cell=positions[row].cell;
                passenger.membership=members[row];passenger.ownerAlive=world_.IsAlive(passenger.owner);
                passenger.alive=lives[row].alive&&health[row].current!=0;
                passenger.hasTask=!tasks.empty();if(passenger.hasTask) passenger.task=tasks[row];
                passenger.hasGoal=!goals.empty();passenger.hasCredit=!credits.empty();passenger.hasRange=!ranges.empty();
                if(passenger.hasGoal) passenger.goal=goals[row];
                if(passenger.hasCredit) passenger.credit=credits[row];
                if(passenger.hasRange) passenger.range=ranges[row];
                const auto definitionIndex=units_.DefinitionIndex(owners[row].definition);
                passenger.garrisonable=definitionIndex<definitions.size()&&
                    definitions[definitionIndex].key==owners[row].definition&&definitions[definitionIndex].garrisonable;
                // Same-wave capacity is based on members that will actually
                // survive Execute's lifecycle validation. Dead, owner-invalid,
                // nested, ineligible, or otherwise stale garrison rows release
                // this wave and do not consume a slot for span-order entries.
                if(IsGarrisonOwned(passenger.membership))
                    if(auto *carrier=Find(passenger.membership.carrier);
                        IsRetainable(passenger,carrier,passenger.membership)) ++carrier->occupied;
            }
            total+=chunk.Count();
        });
        for(std::size_t index=0;index<passengers_.size();++index) passengerOrder_.push_back(index);
        std::sort(passengerOrder_.begin(),passengerOrder_.end(),[&](std::size_t left,std::size_t right) {
            return Less(passengers_[left].entity,passengers_[right].entity);
        });
        decisions_.clear();decisions_.reserve(passengers_.size());
        for(const auto passengerIndex:passengerOrder_)
        {
            auto &passenger=passengers_[passengerIndex];passenger.decisionIndex=decisions_.size();
            decisions_.push_back({passenger.entity,passengerIndex,passenger.membership,passenger.task,passenger.goal,passenger.credit,
                passenger.range,passenger.cell,passenger.hasTask,passenger.hasGoal,passenger.hasCredit,passenger.hasRange,false});
        }

        // Input order is the garrison system's deterministic arbitration order.
        // Transport runs before this system at the root, so a simultaneous
        // cross-owner request observes Transport's committed ownership here.
        const auto inputs=batch_.Inputs();
        for(std::size_t index=0;index<inputs.size();++index)
            batch_.Result(index).outcome=AcceptDirect(inputs[index]);

        // Preparation only snapshots and arbitrates direct input. Retained
        // member projection, stale-task cleanup and release are lifecycle
        // writes performed by Execute below.
    }

    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        using namespace engine::gameplay::containment;
        const auto slot=chunkSlots_[context.ChunkOrder()];
        assert(slot.count==chunk.Count());
        auto members=chunk.Get<Member>();auto positions=chunk.Get<Position>();
        const auto lives=chunk.Get<Life>();const auto health=chunk.Get<Health>();
        auto goals=chunk.Get<Goal>();auto credits=chunk.Get<Credit>();auto ranges=chunk.Get<Range>();auto tasks=chunk.Get<Task>();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            const auto passengerIndex=rowPassengers_[slot.first+row];
            const auto &decision=decisions_[passengers_[passengerIndex].decisionIndex];
            auto &member=members[row];
            // Transport owns this row. Do not let a garrison input, orphan
            // check, projection or cleanup alter any transport state.
            if(IsTransportOwned(member)) continue;
            member=decision.membership;positions[row].cell=decision.position;
            if(!tasks.empty()&&decision.hasTask) tasks[row]=decision.task;
            if(!goals.empty()&&decision.hasGoal) goals[row]=decision.goal;
            if(!credits.empty()&&decision.hasCredit) credits[row]=decision.credit;
            if(!ranges.empty()&&decision.hasRange) ranges[row]=decision.range;
            if(!IsGarrisonOwned(member)) continue;
            const auto &passenger=passengers_[passengerIndex];
            const auto *carrier=Find(member.carrier);
            if(!IsRetainable(passenger,carrier,member)||!lives[row].alive||health[row].current==0)
            {
                member={};
                ClearActuators(tasks,row,goals,credits,ranges);
                continue;
            }
            positions[row].cell=carrier->cell;
            ClearActuators(tasks,row,goals,credits,ranges);
        }
    }

    void AfterChunks(Query &,ecs::SystemContext &) noexcept
    {
        batch_.Publish();
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

    void SyncDecision(const Passenger &passenger) noexcept
    {
        auto &decision=decisions_[passenger.decisionIndex];
        decision.membership=passenger.membership;decision.position=passenger.cell;
        if(passenger.hasTask) decision.task=passenger.task;
        if(passenger.hasGoal) decision.goal=passenger.goal;
        if(passenger.hasCredit) decision.credit=passenger.credit;
        if(passenger.hasRange) decision.range=passenger.range;
    }
    void ClearMovement(Passenger &passenger) const noexcept
    {
        if(passenger.hasGoal) passenger.goal={};
        if(passenger.hasCredit) passenger.credit={};
        if(passenger.hasRange) passenger.range={};
    }

    static void ClearActuators(const auto tasks,const std::size_t row,
        auto goals,auto credits,auto ranges) noexcept
    {
        if(!tasks.empty()) tasks[row]={};
        if(!goals.empty()) goals[row]={};
        if(!ranges.empty()&& !credits.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
        else if(!ranges.empty()) ranges[row]={};
        if(!credits.empty()) credits[row]={};
    }

    static bool IsRetainable(const Passenger &passenger,const Carrier *carrier,const Member &membership) noexcept
    {
        return engine::gameplay::containment::IsGarrisonOwned(membership)&&
            membership.phase==engine::gameplay::containment::ContainmentPhase::Inside&&
            membership.combat==engine::gameplay::containment::PassengerCombatPolicy::Allowed &&
            passenger.garrisonable&&passenger.alive&&passenger.ownerAlive&&carrier&&carrier->definition&&
            carrier->complete&&carrier->alive&&carrier->ownerAlive&&carrier->owner.IsValid()&&
            carrier->owner==passenger.owner&&!carrier->nested;
    }

    Outcome ValidateEnter(const Passenger &passenger,Entity requestedCarrier,const Carrier *carrier) const noexcept
    {
        if(!passenger.alive) return Outcome::NotAlive;
        if(!passenger.garrisonable||passenger.entity==requestedCarrier) return Outcome::UnsupportedActor;
        if(!carrier||!carrier->definition||!carrier->complete) return Outcome::InvalidCarrier;
        if(!carrier->alive) return Outcome::NotAlive;
        if(carrier->nested) return Outcome::UnsupportedActor;
        if(!passenger.ownerAlive||!carrier->ownerAlive||!carrier->owner.IsValid()||
            passenger.owner!=carrier->owner) return Outcome::WrongOwner;
        if(carrier->occupied>=carrier->definition->capacity) return Outcome::Full;
        if(passenger.cell!=carrier->cell) return Outcome::NotArrived;
        return Outcome::Entered;
    }

    Outcome AcceptExit(Passenger &passenger,const engine::gameplay::containment::ContainmentInput &input)
    {
        using namespace engine::gameplay::containment;
        if(!IsGarrisonOwned(passenger.membership)) return Outcome::InvalidCarrier;
        if(!passenger.alive) return Outcome::NotAlive;
        if(passenger.membership.carrier!=input.carrier) return Outcome::InvalidCarrier;
        auto *carrier=Find(passenger.membership.carrier);
        if(!carrier||!carrier->definition) return Outcome::InvalidCarrier;
        if(!carrier->alive) return Outcome::NotAlive;
        // Capacity was prepared from the pre-input retained snapshot. Capture
        // this fact before clearing the relationship so an already-invalid
        // member released by this input cannot decrement another occupant.
        const bool counted=IsRetainable(passenger,carrier,passenger.membership);
        passenger.cell=carrier->cell;passenger.membership={};
        if(counted&&carrier->occupied) --carrier->occupied;
        if(passenger.hasTask) passenger.task={};
        ClearMovement(passenger);SyncDecision(passenger);
        return Outcome::Exited;
    }

    Outcome AcceptDirect(const engine::gameplay::containment::ContainmentInput &input)
    {
        using namespace engine::gameplay::containment;
        auto *passenger=FindPassenger(input.passenger);
        if(!passenger) return Outcome::InvalidPassenger;
        decisions_[passenger->decisionIndex].directHandled=true;
        // The owner discriminator, not combat policy, determines which system
        // may mutate the relationship. Other-owner rows keep every value.
        if(IsTransportOwned(passenger->membership))
            return input.action==ContainmentAction::Enter ? Outcome::AlreadyContained :
                input.action==ContainmentAction::Exit ? Outcome::InvalidCarrier : Outcome::InvalidAction;
        if(input.action==ContainmentAction::Exit)
            return AcceptExit(*passenger,input);
        if(input.action!=ContainmentAction::Enter) return Outcome::InvalidAction;
        if(IsGarrisonOwned(passenger->membership)) return Outcome::AlreadyContained;
        if(IsContained(passenger->membership)) return Outcome::AlreadyContained;
        auto *carrier=Find(input.carrier);
        const auto outcome=ValidateEnter(*passenger,input.carrier,carrier);
        if(outcome!=Outcome::Entered) return outcome;
        passenger->membership={input.carrier,0,ContainmentPhase::Inside,
            PassengerContainmentOwner::Garrison,PassengerCombatPolicy::Allowed};
        ++carrier->occupied;passenger->cell=carrier->cell;if(passenger->hasTask) passenger->task={};
        ClearMovement(*passenger);SyncDecision(*passenger);
        return Outcome::Entered;
    }

    ecs::World &world_;
    const production::BuildCatalog &units_;
    const construction::BuildingCatalog &buildings_;
    Batch &batch_;
    Carriers carriers_;
    std::vector<Carrier> snapshot_;
    std::vector<Passenger> passengers_;
    std::vector<std::size_t> passengerOrder_,rowPassengers_;
    std::vector<Decision> decisions_;
    std::vector<ChunkSlot> chunkSlots_;
    std::vector<std::uint32_t> touched_;
    std::uint64_t currentTick_{};
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::containment::GarrisonContainmentSystem>
{
    static constexpr std::string_view StableName="games.generalszh.containment.garrison";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
