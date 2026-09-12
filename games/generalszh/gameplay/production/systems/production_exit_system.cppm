module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.systems.production_exit_system;
export import games.generalszh.gameplay.production.systems.production_system;
export import games.generalszh.gameplay.production.doors.definitions.production_exit_definition;
namespace generalszh::production_exit_detail
{
class HorizonError final : public std::overflow_error
{ public: HorizonError():std::overflow_error("Production exit exceeds simulation tick horizon") {} };
[[noreturn]] void ThrowHorizon() {throw HorizonError{};}
}
export namespace generalszh::production
{
// Factory exit coordination: progress is complete before this behaviour runs.
// Owns logical doors, release of completed output, and retirement of its queue
// entries. No animation callback, legacy lease, child system or private plan.
class ProductionExitSystem
{
public:
    using Query=ecs::Query<ecs::Read<Producer>,ecs::Read<Life>,
        ecs::OptionalWrite<ProductionExitState>,ecs::Optional<ProductionExitTiming>,
        ecs::Optional<rally::RallyPoint>>;
    using AuxiliaryAccess=ecs::Query<ecs::Read<BuildOrder>,ecs::Read<Ready>,
        ecs::Write<Quantity>,ecs::Write<Member>,ecs::Write<Queue>>;
    using CompletedOrders=ecs::Query<ecs::Read<BuildOrder>,ecs::Read<Ready>,
        ecs::Write<Quantity>,ecs::Read<Member>>;
    ProductionExitSystem(ecs::World &world,BuildBatch &batch)
        :world(world),batch(batch),completed(world),orders(world)
    {
        candidates.reserve(batch.Capacity()); slots.reserve(batch.Capacity());
        offsets.reserve(batch.Capacity()); retired.reserve(batch.Capacity());
        members.reserve(batch.Capacity());
    }
    void BeforeChunks(Query &query,ecs::SystemContext &)
    {
        candidates.clear(); slots.clear(); offsets.clear();
        completed.ForEachChunk([&](auto chunk) {
            const auto values=chunk.template Get<BuildOrder>();
            const auto ready=chunk.template Get<Ready>();
            const auto membership=chunk.template Get<Member>();
            auto quantities=chunk.template Get<Quantity>();
            for(std::size_t i=0;i<chunk.Count();++i)
            {
                if(!ready[i].value || !membership[i].queue.IsValid() || membership[i].position!=0
                    || quantities[i].completed==quantities[i].total) continue;
                if(membership[i].queue!=values[i].producer)
                    throw std::logic_error("Production exit order/queue owner mismatch");
                if(candidates.size()==batch.Capacity()) throw std::length_error("Production exit capacity exhausted");
                candidates.push_back({values[i].producer,&values[i],&quantities[i]});
            }
        });
        std::sort(candidates.begin(),candidates.end(),[](const auto &a,const auto &b){return Key(a.producer)<Key(b.producer);});
        for(std::size_t i=1;i<candidates.size();++i)
            if(candidates[i-1].producer==candidates[i].producer)
                throw std::logic_error("Production exit requires one ready queue head per producer");
        // Resolve cross-archetype relationships once, before jobs. Each quantity
        // pointer belongs to exactly one producer row/job, validated above.
        query.ForEachPreparedChunk([&](auto chunk) {
            if(chunk.Count()>batch.Capacity()-slots.size()) throw std::length_error("Production producer capacity exhausted");
            const auto doors=chunk.template Get<ProductionExitState>();
            const auto timing=chunk.template Get<ProductionExitTiming>();
            if(doors.empty()!=timing.empty()) throw std::logic_error("Production exit requires both state and timing");
            offsets.push_back(slots.size());
            for(auto entity:chunk.Entities())
            {
                const auto found=std::lower_bound(candidates.begin(),candidates.end(),entity,
                    [](const auto &candidate,auto target){return Key(candidate.producer)<Key(target);});
                slots.push_back(found!=candidates.end() && found->producer==entity ? *found : Slot{});
            }
        });
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        const auto producers=chunk.Get<Producer>(); const auto life=chunk.Get<Life>();
        auto doors=chunk.Get<ProductionExitState>(); const auto timing=chunk.Get<ProductionExitTiming>();
        const auto rallyPoints=chunk.Get<rally::RallyPoint>();
        assert(context.ChunkOrder()<offsets.size());
        const auto offset=offsets[context.ChunkOrder()]; const auto now=context.Tick();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            if(!life[row].alive) continue;
            if(!doors.empty()) Advance(doors[row],timing[row],now);
            if(!producers[row].active) continue;
            const auto &slot=slots[offset+row];
            if(!slot.order) continue;
            if(slot.order->kind==EntryKind::Unit && !doors.empty())
            {
                auto &door=doors[row];
                switch(door.phase)
                {
                case ExitDoorPhase::Closed: Start(door,ExitDoorPhase::Opening,now,timing[row].opening); break;
                case ExitDoorPhase::Closing: Start(door,ExitDoorPhase::Open,now,timing[row].waiting); break;
                case ExitDoorPhase::Open: Start(door,ExitDoorPhase::Open,now,timing[row].waiting); break;
                case ExitDoorPhase::Opening: break;
                }
                if(door.phase!=ExitDoorPhase::Open) continue;
            }
            CompleteBuild(*slot.order,*slot.quantity,context.Commands(),now,
                rallyPoints.empty()?engine::gameplay::rts::orders::UnitOrder{}:rally::InitialRallyOrder(rallyPoints[row]));
        }
    }
    void AfterChunks(Query &,ecs::SystemContext &context)
    {
        CollectOrders(orders,retired,batch.Capacity());
        bool removed=false;
        for(auto entry:retired)
        {
            const auto &quantity=*world.Get<Quantity>(entry);
            if(quantity.completed==quantity.total)
            {
                MarkUnlinked(world,entry); context.Commands().Destroy(entry); removed=true;
            }
        }
        if(removed) CompactMembers(world,orders,members,batch.Capacity());
    }
private:
    struct Slot { ecs::Entity producer{}; const BuildOrder *order{}; Quantity *quantity{}; };
    static std::tuple<ecs::EntityIndex,ecs::EntityGeneration> Key(ecs::Entity entity) noexcept {return {entity.index,entity.generation};}
    static void Start(ProductionExitState &door,ExitDoorPhase phase,std::uint64_t now,std::uint64_t duration)
    {
        // Strict expiry requires duration+1 representable future ticks. This is
        // a terminal lifecycle failure, not a removable numeric diagnostic.
        if(duration>=(std::numeric_limits<std::uint64_t>::max)()-now) production_exit_detail::ThrowHorizon();
        door.phase=phase;door.since=now;
    }
    static void Advance(ProductionExitState &door,const ProductionExitTiming &timing,std::uint64_t now)
    {
        assert(now>=door.since);
        // Reference uses strict elapsed > duration, at most one transition/tick.
        const auto elapsed=now-door.since;
        switch(door.phase)
        {
        case ExitDoorPhase::Closed:
            if(door.held) Start(door,ExitDoorPhase::Opening,now,timing.opening);
            break;
        case ExitDoorPhase::Opening:
            if(elapsed>timing.opening) Start(door,ExitDoorPhase::Open,now,timing.waiting);
            break;
        case ExitDoorPhase::Open:
            if(!door.held && elapsed>timing.waiting) Start(door,ExitDoorPhase::Closing,now,timing.closing);
            break;
        case ExitDoorPhase::Closing:
            if(!door.held && elapsed>timing.closing) {door.phase=ExitDoorPhase::Closed;door.since=now;}
            break;
        }
    }
    ecs::World &world; BuildBatch &batch; CompletedOrders completed; Orders orders;
    // Reusable borrowed references are never authoritative state and never survive
    // a structural barrier without being rebuilt. No worker appends to these.
    std::vector<Slot> candidates,slots;
    std::vector<std::size_t> offsets;
    std::vector<ecs::Entity> retired;
    std::vector<MemberPosition> members;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::production::ProductionExitSystem>
{
    static constexpr std::string_view StableName="games.generalszh.production.exit";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<generalszh::production::ProductionSystem>;
};
}
