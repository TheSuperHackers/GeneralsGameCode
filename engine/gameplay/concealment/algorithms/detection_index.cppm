module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.concealment.algorithms.detection_index;
export import engine.ecs.query.query;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.concealment.algorithms.detection_lease;
export import engine.gameplay.concealment.components.concealment_binding;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.concealment.components.detection_state;
export import engine.gameplay.concealment.definitions.detection_definition;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.spatial.grid.point_grid;

export namespace engine::gameplay::concealment
{
// Caller-side bounded join scratch. It is derived from ECS columns at the
// joined boundary and never becomes a second owner of concealment state.
class DetectionIndex
{
public:
    using Access = ecs::Query<ecs::Read<combat::LifeState>, ecs::Read<navigation::GridPosition>,
        ecs::Optional<ConcealmentBinding>, ecs::Optional<ConcealmentState>, ecs::Optional<ConcealmentGroup>,
        ecs::Optional<DetectionBinding>, ecs::OptionalWrite<DetectionState>,
        ecs::Optional<containment::PassengerMembership>>;

    DetectionIndex(ecs::World &world, const navigation::NavigationGrid &grid, std::size_t capacity) :
        grid(grid), rows(world), targets(grid.Width(), grid.Count() / grid.Width(), capacity), generations(capacity),
        deadlines(capacity), touched(), candidates(capacity)
    {
        if (!capacity) throw std::invalid_argument("Detection index capacity must be positive");
        touched.reserve(capacity);
        for (auto &generation : generations) generation = ecs::Entity::InvalidGeneration;
    }

    void Rebuild(std::uint64_t tick, const DetectionDefinitions &definitions)
    {
        targets.Clear();
        ClearDeadlines();

        rows.ForEachChunk([&](auto chunk) {
            const auto lives = chunk.template Get<combat::LifeState>();
            const auto positions = chunk.template Get<navigation::GridPosition>();
            const auto bindings = chunk.template Get<ConcealmentBinding>();
            const auto states = chunk.template Get<ConcealmentState>();
            const auto groups = chunk.template Get<ConcealmentGroup>();
            const auto members = chunk.template Get<containment::PassengerMembership>();
            if (bindings.empty() || states.empty() || groups.empty()) return;
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto position = positions[row].cell;
                const auto group = groups[row].value;
                if (!lives[row].alive || !group.IsValid() || position >= grid.Count()) continue;
                if (!states[row].concealed) continue;
                if (!members.empty() && containment::IsContained(members[row])) continue;
                targets.Add({chunk.Entities()[row], group, position % grid.Width(), position / grid.Width(),
                    TargetCategory});
            }
        });
        targets.Publish();

        rows.ForEachChunk([&](auto chunk) {
            const auto lives = chunk.template Get<combat::LifeState>();
            const auto positions = chunk.template Get<navigation::GridPosition>();
            const auto bindings = chunk.template Get<DetectionBinding>();
            const auto states = chunk.template Get<DetectionState>();
            const auto groups = chunk.template Get<ConcealmentGroup>();
            const auto members = chunk.template Get<containment::PassengerMembership>();
            if (bindings.empty()) return;
            if (states.empty() || groups.empty()) throw std::logic_error("Detection binding lacks runtime state or group");
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                if (!lives[row].alive || !groups[row].value.IsValid()) continue;
                const auto position = positions[row].cell;
                if (position >= grid.Count()) continue;
                auto &state = states[row];
                if (!state.enabled || tick < state.nextScanTick) continue;
                const auto &definition = definitions.Get(bindings[row].definition);
                state.nextScanTick = ConcealmentDeadline(tick, definition.detectionRateTicks);
                if (!members.empty() && containment::IsContained(members[row]) && !definition.canDetectWhileContained) continue;

                const auto count = targets.EnumerateWithinRadius(position % grid.Width(), position / grid.Width(),
                    definition.detectionRangeCells, std::span<spatial::SpatialPoint>{candidates.data(), candidates.size()});
                for (std::size_t candidate = 0; candidate != count; ++candidate)
                {
                    const auto &point = candidates[candidate];
                    if (point.group == groups[row].value) continue;
                    Record(point.entity, DetectionLeaseDeadline(tick, definition.detectionRateTicks));
                }
            }
        });
    }

    std::uint64_t DetectedUntil(ecs::Entity entity) const noexcept
    {
        if (!entity.IsValid() || entity.index >= generations.size() || generations[entity.index] != entity.generation)
            return 0;
        return deadlines[entity.index];
    }

private:
    static constexpr std::uint32_t TargetCategory = 1U;

    void ClearDeadlines() noexcept
    {
        for (const auto index : touched)
        {
            generations[index] = ecs::Entity::InvalidGeneration;
            deadlines[index] = 0;
        }
        touched.clear();
    }

    void Record(ecs::Entity entity, std::uint64_t deadline)
    {
        if (!entity.IsValid() || entity.index >= generations.size())
            throw std::length_error("Detection reduction entity capacity exhausted");
        if (generations[entity.index] == ecs::Entity::InvalidGeneration)
        {
            generations[entity.index] = entity.generation;
            deadlines[entity.index] = deadline;
            touched.push_back(entity.index);
            return;
        }
        if (generations[entity.index] != entity.generation)
            throw std::logic_error("Detection reduction encountered a reused entity index");
        deadlines[entity.index] = (std::max)(deadlines[entity.index], deadline);
    }

    const navigation::NavigationGrid &grid;
    Access rows;
    spatial::PointGrid targets;
    std::vector<ecs::EntityGeneration> generations;
    std::vector<std::uint64_t> deadlines;
    std::vector<ecs::EntityIndex> touched;
    std::vector<spatial::SpatialPoint> candidates;
};
}
