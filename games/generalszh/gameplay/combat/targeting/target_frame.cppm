module;
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.combat.targeting.target_frame;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.spatial.grid.point_grid;
export import engine.gameplay.rts.visibility.algorithms.visibility_read_view;
export import engine.gameplay.rts.visibility.definitions.visibility_participant;
export namespace generalszh::combat
{
// Read-only targeting snapshot for a joined firing phase. Not a second live
// owner: source ECS columns remain authoritative and this frame is rebuilt.
// Columns are dense by entity index so chunk jobs never call World::Get/typeid.
class TargetFrame
{
public:
    TargetFrame(std::size_t entityIndexCapacity, std::uint32_t width, std::uint32_t height) :
        generations(entityIndexCapacity), positions(entityIndexCapacity), owners(entityIndexCapacity),
        targetable(entityIndexCapacity), sourceParticipants(entityIndexCapacity),
        ownerParticipantGenerations(entityIndexCapacity), ownerParticipants(entityIndexCapacity),
        index(width,height,entityIndexCapacity), width(width)
    { touched.reserve(entityIndexCapacity); ownerTouched.reserve(entityIndexCapacity); }
    TargetFrame(std::size_t entityIndexCapacity, std::uint32_t width, std::uint32_t height,
        const engine::gameplay::rts::visibility::VisibilityReadView &visibility,
        const engine::gameplay::rts::visibility::VisibilityParticipantTable &participants) :
        generations(entityIndexCapacity), positions(entityIndexCapacity), owners(entityIndexCapacity),
        targetable(entityIndexCapacity), sourceParticipants(entityIndexCapacity),
        ownerParticipantGenerations(entityIndexCapacity), ownerParticipants(entityIndexCapacity),
        index(width,height,entityIndexCapacity), width(width), startupUnrestricted(visibility.StartupUnrestricted()),
        visibility(&visibility), participants(&participants), visibilityPolicyConfigured(visibility.IsBound())
    { touched.reserve(entityIndexCapacity); ownerTouched.reserve(entityIndexCapacity); }
    void ConfigureVisibilityPolicy()
    {
        if (visibilityPolicyConfigured)
            throw std::logic_error("Target visibility policy was already configured");
        if (!touched.empty() || !ownerTouched.empty())
            throw std::logic_error("Target visibility policy must be configured before snapshot publication");
        if (visibility == nullptr || !visibility->IsBound())
            throw std::logic_error("Target visibility policy requires a bound startup view");
        startupUnrestricted=visibility->StartupUnrestricted();
        visibilityPolicyConfigured=true;
    }
    void Clear() noexcept
    {
        for (auto entry : touched) { generations[entry] = 0; targetable[entry] = false; sourceParticipants[entry] = {}; }
        for (auto entry : ownerTouched) { ownerParticipantGenerations[entry] = 0; ownerParticipants[entry] = {}; }
        touched.clear(); ownerTouched.clear(); index.Clear();
    }
    void Set(ecs::Entity entity, engine::gameplay::navigation::Cell position, ecs::Entity owner,
        std::uint32_t category = 1, bool canTarget = true)
    {
        if (entity.index >= generations.size()) throw std::length_error("Target snapshot entity-index capacity exhausted");
        if (generations[entity.index]) throw std::logic_error("Duplicate target in snapshot");
        const auto participant = CacheOwner(owner);
        if (canTarget) index.Add({entity,owner,position % width,position / width,category});
        generations[entity.index] = entity.generation; positions[entity.index] = position;
        owners[entity.index] = owner; sourceParticipants[entity.index] = participant;
        targetable[entity.index] = canTarget; touched.push_back(entity.index);
    }
    bool Contains(ecs::Entity entity) const noexcept
    { return entity.IsValid() && entity.index < generations.size() && generations[entity.index] == entity.generation; }
    bool CanTarget(ecs::Entity source, ecs::Entity target) const noexcept
    {
        return Contains(source) && Contains(target) && source != target &&
            owners[source.index] != owners[target.index] && targetable[target.index] &&
            VisibleToOwner(sourceParticipants[source.index], positions[target.index]);
    }
    auto Position(ecs::Entity entity) const noexcept { return positions[entity.index]; }
    ecs::Entity Owner(ecs::Entity entity) const noexcept { return owners[entity.index]; }
    std::uint32_t Width() const noexcept { return width; }
    void Publish() { index.Publish(); }
    ecs::Entity Nearest(engine::gameplay::navigation::Cell center, ecs::Entity owner,
        std::uint32_t radius, std::uint32_t categoryMask) const noexcept
    {
        ecs::Entity best{};
        std::uint64_t bestDistance = std::uint64_t{radius} * radius;
        const auto sourceParticipant = CachedOwnerParticipant(owner);
        if (!startupUnrestricted && !sourceParticipant.IsValid()) return best;
        index.VisitWithinRadius(center % width, center / width, radius,
            [&](const engine::gameplay::spatial::SpatialPoint point) noexcept {
                if (point.group == owner || !(point.category & categoryMask)) return;
                if (!VisibleToOwner(sourceParticipant,
                    static_cast<engine::gameplay::navigation::Cell>(point.y * width + point.x))) return;
                const auto dx = center % width > point.x ? center % width - point.x : point.x - center % width;
                const auto dy = center / width > point.y ? center / width - point.y : point.y - center / width;
                const auto dx2 = std::uint64_t{dx} * dx, dy2 = std::uint64_t{dy} * dy;
                const auto distance = dx2 + dy2;
                if (distance < bestDistance || !best.IsValid() ||
                    (distance == bestDistance && (point.entity.index < best.index ||
                        (point.entity.index == best.index && point.entity.generation < best.generation))))
                {
                    best = point.entity;
                    bestDistance = distance;
                }
            }, categoryMask);
        return best;
    }
    ecs::Entity Nearest(ecs::Entity actor, std::uint32_t radius, std::uint32_t categoryMask) const noexcept
    { return Nearest(Position(actor),Owner(actor),radius,categoryMask); }
private:
    engine::gameplay::rts::visibility::ParticipantHandle CacheOwner(const ecs::Entity owner)
    {
        if (startupUnrestricted) return {};
        if (!owner.IsValid() || owner.index >= ownerParticipantGenerations.size()) return {};
        if (ownerParticipantGenerations[owner.index] == 0)
        {
            ownerParticipantGenerations[owner.index] = owner.generation;
            ownerParticipants[owner.index] = participants->Resolve(owner);
            ownerTouched.push_back(owner.index);
        }
        else if (ownerParticipantGenerations[owner.index] != owner.generation)
            throw std::logic_error("Target snapshot contains multiple owner generations");
        return ownerParticipants[owner.index];
    }

    engine::gameplay::rts::visibility::ParticipantHandle CachedOwnerParticipant(const ecs::Entity owner) const noexcept
    {
        if (startupUnrestricted || !owner.IsValid() || owner.index >= ownerParticipantGenerations.size() ||
            ownerParticipantGenerations[owner.index] != owner.generation) return {};
        return ownerParticipants[owner.index];
    }

    bool VisibleToOwner(const engine::gameplay::rts::visibility::ParticipantHandle owner,
        const engine::gameplay::navigation::Cell cell) const noexcept
    {
        if (startupUnrestricted) return true;
        assert(visibility != nullptr && participants != nullptr);
        return owner.IsValid() && visibility->Visible(owner, cell);
    }

    std::vector<ecs::EntityGeneration> generations;
    std::vector<engine::gameplay::navigation::Cell> positions;
    std::vector<ecs::Entity> owners;
    std::vector<bool> targetable;
    std::vector<engine::gameplay::rts::visibility::ParticipantHandle> sourceParticipants;
    std::vector<ecs::EntityGeneration> ownerParticipantGenerations;
    std::vector<engine::gameplay::rts::visibility::ParticipantHandle> ownerParticipants;
    std::vector<ecs::EntityIndex> touched;
    std::vector<ecs::EntityIndex> ownerTouched;
    engine::gameplay::spatial::PointGrid index;
    std::uint32_t width;
    bool startupUnrestricted{true};
    const engine::gameplay::rts::visibility::VisibilityReadView *visibility{};
    const engine::gameplay::rts::visibility::VisibilityParticipantTable *participants{};
    bool visibilityPolicyConfigured{true};
};
}
