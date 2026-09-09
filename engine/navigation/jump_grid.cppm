// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Independent C++ JPS core. Its fixed direction ordering and
 * no-corner-cutting forced-neighbour semantics are informed by qiqian/JPS,
 * commit 87b6580c65b2ae9eebf5c8e4f7a83bc066687225 (MIT, Copyright (c) 2026
 * Qian Qian). No source from that repository is included here.
 */

module;

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <span>
#include <utility>
#include <vector>

export module engine.navigation.jump_grid;

namespace navigation::detail {

using Coordinate = std::int32_t;
using Cost = std::uint64_t;
using Index = std::size_t;

inline constexpr Cost CardinalCost = 1000;
inline constexpr Cost DiagonalCost = 1414;
inline constexpr Index NoIndex = std::numeric_limits<Index>::max();

inline Cost saturatedAdd(Cost left, Cost right) noexcept {
    const auto maximum = std::numeric_limits<Cost>::max();
    return left > maximum - right ? maximum : left + right;
}

struct JumpGridData {
    Coordinate width = 0;
    Coordinate height = 0;
    std::uint64_t version = 0;
    std::vector<std::uint64_t> blockedBits;
    // E, W, S and N respectively. A positive value is the number of steps to
    // the first jump point; a negative value is the number of free steps
    // before the wall. Zero means that the next cell is a wall.
    std::array<std::vector<std::int32_t>, 4> cardinal;

    bool inBounds(std::int64_t x, std::int64_t y) const noexcept {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    Index index(Coordinate x, Coordinate y) const noexcept {
        return static_cast<Index>(y) * static_cast<Index>(width)
             + static_cast<Index>(x);
    }

    bool blocked(Coordinate x, Coordinate y) const noexcept {
        if (!inBounds(x, y))
            return true;
        const auto i = index(x, y);
        return (blockedBits[i >> 6] & (std::uint64_t{1} << (i & 63))) != 0;
    }

    bool walkable(std::int64_t x, std::int64_t y) const noexcept {
        if (!inBounds(x, y))
            return false;
        const auto i = static_cast<Index>(y) * static_cast<Index>(width)
                     + static_cast<Index>(x);
        return (blockedBits[i >> 6] & (std::uint64_t{1} << (i & 63))) == 0;
    }
};

inline bool validDimensions(std::int32_t width, std::int32_t height,
                            std::size_t& cellCount,
                            std::size_t& wordCount) noexcept {
    if (width <= 0 || height <= 0)
        return false;

    const auto w = static_cast<std::uint64_t>(width);
    const auto h = static_cast<std::uint64_t>(height);
    const auto maxSize = std::numeric_limits<std::size_t>::max();
    if (w > maxSize / h)
        return false;
    cellCount = static_cast<std::size_t>(w * h);
    if (cellCount == 0 || cellCount > std::vector<std::uint8_t>().max_size())
        return false;
    if (cellCount > maxSize - 63)
        return false;
    wordCount = (cellCount + 63) / 64;
    return wordCount <= std::vector<std::uint64_t>().max_size();
}

inline std::vector<std::uint64_t> makeBlockedBits(
    std::span<const std::uint8_t> blocked, std::size_t cellCount,
    std::size_t wordCount) {
    std::vector<std::uint64_t> bits(wordCount, 0);
    for (std::size_t i = 0; i < cellCount; ++i) {
        if (blocked[i] != 0)
            bits[i >> 6] |= std::uint64_t{1} << (i & 63);
    }
    return bits;
}

inline bool cardinalForced(const JumpGridData& data, Coordinate x,
                           Coordinate y, Coordinate dx) noexcept {
    // These are the no-corner-cutting forced-neighbour rules from the
    // reference JPS implementation. For horizontal motion the side cell
    // must be open while the cell behind that side is blocked; vertical
    // motion is the transposed case.
    if (dx != 0) {
        return (data.walkable(x, static_cast<std::int64_t>(y) + 1) &&
                !data.walkable(static_cast<std::int64_t>(x) - dx,
                               static_cast<std::int64_t>(y) + 1)) ||
               (data.walkable(x, static_cast<std::int64_t>(y) - 1) &&
                !data.walkable(static_cast<std::int64_t>(x) - dx,
                               static_cast<std::int64_t>(y) - 1));
    }
    return false;
}

inline bool verticalForced(const JumpGridData& data, Coordinate x,
                           Coordinate y, Coordinate dy) noexcept {
    return (data.walkable(static_cast<std::int64_t>(x) + 1, y) &&
            !data.walkable(static_cast<std::int64_t>(x) + 1,
                           static_cast<std::int64_t>(y) - dy)) ||
           (data.walkable(static_cast<std::int64_t>(x) - 1, y) &&
            !data.walkable(static_cast<std::int64_t>(x) - 1,
                           static_cast<std::int64_t>(y) - dy));
}

inline void buildRow(JumpGridData& data, Coordinate y, Coordinate dx) {
    auto& distances = data.cardinal[dx > 0 ? 0 : 1];
    Coordinate nextJump = -1;
    std::int32_t freeAhead = 0;

    if (dx > 0) {
        for (Coordinate x = data.width; x-- > 0;) {
            const auto i = data.index(x, y);
            if (data.blocked(x, y)) {
                distances[i] = 0;
                nextJump = -1;
                freeAhead = 0;
                continue;
            }

            freeAhead = data.walkable(static_cast<std::int64_t>(x) + 1, y)
                ? freeAhead + 1 : 0;
            distances[i] = nextJump >= 0 ? nextJump - x : -freeAhead;
            if (cardinalForced(data, x, y, dx))
                nextJump = x;
        }
    } else {
        for (Coordinate x = 0; x < data.width; ++x) {
            const auto i = data.index(x, y);
            if (data.blocked(x, y)) {
                distances[i] = 0;
                nextJump = -1;
                freeAhead = 0;
                continue;
            }

            freeAhead = data.walkable(static_cast<std::int64_t>(x) - 1, y)
                ? freeAhead + 1 : 0;
            distances[i] = nextJump >= 0 ? x - nextJump : -freeAhead;
            if (cardinalForced(data, x, y, dx))
                nextJump = x;
        }
    }
}

inline void buildColumn(JumpGridData& data, Coordinate x, Coordinate dy) {
    auto& distances = data.cardinal[dy > 0 ? 2 : 3];
    Coordinate nextJump = -1;
    std::int32_t freeAhead = 0;

    if (dy > 0) {
        for (Coordinate y = data.height; y-- > 0;) {
            const auto i = data.index(x, y);
            if (data.blocked(x, y)) {
                distances[i] = 0;
                nextJump = -1;
                freeAhead = 0;
                continue;
            }

            freeAhead = data.walkable(x, static_cast<std::int64_t>(y) + 1)
                ? freeAhead + 1 : 0;
            distances[i] = nextJump >= 0 ? nextJump - y : -freeAhead;
            if (verticalForced(data, x, y, dy))
                nextJump = y;
        }
    } else {
        for (Coordinate y = 0; y < data.height; ++y) {
            const auto i = data.index(x, y);
            if (data.blocked(x, y)) {
                distances[i] = 0;
                nextJump = -1;
                freeAhead = 0;
                continue;
            }

            freeAhead = data.walkable(x, static_cast<std::int64_t>(y) - 1)
                ? freeAhead + 1 : 0;
            distances[i] = nextJump >= 0 ? y - nextJump : -freeAhead;
            if (verticalForced(data, x, y, dy))
                nextJump = y;
        }
    }
}

inline std::shared_ptr<JumpGridData> makeData(
    std::int32_t width, std::int32_t height,
    std::span<const std::uint8_t> blocked, std::uint64_t version,
    const JumpGridData* previous, std::span<const Coordinate> dirtyRows,
    std::span<const Coordinate> dirtyColumns, bool rebuildAll) {
    std::size_t cellCount = 0;
    std::size_t wordCount = 0;
    if (!validDimensions(width, height, cellCount, wordCount) ||
        blocked.size() != cellCount)
        return {};

    auto data = std::make_shared<JumpGridData>();
    data->width = width;
    data->height = height;
    data->version = version;
    if (previous && previous->width == width && previous->height == height) {
        for (std::size_t dir = 0; dir < data->cardinal.size(); ++dir)
            data->cardinal[dir] = previous->cardinal[dir];
    } else {
        for (auto& line : data->cardinal)
            line.resize(cellCount, 0);
    }
    data->blockedBits = makeBlockedBits(blocked, cellCount, wordCount);

    if (rebuildAll) {
        for (Coordinate y = 0; y < height; ++y) {
            buildRow(*data, y, 1);
            buildRow(*data, y, -1);
        }
        for (Coordinate x = 0; x < width; ++x) {
            buildColumn(*data, x, 1);
            buildColumn(*data, x, -1);
        }
    } else {
        for (const auto y : dirtyRows) {
            buildRow(*data, y, 1);
            buildRow(*data, y, -1);
        }
        for (const auto x : dirtyColumns) {
            buildColumn(*data, x, 1);
            buildColumn(*data, x, -1);
        }
    }
    return data;
}

} // namespace navigation::detail

export extern "C++" {
namespace navigation {

struct JumpGridCell {
    std::int32_t x = 0;
    std::int32_t y = 0;

    friend bool operator==(const JumpGridCell&, const JumpGridCell&) = default;
};

struct JumpGridEdit {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint8_t blocked = 0;
};

struct JumpGridSearchResult {
    bool found = false;
    std::span<const JumpGridCell> path;
    std::uint64_t expanded = 0;
    std::uint64_t scanned = 0;
    std::uint64_t cost = 0;
};

class JumpGridSnapshot {
public:
    JumpGridSnapshot() = default;

    bool valid() const noexcept { return static_cast<bool>(data_); }
    std::int32_t width() const noexcept { return data_ ? data_->width : 0; }
    std::int32_t height() const noexcept { return data_ ? data_->height : 0; }
    std::uint64_t version() const noexcept { return data_ ? data_->version : 0; }

    bool isBlocked(std::int32_t x, std::int32_t y) const noexcept {
        return !data_ || data_->blocked(x, y);
    }

private:
    std::shared_ptr<const detail::JumpGridData> data_;

    explicit JumpGridSnapshot(std::shared_ptr<const detail::JumpGridData> data)
        : data_(std::move(data)) {}

    friend class JumpGridTopology;
    friend class JumpGridWorkspace;
};

class JumpGridTopology {
public:
    JumpGridTopology() = default;
    JumpGridTopology(const JumpGridTopology&) = delete;
    JumpGridTopology& operator=(const JumpGridTopology&) = delete;
    JumpGridTopology(JumpGridTopology&&) = delete;
    JumpGridTopology& operator=(JumpGridTopology&&) = delete;

    // An empty blocked span means an all-walkable map. A non-empty span must
    // contain exactly width*height row-major bytes. The new snapshot is
    // published only after all four directional tables are built.
    bool configure(std::int32_t width, std::int32_t height,
                   std::span<const std::uint8_t> blocked = {}) {
        std::size_t cellCount = 0;
        std::size_t wordCount = 0;
        if (!detail::validDimensions(width, height, cellCount, wordCount) ||
            (!blocked.empty() && blocked.size() != cellCount))
            return false;

        std::vector<std::uint8_t> nextBlocked;
        std::vector<detail::Coordinate> nextDirtyRows;
        std::vector<detail::Coordinate> nextDirtyColumns;
        std::vector<std::uint8_t> nextDirtyRowMark;
        std::vector<std::uint8_t> nextDirtyColumnMark;
        try {
            nextBlocked.assign(cellCount, 0);
            if (!blocked.empty())
                std::copy(blocked.begin(), blocked.end(), nextBlocked.begin());
            // Reserve the complete dirty-line lists before committing the new
            // map. Subsequent sparse edits are then allocation-free and a
            // failed push cannot leave a partially marked line.
            nextDirtyRows.reserve(static_cast<std::size_t>(height));
            nextDirtyColumns.reserve(static_cast<std::size_t>(width));
            nextDirtyRowMark.assign(static_cast<std::size_t>(height), 0);
            nextDirtyColumnMark.assign(static_cast<std::size_t>(width), 0);

            const auto nextVersion = version_ == std::numeric_limits<std::uint64_t>::max()
                ? 1 : version_ + 1;
            auto next = detail::makeData(width, height, nextBlocked, nextVersion,
                                         nullptr, {}, {}, true);
            if (!next)
                return false;

            width_ = width;
            height_ = height;
            staged_.swap(nextBlocked);
            version_ = nextVersion;
            dirtyAll_ = false;
            dirtyRows_.swap(nextDirtyRows);
            dirtyColumns_.swap(nextDirtyColumns);
            dirtyRowMark_.swap(nextDirtyRowMark);
            dirtyColumnMark_.swap(nextDirtyColumnMark);
            std::atomic_store_explicit(&published_,
                std::shared_ptr<const detail::JumpGridData>(std::move(next)),
                std::memory_order_release);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool reset(std::int32_t width, std::int32_t height,
               std::span<const std::uint8_t> blocked = {}) {
        return configure(width, height, blocked);
    }

    bool configured() const noexcept {
        return width_ > 0 && height_ > 0 &&
            static_cast<bool>(std::atomic_load_explicit(&published_,
                                                        std::memory_order_acquire));
    }

    std::int32_t width() const noexcept { return width_; }
    std::int32_t height() const noexcept { return height_; }

    JumpGridSnapshot snapshot() const noexcept {
        return JumpGridSnapshot(std::atomic_load_explicit(&published_,
                                                          std::memory_order_acquire));
    }

    bool isBlocked(std::int32_t x, std::int32_t y) const noexcept {
        return snapshot().isBlocked(x, y);
    }

    // Edits are staged in input order. Out-of-bounds edits are ignored; the
    // return value counts only cells whose staged value actually changed.
    std::size_t applyEdits(std::span<const JumpGridEdit> edits) {
        if (!configured())
            return 0;

        std::size_t changed = 0;
        for (const auto& edit : edits) {
            if (edit.x < 0 || edit.x >= width_ || edit.y < 0 || edit.y >= height_)
                continue;
            const auto i = static_cast<std::size_t>(edit.y) *
                               static_cast<std::size_t>(width_) +
                           static_cast<std::size_t>(edit.x);
            const auto value = static_cast<std::uint8_t>(edit.blocked != 0);
            if (staged_[i] == value)
                continue;
            staged_[i] = value;
            markDirty(edit.x, edit.y);
            ++changed;
        }
        if (changed != 0)
            version_ = version_ == std::numeric_limits<std::uint64_t>::max()
                ? 1 : version_ + 1;
        return changed;
    }

    bool setBlocked(std::int32_t x, std::int32_t y, bool blocked) {
        JumpGridEdit edit{x, y, static_cast<std::uint8_t>(blocked)};
        return applyEdits(std::span<const JumpGridEdit>(&edit, 1)) != 0;
    }

    // Builds a complete immutable replacement. Only this method writes the
    // derived directional tables, and it must run outside the query batch.
    bool synchronize() {
        const auto old = std::atomic_load_explicit(&published_,
                                                   std::memory_order_acquire);
        if (!old || !configured())
            return false;
        if (dirtyRows_.empty() && dirtyColumns_.empty() && !dirtyAll_)
            return true;

        try {
            const auto next = detail::makeData(
                width_, height_, staged_, version_, old.get(), dirtyRows_,
                dirtyColumns_, dirtyAll_ || old->width != width_ || old->height != height_);
            if (!next)
                return false;
            std::atomic_store_explicit(&published_,
                std::shared_ptr<const detail::JumpGridData>(next),
                std::memory_order_release);
            dirtyRows_.clear();
            dirtyColumns_.clear();
            dirtyAll_ = false;
            std::fill(dirtyRowMark_.begin(), dirtyRowMark_.end(), 0);
            std::fill(dirtyColumnMark_.begin(), dirtyColumnMark_.end(), 0);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::uint64_t version() const noexcept { return snapshot().version(); }

    std::size_t storageBytes() const noexcept {
        std::size_t result = staged_.capacity() * sizeof(staged_[0]) +
            dirtyRows_.capacity() * sizeof(dirtyRows_[0]) +
            dirtyColumns_.capacity() * sizeof(dirtyColumns_[0]) +
            dirtyRowMark_.capacity() + dirtyColumnMark_.capacity();
        const auto current = snapshot();
        if (current.data_) {
            const auto& data = *current.data_;
            result += data.blockedBits.capacity() * sizeof(data.blockedBits[0]);
            for (const auto& line : data.cardinal)
                result += line.capacity() * sizeof(line[0]);
        }
        return result;
    }

private:
    std::int32_t width_ = 0;
    std::int32_t height_ = 0;
    std::uint64_t version_ = 0;
    std::vector<std::uint8_t> staged_;
    std::vector<detail::Coordinate> dirtyRows_;
    std::vector<detail::Coordinate> dirtyColumns_;
    std::vector<std::uint8_t> dirtyRowMark_;
    std::vector<std::uint8_t> dirtyColumnMark_;
    bool dirtyAll_ = true;
    std::shared_ptr<const detail::JumpGridData> published_;

    void markDirty(std::int32_t x, std::int32_t y) {
        markRow(static_cast<std::int64_t>(y) - 1);
        markRow(y);
        markRow(static_cast<std::int64_t>(y) + 1);
        markColumn(static_cast<std::int64_t>(x) - 1);
        markColumn(x);
        markColumn(static_cast<std::int64_t>(x) + 1);
    }

    void markRow(std::int64_t y) {
        if (y < 0 || y >= height_)
            return;
        const auto i = static_cast<std::size_t>(y);
        if (dirtyRowMark_[i] == 0) {
            dirtyRowMark_[i] = 1;
            dirtyRows_.push_back(static_cast<detail::Coordinate>(y));
        }
    }

    void markColumn(std::int64_t x) {
        if (x < 0 || x >= width_)
            return;
        const auto i = static_cast<std::size_t>(x);
        if (dirtyColumnMark_[i] == 0) {
            dirtyColumnMark_[i] = 1;
            dirtyColumns_.push_back(static_cast<detail::Coordinate>(x));
        }
    }

    friend class JumpGridWorkspace;
};

class JumpGridWorkspace {
public:
    JumpGridWorkspace() = default;
    JumpGridWorkspace(const JumpGridWorkspace&) = delete;
    JumpGridWorkspace& operator=(const JumpGridWorkspace&) = delete;
    JumpGridWorkspace(JumpGridWorkspace&&) = delete;
    JumpGridWorkspace& operator=(JumpGridWorkspace&&) = delete;

    JumpGridSearchResult find(const JumpGridSnapshot& snapshot,
                              JumpGridCell start, JumpGridCell goal) {
        resetResult();
        const auto data = snapshot.data_;
        if (!data || !data->inBounds(start.x, start.y) ||
            !data->inBounds(goal.x, goal.y) || data->blocked(start.x, start.y) ||
            data->blocked(goal.x, goal.y))
            return result();

        ensureCapacity(*data);
        beginSearch();
        const auto startId = data->index(start.x, start.y);
        const auto goalId = data->index(goal.x, goal.y);
        touch(startId);
        g_[startId] = 0;
        parent_[startId] = detail::NoIndex;
        push(startId, heuristic(start, goal));

        if (startId == goalId) {
            expanded_ = 1;
            reconstruct(*data, startId);
            return result(true);
        }

        constexpr std::array<Direction, 8> directions{{
            {1, 0, false}, {-1, 0, false}, {0, 1, false}, {0, -1, false},
            {1, 1, true}, {-1, 1, true}, {1, -1, true}, {-1, -1, true}
        }};

        while (!heap_.empty()) {
            const auto entry = pop();
            if (entry.id == detail::NoIndex || entry.g != g_[entry.id] ||
                closed_[entry.id] != 0)
                continue;
            closed_[entry.id] = 1;
            ++expanded_;

            if (entry.id == goalId) {
                reconstruct(*data, goalId);
                return result(true);
            }

            const auto current = point(*data, entry.id);
            std::array<std::int32_t, 8> directionIndices{};
            const auto directionCount = fillDirections(*data, entry.id,
                                                       directionIndices);
            for (std::size_t directionIndex = 0;
                 directionIndex < directionCount; ++directionIndex) {
                const auto& direction = directions[
                    static_cast<std::size_t>(directionIndices[directionIndex])];
                Jump jump;
                if (direction.diagonal)
                    jumpDiagonal(*data, current, direction.dx, direction.dy,
                                 goal, jump);
                else
                    jumpCardinal(*data, current, direction.dx, direction.dy,
                                 goal, jump);
                if (!jump.found)
                    continue;

                const auto id = data->index(jump.x, jump.y);
                touch(id);
                if (closed_[id] != 0)
                    continue;
                const auto stepCost = static_cast<detail::Cost>(jump.steps) *
                    (direction.diagonal ? detail::DiagonalCost : detail::CardinalCost);
                if (g_[entry.id] > std::numeric_limits<detail::Cost>::max() - stepCost)
                    continue;
                const auto tentative = g_[entry.id] + stepCost;
                if (tentative >= g_[id])
                    continue;
                g_[id] = tentative;
                parent_[id] = entry.id;
                closed_[id] = 0;
                push(id, detail::saturatedAdd(tentative,
                                              heuristic({jump.x, jump.y}, goal)));
            }
        }

        return result();
    }

    JumpGridSearchResult find(const JumpGridTopology& topology,
                              JumpGridCell start, JumpGridCell goal) {
        return find(topology.snapshot(), start, goal);
    }

    std::span<const JumpGridCell> path() const noexcept { return path_; }
    std::uint64_t expanded() const noexcept { return expanded_; }
    std::uint64_t scanned() const noexcept { return scanned_; }
    std::uint64_t cost() const noexcept { return cost_; }
    std::size_t storageBytes() const noexcept {
        return g_.capacity() * sizeof(g_[0]) +
            parent_.capacity() * sizeof(parent_[0]) +
            stamp_.capacity() * sizeof(stamp_[0]) +
            closed_.capacity() * sizeof(closed_[0]) +
            touched_.capacity() * sizeof(touched_[0]) +
            heap_.capacity() * sizeof(heap_[0]) +
            path_.capacity() * sizeof(path_[0]);
    }

private:
    struct Direction {
        std::int32_t dx;
        std::int32_t dy;
        bool diagonal;
    };
    struct Jump {
        bool found = false;
        std::int32_t x = 0;
        std::int32_t y = 0;
        std::int32_t steps = 0;
    };
    struct HeapEntry {
        detail::Index id = detail::NoIndex;
        detail::Cost f = 0;
        detail::Cost g = 0;
        std::uint64_t order = 0;
    };

    std::int32_t width_ = 0;
    std::int32_t height_ = 0;
    std::uint32_t epoch_ = 1;
    std::vector<detail::Cost> g_;
    std::vector<detail::Index> parent_;
    std::vector<std::uint32_t> stamp_;
    std::vector<std::uint8_t> closed_;
    std::vector<detail::Index> touched_;
    std::vector<HeapEntry> heap_;
    std::vector<JumpGridCell> path_;
    bool found_ = false;
    std::uint64_t expanded_ = 0;
    std::uint64_t scanned_ = 0;
    std::uint64_t cost_ = 0;
    std::uint64_t insertionOrder_ = 0;

    void resetResult() noexcept {
        found_ = false;
        expanded_ = 0;
        scanned_ = 0;
        cost_ = 0;
        path_.clear();
    }

    JumpGridSearchResult result(bool found = false) {
        found_ = found;
        return {found_, std::span<const JumpGridCell>(path_), expanded_,
                scanned_, cost_};
    }

    void ensureCapacity(const detail::JumpGridData& data) {
        const auto cellCount = static_cast<std::size_t>(data.width) *
                               static_cast<std::size_t>(data.height);
        if (width_ == data.width && height_ == data.height &&
            g_.size() == cellCount)
            return;

        width_ = data.width;
        height_ = data.height;
        g_.assign(cellCount, 0);
        parent_.assign(cellCount, detail::NoIndex);
        stamp_.assign(cellCount, 0);
        closed_.assign(cellCount, 0);
        touched_.clear();
        heap_.clear();
        path_.clear();
        epoch_ = 1;
    }

    void beginSearch() {
        heap_.clear();
        touched_.clear();
        insertionOrder_ = 0;
        if (++epoch_ == 0) {
            std::fill(stamp_.begin(), stamp_.end(), 0);
            epoch_ = 1;
        }
    }

    void touch(detail::Index id) {
        if (stamp_[id] == epoch_)
            return;
        stamp_[id] = epoch_;
        g_[id] = std::numeric_limits<detail::Cost>::max();
        parent_[id] = detail::NoIndex;
        closed_[id] = 0;
        touched_.push_back(id);
    }

    static bool less(const HeapEntry& left, const HeapEntry& right) noexcept {
        if (left.f != right.f) return left.f < right.f;
        // Among equal f scores, continue the deeper route first. This keeps
        // open areas narrow and avoids broad equal-cost plateaus around
        // obstacle corners while preserving deterministic insertion order.
        if (left.g != right.g) return left.g > right.g;
        if (left.order != right.order) return left.order < right.order;
        return left.id < right.id;
    }

    static std::int32_t directionIndex(std::int32_t dx,
                                       std::int32_t dy) noexcept {
        if (dx == 1 && dy == 0) return 0;
        if (dx == -1 && dy == 0) return 1;
        if (dx == 0 && dy == 1) return 2;
        if (dx == 0 && dy == -1) return 3;
        if (dx == 1 && dy == 1) return 4;
        if (dx == -1 && dy == 1) return 5;
        if (dx == 1 && dy == -1) return 6;
        if (dx == -1 && dy == -1) return 7;
        return -1;
    }

    std::size_t fillDirections(
        const detail::JumpGridData& data, detail::Index currentId,
        std::array<std::int32_t, 8>& output) const noexcept {
        std::array<std::uint8_t, 8> keep{};
        const auto current = point(data, currentId);
        const auto previousId = parent_[currentId];

        // At the start there is no incoming direction to prune. Keeping the
        // canonical order here also fixes the tie order of the first wave.
        if (previousId == detail::NoIndex) {
            keep.fill(1);
        } else {
            const auto previous = point(data, previousId);
            const auto dx = (current.x > previous.x) -
                            (current.x < previous.x);
            const auto dy = (current.y > previous.y) -
                            (current.y < previous.y);
            const auto incoming = directionIndex(dx, dy);

            // A valid jump parent always lies on one of the eight rays. Keep
            // the conservative all-directions fallback if a malformed or
            // externally corrupted parent ever reaches this helper.
            if (incoming < 0) {
                keep.fill(1);
            } else if (dx != 0 && dy != 0) {
                // A diagonal arrival has three natural successors: continue
                // diagonally or take either cardinal component. With strict
                // no-corner-cutting, diagonal motion creates no forced
                // neighbours because both rear side cells were open.
                keep[static_cast<std::size_t>(incoming)] = 1;
                keep[static_cast<std::size_t>(directionIndex(dx, 0))] = 1;
                keep[static_cast<std::size_t>(directionIndex(0, dy))] = 1;
            } else {
                // A cardinal arrival naturally continues straight. A side
                // wall that ends at this cell forces both ways into the newly
                // exposed side: the side cardinal and its forward diagonal.
                keep[static_cast<std::size_t>(incoming)] = 1;
                if (dx != 0) {
                    if (data.walkable(current.x,
                                      static_cast<std::int64_t>(current.y) + 1) &&
                        !data.walkable(static_cast<std::int64_t>(current.x) - dx,
                                       static_cast<std::int64_t>(current.y) + 1)) {
                        keep[static_cast<std::size_t>(directionIndex(0, 1))] = 1;
                        keep[static_cast<std::size_t>(directionIndex(dx, 1))] = 1;
                    }
                    if (data.walkable(current.x,
                                      static_cast<std::int64_t>(current.y) - 1) &&
                        !data.walkable(static_cast<std::int64_t>(current.x) - dx,
                                       static_cast<std::int64_t>(current.y) - 1)) {
                        keep[static_cast<std::size_t>(directionIndex(0, -1))] = 1;
                        keep[static_cast<std::size_t>(directionIndex(dx, -1))] = 1;
                    }
                } else {
                    if (data.walkable(static_cast<std::int64_t>(current.x) + 1,
                                      current.y) &&
                        !data.walkable(static_cast<std::int64_t>(current.x) + 1,
                                       static_cast<std::int64_t>(current.y) - dy)) {
                        keep[static_cast<std::size_t>(directionIndex(1, 0))] = 1;
                        keep[static_cast<std::size_t>(directionIndex(1, dy))] = 1;
                    }
                    if (data.walkable(static_cast<std::int64_t>(current.x) - 1,
                                      current.y) &&
                        !data.walkable(static_cast<std::int64_t>(current.x) - 1,
                                       static_cast<std::int64_t>(current.y) - dy)) {
                        keep[static_cast<std::size_t>(directionIndex(-1, 0))] = 1;
                        keep[static_cast<std::size_t>(directionIndex(-1, dy))] = 1;
                    }
                }
            }
        }

        std::size_t count = 0;
        for (std::size_t i = 0; i < keep.size(); ++i) {
            if (keep[i] != 0)
                output[count++] = static_cast<std::int32_t>(i);
        }
        return count;
    }

    void push(detail::Index id, detail::Cost f) {
        HeapEntry entry{id, f, g_[id], insertionOrder_++};
        heap_.push_back(entry);
        std::size_t at = heap_.size() - 1;
        while (at != 0) {
            const auto parent = (at - 1) / 2;
            if (!less(heap_[at], heap_[parent]))
                break;
            std::swap(heap_[at], heap_[parent]);
            at = parent;
        }
    }

    HeapEntry pop() {
        if (heap_.empty())
            return {};
        const auto result = heap_.front();
        heap_.front() = heap_.back();
        heap_.pop_back();
        std::size_t at = 0;
        while (true) {
            const auto left = at * 2 + 1;
            if (left >= heap_.size())
                break;
            auto best = left;
            const auto right = left + 1;
            if (right < heap_.size() && less(heap_[right], heap_[left]))
                best = right;
            if (!less(heap_[best], heap_[at]))
                break;
            std::swap(heap_[at], heap_[best]);
            at = best;
        }
        return result;
    }

    static JumpGridCell point(const detail::JumpGridData& data,
                              detail::Index id) noexcept {
        const auto width = static_cast<std::size_t>(data.width);
        return {static_cast<std::int32_t>(id % width),
                static_cast<std::int32_t>(id / width)};
    }

    static detail::Cost heuristic(JumpGridCell from,
                                  JumpGridCell to) noexcept {
        const auto dx = from.x > to.x
            ? static_cast<std::uint64_t>(from.x) - static_cast<std::uint64_t>(to.x)
            : static_cast<std::uint64_t>(to.x) - static_cast<std::uint64_t>(from.x);
        const auto dy = from.y > to.y
            ? static_cast<std::uint64_t>(from.y) - static_cast<std::uint64_t>(to.y)
            : static_cast<std::uint64_t>(to.y) - static_cast<std::uint64_t>(from.y);
        const auto diagonal = std::min(dx, dy);
        const auto cardinal = std::max(dx, dy) - diagonal;
        return cardinal * detail::CardinalCost + diagonal * detail::DiagonalCost;
    }

    void jumpCardinal(const detail::JumpGridData& data, JumpGridCell from,
                      std::int32_t dx, std::int32_t dy, JumpGridCell goal,
                      Jump& out) {
        ++scanned_;
        const auto dir = directionIndex(dx, dy);
        const auto encoded = data.cardinal[static_cast<std::size_t>(dir)]
            [data.index(from.x, from.y)];
        const auto maxTravel = encoded < 0 ? -static_cast<std::int64_t>(encoded)
                                           : static_cast<std::int64_t>(encoded);
        const auto goalDx = static_cast<std::int64_t>(goal.x) - from.x;
        const auto goalDy = static_cast<std::int64_t>(goal.y) - from.y;
        const bool onRay = (dy == 0 && goal.y == from.y &&
                            ((goalDx > 0) - (goalDx < 0)) == dx) ||
                           (dx == 0 && goal.x == from.x &&
                            ((goalDy > 0) - (goalDy < 0)) == dy);
        if (onRay) {
            const auto distance = dx != 0 ? (goalDx < 0 ? -goalDx : goalDx)
                                          : (goalDy < 0 ? -goalDy : goalDy);
            if (distance > 0 && distance <= maxTravel) {
                out = {true, goal.x, goal.y, static_cast<std::int32_t>(distance)};
                return;
            }
        }
        if (encoded > 0) {
            out = {true,
                   static_cast<std::int32_t>(static_cast<std::int64_t>(from.x) +
                                              static_cast<std::int64_t>(dx) * encoded),
                   static_cast<std::int32_t>(static_cast<std::int64_t>(from.y) +
                                              static_cast<std::int64_t>(dy) * encoded),
                   encoded};
        }
    }

    void jumpDiagonal(const detail::JumpGridData& data, JumpGridCell from,
                      std::int32_t dx, std::int32_t dy, JumpGridCell goal,
                      Jump& out) {
        auto x = from.x;
        auto y = from.y;
        std::int32_t steps = 0;
        const auto horizontalDir = directionIndex(dx, 0);
        const auto verticalDir = directionIndex(0, dy);
        while (true) {
            ++scanned_;
            const auto nextX = static_cast<std::int64_t>(x) + dx;
            const auto nextY = static_cast<std::int64_t>(y) + dy;
            if (!data.walkable(nextX, y) || !data.walkable(x, nextY) ||
                !data.walkable(nextX, nextY))
                return;
            x = static_cast<std::int32_t>(nextX);
            y = static_cast<std::int32_t>(nextY);
            ++steps;
            if (x == goal.x && y == goal.y) {
                out = {true, x, y, steps};
                return;
            }

            ++scanned_;
            const auto horizontal = data.cardinal[static_cast<std::size_t>(horizontalDir)]
                [data.index(x, y)];
            if (horizontal > 0 ||
                (y == goal.y && ((static_cast<std::int64_t>(goal.x) - x > 0) -
                                  (static_cast<std::int64_t>(goal.x) - x < 0)) == dx &&
                 std::llabs(static_cast<long long>(goal.x) - x) <=
                     (horizontal < 0 ? -static_cast<std::int64_t>(horizontal)
                                     : static_cast<std::int64_t>(horizontal)))) {
                out = {true, x, y, steps};
                return;
            }

            ++scanned_;
            const auto vertical = data.cardinal[static_cast<std::size_t>(verticalDir)]
                [data.index(x, y)];
            if (vertical > 0 ||
                (x == goal.x && ((static_cast<std::int64_t>(goal.y) - y > 0) -
                                  (static_cast<std::int64_t>(goal.y) - y < 0)) == dy &&
                 std::llabs(static_cast<long long>(goal.y) - y) <=
                     (vertical < 0 ? -static_cast<std::int64_t>(vertical)
                                   : static_cast<std::int64_t>(vertical)))) {
                out = {true, x, y, steps};
                return;
            }
        }
    }

    void reconstruct(const detail::JumpGridData& data, detail::Index goalId) {
        path_.clear();
        auto id = goalId;
        while (id != detail::NoIndex) {
            path_.push_back(point(data, id));
            if (parent_[id] == detail::NoIndex)
                break;
            id = parent_[id];
            if (path_.size() > touched_.size()) {
                path_.clear();
                return;
            }
        }
        std::reverse(path_.begin(), path_.end());
        cost_ = g_[goalId];
    }
};

} // namespace navigation
}
