module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
export module engine.gameplay.navigation.systems.movement_system;
export import engine.gameplay.navigation.fields.flow_fields;
export import engine.ecs.system.system;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.navigation.components.movement_activity;
export import engine.gameplay.navigation.inputs.move_input;
export namespace engine::gameplay::navigation
{
struct MovementSystem
{
    MovementSystem(ecs::World &world, engine::jobs::JobSystem &jobs, const NavigationGrid &grid,
        FlowFields &fields, std::size_t capacity = 65536) :
        world(world), jobs(jobs), grid(grid), fields(fields), capacity(capacity) { goals.reserve(capacity); }

    using Query = ecs::Query<ecs::Write<GridPosition>, ecs::Write<MoveGoal>, ecs::Read<MoveSpeed>,
        ecs::Write<MoveCredit>, ecs::OptionalWrite<MoveRangeGoal>, ecs::Write<MoveField>,
        ecs::Read<MoveEnabled>, ecs::Write<MoveResult>, ecs::OptionalWrite<MovementActivity>>;

    void SetInputs(std::span<const MoveInput> value)
    {
        if (world.IsScheduledExecutionActive()) throw std::logic_error("Movement inputs require a joined boundary");
        if (value.size() > capacity) throw std::length_error("Movement input capacity exhausted");
        for (const auto &input : value)
            if (input.destination != InvalidCell && !grid.Walkable(input.destination))
                throw std::invalid_argument("Move destination is outside traversable terrain");
        inputs = value;
    }

    void BeforeChunks(Query &query, ecs::SystemContext &context)
    {
        for (const auto input : inputs)
        {
            auto *goal = world.Get<MoveGoal>(input.actor);
            auto *credit = world.Get<MoveCredit>(input.actor);
            auto *range = world.Get<MoveRangeGoal>(input.actor);
            const auto *enabled = world.Get<MoveEnabled>(input.actor);
            if (goal && credit && enabled && enabled->value)
            {
                if (range) ClearMoveRangeGoal(*range, *credit);
                if (goal->cell != input.destination) credit->remainder = 0;
                goal->cell = input.destination;
            }
        }
        inputs = {};

        goals.clear();
        std::size_t count = 0;
        query.ForEachChunk([&](auto chunk) {
            if (chunk.Count() > capacity - count) throw std::length_error("Movement agent capacity exhausted");
            count += chunk.Count();
            const auto positions = chunk.template Get<GridPosition>();
            const auto targets = chunk.template Get<MoveGoal>();
            const auto ranges = chunk.template Get<MoveRangeGoal>();
            const auto enabled = chunk.template Get<MoveEnabled>();
            const auto speeds = chunk.template Get<MoveSpeed>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                assert(grid.Walkable(positions[row].cell));
                assert(std::uint64_t{speeds[row].cellsPerSecond} <=
                    std::uint64_t{context.Time().Step().TicksPerSecond()} * 64);
                if (!enabled[row].value) continue;
                if (!ranges.empty() && ranges[row].Active())
                {
                    if (ranges[row].center < grid.Count() && !InRange(positions[row].cell, ranges[row].center,
                        grid.Width(), ranges[row].radius))
                        goals.push_back(FlowGoalKey::ForRange(ranges[row].center, ranges[row].radius));
                }
                else if (targets[row].cell != InvalidCell && targets[row].cell != positions[row].cell)
                    goals.push_back(FlowGoalKey::ForCell(targets[row].cell));
            }
        });
        std::sort(goals.begin(), goals.end());
        goals.erase(std::unique(goals.begin(), goals.end()), goals.end());

        // Batched searches finish before this system's movement chunks begin.
        // FlowFields is an injected algorithm/cache, not a registered child system.
        fields.Prepare(std::span<const FlowGoalKey>{goals.data(), goals.size()}, jobs);
        query.ForEachChunk([&](auto chunk) {
            const auto positions = chunk.template Get<GridPosition>();
            const auto targets = chunk.template Get<MoveGoal>();
            const auto ranges = chunk.template Get<MoveRangeGoal>();
            const auto enabled = chunk.template Get<MoveEnabled>();
            auto bindings = chunk.template Get<MoveField>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                bindings[row].index = InvalidCell;
                if (!enabled[row].value) continue;
                if (!ranges.empty() && ranges[row].Active())
                {
                    if (ranges[row].center < grid.Count() && !InRange(positions[row].cell, ranges[row].center,
                        grid.Width(), ranges[row].radius))
                        bindings[row].index = fields.Find(
                            FlowGoalKey::ForRange(ranges[row].center, ranges[row].radius));
                }
                else if (targets[row].cell != InvalidCell && targets[row].cell != positions[row].cell)
                    bindings[row].index = fields.Find(targets[row].cell);
            }
        });
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        auto positions = chunk.Get<GridPosition>();
        const auto goals = chunk.Get<MoveGoal>();
        const auto speeds = chunk.Get<MoveSpeed>();
        auto credits = chunk.Get<MoveCredit>();
        const auto ranges = chunk.Get<MoveRangeGoal>();
        const auto bindings = chunk.Get<MoveField>();
        const auto enabled = chunk.Get<MoveEnabled>();
        auto results = chunk.Get<MoveResult>();
        auto activities = chunk.Get<MovementActivity>();
        const auto rate = context.Time().Step().TicksPerSecond();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            if (!activities.empty()) activities[row].cellsMovedThisTick = 0;
            const auto moveOneCell = [&](const Cell next) noexcept {
                if (next == positions[row].cell) return;
                positions[row].cell = next;
                if (!activities.empty()) ++activities[row].cellsMovedThisTick;
            };
            if (!enabled[row].value)
            {
                results[row].status = MoveStatus::Idle;
                credits[row].remainder = 0;
                continue;
            }

            if (!ranges.empty() && ranges[row].Active())
            {
                const bool validCenter = ranges[row].center < grid.Count();
                if (bindings[row].index == InvalidCell)
                {
                    results[row].status = validCenter && InRange(positions[row].cell, ranges[row].center,
                        grid.Width(), ranges[row].radius) ? MoveStatus::Arrived : MoveStatus::Unreachable;
                    credits[row].remainder = 0;
                    continue;
                }
                if (!validCenter)
                {
                    results[row].status = MoveStatus::Unreachable;
                    credits[row].remainder = 0;
                    continue;
                }
                auto successor = fields.Next(bindings[row].index, positions[row].cell);
                if (successor == InvalidCell)
                {
                    results[row].status = MoveStatus::Unreachable;
                    credits[row].remainder = 0;
                    continue;
                }
                if (successor == positions[row].cell)
                {
                    results[row].status = MoveStatus::Arrived;
                    credits[row].remainder = 0;
                    continue;
                }

                const auto credit = std::uint64_t{credits[row].remainder} + speeds[row].cellsPerSecond;
                auto steps = credit / rate;
                credits[row].remainder = static_cast<std::uint32_t>(credit % rate);
                results[row].status = MoveStatus::Moving;
                while (steps != 0)
                {
                    successor = fields.Next(bindings[row].index, positions[row].cell);
                    if (successor == InvalidCell)
                    {
                        results[row].status = MoveStatus::Unreachable;
                        credits[row].remainder = 0;
                        break;
                    }
                    if (successor == positions[row].cell)
                    {
                        results[row].status = MoveStatus::Arrived;
                        credits[row].remainder = 0;
                        break;
                    }
                    moveOneCell(successor);
                    --steps;
                }
                if (results[row].status == MoveStatus::Moving &&
                    fields.Next(bindings[row].index, positions[row].cell) == positions[row].cell)
                {
                    results[row].status = MoveStatus::Arrived;
                    credits[row].remainder = 0;
                }
                continue;
            }

            if (goals[row].cell == InvalidCell)
            {
                results[row].status = MoveStatus::Idle;
                credits[row].remainder = 0;
                continue;
            }
            if (positions[row].cell == goals[row].cell)
            {
                results[row].status = MoveStatus::Arrived;
                credits[row].remainder = 0;
                continue;
            }
            if (bindings[row].index == InvalidCell ||
                fields.Next(bindings[row].index, positions[row].cell) == InvalidCell)
            {
                results[row].status = MoveStatus::Unreachable;
                credits[row].remainder = 0;
                continue;
            }

            const auto credit = std::uint64_t{credits[row].remainder} + speeds[row].cellsPerSecond;
            auto steps = credit / rate;
            credits[row].remainder = static_cast<std::uint32_t>(credit % rate);
            results[row].status = MoveStatus::Moving;
            while (steps != 0 && positions[row].cell != goals[row].cell)
            {
                --steps;
                moveOneCell(fields.Next(bindings[row].index, positions[row].cell));
            }
            if (positions[row].cell == goals[row].cell)
            {
                results[row].status = MoveStatus::Arrived;
                credits[row].remainder = 0;
            }
        }
    }

private:
    static bool InRange(const Cell position, const Cell center, const std::uint32_t width,
        const std::uint32_t radius) noexcept
    {
        assert(width != 0);
        return engine::gameplay::spatial::IsWithinGridRadius(
            {center % width, center / width}, {position % width, position / width}, radius);
    }

    ecs::World &world;
    engine::jobs::JobSystem &jobs;
    const NavigationGrid &grid;
    FlowFields &fields;
    std::size_t capacity;
    std::vector<FlowGoalKey> goals;
    std::span<const MoveInput> inputs;
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::navigation::MovementSystem>
{
    static constexpr std::string_view StableName = "engine.gameplay.navigation.move";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
export namespace engine::gameplay::navigation
{
inline void RegisterMovementComponents(ecs::World &world)
{
    world.RegisterComponent<GridPosition>();
    world.RegisterComponent<MoveGoal>();
    world.RegisterComponent<MoveRangeGoal>();
    world.RegisterComponent<MoveSpeed>();
    world.RegisterComponent<MoveCredit>();
    world.RegisterComponent<MoveField>();
    world.RegisterComponent<MoveEnabled>();
    world.RegisterComponent<MoveResult>();
    world.RegisterComponent<MovementActivity>();
}
}
