module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

export module engine.navigation.topology.clearance_cache;
import engine.navigation.jump_grid;

export namespace navigation {
// Inclusive, grid-local bounds. Each clearance variant retains its own dirty
// region so preparing one footprint cannot consume another footprint's edits.
struct GridRegion {
    int left = 0, top = 0, right = -1, bottom = -1;
    bool empty() const { return left > right || top > bottom; }
    void include(GridRegion other) {
        if (other.empty()) return;
        if (empty()) { *this = other; return; }
        left = std::min(left, other.left); top = std::min(top, other.top);
        right = std::max(right, other.right); bottom = std::max(bottom, other.bottom);
    }
};

class ClearanceCache {
    struct Variant {
        JumpGridTopology topology;
        std::vector<std::uint8_t> mask;
        std::vector<JumpGridEdit> edits;
        GridRegion dirty;
        bool configured = false;
    };
    std::array<Variant, 6> variants_;
    std::vector<std::uint8_t> base_;
    GridRegion baseDirty_;
    int width_ = 0, height_ = 0;
    std::uint64_t baseCells_ = 0, clearanceCells_ = 0, rebuilds_ = 0;

    GridRegion clipped(GridRegion r) const {
        return {std::max(0, r.left), std::max(0, r.top),
            std::min(width_ - 1, r.right), std::min(height_ - 1, r.bottom)};
    }
public:
    static int variantIndex(int radius, bool center) {
        assert(radius >= 0 && radius <= 2);
        return radius * 2 + (center ? 1 : 0);
    }
    void reset() {
        for (auto& v : variants_) {
            v.mask.clear(); v.edits.clear(); v.dirty = {}; v.configured = false;
        }
        base_.clear(); baseDirty_ = {}; width_ = height_ = 0;
        baseCells_ = clearanceCells_ = rebuilds_ = 0;
    }
    void setShape(int width, int height) {
        assert(width > 0 && height > 0);
        if (width == width_ && height == height_) return;
        for (auto& v : variants_) { v.configured = false; v.mask.clear(); }
        width_ = width; height_ = height;
        base_.resize(static_cast<std::size_t>(width) * height);
        invalidateAll();
    }
    void invalidateAll() { invalidate({0, 0, width_ - 1, height_ - 1}); }
    void invalidate(GridRegion region) {
        region = clipped(region);
        if (region.empty()) return;
        baseDirty_.include(region);
        for (int radius = 0; radius <= 2; ++radius) {
            for (bool center : {false, true}) {
                // Invert [x-radius, x+above] to find affected anchor cells.
                const int above = radius == 0 ? 0 : radius - (center ? 0 : 1);
                variants_[variantIndex(radius, center)].dirty.include(clipped({
                    region.left - above, region.top - above,
                    region.right + radius, region.bottom + radius}));
            }
        }
    }
    template<class ReadBlocked>
    bool prepare(int radius, bool center, ReadBlocked readBlocked) {
        assert(width_ > 0 && height_ > 0);
        for (int y = baseDirty_.top; y <= baseDirty_.bottom; ++y)
            for (int x = baseDirty_.left; x <= baseDirty_.right; ++x) {
                base_[static_cast<std::size_t>(y) * width_ + x] = readBlocked(x, y);
                ++baseCells_;
            }
        baseDirty_ = {};
        auto& v = variants_[variantIndex(radius, center)];
        if (!v.configured) {
            v.mask.resize(base_.size());
            v.dirty = {0, 0, width_ - 1, height_ - 1};
        }
        v.edits.clear();
        const int above = radius == 0 ? 0 : radius - (center ? 0 : 1);
        for (int y = v.dirty.top; y <= v.dirty.bottom; ++y) {
            for (int x = v.dirty.left; x <= v.dirty.right; ++x) {
                bool blocked = false;
                for (int fy = y - radius; fy <= y + above && !blocked; ++fy)
                    for (int fx = x - radius; fx <= x + above; ++fx)
                        if (fx < 0 || fy < 0 || fx >= width_ || fy >= height_ ||
                            base_[static_cast<std::size_t>(fy) * width_ + fx]) {
                            blocked = true; break;
                        }
                const auto index = static_cast<std::size_t>(y) * width_ + x;
                if (v.configured && v.mask[index] != static_cast<std::uint8_t>(blocked))
                    v.edits.push_back({x, y, static_cast<std::uint8_t>(blocked)});
                v.mask[index] = blocked;
                ++clearanceCells_;
            }
        }
        if (!v.configured) {
            if (!v.topology.configure(width_, height_, v.mask)) return false;
            v.configured = true;
            ++rebuilds_;
        } else if (!v.edits.empty()) {
            if (v.topology.applyEdits(v.edits) != v.edits.size() ||
                !v.topology.synchronize()) return false;
            ++rebuilds_;
        }
        v.dirty = {};
        return true;
    }
    auto snapshot(int radius, bool center) const {
        return variants_[variantIndex(radius, center)].topology.snapshot();
    }
    std::uint64_t baseCells() const { return baseCells_; }
    std::uint64_t clearanceCells() const { return clearanceCells_; }
    std::uint64_t rebuilds() const { return rebuilds_; }
};
}
