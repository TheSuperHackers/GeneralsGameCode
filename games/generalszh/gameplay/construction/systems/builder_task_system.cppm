module;
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.construction.systems.builder_task_system;
export import games.generalszh.gameplay.construction.inputs.construction_batches;
export import engine.gameplay.rts.construction.algorithms.builder_routing;
export namespace generalszh::construction {
class BuilderTaskSystem {
    using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
    using Site=engine::gameplay::rts::construction::ConstructionSite;
    using Position=engine::gameplay::navigation::GridPosition;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
public:
    using Query=ecs::Query<ecs::Write<Assignment>,ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Write<engine::gameplay::navigation::MoveGoal>,ecs::Write<engine::gameplay::navigation::MoveCredit>,
        ecs::OptionalWrite<Range>>;
    using AuxiliaryAccess=ecs::Query<ecs::Read<ConstructionAdmission>,ecs::Read<Site>,ecs::Read<Position>>;
    BuilderTaskSystem(ecs::World &world,ConstructionReceipts &receipts):world(world),receipts(receipts) {}
    void Configure() { admissions=std::make_unique<Admissions>(world); }
    void BeforeChunks(Query &,ecs::SystemContext &context)
    {
        admissions->ForEachChunk([&](auto chunk) {
            const auto markers=chunk.template Get<ConstructionAdmission>(); const auto sites=chunk.template Get<Site>();
            const auto positions=chunk.template Get<Position>();
            for(std::size_t i=0;i<chunk.Count();++i)
            {
                receipts.Resolve(markers[i].receipt,chunk.Entities()[i]);
                *world.Get<Assignment>(sites[i].builder)={chunk.Entities()[i],positions[i].cell};
                context.Commands().Remove<ConstructionAdmission>(chunk.Entities()[i]);
            }
        });
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept
    {
        auto assignments=chunk.Get<Assignment>();
        const auto lives=chunk.Get<engine::gameplay::combat::LifeState>();
        auto goals=chunk.Get<engine::gameplay::navigation::MoveGoal>();
        auto credits=chunk.Get<engine::gameplay::navigation::MoveCredit>();
        auto ranges=chunk.Get<Range>();
        for(std::size_t i=0;i<chunk.Count();++i)
            engine::gameplay::rts::construction::UpdateBuilderRoute(assignments[i],lives[i],goals[i],credits[i],
                ranges.empty()?nullptr:&ranges[i]);
    }
private:
    using Admissions=ecs::Query<ecs::Read<ConstructionAdmission>,ecs::Read<Site>,ecs::Read<Position>>;
    ecs::World &world;
    ConstructionReceipts &receipts;
    std::unique_ptr<Admissions> admissions;
};
}
export namespace ecs {
template<> struct SystemTraits<generalszh::construction::BuilderTaskSystem> {
 static constexpr std::string_view StableName="games.generalszh.construction.builder_task";
 static constexpr SystemPhase Phase=SystemPhase::Simulation; static constexpr bool Batch=false;
 using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
