#pragma once
#include <array>
#include <chrono>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
import games.generalszh.gameplay.production.registration.production_registration;
import engine.ecs.scheduler.scheduler;
using namespace generalszh;
using namespace generalszh::production;
using namespace std::chrono_literals;
namespace {
const std::array definitions{BuildDefinition{1, 100, 100ms, EntryKind::Unit, 2},
    BuildDefinition{2, 50, 50ms, EntryKind::Upgrade, 1}};
// Test composition only. Every gameplay operation is the actual production code.
class ProductionTestHost
{
public:
    ProductionTestHost(ecs::World &world, engine::jobs::JobSystem &jobs,
        std::span<const BuildDefinition> definitions, std::size_t capacity = 65536)
        : world(world), definitions(definitions), batch(capacity), scheduler(world, registry, jobs) {}
    static void RegisterComponents(ecs::World &world) { RegisterProductionComponents(world); }
    void Configure(engine::time::FixedStep step)
    {
        if (state != ProductionBoundaryState::Configuring || catalog)
            throw std::logic_error("Production test composition startup is one-shot");
        try {
            catalog.emplace(world, definitions, step);
            admission.emplace(world, *catalog, batch);
            production.emplace(world, batch, powerFrame);
            exits.emplace(world,batch);
            state = ProductionBoundaryState::Ready;
        } catch (...) { state = ProductionBoundaryState::Failed; throw; }
    }
    void RegisterSystems(ecs::SystemRegistry &registry)
    { RegisterProductionSystems(registry, *admission, *production,*exits); }
    void Finalize(engine::time::FixedStep step)
    {
        try {
            Configure(step); RegisterSystems(registry);
            registry.Finalize(world.Components()); scheduler.Finalize(step);
        } catch (...) { state = ProductionBoundaryState::Failed; throw; }
    }
    void Execute(engine::time::SimulationTime time)
    {
        RequireProductionBoundary(world, state);
        try { scheduler.Execute(time); }
        catch (...) { state = ProductionBoundaryState::Failed; throw; }
    }
    ecs::Entity CreateProducer(ecs::Entity account, std::uint32_t limit = 9,
        std::uint64_t health = 1000, engine::gameplay::navigation::Cell spawn = 0)
    { return generalszh::production::CreateProducer(world, state, batch.Capacity(), account, limit, health, spawn); }
    BuildReceipt Enqueue(ecs::Entity actor, std::uint32_t key)
    {
        RequireProductionBoundary(world, state);
        return EnqueueBuild(world, state, *catalog, batch.Capacity(), actor, key);
    }
    bool Cancel(ecs::Entity actor, ecs::Entity order)
    { return CancelBuild(world, state, batch.Capacity(), actor, order); }
    void StopProducer(ecs::Entity actor)
    { generalszh::production::StopProducer(world, state, batch.Capacity(), actor); }
    void StageInputs(std::span<const BuildInput> input) { batch.StageInputs(world, state, input); }
    std::span<const BuildReceipt> Receipts() const { return batch.Receipts(); }
private:
    ecs::World &world;
    std::span<const BuildDefinition> definitions;
    BuildBatch batch;
    power::PowerFrame powerFrame;
    std::optional<BuildCatalog> catalog;
    std::optional<ProductionAdmissionSystem> admission;
    std::optional<ProductionSystem> production;
    std::optional<ProductionExitSystem> exits;
    ProductionBoundaryState state{ProductionBoundaryState::Configuring};
    ecs::SystemRegistry registry;
    ecs::Scheduler scheduler;
};
}
