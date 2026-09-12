module;
#include <algorithm>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module engine.gameplay.rts.harvesting.systems.harvest_system;
export import engine.events.storage.event_batch;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.harvesting.snapshots.harvest_snapshots;
export namespace engine::gameplay::rts::harvesting {
class HarvestSystem {
    using Sources=ecs::Query<ecs::Read<SupplySource>,ecs::Read<GridPosition>,ecs::Read<Health>,ecs::Read<LifeState>>;
    using Dropoffs=ecs::Query<ecs::Read<SupplyDropoff>,ecs::Read<GridPosition>,ecs::Read<Health>,ecs::Read<LifeState>>;
public:
    using Query=ecs::Query<ecs::Write<HarvestState>,ecs::Write<HarvestCargo>,ecs::Read<HarvestPolicy>,ecs::Write<HarvestIntent>,ecs::Write<HarvestDelivery>,ecs::Read<GridPosition>,ecs::Write<MoveGoal>,ecs::Write<MoveCredit>,ecs::OptionalWrite<MoveRangeGoal>,ecs::Read<Health>,ecs::Read<LifeState>>;
    using AuxiliaryAccess=ecs::Query<ecs::Write<SupplySource>,ecs::Read<SupplyDropoff>>;
    using LateCancellationBatch=engine::events::PublishedBatch<ecs::Entity>;
    HarvestSystem(ecs::World &world,const NavigationGrid &grid,HarvestSnapshots &snapshots,
        const LateCancellationBatch *lateCancellations=nullptr):world(world),grid(grid),snapshots(snapshots),
        lateCancellations(lateCancellations) { pending.reserve(snapshots.capacity); }
    void Configure(engine::time::FixedStep value) {
        if(step) throw std::logic_error("Harvest configuration is one-shot");
        sources=std::make_unique<Sources>(world); dropoffs=std::make_unique<Dropoffs>(world); step=value;
    }
    void SetInputs(std::span<const HarvestInput> value, std::span<const ecs::Entity> cancelled={}) {
        if(value.size()>snapshots.capacity) throw std::length_error("Harvest input capacity exhausted");
        if(cancelled.size()>snapshots.capacity) throw std::length_error("Harvest cancellation capacity exhausted");
        inputs=value; cancellations=cancelled;
    }
    void BeforeChunks(Query &,ecs::SystemContext &context) {
        if(!step||*step!=context.Time().Step()) throw std::logic_error("Harvest step mismatch");
        snapshots.sources.clear(); snapshots.dropoffs.clear();
        sources->ForEachChunk([&](auto chunk){
            auto s=chunk.template Get<SupplySource>(); auto p=chunk.template Get<GridPosition>(); auto h=chunk.template Get<Health>(); auto l=chunk.template Get<LifeState>();
            for(std::size_t i=0;i<chunk.Count();++i) AppendHarvestDock(snapshots.sources,{chunk.Entities()[i],p[i].cell,s[i].available&&s[i].boxes&&h[i].current&&l[i].alive&&grid.Walkable(p[i].cell)},snapshots.capacity);
        });
        dropoffs->ForEachChunk([&](auto chunk){
            auto s=chunk.template Get<SupplyDropoff>(); auto p=chunk.template Get<GridPosition>(); auto h=chunk.template Get<Health>(); auto l=chunk.template Get<LifeState>();
            for(std::size_t i=0;i<chunk.Count();++i) AppendHarvestDock(snapshots.dropoffs,{chunk.Entities()[i],p[i].cell,s[i].available&&h[i].current&&l[i].alive&&grid.Walkable(p[i].cell)},snapshots.capacity);
        });
        const auto less=[](const auto &a,const auto &b){return HarvestEntityLess(a.entity,b.entity);};
        std::sort(snapshots.sources.begin(),snapshots.sources.end(),less); std::sort(snapshots.dropoffs.begin(),snapshots.dropoffs.end(),less);
        for(auto input:inputs) {
            auto *state=world.Get<HarvestState>(input.actor); auto *cargo=world.Get<HarvestCargo>(input.actor);
            auto *goal=world.Get<MoveGoal>(input.actor); auto *credit=world.Get<MoveCredit>(input.actor);
            auto *range=world.Get<MoveRangeGoal>(input.actor);
            const auto *health=world.Get<Health>(input.actor); const auto *life=world.Get<LifeState>(input.actor);
            if(!state||!cargo||!goal||!credit||!health||!life||!health->current||!life->alive) continue;
            *state=input.cancel?HarvestState{}:HarvestState{input.source,input.dropoff,cargo->boxes?HarvestPhase::ToDropoff:HarvestPhase::ToSource,0};
            if(range) ClearMoveRangeGoal(*range,*credit);
            goal->cell=InvalidCell; credit->remainder=0;
        }
        for(auto actor:cancellations) {
            if(auto *state=world.Get<HarvestState>(actor)) {
                CancelHarvest(*state);
                auto *goal=world.Get<MoveGoal>(actor); auto *credit=world.Get<MoveCredit>(actor);
                auto *range=world.Get<MoveRangeGoal>(actor);
                if(goal) goal->cell=InvalidCell;
                if(credit) credit->remainder=0;
                if(range&&credit) ClearMoveRangeGoal(*range,*credit);
            }
        }
        // ManualRepairSystem publishes after its chunk work.  Resolve this
        // late-bound read here, after publication and before harvest chunks,
        // so a same-tick repair request cancels harvest before it can pick up.
        // The batch is an injected control publication, not a generic event
        // bus or an authoritative second harvest state.
        if(lateCancellations) {
            if(!lateCancellations->IsPublished()) throw std::logic_error("Harvest late cancellation batch was not published");
            for(const auto actor:lateCancellations->Values()) {
                if(auto *state=world.Get<HarvestState>(actor)) {
                    CancelHarvest(*state);
                    auto *goal=world.Get<MoveGoal>(actor); auto *credit=world.Get<MoveCredit>(actor);
                    auto *range=world.Get<MoveRangeGoal>(actor);
                    if(goal) goal->cell=InvalidCell;
                    if(credit) credit->remainder=0;
                    if(range&&credit) ClearMoveRangeGoal(*range,*credit);
                }
            }
        }
        inputs={}; cancellations={};
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept {
        auto states=chunk.Get<HarvestState>(); auto cargo=chunk.Get<HarvestCargo>(); auto policies=chunk.Get<HarvestPolicy>(); auto intents=chunk.Get<HarvestIntent>(); auto deliveries=chunk.Get<HarvestDelivery>();
        auto positions=chunk.Get<GridPosition>(); auto goals=chunk.Get<MoveGoal>(); auto credits=chunk.Get<MoveCredit>(); auto ranges=chunk.Get<MoveRangeGoal>(); auto health=chunk.Get<Health>(); auto life=chunk.Get<LifeState>();
        for(std::size_t i=0;i<chunk.Count();++i) {
            intents[i]={}; deliveries[i]={}; auto &s=states[i]; const auto &policy=policies[i];
            const auto stop=[&]{if(!ranges.empty()) ClearMoveRangeGoal(ranges[i],credits[i]); goals[i].cell=InvalidCell; credits[i].remainder=0;};
            if(!health[i].current||!life[i].alive) {s={}; cargo[i]={}; stop(); continue;}
            if(s.phase==HarvestPhase::Idle||s.phase==HarvestPhase::NeedsInput) continue;
            bool returning=s.phase==HarvestPhase::ToDropoff||s.phase==HarvestPhase::Unload;
            const auto *dock=FindHarvestDock(returning?snapshots.dropoffs:snapshots.sources,returning?s.dropoff:s.source);
            if(!dock||!dock->available) {
                s.remaining=0; stop();
                s.phase=!returning&&cargo[i].boxes?HarvestPhase::ToDropoff:HarvestPhase::NeedsInput;
                continue;
            }
            if(positions[i].cell!=dock->cell) {
                s.phase=returning?HarvestPhase::ToDropoff:HarvestPhase::ToSource; s.remaining=0;
                if(!ranges.empty()) ClearMoveRangeGoal(ranges[i],credits[i]);
                if(goals[i].cell!=dock->cell) credits[i].remainder=0;
                goals[i].cell=dock->cell; continue;
            }
            stop();
            if(s.phase==HarvestPhase::ToSource||s.phase==HarvestPhase::ToDropoff) {
                s.phase=returning?HarvestPhase::Unload:HarvestPhase::Pickup;
                s.remaining=returning?policy.unloadTicks:policy.pickupTicks;
            }
            if(s.remaining) --s.remaining;
            if(s.remaining) continue;
            if(returning) {deliveries[i]={s.dropoff,cargo[i].boxes}; cargo[i].boxes=0; s.phase=HarvestPhase::ToSource;}
            else intents[i].pickup=true;
        }
    }
    void AfterChunks(Query &query,ecs::SystemContext &) {
        pending.clear();
        query.ForEachChunk([&](auto chunk){auto intents=chunk.template Get<HarvestIntent>(); for(std::size_t i=0;i<chunk.Count();++i) if(intents[i].pickup) {
            if(pending.size()==snapshots.capacity) throw std::length_error("Harvest settlement capacity exhausted");
            pending.push_back(chunk.Entities()[i]);
        }});
        std::sort(pending.begin(),pending.end(),HarvestEntityLess);
        for(auto entity:pending) {
            auto &s=*world.Get<HarvestState>(entity); auto &cargo=*world.Get<HarvestCargo>(entity); const auto &policy=*world.Get<HarvestPolicy>(entity);
            world.Get<HarvestIntent>(entity)->pickup=false;
            const auto *health=world.Get<Health>(entity); const auto *life=world.Get<LifeState>(entity);
            if(!health->current||!life->alive||s.phase!=HarvestPhase::Pickup) continue;
            auto *source=world.Get<SupplySource>(s.source); const auto *sh=world.Get<Health>(s.source); const auto *sl=world.Get<LifeState>(s.source);
            if(source&&source->available&&sh&&sl&&sh->current&&sl->alive&&source->boxes&&cargo.boxes<policy.capacity) {--source->boxes; ++cargo.boxes;}
            if(cargo.boxes>=policy.capacity||!source||!source->available||!source->boxes) s.phase=cargo.boxes?HarvestPhase::ToDropoff:HarvestPhase::NeedsInput;
            s.remaining=policy.pickupTicks;
        }
    }
private:
    ecs::World &world; const NavigationGrid &grid; HarvestSnapshots &snapshots;
    std::unique_ptr<Sources> sources; std::unique_ptr<Dropoffs> dropoffs;
    std::optional<engine::time::FixedStep> step; std::span<const HarvestInput> inputs;
    std::span<const ecs::Entity> cancellations;
    const LateCancellationBatch *lateCancellations{};
    std::vector<ecs::Entity> pending;
};
}
export namespace ecs {
template<> struct SystemTraits<engine::gameplay::rts::harvesting::HarvestSystem> {
    static constexpr std::string_view StableName="engine.gameplay.rts.harvesting.HarvestSystem";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    static constexpr bool Batch=false;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
