module;
#include <cassert>
#include <cstddef>
#include <cstdint>
export module games.generalszh.gameplay.combat.targeting.target_index;
export import games.generalszh.gameplay.combat.targeting.target_frame;
export import games.generalszh.gameplay.combat.acquisition.acquisition_policy;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.concealment.algorithms.detection_lease;
export import engine.gameplay.rts.visibility.algorithms.visibility_read_view;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;

export namespace generalszh::combat
{
// Shared derived spatial data, not a gameplay executor. The composition owns it;
// ordered consuming systems refresh it in their caller-side BeforeChunks hook.
// No components, child systems, scheduler or structural commands are owned here.
class TargetIndex
{
    using Position = engine::gameplay::navigation::GridPosition;
    using Health = engine::gameplay::combat::Health;
    using Life = engine::gameplay::combat::LifeState;
public:
    using Access = ecs::Query<ecs::Read<Health>, ecs::Read<Life>, ecs::Read<Position>,
        ecs::Optional<production::ProducedUnit>, ecs::Optional<production::Producer>,
        ecs::Optional<construction::Structure>,ecs::Optional<engine::gameplay::containment::PassengerMembership>,
        ecs::Optional<engine::gameplay::concealment::ConcealmentState>>;
    using VisibilityAccess = ecs::Query<ecs::Read<Health>, ecs::Read<Life>, ecs::Read<Position>,
        ecs::Optional<production::ProducedUnit>, ecs::Optional<production::Producer>,
        ecs::Optional<construction::Structure>,ecs::Optional<engine::gameplay::containment::PassengerMembership>,
        ecs::Optional<engine::gameplay::concealment::ConcealmentState>,
        ecs::Read<engine::gameplay::rts::visibility::VisibilityCellIndex>,
        ecs::Read<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>>;

    TargetIndex(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t capacity) : world(world), grid(grid), targets(world),
        frame(capacity, grid.Width(), grid.Count() / grid.Width()) {}

    TargetIndex(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t capacity, const engine::gameplay::rts::visibility::VisibilityReadView &visibility,
        const engine::gameplay::rts::visibility::VisibilityParticipantTable &participants) :
        world(world), grid(grid), targets(world),
        frame(capacity, grid.Width(), grid.Count() / grid.Width(), visibility, participants) {}

    const TargetFrame &Frame() const noexcept { return frame; }

    // The composition root selects the explicit startup policy after the
    // visibility view has been bound.  This is setup metadata, not a hot-path
    // fallback or a second visibility authority.
    void ConfigureVisibilityPolicy()
    { frame.ConfigureVisibilityPolicy(); }

    void Rebuild(std::uint64_t tick)
    {
        frame.Clear();
        targets.ForEachChunk([&](auto chunk) {
            const auto lives = chunk.template Get<Life>();
            const auto positions = chunk.template Get<Position>();
            const auto units = chunk.template Get<production::ProducedUnit>();
            const auto producers = chunk.template Get<production::Producer>();
            const auto structures = chunk.template Get<construction::Structure>();
            const auto members=chunk.template Get<engine::gameplay::containment::PassengerMembership>();
            const auto concealment=chunk.template Get<engine::gameplay::concealment::ConcealmentState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const bool contained = !members.empty() && engine::gameplay::containment::IsContained(members[row]);
                const bool garrisoned = contained && engine::gameplay::containment::IsGarrisonOwned(members[row]);
                if (!lives[row].alive || (contained && !garrisoned)) continue;
                const auto owner = !units.empty() ? units[row].account : !producers.empty() ? producers[row].account
                    : !structures.empty() ? structures[row].account : ecs::Entity{};
                if (!owner.IsValid() || !world.IsAlive(owner) || positions[row].cell >= grid.Count()) continue;
                const bool canTarget = concealment.empty() || !concealment[row].concealed ||
                    engine::gameplay::concealment::IsDetected(tick, concealment[row].detectedUntilTick);
                frame.Set(chunk.Entities()[row], positions[row].cell, owner,
                    !units.empty() ? UnitTarget : BuildingTarget, canTarget && !garrisoned);
            }
        });
        frame.Publish();
    }
private:
    ecs::World &world;
    const engine::gameplay::navigation::NavigationGrid &grid;
    Access targets;
    TargetFrame frame;
};
}
