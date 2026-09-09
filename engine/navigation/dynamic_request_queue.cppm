module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

export module engine.navigation.dynamic_request_queue;

export extern "C++" {
namespace navigation {

// Shared deterministic simulation policy, not an adaptive wall-clock limit.
// Measured with 10,000-query batches; see BENCHMARKS.md for hardware and costs.
inline constexpr unsigned int CellsPerFrame = 50000;

// Growable FIFO plus an open-addressed membership table. No per-request nodes,
// no fixed unit-count limit, and no linear scan to suppress duplicate requests.
class PathRequestQueue {
    using Id = std::uint32_t;
    static constexpr std::uint64_t Deleted = std::numeric_limits<std::uint64_t>::max();
    std::vector<Id> ring_;
    std::vector<std::uint64_t> membership_;
    std::size_t head_ = 0;
    std::size_t count_ = 0;
    std::size_t deleted_ = 0;

    static std::uint32_t hash(Id value) {
        value ^= value >> 16;
        value *= 0x7feb352dU;
        value ^= value >> 15;
        value *= 0x846ca68bU;
        return value ^ (value >> 16);
    }
    std::size_t find(Id id) const {
        const auto key = std::uint64_t(id) + 1;
        auto position = std::size_t(hash(id)) & (membership_.size() - 1);
        while (membership_[position] && membership_[position] != key)
            position = (position + 1) & (membership_.size() - 1);
        return position;
    }
    void insertMember(Id id) {
        auto position = std::size_t(hash(id)) & (membership_.size() - 1);
        while (membership_[position] && membership_[position] != Deleted)
            position = (position + 1) & (membership_.size() - 1);
        if (membership_[position] == Deleted) --deleted_;
        membership_[position] = std::uint64_t(id) + 1;
    }
    void rehash(std::size_t size) {
        membership_.assign(size, 0);
        deleted_ = 0;
        for (std::size_t i = 0; i < count_; ++i) insertMember(at(i));
    }
    void growRing() {
        std::vector<Id> next(std::max<std::size_t>(64, ring_.size() * 2));
        for (std::size_t i = 0; i < count_; ++i) next[i] = at(i);
        ring_.swap(next);
        head_ = 0;
    }
public:
    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    Id at(std::size_t offset) const {
        assert(offset < count_);
        return ring_[(head_ + offset) & (ring_.size() - 1)];
    }
    bool contains(Id id) const {
        return !membership_.empty() && membership_[find(id)] != 0;
    }
    bool push(Id id) {
        if (contains(id)) return false; // Already pending; preserve original order.
        if (membership_.empty()) rehash(128);
        else if ((count_ + deleted_ + 1) * 10 >= membership_.size() * 7)
            rehash(count_ * 10 >= membership_.size() * 4 ? membership_.size() * 2 : membership_.size());
        if (count_ == ring_.size()) growRing();
        ring_[(head_ + count_) & (ring_.size() - 1)] = id;
        insertMember(id);
        ++count_;
        return true;
    }
    Id pop() {
        assert(!empty());
        const auto id = ring_[head_];
        head_ = (head_ + 1) & (ring_.size() - 1);
        --count_;
        membership_[find(id)] = Deleted;
        ++deleted_;
        return id;
    }
    void clear() {
        head_ = count_ = deleted_ = 0;
        std::fill(membership_.begin(), membership_.end(), 0);
    }
    std::size_t storageBytes() const {
        return ring_.capacity() * sizeof(Id) + membership_.capacity() * sizeof(std::uint64_t);
    }
};

// Deterministic work budget, never a wall-clock deadline. New requests from a
// callback wait until the next batch; zero-work/deleted requests still cost one
// credit so reentrant or trivial work cannot monopolize a frame.
template<class Resolve, class Process>
int processRequests(PathRequestQueue& requests, int& work, Resolve resolve, Process process,
                    int workBudget = int(CellsPerFrame))
{
    int completed = 0;
    const auto batch = requests.size();
    for (std::size_t i = 0; i < batch && !requests.empty() && work < workBudget; ++i) {
        const auto before = work;
        const auto id = requests.pop(); // Remove membership before callbacks can requeue.
        if (auto* object = resolve(id))
            if (process(object)) ++completed;
        if (work <= before) work = before + 1;
    }
    return completed;
}
} // namespace navigation
} // extern "C++"
