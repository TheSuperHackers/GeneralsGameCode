#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>
import games.generalszh.gameplay.construction.setup.construction_registration;
import engine.gameplay.navigation.systems.movement_system;
import engine.ecs.scheduler.scheduler;
namespace c=generalszh::construction;
namespace n=engine::gameplay::navigation;
namespace h=engine::gameplay::combat;
namespace p=engine::gameplay::rts::production;
using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
using Balance=engine::gameplay::rts::economy::ResourceBalance;
namespace {
struct Host
{
    std::vector<std::uint8_t> cells=std::vector<std::uint8_t>(512,1);
    n::NavigationGrid grid{512,1,cells};
    ecs::World world;
    engine::jobs::JobSystem jobs;
    std::array<c::BuildingDefinition,2> definitions{{
        {1,10,std::chrono::seconds(2),100,9,5},
        {2,10,std::chrono::seconds(2),100,0,0}}};
    engine::time::FixedStep step{1};
    c::BuildingCatalog catalog{definitions,step};
    c::ConstructionRequests requests{1024};
    c::ConstructionReceipts receipts{1024};
    engine::gameplay::rts::construction::BuilderFrame frame{1024};
    c::ConstructionPlacementSystem placement;
    c::BuilderTaskSystem builders{world,receipts};
    c::ConstructionSystem construction{world,frame};
    n::FlowFields fields{grid,{512,512}};
    n::MovementSystem movement{world,jobs,grid,fields,1024};
    h::HealthSystem health;
    ecs::SystemRegistry registry;
    std::unique_ptr<ecs::Scheduler> scheduler;
    ecs::Entity account;
    explicit Host(std::uint32_t workers):jobs(engine::jobs::JobSystemConfig{workers}),
        placement(world,grid,catalog,requests,receipts,1024)
    {
        c::RegisterConstructionComponents(world);
        n::RegisterMovementComponents(world);
        world.RegisterComponent<generalszh::production::ProducedUnit>();
        world.FinalizeComponents(); placement.Configure(); builders.Configure(); construction.Configure();
        c::RegisterConstructionSystems(registry,placement,builders,construction);
        registry.Register(movement);
        registry.Register(health);
        registry.OrderBefore<c::BuilderTaskSystem,n::MovementSystem>();
        registry.OrderBefore<n::MovementSystem,c::ConstructionSystem>();
        registry.Finalize(world.Components()); scheduler=std::make_unique<ecs::Scheduler>(world,registry,engine::jobs::JobSystemConfig{workers});
        scheduler->Finalize(step); account=world.Create<Balance>(); world.Get<Balance>(account)->quantity=10000;
    }
    ecs::Entity Builder(n::Cell cell=0)
    {
        const auto entity=world.Create<c::Builder,h::LifeState,h::Health,h::PendingDamage,h::DamageResult,
            n::GridPosition,n::MoveGoal,n::MoveCredit,n::MoveSpeed,n::MoveField,n::MoveEnabled,n::MoveResult>();
        *world.Get<c::Builder>(entity)={account}; world.Get<h::Health>(entity)->current=10;
        world.Get<n::GridPosition>(entity)->cell=cell;
        return entity;
    }
    void Tick(std::uint64_t tick) { scheduler->Execute({tick,step}); }
};
}
