module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.combat.systems.targeting_system;
export import games.generalszh.gameplay.combat.targeting.target_frame;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import engine.gameplay.combat.systems.weapon_system;
export import engine.gameplay.spatial.algorithms.grid_radius;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;
export namespace generalszh::combat
{
struct TargetingSystem
{
    TargetingSystem(const TargetFrame &frame, const engine::gameplay::navigation::NavigationGrid &grid) : frame(frame), grid(grid) {}
    using AuxiliaryAccess = ecs::Query<ecs::Read<engine::gameplay::rts::visibility::VisibilityCellIndex>,
        ecs::Read<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>>;
    using Query = ecs::Query<ecs::Read<engine::gameplay::navigation::GridPosition>, ecs::Read<engine::gameplay::combat::WeaponTarget>,
        ecs::Read<engine::gameplay::combat::WeaponDefinition>, ecs::Write<engine::gameplay::combat::WeaponContact>,
        ecs::Optional<capture::CaptureActorState>,ecs::Optional<production::ProducedUnit>,
        ecs::OptionalWrite<engine::gameplay::combat::DamageEmitter>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        using namespace engine::gameplay::combat;
        const auto positions = chunk.Get<engine::gameplay::navigation::GridPosition>(); const auto targets = chunk.Get<WeaponTarget>();
        const auto definitions = chunk.Get<WeaponDefinition>(); auto contacts = chunk.Get<WeaponContact>(); const auto width = grid.Width();
        const auto captures=chunk.Get<capture::CaptureActorState>();
        const auto units=chunk.Get<production::ProducedUnit>();auto emitters=chunk.Get<DamageEmitter>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto actor = chunk.Entities()[row]; contacts[row] = {};
            // Derived emission provenance, not an independently writable owner.
            // Weapon jobs copy it into the projectile at launch so later actor
            // deletion cannot erase who authored the hit.
            if(!emitters.empty()) emitters[row]=units.empty()?DamageEmitter{}:DamageEmitter{units[row].account,units[row].definition};
            if(!captures.empty() && capture::IsCaptureBusy(captures[row])) continue;
            if (targets[row].IsPosition())
            {
                const auto aim=targets[row].position;
                const auto height=grid.Count()/width;
                if (positions[row].cell >= grid.Count() || aim.x >= width || aim.y >= height) continue;
                const auto from=positions[row].cell;
                const auto point=engine::gameplay::spatial::GridPoint{from%width,from/width};
                contacts[row]={true,engine::gameplay::spatial::IsWithinGridRadius(point,aim,definitions[row].rangeCells)};
                continue;
            }
            const auto target = targets[row].entity;
            if (!targets[row].IsEntity() || !frame.CanTarget(actor, target)) continue;
            const auto from = positions[row].cell, to = frame.Position(target);
            const auto fx = from % width, tx = to % width, fy = from / width, ty = to / width;
            const std::uint64_t dx = fx > tx ? fx - tx : tx - fx, dy = fy > ty ? fy - ty : ty - fy;
            const std::uint64_t range = definitions[row].rangeCells;
            contacts[row] = {true, dx * dx + dy * dy <= range * range};
        }
    }
private:
    const TargetFrame &frame;
    const engine::gameplay::navigation::NavigationGrid &grid;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::combat::TargetingSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.combat.targeting";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
