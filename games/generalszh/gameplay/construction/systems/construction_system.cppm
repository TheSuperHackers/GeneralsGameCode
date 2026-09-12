module;
#include <algorithm>
#include <cassert>
#include <memory>
#include <string_view>
export module games.generalszh.gameplay.construction.systems.construction_system;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import games.generalszh.gameplay.construction.components.construction_health;
export import games.generalszh.gameplay.construction.algorithms.completed_structure;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.match.systems.defeat_system;
export import engine.gameplay.rts.power.systems.power_projection_system;
export import engine.gameplay.rts.construction.observations.builder_frame;
export import games.generalszh.gameplay.harvesting.revenue.harvest_revenue;
export import games.generalszh.gameplay.selling.components.sale_state;
export import engine.gameplay.navigation.components.movement;
export import games.generalszh.gameplay.production.rally.components.rally_point;
export namespace generalszh::construction
{
struct ConstructionSystem
{
    using Work=engine::gameplay::rts::production::BuildWork;
    using Enabled=engine::gameplay::rts::production::BuildEnabled;
    using Ready=engine::gameplay::rts::production::BuildReady;
    using Health=engine::gameplay::combat::Health;
    using Capacity=engine::gameplay::combat::HealthCapacity;
    using Life=engine::gameplay::combat::LifeState;
    using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Site=engine::gameplay::rts::construction::ConstructionSite;
    using Position=engine::gameplay::navigation::GridPosition;
    using Frame=engine::gameplay::rts::construction::BuilderFrame;
    using Query=ecs::Query<ecs::Read<Site>,ecs::Read<Position>,ecs::Write<Structure>,ecs::Write<Work>,ecs::Write<Enabled>,ecs::Write<Ready>,
        ecs::Read<Life>,ecs::Write<Health>,ecs::Read<Capacity>,ecs::Write<ConstructionHealth>,
        ecs::Optional<engine::gameplay::rts::visibility::VisibilityObserver>,
        ecs::Optional<engine::gameplay::rts::visibility::VisibilityEligibility>>;
    using AuxiliaryAccess=ecs::Query<ecs::Write<Assignment>,ecs::Write<Goal>,ecs::Write<Credit>,ecs::OptionalWrite<Range>,ecs::Read<Position>,ecs::Read<Life>>;
    ConstructionSystem(ecs::World &world,Frame &frame):world(world),frame(frame) {}
    void Configure(const engine::gameplay::rts::repair::RepairDefinitions *repair=nullptr)
    {
        repairDefinitions=repair;
        assignments=std::make_unique<Assignments>(world);
        builders=std::make_unique<Builders>(world);
    }
    void BeforeChunks(Query &,ecs::SystemContext &)
    {
        frame.Clear();
        builders->ForEachChunk([&](auto chunk) {
            const auto assigned=chunk.template Get<Assignment>();
            const auto positions=chunk.template Get<Position>(); const auto lives=chunk.template Get<Life>();
            for(std::size_t i=0;i<chunk.Count();++i)
                frame.Append({chunk.Entities()[i],assigned[i].site,positions[i].cell,lives[i].alive});
        });
        frame.Finalize();
    }
    void AfterChunks(Query &,ecs::SystemContext &)
    {
        // Chunk writes to complete/life are joined; structural removals are still deferred.
        // This is the cross-entity assignment release belonging to site completion.
        assignments->ForEachChunk([&](auto chunk) {
            auto assigned=chunk.template Get<Assignment>(); auto goals=chunk.template Get<Goal>();
            auto credits=chunk.template Get<Credit>(); auto ranges=chunk.template Get<Range>();
            for(std::size_t i=0;i<chunk.Count();++i)
            {
                if(!assigned[i].site.IsValid()) continue;
                const auto *structure=world.Get<Structure>(assigned[i].site);
                const auto *life=world.Get<Life>(assigned[i].site);
                if(!structure || structure->complete || !life || !life->alive)
                {
                    assigned[i]={};
                    if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[i],credits[i]);
                    goals[i]={}; credits[i]={};
                }
            }
        });
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        const auto sites=chunk.Get<Site>(); const auto positions=chunk.Get<Position>();
        auto structures=chunk.Get<Structure>(); auto work=chunk.Get<Work>();
        auto enabled=chunk.Get<Enabled>(); auto ready=chunk.Get<Ready>();
        const auto life=chunk.Get<Life>(); auto health=chunk.Get<Health>();
        const auto maximum=chunk.Get<Capacity>(); auto fractions=chunk.Get<ConstructionHealth>();
        const auto observers=chunk.Get<engine::gameplay::rts::visibility::VisibilityObserver>();
        const auto eligibilities=chunk.Get<engine::gameplay::rts::visibility::VisibilityEligibility>();
        for(std::size_t i=0;i<chunk.Count();++i)
        {
            const auto *builder=frame.Find(sites[i].builder);
            enabled[i].value=!structures[i].complete && life[i].alive && builder && builder->alive
                && builder->site==chunk.Entities()[i] && builder->cell==positions[i].cell;
            engine::gameplay::rts::production::AdvanceBuild(work[i],enabled[i],ready[i]);
            if(!enabled[i].value) continue;
            const auto required=work[i].required;
            auto gain=required ? maximum[i].maximum/required : maximum[i].maximum;
            const auto remainder=required ? maximum[i].maximum%required : 0;
            if(remainder && fractions[i].remainder>=required-remainder)
            { ++gain; fractions[i].remainder-=required-remainder; }
            else fractions[i].remainder+=remainder;
            health[i].current+=std::min(gain,maximum[i].maximum-health[i].current);
            if(!ready[i].value) continue;
            structures[i].complete=true;
            const auto entity=chunk.Entities()[i];
            EnrollCompletedStructureCapabilities(context.Commands(), entity, structures[i], repairDefinitions, context.Time(),
                !observers.empty(), !eligibilities.empty());
            context.Commands().Remove<Work>(entity);
            context.Commands().Remove<Enabled>(entity);
            context.Commands().Remove<Ready>(entity);
            context.Commands().Remove<ConstructionHealth>(entity);
            context.Commands().Remove<engine::gameplay::rts::construction::ConstructionSite>(entity);
        }
    }
private:
    ecs::World &world;
    using Assignments=ecs::Query<ecs::Write<Assignment>,ecs::Write<Goal>,ecs::Write<Credit>,ecs::OptionalWrite<Range>>;
    using Builders=ecs::Query<ecs::Read<Assignment>,ecs::Read<Position>,ecs::Read<Life>>;
    Frame &frame;
    const engine::gameplay::rts::repair::RepairDefinitions *repairDefinitions{};
    std::unique_ptr<Assignments> assignments;
    std::unique_ptr<Builders> builders;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::construction::ConstructionSystem>
{
    static constexpr std::string_view StableName="games.generalszh.construction.construct";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
