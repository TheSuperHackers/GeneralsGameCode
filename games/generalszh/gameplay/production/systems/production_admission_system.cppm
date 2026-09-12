module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.systems.production_admission_system;
export import games.generalszh.gameplay.production.algorithms.prerequisite_presence;
export import games.generalszh.gameplay.production.boundary.production_operations;

export namespace generalszh::production
{
// Joined input/refund reduction: request order decides competing debits and
// duplicate upgrades. Structural commands commit once before receipt resolution.
// The prerequisite snapshot is derived once for this joined batch; it is not a
// second gameplay system or a per-request world traversal.
class ProductionAdmissionSystem
{
public:
    using Query = ecs::Query<ecs::Write<Producer>, ecs::Write<Queue>,
        ecs::Read<Life>, ecs::Read<Position>, ecs::Write<Balance>,
        ecs::Read<BuildOrder>, ecs::Write<Member>, ecs::Read<Quantity>,
        ecs::Read<ResearchedUpgrade>, ecs::Read<production::ProducedUnit>,
        ecs::Read<construction::Structure>,
        ecs::Read<engine::gameplay::rts::unlocks::UnlockState>>;
    ProductionAdmissionSystem(ecs::World &world, const BuildCatalog &catalog, BuildBatch &batch)
        : world(world), catalog(catalog), batch(batch),
          orders(world), grants(world), producers(world), prerequisitePresence(world, batch.Capacity())
    {
        ordered.reserve(batch.Capacity()); members.reserve(batch.Capacity()); pending.reserve(batch.Capacity());
    }
    void Execute(ecs::SystemContext &context)
    {
        // Composition binds this immutable catalog to the scheduler's step.
        assert(context.Time().Step() == catalog.Step());
        if (lastTick && (context.Tick() <= *lastTick || context.Tick() - *lastTick != 1))
            throw std::invalid_argument("Production requires consecutive fixed steps");
        lastTick = context.Tick();
        auto &commands = context.Commands();
        batch.receipts.clear(); pending.clear(); CollectOrders(orders, ordered, batch.Capacity());
        std::size_t live = ordered.size();
        producers.ForEachChunk([](auto chunk) {
            auto actors = chunk.template Get<Producer>(); const auto life = chunk.template Get<Life>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                if (!life[row].alive) actors[row].active = false;
        });
        for (const auto entry : ordered)
        {
            const auto &order = *world.Get<BuildOrder>(entry);
            const auto *actor = world.Get<Producer>(order.producer);
            const auto *life = world.Get<Life>(order.producer);
            if (!actor || !actor->active || !life || !life->alive)
            {
                Refund(world, order); MarkUnlinked(world, entry); commands.Destroy(entry); --live;
            }
        }
        if (live != ordered.size()) CompactMembers(world, orders, members, batch.Capacity());
        if (!batch.inputs.empty() && catalog.HasPrerequisites()) prerequisitePresence.Rebuild();
        const auto input = batch.ConsumeInputs();
        for (const auto &request : input)
        {
            const auto status = Quote(world, catalog, orders, grants, request.producer, request.definition,
                pending, prerequisitePresence);
            const auto receipt = batch.receipts.size();
            batch.receipts.push_back({status});
            if (status != BuildAcceptance::Accepted) continue;
            if (live == batch.Capacity()) throw std::length_error("Modern production order capacity exhausted");
            const auto index = catalog.DefinitionIndex(request.definition);
            const auto order = MakeOrder(world, catalog, request.producer, index);
            auto &queue = *world.Get<Queue>(request.producer);
            const auto created = commands.Create();
            commands.Add<BuildOrder>(created, order);
            commands.Add<Work>(created, catalog.Requirement(index));
            commands.Add<Enabled>(created); commands.Add<Ready>(created);
            commands.Add<Quantity>(created, Quantity{catalog.Definitions()[index].quantity, 0});
            commands.Add<Member>(created, Member{request.producer, queue.count});
            commands.Add<BuildAdmission>(created, BuildAdmission{receipt});
            ++queue.count; ++live;
            world.Get<Balance>(order.account)->quantity -= order.paid;
            pending.push_back(order);
        }

    }
private:
    ecs::World &world;
    const BuildCatalog &catalog;
    BuildBatch &batch;
    Orders orders;
    Grants grants;
    ecs::Query<ecs::Write<Producer>, ecs::Read<Queue>, ecs::Read<Life>> producers;
    PrerequisitePresence prerequisitePresence;
    std::vector<ecs::Entity> ordered;
    std::vector<MemberPosition> members;
    std::vector<BuildOrder> pending;
    std::optional<std::uint64_t> lastTick;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::production::ProductionAdmissionSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.production.admission.prepare";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    static constexpr bool Batch = true;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
