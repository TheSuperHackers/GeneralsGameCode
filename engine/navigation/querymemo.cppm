module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

export module engine.navigation.querymemo;

// This cache is deliberately independent of the game-facing navigation
// adapter. It stores only a transient result of one occupancy query and has
// no authority over terrain, zones, search costs, parents or the frontier.
export extern "C++" {
namespace navigation {

class GoalRayQueryMemo {
public:
    struct Stats {
        std::uint64_t hits = 0;
        std::uint64_t misses = 0;
        std::uint64_t inserts = 0;
    };

private:
    // The context is cold and exists once per Pathfinder, never once per map
    // cell or search slot. The epoch prevents values surviving a search,
    // while the remaining fields protect against a changed movement query in
    // the same epoch.
    std::uint32_t contextEpoch_ = 0;
    bool contextValid_ = false;
    std::uintptr_t contextObject_ = 0;
    std::int32_t contextRadius_ = 0;
    bool contextCenterInCell_ = false;
    std::uint32_t contextSurfaces_ = 0;
    std::uint64_t contextIgnoredObject_ = 0;
    std::uint32_t queryTag_ = 0;
    static constexpr std::uint32_t verdictBit_ = 0x80000000U;
    static constexpr std::uint32_t generationMask_ = 0x7fffffffU;

    struct LayerPlane {
        std::vector<std::uint32_t> entries;
    };

    std::int32_t originX_ = 0;
    std::int32_t originY_ = 0;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::size_t cellCount_ = 0;
    std::uint8_t layerCount_ = 0;
    std::vector<LayerPlane> layers_;
    std::size_t size_ = 0;
    Stats stats_;

    bool indexFor(std::int32_t x, std::int32_t y, std::uint8_t layer,
                  std::size_t& index) const noexcept {
        if (layer >= layerCount_ || width_ == 0 || height_ == 0)
            return false;

        // Do the subtraction in a wider signed type. This keeps a coordinate
        // just outside a high-origin map from wrapping into the plane.
        const auto localX = static_cast<std::int64_t>(x) - originX_;
        const auto localY = static_cast<std::int64_t>(y) - originY_;
        if (localX < 0 || localY < 0 ||
            localX >= static_cast<std::int64_t>(width_) ||
            localY >= static_cast<std::int64_t>(height_))
            return false;

        // Keep the same x-major/y-minor order as Pathfinder's m_map[x][y].
        index = static_cast<std::size_t>(localX) * height_
              + static_cast<std::size_t>(localY);
        return true;
    }

    static bool validGrid(std::int32_t originX, std::int32_t originY,
                          std::uint32_t width, std::uint32_t height,
                          std::uint8_t layerCount,
                          std::size_t& cellCount) noexcept {
        if (width == 0 || height == 0 || layerCount == 0)
            return false;

        // A grid is addressed with the signed coordinates accepted by the
        // public lookup API. Reject a configuration whose final coordinate
        // would not fit that API rather than allowing an ambiguous wrap.
        const auto maxX = static_cast<std::int64_t>(originX)
                        + static_cast<std::int64_t>(width) - 1;
        const auto maxY = static_cast<std::int64_t>(originY)
                        + static_cast<std::int64_t>(height) - 1;
        if (maxX > std::numeric_limits<std::int32_t>::max() ||
            maxY > std::numeric_limits<std::int32_t>::max())
            return false;

        const auto maxSize = std::numeric_limits<std::size_t>::max();
        if (static_cast<std::size_t>(width) > maxSize / height)
            return false;
        cellCount = static_cast<std::size_t>(width) * height;
        return cellCount <= std::vector<std::uint32_t>().max_size();
    }

    void advanceTag() {
        if (queryTag_ >= generationMask_) {
            // The verdict occupies the high bit, leaving a 31-bit generation
            // counter. A rare wrap must clear every materialized plane before
            // generation one is reused.
            for (auto& layer : layers_)
                std::fill(layer.entries.begin(), layer.entries.end(), 0);
            queryTag_ = 1;
            return;
        }
        ++queryTag_;
    }

    static std::uint32_t pack(std::uint32_t generation, bool verdict) noexcept {
        return generation | (verdict ? verdictBit_ : 0U);
    }

    static bool hasGeneration(std::uint32_t entry,
                              std::uint32_t generation) noexcept {
        return (entry & generationMask_) == generation;
    }

public:
    GoalRayQueryMemo() = default;
    GoalRayQueryMemo(const GoalRayQueryMemo&) = delete;
    GoalRayQueryMemo& operator=(const GoalRayQueryMemo&) = delete;

    // Binds the memo to the actual Pathfinder grid. Width and height are
    // counts, while originX/originY are the coordinates of cell [0][0].
    // Planes are allocated only when a layer receives its first stored result.
    // Calling this again invalidates the current scope, including when the
    // dimensions are unchanged. A rejected configuration leaves the memo
    // empty and unconfigured.
    bool configureGrid(std::int32_t originX, std::int32_t originY,
                       std::uint32_t width, std::uint32_t height,
                       std::uint8_t layerCount) {
        std::size_t cellCount = 0;
        if (!validGrid(originX, originY, width, height, layerCount, cellCount)) {
            originX_ = 0;
            originY_ = 0;
            width_ = 0;
            height_ = 0;
            cellCount_ = 0;
            layerCount_ = 0;
            layers_.clear();
            size_ = 0;
            advanceTag();
            contextValid_ = false;
            return false;
        }

        const bool sameGrid = originX_ == originX && originY_ == originY &&
            width_ == width && height_ == height && layerCount_ == layerCount;
        if (!sameGrid) {
            std::vector<LayerPlane> newLayers(layerCount);
            layers_.swap(newLayers);
        }
        originX_ = originX;
        originY_ = originY;
        width_ = width;
        height_ = height;
        cellCount_ = cellCount;
        layerCount_ = layerCount;
        size_ = 0;
        advanceTag();
        contextValid_ = false;
        return true;
    }

    // Starts or confirms the logical memo scope for a search. Calling this
    // for every callback is cheap when the scope is unchanged and makes the
    // context safety local to the cache rather than to a caller's discipline.
    void beginQuery(std::uint32_t searchEpoch,
                    std::uintptr_t objectIdentity,
                    std::int32_t radius,
                    bool centerInCell,
                    std::uint32_t acceptableSurfaces,
                    std::uint64_t ignoredObject) {
        if (contextValid_ && contextEpoch_ == searchEpoch && contextObject_ == objectIdentity &&
            contextRadius_ == radius && contextCenterInCell_ == centerInCell &&
            contextSurfaces_ == acceptableSurfaces &&
            contextIgnoredObject_ == ignoredObject && queryTag_ != 0) {
            return;
        }
        contextEpoch_ = searchEpoch;
        contextValid_ = true;
        contextObject_ = objectIdentity;
        contextRadius_ = radius;
        contextCenterInCell_ = centerInCell;
        contextSurfaces_ = acceptableSurfaces;
        contextIgnoredObject_ = ignoredObject;
        size_ = 0;
        advanceTag();
    }

    // Invalidates the current scope without releasing table capacity. This
    // is used when the owning Pathfinder is reset for a new map.
    void invalidate() noexcept {
        contextEpoch_ = 0;
        contextValid_ = false;
        contextObject_ = 0;
        contextRadius_ = 0;
        contextCenterInCell_ = false;
        contextSurfaces_ = 0;
        contextIgnoredObject_ = 0;
        size_ = 0;
        advanceTag();
    }

    bool find(std::int32_t x, std::int32_t y, std::uint8_t layer,
              bool& verdict) {
        std::size_t index = 0;
        if (queryTag_ == 0 || !indexFor(x, y, layer, index) ||
            layers_[layer].entries.empty()) {
            ++stats_.misses;
            return false;
        }
        const auto entry = layers_[layer].entries[index];
        if (!hasGeneration(entry, queryTag_)) {
            ++stats_.misses;
            return false;
        }
        verdict = (entry & verdictBit_) != 0;
        ++stats_.hits;
        return true;
    }

    void store(std::int32_t x, std::int32_t y, std::uint8_t layer,
               bool verdict) {
        assert(queryTag_ != 0);
        std::size_t index = 0;
        if (queryTag_ == 0 || !indexFor(x, y, layer, index))
            return;

        auto& entries = layers_[layer].entries;
        if (entries.empty())
            entries.assign(cellCount_, 0);

        auto& entry = entries[index];
        if (!hasGeneration(entry, queryTag_)) {
            ++size_;
            ++stats_.inserts;
        }
        entry = pack(queryTag_, verdict);
    }

    void resetStats() noexcept { stats_ = {}; }
    const Stats& stats() const noexcept { return stats_; }
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept {
        std::size_t result = 0;
        for (const auto& layer : layers_)
            result += layer.entries.capacity();
        return result;
    }
    std::size_t storageBytes() const noexcept {
        return capacity() * sizeof(std::uint32_t);
    }
};

} // namespace navigation
} // extern "C++"
