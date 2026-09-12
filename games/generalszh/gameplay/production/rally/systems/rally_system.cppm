module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.rally.systems.rally_system;
export import games.generalszh.gameplay.production.rally.components.rally_point;
export import games.generalszh.gameplay.production.rally.inputs.rally_batch;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import engine.gameplay.navigation.fields.flow_fields;
export namespace generalszh::production::rally
{
class RallySystem
{
    using Cell=engine::gameplay::navigation::Cell;
    using Health=engine::gameplay::combat::Health;
    using Life=engine::gameplay::combat::LifeState;
    using Position=engine::gameplay::navigation::GridPosition;
    static constexpr Cell Invalid=engine::gameplay::navigation::InvalidCell;
    struct Candidate {ecs::Entity producer;Cell destination,field;std::size_t receipt;};
    static auto Identity(ecs::Entity entity) noexcept {return std::tuple{entity.index,entity.generation};}
public:
    using Query=ecs::Query<ecs::Read<Producer>,ecs::Read<Health>,ecs::Read<Life>,ecs::Read<Position>,ecs::Write<RallyPoint>>;
    // Dedicated validationFields must use this same immutable grid. Root owns
    // all lifetimes and must not share the cache concurrently with movement.
    RallySystem(const engine::gameplay::navigation::NavigationGrid &grid,engine::jobs::JobSystem &jobs,
        engine::gameplay::navigation::FlowFields &validationFields,RallyBatch &batch,std::size_t producerCapacity=65536)
        :grid_(grid),jobs_(jobs),fields_(validationFields),batch_(batch),capacity_(producerCapacity)
    {candidates_.reserve(batch.Capacity());goals_.reserve(batch.Capacity());}
    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        std::size_t count=0;
        query.ForEachPreparedChunk([&](Query::Chunk chunk){
            if(chunk.Count()>capacity_-count) throw std::length_error("Rally producer capacity exhausted");
            count+=chunk.Count();
        });
        candidates_.clear();goals_.clear();const auto inputs=batch_.Prepare();auto &world=context.GetWorld();
        for(std::size_t i=0;i<inputs.size();++i)
        {
            const auto &input=inputs[i];
            const auto *producer=world.Get<Producer>(input.producer);const auto *health=world.Get<Health>(input.producer);
            const auto *life=world.Get<Life>(input.producer);const auto *position=world.Get<Position>(input.producer);
            // Actual eligibility signature, not a generic feature context.
            if(!producer||!health||!life||!position||!world.Get<RallyPoint>(input.producer)) continue;
            if(!world.IsAlive(input.account)||input.account!=producer->account) {batch_.Record(i,RallyAcceptance::WrongOwner);continue;}
            if(!life->alive||!health->current) {batch_.Record(i,RallyAcceptance::NotAlive);continue;}
            if(!producer->active) {batch_.Record(i,RallyAcceptance::Inactive);continue;}
            if(input.destination!=Invalid && !grid_.Walkable(input.destination))
            {batch_.Record(i,RallyAcceptance::InvalidDestination);continue;}
            // Clearing does not need a path or a valid source navigation cell.
            if(input.destination!=Invalid && !grid_.Walkable(position->cell))
            {batch_.Record(i,RallyAcceptance::Unreachable);continue;}
            candidates_.push_back({input.producer,input.destination,Invalid,i});
            if(input.destination!=Invalid) goals_.push_back(input.destination);
        }
        std::sort(goals_.begin(),goals_.end());goals_.erase(std::unique(goals_.begin(),goals_.end()),goals_.end());
        // Existing narrow reverse-BFS algorithm. Its bounded jobs join here,
        // before chunk writes; this is not a second gameplay execution plan.
        if(!goals_.empty()) fields_.Prepare(goals_,jobs_);
        for(auto &candidate:candidates_) if(candidate.destination!=Invalid)
        {candidate.field=fields_.Find(candidate.destination);assert(candidate.field!=Invalid);}
        std::sort(candidates_.begin(),candidates_.end(),[](const auto &a,const auto &b){
            return std::tuple{a.producer.index,a.producer.generation,a.receipt}<std::tuple{b.producer.index,b.producer.generation,b.receipt};
        });
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept
    {
        if(candidates_.empty()) return;
        const auto positions=chunk.Get<Position>();auto points=chunk.Get<RallyPoint>();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            const auto entity=chunk.Entities()[row];
            auto it=std::lower_bound(candidates_.begin(),candidates_.end(),Identity(entity),
                [](const Candidate &candidate,const auto &identity){return Identity(candidate.producer)<identity;});
            for(;it!=candidates_.end()&&it->producer==entity;++it)
            {
                if(it->destination==Invalid) {points[row]={};batch_.Record(it->receipt,RallyAcceptance::Cleared);}
                else if(fields_.Next(it->field,positions[row].cell)==Invalid) batch_.Record(it->receipt,RallyAcceptance::Unreachable);
                else {points[row].destination=it->destination;batch_.Record(it->receipt,RallyAcceptance::Accepted);}
            }
        }
    }
    void AfterChunks(Query &,ecs::SystemContext &) noexcept {batch_.Publish();}
private:
    const engine::gameplay::navigation::NavigationGrid &grid_;
    engine::jobs::JobSystem &jobs_;
    engine::gameplay::navigation::FlowFields &fields_;
    RallyBatch &batch_;
    const std::size_t capacity_;
    std::vector<Candidate> candidates_;
    std::vector<Cell> goals_;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::production::rally::RallySystem>
{
    static constexpr std::string_view StableName="games.generalszh.production.rally";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    // Root supplies Health/Sell -> Rally -> Production edges for its composition.
    using Before=SystemTypeList<>;using After=SystemTypeList<>;
};
}
