module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.navigation.fields.flow_fields;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.spatial.algorithms.grid_radius;
export import engine.jobs.job_system;
export namespace engine::gameplay::navigation
{
struct FlowLimits { std::uint32_t destinations{32}, cells{262144}; };
enum class FlowGoalKind : std::uint8_t { Cell, Range };
struct FlowGoalKey
{
    FlowGoalKind kind{FlowGoalKind::Cell};
    Cell center{InvalidCell};
    std::uint32_t radius{};

    static constexpr FlowGoalKey ForCell(const Cell value) noexcept
    { return {FlowGoalKind::Cell, value, 0}; }
    static constexpr FlowGoalKey ForRange(const Cell value, const std::uint32_t valueRadius) noexcept
    { return {FlowGoalKind::Range, value, valueRadius}; }
    friend constexpr bool operator==(const FlowGoalKey &, const FlowGoalKey &) noexcept = default;
    friend constexpr bool operator<(const FlowGoalKey &left, const FlowGoalKey &right) noexcept
    {
        if (left.kind != right.kind)
            return static_cast<std::uint8_t>(left.kind) < static_cast<std::uint8_t>(right.kind);
        if (left.center != right.center) return left.center < right.center;
        return left.radius < right.radius;
    }
};
// A bounded, destination-shared reverse BFS. One contiguous successor column and
// queue per field, allocated once. No search inside a per-agent update. The map
// must outlive this service and must not be replaced while it is in use.
class FlowFields
{
    struct Field
    {
        const NavigationGrid *grid{};
        FlowGoalKey key{};
        std::vector<Cell> next, queue;
    };
public:
    FlowFields(const NavigationGrid &grid, FlowLimits limits = {}) : grid(grid), fields(limits.destinations)
    {
        if (!limits.destinations || grid.Count() > limits.cells) throw std::length_error("Navigation capacity exceeded");
        jobs.reserve(limits.destinations);
        for (auto &field : fields)
        {
            field.grid = &grid;
            field.next.resize(grid.Count());
            field.queue.resize(grid.Count());
        }
    }
    // Caller provides sorted unique logical goals, not worker completion order.
    // Cache eviction chooses the first unrequested slot; all new fields join
    // before any agent receives a field index. A failing batch is not reusable.
    void Prepare(std::span<const Cell> goals, engine::jobs::JobSystem &workers)
    {
        if (failed) throw std::logic_error("Navigation failed; discard the simulation");
        try
        {
            if (goals.size() > fields.size()) throw std::length_error("Too many simultaneous navigation destinations");
            for (std::size_t i = 0; i != goals.size(); ++i)
                if (!grid.Walkable(goals[i]) || (i && goals[i - 1] >= goals[i]))
                    throw std::invalid_argument("Navigation goals must be sorted unique walkable cells");
            jobs.clear();
            for (const auto goal : goals)
            {
                const auto key = FlowGoalKey::ForCell(goal);
                if (Find(key) != InvalidCell) continue;
                auto slot = std::find_if(fields.begin(), fields.end(), [&](const auto &field) {
                    return std::none_of(goals.begin(), goals.end(), [&](const auto requested) {
                        return field.key == FlowGoalKey::ForCell(requested);
                    });
                });
                assert(slot != fields.end());
                slot->key = key; jobs.push_back({&Build, &*slot});
            }
            workers.Execute(jobs); rebuilt = jobs.size();
        }
        catch (...) { failed = true; throw; }
    }
    void Prepare(std::span<const FlowGoalKey> goals, engine::jobs::JobSystem &workers)
    {
        if (failed) throw std::logic_error("Navigation failed; discard the simulation");
        try
        {
            if (goals.size() > fields.size()) throw std::length_error("Too many simultaneous navigation destinations");
            for (std::size_t i = 0; i != goals.size(); ++i)
            {
                Validate(goals[i]);
                if (i && !(goals[i - 1] < goals[i]))
                    throw std::invalid_argument("Navigation goals must be sorted unique logical keys");
            }
            jobs.clear();
            for (const auto key : goals)
            {
                if (Find(key) != InvalidCell) continue;
                auto slot = std::find_if(fields.begin(), fields.end(), [&](const auto &field) {
                    return !std::binary_search(goals.begin(), goals.end(), field.key);
                });
                assert(slot != fields.end());
                slot->key = key; jobs.push_back({&Build, &*slot});
            }
            workers.Execute(jobs); rebuilt = jobs.size();
        }
        catch (...) { failed = true; throw; }
    }
    Cell Find(Cell goal) const noexcept
    {
        return Find(FlowGoalKey::ForCell(goal));
    }
    Cell Find(const FlowGoalKey &key) const noexcept
    {
        if (key.center == InvalidCell) return InvalidCell;
        for (std::size_t i = 0; i != fields.size(); ++i)
            if (fields[i].key == key) return static_cast<Cell>(i);
        return InvalidCell;
    }
    Cell Next(Cell field, Cell from) const noexcept
    { assert(field < fields.size() && from < grid.Count()); return fields[field].next[from]; }
    std::size_t RebuiltFields() const noexcept { return rebuilt; }
private:
    void Validate(const FlowGoalKey key) const
    {
        if (key.kind == FlowGoalKind::Cell)
        {
            if (key.radius != 0 || !grid.Walkable(key.center))
                throw std::invalid_argument("Ordinary navigation goals must be sorted unique walkable cells");
            return;
        }
        if (key.kind != FlowGoalKind::Range || key.center == InvalidCell || key.center >= grid.Count())
            throw std::invalid_argument("Range navigation goal must be inside the navigation grid");
    }
    static void Build(void *context) noexcept
    {
        auto &field = *static_cast<Field *>(context); const auto &grid = *field.grid;
        std::fill(field.next.begin(), field.next.end(), InvalidCell);
        std::size_t read = 0, written = 0;
        const auto seed = [&](const Cell cell) noexcept
        {
            if (field.next[cell] == InvalidCell)
            {
                field.next[cell] = cell;
                field.queue[written++] = cell;
            }
        };
        if (field.key.kind == FlowGoalKind::Cell)
        {
            if (field.key.center < grid.Count() && grid.Walkable(field.key.center)) seed(field.key.center);
        }
        else
        {
            const auto width = grid.Width();
            const auto center = spatial::GridPoint{field.key.center % width, field.key.center / width};
            for (Cell cell = 0; cell < grid.Count(); ++cell)
            {
                if (!grid.Walkable(cell)) continue;
                const auto position = spatial::GridPoint{cell % width, cell / width};
                if (spatial::IsWithinGridRadius(center, position, field.key.radius)) seed(cell);
            }
        }
        while (read != written)
        {
            const auto cell = field.queue[read++]; const auto width = grid.Width();
            // Canonical cardinal-neighbor order also fixes equal-distance ties.
            const std::array adjacent{cell >= width ? cell - width : InvalidCell,
                cell % width ? cell - 1 : InvalidCell,
                cell % width + 1 < width ? cell + 1 : InvalidCell,
                cell < grid.Count() - width ? cell + width : InvalidCell};
            for (const auto neighbor : adjacent)
                if (grid.Walkable(neighbor) && field.next[neighbor] == InvalidCell)
                { field.next[neighbor] = cell; field.queue[written++] = neighbor; }
        }
    }
    const NavigationGrid &grid;
    std::vector<Field> fields;
    std::vector<engine::jobs::Job> jobs;
    std::size_t rebuilt{};
    bool failed{};
};
}
