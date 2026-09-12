module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.capture.systems.capture_system;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
export import games.generalszh.gameplay.capture.inputs.capture_batch;
export import games.generalszh.gameplay.match.systems.defeat_system;
export import games.generalszh.gameplay.harvesting.revenue.harvest_revenue;
export import games.generalszh.gameplay.economy.income.income_account;
export import games.generalszh.gameplay.selling.components.sale_state;
export import engine.gameplay.rts.power.components.power_source;
export import engine.gameplay.rts.economy.components.resource_balance;
export namespace generalszh::capture
{
class CaptureSystem
{
    using Structure=construction::Structure;
    using Unit=production::ProducedUnit;
    using Producer=production::Producer;
    using Life=engine::gameplay::combat::LifeState;
    using Health=engine::gameplay::combat::Health;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Order=engine::gameplay::rts::orders::UnitOrder;
    using Member=engine::gameplay::containment::PassengerMembership;
    using Transport=engine::gameplay::containment::TransportBinding;
    using Harvest=engine::gameplay::rts::harvesting::HarvestPolicy;
    using Queue=engine::gameplay::rts::production::ProductionQueue;
    using Power=engine::gameplay::rts::power::PowerSource;
    using Victory=match::VictoryAsset;
    using Dropoff=harvesting::SupplyDropoffOwner;
    using Income=economy::IncomeAccount;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    using Sale=selling::SaleState;
    using Actors=ecs::Query<ecs::Read<CaptureCapability>,ecs::Write<CaptureActorState>,ecs::Write<CaptureRecharge>,ecs::Read<Unit>,
        ecs::Read<Position>,ecs::Read<Life>,ecs::Read<Health>,ecs::Optional<Member>,ecs::Optional<Goal>,
        ecs::Optional<Order>,ecs::Optional<construction::Builder>,ecs::Optional<Harvest>>;
    struct Actor {
        ecs::Entity entity{},account{};
        engine::gameplay::navigation::Cell cell{};
        CaptureActorState *state{};
        CaptureRecharge *recharge{};
        const CompiledCaptureDefinition *definition{};
        bool eligible{},busy{},interrupted{};
    };
    struct Completion {
        CaptureResult result{};ecs::Entity actor{};std::uint64_t recovery{};bool ready{};
        std::uint64_t rechargeUntil{};bool rechargeChanged{};
    };
public:
    using Query=ecs::Query<ecs::Write<Structure>,ecs::Write<CaptureState>,ecs::Read<Capturable>,
        ecs::Read<Position>,ecs::Read<Life>,ecs::Read<Health>,ecs::OptionalWrite<Producer>,
        ecs::OptionalWrite<Power>,ecs::OptionalWrite<Victory>,ecs::OptionalWrite<Dropoff>,
        ecs::Optional<Queue>,ecs::Optional<Sale>,ecs::Optional<Income>,ecs::Optional<Unit>,ecs::Optional<Transport>>;
    using AuxiliaryAccess=ecs::Query<ecs::Read<CaptureCapability>,ecs::Write<CaptureActorState>,ecs::Write<CaptureRecharge>,
        ecs::Read<Unit>,ecs::Read<Position>,ecs::Read<Life>,ecs::Read<Health>,ecs::Read<Member>,
        ecs::Read<Goal>,ecs::Read<Order>,ecs::Read<construction::Builder>,ecs::Read<Harvest>,ecs::Read<Balance>>;
    CaptureSystem(ecs::World &world,const CaptureDefinitions &definitions,CaptureBatch &batch,std::size_t entityCapacity=65536)
        :world_(world),definitions_(definitions),batch_(batch),actors_(world),snapshot_(entityCapacity),completed_(entityCapacity)
    {
        if(entityCapacity>UINT32_MAX
            ||batch.ResultCapacity()<CaptureBatch::RequiredResultCapacity(batch.InputCapacity(),entityCapacity))
            throw std::invalid_argument("Capture results require input capacity plus twice entity capacity");
        touched_.reserve(entityCapacity);targets_.reserve(entityCapacity);actorCancellations_.reserve(entityCapacity);
    }
    // Borrowed sorted (index,generation) span survives through AfterChunks.
    // Root orders this consumer before TaskSelection clears ManualActors().
    void SetInterruptions(std::span<const ecs::Entity> actors) {
        if(!std::is_sorted(actors.begin(),actors.end(),Less)) throw std::invalid_argument("Capture interruptions must be sorted");
        interruptions_=actors;
    }
    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        assert(context.Time().Step()==definitions_.Step());
        const auto inputs=batch_.BeginTick();
        actorCancellations_.clear();
        for(const auto index:touched_) snapshot_[index]={};touched_.clear();
        for(const auto entity:targets_) completed_[entity.index]={};targets_.clear();
        actors_.ForEachChunk([&](Actors::Chunk chunk) {
            const auto capabilities=chunk.Get<CaptureCapability>();auto states=chunk.Get<CaptureActorState>();
            auto recharges=chunk.Get<CaptureRecharge>();
            const auto units=chunk.Get<Unit>();const auto positions=chunk.Get<Position>();
            const auto life=chunk.Get<Life>();const auto health=chunk.Get<Health>();
            const auto members=chunk.Get<Member>();const auto goals=chunk.Get<Goal>();const auto orders=chunk.Get<Order>();
            const bool unsupported=!chunk.Get<construction::Builder>().empty()||!chunk.Get<Harvest>().empty();
            for(std::size_t i=0;i<chunk.Count();++i) {
                const auto entity=chunk.Entities()[i];CheckIndex(entity);
                auto &state=states[i];
                if(state.phase==CaptureActorPhase::Recovering&&context.Tick()>=state.recoveryUntil) state={};
                const bool interrupted=std::binary_search(interruptions_.begin(),interruptions_.end(),entity,Less);
                if(state.phase==CaptureActorPhase::Capturing) {
                    const auto *reservation=world_.Get<CaptureState>(state.target);
                    if(!reservation||!reservation->active||reservation->actor!=entity) {
                        actorCancellations_.push_back({entity,state.target,units[i].account,context.Tick(),CaptureOutcome::Interrupted});
                        state={};
                    }
                }
                if(interrupted&&state.phase==CaptureActorPhase::Recovering) {
                    actorCancellations_.push_back({entity,{},units[i].account,context.Tick(),CaptureOutcome::Interrupted});state={};
                }
                // An arrived ordinary Move may retain its destination. That is
                // not ongoing travel and must not block a new capture task.
                const bool moving=!goals.empty()&&goals[i].cell!=engine::gameplay::navigation::InvalidCell
                    &&goals[i].cell!=positions[i].cell;
                const bool ordered=!orders.empty()&&orders[i].kind!=engine::gameplay::rts::orders::OrderKind::None
                    &&orders[i].kind!=engine::gameplay::rts::orders::OrderKind::Stop;
                snapshot_[entity.index]={entity,units[i].account,positions[i].cell,&state,&recharges[i],
                    &definitions_.GetUnchecked(capabilities[i].definition),capabilities[i].enabled&&recharges[i].initialized&&life[i].alive&&health[i].current
                    &&!unsupported&&world_.IsAlive(units[i].account)&&world_.Get<Balance>(units[i].account)
                    &&(members.empty()||!engine::gameplay::containment::IsContained(members[i])),moving||ordered,interrupted};
                touched_.push_back(entity.index);
            }
        });
        std::sort(actorCancellations_.begin(),actorCancellations_.end(),[](const auto &a,const auto &b){return Less(a.actor,b.actor);});
        for(const auto &result:actorCancellations_) batch_.Append(result);
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            auto states=chunk.Get<CaptureState>();
            for(std::size_t i=0;i<chunk.Count();++i) {
                const auto entity=chunk.Entities()[i];CheckIndex(entity);targets_.push_back(entity);
                auto &state=states[i];const auto *actor=Find(state.actor);
                // Ordinary requests beat completion, even at the exact deadline.
                if(state.active&&(!world_.IsAlive(state.originalAccount)||!world_.Get<Balance>(state.originalAccount)
                    ||(actor&&actor->interrupted))) {
                    batch_.Append({state.actor,entity,state.acceptedAccount,context.Tick(),CaptureOutcome::Interrupted});
                    if(actor&&actor->state->target==entity) *actor->state={};state={};
                }
            }
        });
        for(const auto &input:inputs) batch_.Append({input.actor,input.target,input.account,context.Tick(),Accept(input,context.Tick())});
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &context)
    {
        auto structures=chunk.Get<Structure>();auto states=chunk.Get<CaptureState>();const auto eligible=chunk.Get<Capturable>();
        const auto positions=chunk.Get<Position>();const auto lives=chunk.Get<Life>();const auto health=chunk.Get<Health>();
        auto producers=chunk.Get<Producer>();auto power=chunk.Get<Power>();auto victory=chunk.Get<Victory>();auto dropoffs=chunk.Get<Dropoff>();
        const auto queues=chunk.Get<Queue>();const auto sales=chunk.Get<Sale>();
        const bool unsupported=!chunk.Get<Income>().empty()||!chunk.Get<Unit>().empty()||!chunk.Get<Transport>().empty();
        for(std::size_t i=0;i<chunk.Count();++i) {
            auto &state=states[i];if(!state.active) continue;
            const auto target=chunk.Entities()[i];const auto *actor=Find(state.actor);
            const auto owner=structures[i].account;
            const bool bindings=(producers.empty()||producers[i].account==owner)&&(power.empty()||power[i].account==owner)
                &&(victory.empty()||victory[i].account==owner)&&(dropoffs.empty()||dropoffs[i].account==owner);
            const bool valid=actor&&actor->eligible&&!actor->busy&&!actor->interrupted&&actor->account==state.acceptedAccount
                &&actor->state->phase==CaptureActorPhase::Capturing&&actor->state->target==target
                &&actor->cell==positions[i].cell&&owner==state.originalAccount&&structures[i].complete
                &&eligible[i].enabled&&lives[i].alive&&health[i].current&&!unsupported&&bindings
                &&(queues.empty()||!queues[i].count)&&(producers.empty()||!queues.empty())
                &&(sales.empty()||sales[i].phase==selling::SalePhase::Idle);
            // Account validity is snapshotted at the joined boundary, below;
            // no dynamic per-target ownership lookup in these ordinary rows.
            auto &completion=completed_[target.index];
            // Start preparation consumes recharge, including zero preparation.
            // Continuing preparation resets it; the final success tick does not.
            // Jobs record target-local updates, applied to actors only after join.
            if(valid&&context.Tick()>=state.preparationTick
                &&(!state.preparationStarted||context.Tick()<state.completionTick)) {
                completion.actor=state.actor;
                completion.rechargeUntil=CaptureDeadline(context.Tick(),actor->definition->rechargeTicks);
                completion.rechargeChanged=true;state.preparationStarted=true;
            }
            if(valid&&context.Tick()<state.completionTick) continue;
            completion.result={state.actor,target,state.acceptedAccount,context.Tick(),valid?CaptureOutcome::Captured:CaptureOutcome::Interrupted};
            completion.actor=state.actor;
            completion.recovery=valid?CaptureDeadline(context.Tick(),actor->definition->recoveryTicks):0;
            completion.ready=true;
            if(valid) {
                structures[i].account=state.acceptedAccount;
                if(!producers.empty()) producers[i].account=state.acceptedAccount;
                if(!power.empty()) power[i].account=state.acceptedAccount;
                if(!victory.empty()) victory[i].account=state.acceptedAccount;
                if(!dropoffs.empty()) dropoffs[i].account=state.acceptedAccount;
            }
            state={};
        }
    }
    void AfterChunks(Query &,ecs::SystemContext &) {
        std::sort(targets_.begin(),targets_.end(),Less);
        for(const auto target:targets_) {
            const auto &value=completed_[target.index];
            if(value.rechargeChanged)
                if(auto *actor=Find(value.actor);actor&&actor->state->target==target)
                    actor->recharge->readyAt=value.rechargeUntil;
            if(!value.ready) continue;
            if(auto *actor=Find(value.actor);actor&&actor->state->target==target) {
                *actor->state={};
                if(value.result.outcome==CaptureOutcome::Captured)
                    *actor->state={{},CaptureActorPhase::Recovering,value.recovery};
            }
            batch_.Append(value.result);
        }
        interruptions_={};batch_.Publish();
    }
private:
    static bool Less(ecs::Entity a,ecs::Entity b) noexcept {return std::tie(a.index,a.generation)<std::tie(b.index,b.generation);}
    void CheckIndex(ecs::Entity entity) const {if(entity.index>=snapshot_.size()) throw std::length_error("Capture entity-index capacity");}
    Actor *Find(ecs::Entity entity) noexcept {return entity.IsValid()&&entity.index<snapshot_.size()&&snapshot_[entity.index].entity==entity?&snapshot_[entity.index]:nullptr;}
    CaptureOutcome Accept(const CaptureInput &input,std::uint64_t tick) {
        auto *actor=Find(input.actor);if(!actor) return CaptureOutcome::InvalidActor;
        if(actor->account!=input.account||!world_.IsAlive(input.account)||!world_.Get<Balance>(input.account)) return CaptureOutcome::WrongOwner;
        auto *state=world_.Get<CaptureState>(input.target);
        if(input.action==CaptureAction::Cancel) {
            if(!state||!state->active||state->actor!=input.actor) return CaptureOutcome::InvalidTarget;
            *state={};*actor->state={};return CaptureOutcome::Cancelled;
        }
        if(input.action!=CaptureAction::Start) return CaptureOutcome::InvalidAction;
        if(actor->interrupted||actor->busy||IsCaptureBusy(*actor->state)) return CaptureOutcome::Busy;
        if(!actor->eligible) return CaptureOutcome::Ineligible;
        if(tick<actor->recharge->readyAt) return CaptureOutcome::Ineligible;
        auto *structure=world_.Get<Structure>(input.target);const auto *eligibility=world_.Get<Capturable>(input.target);
        const auto *life=world_.Get<Life>(input.target);const auto *health=world_.Get<Health>(input.target);const auto *position=world_.Get<Position>(input.target);
        if(!state||!structure||!eligibility||!life||!health||!position) return CaptureOutcome::InvalidTarget;
        if(state->active) return CaptureOutcome::Busy;
        if(structure->account==input.account||!world_.IsAlive(structure->account)||!world_.Get<Balance>(structure->account)) return CaptureOutcome::WrongOwner;
        if(!eligibility->enabled||!structure->complete||!life->alive||!health->current||position->cell!=actor->cell) return CaptureOutcome::Ineligible;
        if(world_.Get<Income>(input.target)||world_.Get<Unit>(input.target)||world_.Get<Transport>(input.target)) return CaptureOutcome::UnsupportedSignature;
        const auto *sale=world_.Get<Sale>(input.target);if(sale&&sale->phase!=selling::SalePhase::Idle) return CaptureOutcome::Ineligible;
        const auto *producer=world_.Get<Producer>(input.target);const auto *queue=world_.Get<Queue>(input.target);
        if((producer&&!queue)||(queue&&queue->count)) return CaptureOutcome::UnsupportedSignature;
        const auto *power=world_.Get<Power>(input.target);const auto *victory=world_.Get<Victory>(input.target);const auto *dropoff=world_.Get<Dropoff>(input.target);
        if((producer&&producer->account!=structure->account)||(power&&power->account!=structure->account)
            ||(victory&&victory->account!=structure->account)||(dropoff&&dropoff->account!=structure->account)) return CaptureOutcome::InvalidBindings;
        const auto completionTick=CaptureDeadline(tick,actor->definition->captureTicks);
        const auto preparationTick=CaptureDeadline(tick,actor->definition->unpackTicks);
        *state={input.actor,input.account,structure->account,completionTick,true,preparationTick,false};
        *actor->state={input.target,CaptureActorPhase::Capturing,0};return CaptureOutcome::Started;
    }
    ecs::World &world_;const CaptureDefinitions &definitions_;CaptureBatch &batch_;Actors actors_;
    std::vector<Actor> snapshot_;std::vector<Completion> completed_;
    std::vector<std::uint32_t> touched_;std::vector<ecs::Entity> targets_;std::span<const ecs::Entity> interruptions_;
    std::vector<CaptureResult> actorCancellations_;
};
}
export namespace ecs {
template<> struct SystemTraits<generalszh::capture::CaptureSystem> {
    static constexpr std::string_view StableName="games.generalszh.capture.transfer";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;using After=SystemTypeList<engine::gameplay::combat::HealthSystem>;
};
}
