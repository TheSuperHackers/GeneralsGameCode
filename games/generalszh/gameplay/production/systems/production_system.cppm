module;
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.production.systems.production_system;
export import games.generalszh.gameplay.production.boundary.production_operations;
export import games.generalszh.gameplay.production.systems.build_completion_system;
export import games.generalszh.gameplay.production.power.build_power_rate;
export import games.generalszh.gameplay.production.rally.components.rally_point;
export namespace generalszh::production
{
// Committed build work. Factory exit coordination consumes readiness in a later
// wave; this system does not decide whether a completed unit can leave a door.
// The scheduler owns its chunk join and commit;
// this system owns neither child systems nor another execution plan.
class ProductionSystem
{
public:
    using Query = ecs::Query<ecs::Read<BuildOrder>, ecs::Write<Work>, ecs::Write<Enabled>,
        ecs::Write<Ready>, ecs::Read<Member>, ecs::Optional<BuildAdmission>>;
    // Hooks read actors/accounts on other archetypes and reduce into producer queues.
    // AuxiliaryAccess contributes metadata without changing order chunk selection.
    using AuxiliaryAccess = ecs::Query<ecs::Read<Producer>, ecs::Read<Life>,
        ecs::Read<Balance>>;
    ProductionSystem(ecs::World &world, BuildBatch &batch, const power::PowerFrame &frame)
        : world(world), batch(batch), frame(frame) {}
    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        std::size_t count = 0;
        // Same prepared traversal used by scheduler logical ChunkOrder. The
        // scratch is immutable during jobs, bounded by admitted-order capacity.
        query.ForEachPreparedChunk([&](auto chunk) {
            const auto values = chunk.template Get<BuildOrder>();
            const auto membership = chunk.template Get<Member>();
            auto enabled = chunk.template Get<Enabled>();
            if (chunk.Count() > batch.Capacity() - count)
                throw std::length_error("Production order capacity exhausted");
            count += chunk.Count();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto &order = values[row];
                const auto *actor = world.Get<Producer>(order.producer);
                const auto *life = world.Get<Life>(order.producer);
                enabled[row].value = membership[row].queue.IsValid() && membership[row].position == 0
                    && actor && actor->active && life && life->alive && world.Get<Balance>(order.account);
            }
        });
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto values = chunk.Get<BuildOrder>();
        const auto markers = chunk.Get<BuildAdmission>();
        auto work = chunk.Get<Work>(); auto enabled = chunk.Get<Enabled>();
        auto ready = chunk.Get<Ready>();
        auto &commands = context.Commands();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto entry = chunk.Entities()[row];
            if (!markers.empty())
            {
                assert(markers[row].receipt < batch.receipts.size());
                // Admission gives every accepted order a distinct preallocated slot.
                batch.receipts[markers[row].receipt].order = entry;
                commands.Remove<BuildAdmission>(entry);
            }
            // The root orders PowerSystem publication before this consumer.
            // No ECS lookup or child-system dispatch is needed for power policy.
            if (enabled[row].value)
                ApplyBuildPowerRate(work[row], true, values[row].kind, frame.Find(values[row].account));
            engine::gameplay::rts::production::AdvanceBuild(work[row], enabled[row], ready[row]);
        }
    }
private:
    ecs::World &world;
    BuildBatch &batch;
    const power::PowerFrame &frame;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::production::ProductionSystem>
{
    // Retain the identity of the game-specific output behaviour.
    static constexpr std::string_view StableName = "games.generalszh.production.build_completion";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
