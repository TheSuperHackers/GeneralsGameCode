module;
#include <array>
#include <cstdint>

export module engine.navigation.movement.occupancy_policy;

export namespace navigation {
struct OccupancyCell {
    std::uint32_t unit = 0;
    bool valid = true, empty = true, goal = false, moving = false, fixed = false;
};
struct OccupancyQuery {
    std::uint32_t self = 0, ignored = 0;
    int x = 0, y = 0, radius = 0, cellsAbove = 0;
    bool considerTransient = false, infantryPassThrough = false;
};
struct OccupancyResult {
    int allyFixedCount = 0;
    bool enemyFixed = false, allyMoving = false, allyGoal = false;
};

// Movement policy depends on the occupancy grid and unit relationships, not on
// object ownership or game singletons. The game adapter supplies those views.
template<class ReadCell, class Units>
bool checkOccupancy(const OccupancyQuery& query, OccupancyResult& result,
                    ReadCell readCell, const Units& units)
{
    result = {};
    std::array<std::uint32_t, 5> allies;
    int numAllies = 0;
    for (int x = query.x - query.radius; x < query.x + query.cellsAbove; ++x) {
        for (int y = query.y - query.radius; y < query.y + query.cellsAbove; ++y) {
            const auto cell = readCell(x, y);
            if (!cell.valid) return false;
            if (cell.goal) result.allyGoal = true;
            else if (cell.empty) continue;
            if (cell.unit == query.self || cell.unit == query.ignored) continue;
            if (!cell.moving && !cell.fixed) continue;
            const auto unit = units.find(cell.unit);
            if (!unit) continue;
            const bool allied = units.allied(unit);
            if (cell.moving && allied) result.allyMoving = true;
            if (!cell.fixed && !query.considerTransient) continue;
            if (query.infantryPassThrough && units.infantry(unit)) continue;
            if (allied) {
                if (!units.canMoveAside(unit)) return false;
                bool found = false;
                for (int i = 0; i < numAllies; ++i)
                    if (allies[i] == cell.unit) found = true;
                if (!found) {
                    ++result.allyFixedCount;
                    if (numAllies < static_cast<int>(allies.size()))
                        allies[numAllies++] = cell.unit;
                }
            } else if (!units.canCrush(unit)) {
                result.enemyFixed = true;
            }
        }
    }
    return true;
}
}
