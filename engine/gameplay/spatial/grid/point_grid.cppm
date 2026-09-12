module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.spatial.grid.point_grid;
export import engine.ecs.core.entity;
export namespace engine::gameplay::spatial
{
// Group and category meanings are supplied by the consumer, not by this index.
struct SpatialPoint { ecs::Entity entity{}, group{}; std::uint32_t x{}, y{}, category{1}; };
// Rebuilt at a joined boundary, immutable for the following query jobs. Uniform
// buckets use CSR offsets and contiguous SoA columns; no pointer-linked lists.
class PointGrid
{
public:
    PointGrid(std::uint32_t width, std::uint32_t height, std::size_t capacity, std::uint32_t bucketSide = 8) :
        width(width), height(height), side(bucketSide), capacity(capacity)
    {
        if (!width || !height || !side || !capacity) throw std::invalid_argument("Invalid spatial index dimensions/capacity");
        columns = (std::uint64_t{width} + side - 1) / side;
        const auto buckets = columns * ((std::uint64_t{height} + side - 1) / side);
        if (buckets >= (std::numeric_limits<std::uint32_t>::max)()) throw std::length_error("Spatial bucket count exceeds dense index limit");
        offsets.resize(static_cast<std::size_t>(buckets) + 1); cursors.resize(static_cast<std::size_t>(buckets));
        staged.reserve(capacity); entities.resize(capacity); groups.resize(capacity);
        xs.resize(capacity); ys.resize(capacity); categories.resize(capacity);
        lookupGenerations.resize(capacity); lookupPoints.resize(capacity); touchedLookup.reserve(capacity);
    }
    void Clear() noexcept
    {
        for (const auto index : touchedLookup) lookupGenerations[index] = ecs::Entity::InvalidGeneration;
        touchedLookup.clear(); staged.clear(); ready = false;
    }
    void Add(SpatialPoint point)
    {
        if (ready) throw std::logic_error("Clear spatial index before rebuilding");
        if (!point.entity.IsValid() || point.x >= width || point.y >= height) throw std::invalid_argument("Invalid spatial point");
        if (point.entity.index >= capacity) throw std::length_error("Spatial entity index exceeds lookup capacity");
        if (lookupGenerations[point.entity.index] != ecs::Entity::InvalidGeneration)
            throw std::logic_error("Duplicate spatial entity index");
        if (staged.size() == capacity) throw std::length_error("Spatial point capacity exhausted");
        staged.push_back(point);
        lookupGenerations[point.entity.index] = point.entity.generation;
        lookupPoints[point.entity.index] = point;
        touchedLookup.push_back(point.entity.index);
    }
    void Publish()
    {
        if (ready) throw std::logic_error("Spatial index already published");
        std::fill(offsets.begin(), offsets.end(), 0);
        for (const auto &point : staged) ++offsets[Bucket(point.x,point.y) + 1];
        for (std::size_t i = 1; i != offsets.size(); ++i) offsets[i] += offsets[i-1];
        std::copy(offsets.begin(), offsets.end()-1, cursors.begin());
        for (const auto &point : staged)
        {
            const auto row = cursors[Bucket(point.x,point.y)]++;
            entities[row] = point.entity; groups[row] = point.group;
            xs[row] = point.x; ys[row] = point.y; categories[row] = point.category;
        }
        ready = true;
    }
    [[nodiscard]] std::uint32_t Width() const noexcept { return width; }
    [[nodiscard]] std::uint32_t Height() const noexcept { return height; }
    [[nodiscard]] std::size_t Capacity() const noexcept { return capacity; }
    [[nodiscard]] bool IsPublished() const noexcept { return ready; }
    bool TryGet(ecs::Entity entity, SpatialPoint &point) const noexcept
    {
        if (!ready || !entity.IsValid() || entity.index >= capacity || lookupGenerations[entity.index] != entity.generation)
            return false;
        point = lookupPoints[entity.index];
        return true;
    }
    // Calls visitor once for every inclusive candidate matching categoryMask.
    // The visitation order is an index detail: this function neither sorts nor
    // short-circuits, so callers can use order-independent reductions without
    // a caller-owned result buffer.
    template<typename Visitor>
    void VisitWithinRadius(std::uint32_t x, std::uint32_t y, std::uint32_t radius,
        Visitor &&visitor, std::uint32_t categoryMask = (std::numeric_limits<std::uint32_t>::max)()) const
    {
        VisitRadiusCandidates(x, y, radius, categoryMask, [&](SpatialPoint point) { visitor(point); });
    }
    std::size_t EnumerateWithinRadius(std::uint32_t x, std::uint32_t y, std::uint32_t radius,
        std::span<SpatialPoint> results, std::uint32_t categoryMask = (std::numeric_limits<std::uint32_t>::max)()) const
    {
        std::size_t count{};
        VisitRadiusCandidates(x, y, radius, categoryMask, [&](SpatialPoint point) {
            if (count == results.size()) throw std::length_error("Spatial radius result capacity exhausted");
            results[count++] = point;
        });
        std::sort(results.begin(), results.begin()+count, [](const SpatialPoint &left, const SpatialPoint &right) {
            return left.entity.index < right.entity.index ||
                (left.entity.index == right.entity.index && left.entity.generation < right.entity.generation);
        });
        return count;
    }
    ecs::Entity Nearest(std::uint32_t x, std::uint32_t y, std::uint32_t radius,
        ecs::Entity excludedGroup, std::uint32_t categoryMask) const noexcept
    {
        assert(ready && x < width && y < height);
        const auto minX = (x > radius ? x-radius : 0) / side, minY = (y > radius ? y-radius : 0) / side;
        const auto maxX = std::min<std::uint64_t>(width-1,std::uint64_t{x}+radius) / side;
        const auto maxY = std::min<std::uint64_t>(height-1,std::uint64_t{y}+radius) / side;
        std::uint64_t bestDistance = std::uint64_t{radius} * radius; ecs::Entity best{};
        for (std::uint64_t by = minY; by <= maxY; ++by)
            for (std::uint64_t bx = minX; bx <= maxX; ++bx)
            {
                const auto bucket = static_cast<std::size_t>(by * columns + bx);
                for (auto row = offsets[bucket]; row != offsets[bucket+1]; ++row)
                {
                    if (groups[row] == excludedGroup || !(categories[row] & categoryMask)) continue;
                    const std::uint64_t dx = x > xs[row] ? x-xs[row] : xs[row]-x, dy = y > ys[row] ? y-ys[row] : ys[row]-y;
                    // Compare before summing: defined even for the full uint32 coordinate domain.
                    const auto dx2 = dx*dx, dy2 = dy*dy;
                    if (dx2 > bestDistance || dy2 > bestDistance-dx2) continue;
                    const auto distance = dx2+dy2; const auto candidate = entities[row];
                    if (distance < bestDistance || !best.IsValid() || candidate.index < best.index ||
                        (candidate.index == best.index && candidate.generation < best.generation))
                    { best = candidate; bestDistance = distance; }
                }
            }
        return best;
    }
private:
    template<typename Visitor>
    void VisitRadiusCandidates(std::uint32_t x, std::uint32_t y, std::uint32_t radius,
        std::uint32_t categoryMask, Visitor &&visitor) const
    {
        assert(ready && x < width && y < height);
        const auto minX = (x > radius ? x-radius : 0) / side, minY = (y > radius ? y-radius : 0) / side;
        const auto maxX = std::min<std::uint64_t>(width-1,std::uint64_t{x}+radius) / side;
        const auto maxY = std::min<std::uint64_t>(height-1,std::uint64_t{y}+radius) / side;
        const auto radiusSquared = std::uint64_t{radius} * radius;
        for (std::uint64_t by = minY; by <= maxY; ++by)
            for (std::uint64_t bx = minX; bx <= maxX; ++bx)
            {
                const auto bucket = static_cast<std::size_t>(by * columns + bx);
                for (auto row = offsets[bucket]; row != offsets[bucket+1]; ++row)
                {
                    if (!(categories[row] & categoryMask)) continue;
                    const std::uint64_t dx = x > xs[row] ? x-xs[row] : xs[row]-x, dy = y > ys[row] ? y-ys[row] : ys[row]-y;
                    const auto dx2 = dx*dx, dy2 = dy*dy;
                    if (dx2 > radiusSquared || dy2 > radiusSquared-dx2) continue;
                    visitor(SpatialPoint{entities[row], groups[row], xs[row], ys[row], categories[row]});
                }
            }
    }
    std::size_t Bucket(std::uint32_t x, std::uint32_t y) const noexcept
    { return static_cast<std::size_t>(std::uint64_t{y/side} * columns + x/side); }
    std::uint32_t width, height, side;
    std::uint64_t columns{};
    std::size_t capacity;
    std::vector<SpatialPoint> staged; // Boundary-only input; published hot data is columnar.
    std::vector<std::size_t> offsets, cursors;
    std::vector<ecs::Entity> entities, groups;
    std::vector<std::uint32_t> xs, ys, categories;
    std::vector<ecs::EntityGeneration> lookupGenerations;
    std::vector<SpatialPoint> lookupPoints;
    std::vector<ecs::EntityIndex> touchedLookup;
    bool ready{};
};
}
