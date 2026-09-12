module;
#include <cstdint>
export module games.generalszh.gameplay.combat.targeting.launch_snapshot;
export import games.generalszh.gameplay.combat.targeting.target_frame;
export import games.generalszh.gameplay.combat.targeting.target_index;
export import engine.gameplay.spatial.grid.point_grid;
export namespace generalszh::combat
{
// Non-owning adapter over the post-movement TargetFrame. WeaponSystemT takes
// this by value, but the frame itself remains owned and published by TargetIndex.
struct LaunchSnapshotView
{
    static constexpr bool Spatial = true;
    // This is the source join whose publication produces TargetFrame. The
    // weapon system exposes it as auxiliary access so movement -> targeting ->
    // weapon is explicit without introducing a projectile/acquisition cycle.
    using Access = TargetIndex::Access;
    explicit LaunchSnapshotView(const TargetFrame &frame) noexcept : frame(&frame) {}
    bool TryGet(ecs::Entity entity, engine::gameplay::spatial::SpatialPoint &point) const noexcept
    {
        if (!frame->Contains(entity)) return false;
        const auto position = frame->Position(entity), width = frame->Width();
        point = {entity,frame->Owner(entity),position % width,position / width,1};
        return true;
    }
private:
    const TargetFrame *frame;
};
}
