module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

export module engine.navigation.search_workspace;

// C++ linkage permits forward declarations in the remaining game-facing header.
export extern "C++" {
namespace navigation {

using SearchIndex = std::uint32_t;
inline constexpr SearchIndex NoSearchIndex = std::numeric_limits<SearchIndex>::max();

// Priorities and IDs are separate contiguous arrays. Heap comparisons never
// dereference a game cell. The inverse array makes arbitrary removal O(log N).
template<unsigned Arity = 4>
class IndexedMinHeap {
    static_assert(Arity >= 2);
    std::vector<std::uint64_t> keys_;
    std::vector<SearchIndex> ids_;
    std::vector<SearchIndex> positions_;
    std::uint32_t ticket_ = 0;

    std::uint64_t nextKey(std::uint32_t cost) {
        if (ticket_ == std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Navigation insertion sequence overflow");
        return (std::uint64_t(cost) << 32) | ticket_++;
    }

    void place(SearchIndex position, SearchIndex id, std::uint64_t key) {
        ids_[position] = id;
        keys_[position] = key;
        positions_[id] = position;
    }
    void siftUp(SearchIndex position, SearchIndex id, std::uint64_t key) {
        while (position != 0) {
            const SearchIndex parent = (position - 1) / Arity;
            if (keys_[parent] <= key) break;
            place(position, ids_[parent], keys_[parent]);
            position = parent;
        }
        place(position, id, key);
    }
    void siftDown(SearchIndex position, SearchIndex id, std::uint64_t key) {
        for (;;) {
            const std::size_t first = std::size_t(position) * Arity + 1;
            if (first >= ids_.size()) break;
            SearchIndex best = static_cast<SearchIndex>(first);
            const auto end = std::min(first + Arity, ids_.size());
            for (std::size_t child = first + 1; child < end; ++child)
                if (keys_[child] < keys_[best]) best = static_cast<SearchIndex>(child);
            if (key <= keys_[best]) break;
            place(position, ids_[best], keys_[best]);
            position = best;
        }
        place(position, id, key);
    }
public:
    void reserveIndices(std::size_t count) {
        if (count > positions_.size()) positions_.resize(count, NoSearchIndex);
    }
    bool empty() const { return ids_.empty(); }
    std::size_t size() const { return ids_.size(); }
    bool contains(SearchIndex id) const {
        return id < positions_.size() && positions_[id] != NoSearchIndex;
    }
    SearchIndex top() const { return empty() ? NoSearchIndex : ids_.front(); }
    SearchIndex at(std::size_t i) const { return ids_[i]; }
    SearchIndex next(SearchIndex id) const {
        assert(contains(id));
        const auto i = positions_[id] + 1;
        return i < ids_.size() ? ids_[i] : NoSearchIndex;
    }
    void push(SearchIndex id, std::uint32_t cost) {
        assert(!contains(id));
        reserveIndices(std::size_t(id) + 1);
        // A unique insertion rank defines deterministic FIFO ties. Reset per
        // search; never silently wrap and change ordering in an enormous query.
        const auto key = nextKey(cost);
        const auto position = static_cast<SearchIndex>(ids_.size());
        ids_.push_back(id);
        keys_.push_back(key);
        siftUp(position, id, key);
    }
    void erase(SearchIndex id) {
        assert(contains(id));
        const auto position = positions_[id];
        const auto lastId = ids_.back();
        const auto lastKey = keys_.back();
        ids_.pop_back();
        keys_.pop_back();
        positions_[id] = NoSearchIndex;
        if (position == ids_.size()) return;
        if (position && lastKey < keys_[(position - 1) / Arity])
            siftUp(position, lastId, lastKey);
        else
            siftDown(position, lastId, lastKey);
    }
    void update(SearchIndex id, std::uint32_t cost, bool renewInsertionOrder = false) {
        assert(contains(id));
        const auto position = positions_[id];
        const auto oldKey = keys_[position];
        const auto key = renewInsertionOrder ? nextKey(cost) :
            (std::uint64_t(cost) << 32) | std::uint32_t(oldKey);
        if (key < oldKey) siftUp(position, id, key);
        else if (key > oldKey) siftDown(position, id, key);
    }
    SearchIndex pop() {
        const auto id = top();
        if (id != NoSearchIndex) erase(id);
        return id;
    }
    // Cleanup does not need sorted order. Drain in O(N), not N heap removals.
    SearchIndex dropBack() {
        assert(!empty());
        const auto id = ids_.back();
        positions_[id] = NoSearchIndex;
        ids_.pop_back();
        keys_.pop_back();
        return id;
    }
    void clear() {
        while (!empty()) dropBack();
        ticket_ = 0;
    }
    std::size_t storageBytes() const {
        return keys_.capacity() * sizeof(keys_[0]) + ids_.capacity() * sizeof(ids_[0])
             + positions_.capacity() * sizeof(positions_[0]);
    }
};

// Slots grow geometrically and remain valid across vector reallocations. Only
// the cold owner array contains pointers; costs, parents and queue entries use IDs.
template<class Owner>
class SearchWorkspace {
public:
    enum State : std::uint8_t { Unseen, Open, Closed };
    std::vector<std::uint32_t> g, f;
    std::vector<SearchIndex> parent;
    std::vector<std::uint16_t> x, y;
    std::vector<std::uint32_t> generation;
    std::vector<State> state;
    std::vector<Owner*> owner;
    std::vector<std::uint32_t> goalUnit, positionUnit, goalAircraft;
    IndexedMinHeap<4> open;
private:
    std::vector<SearchIndex> free_;
    std::vector<std::uint8_t> allocated_;
    std::vector<SearchIndex> closed_;
    std::vector<SearchIndex> closedPosition_;
    std::uint32_t epoch_ = 1;

public:
    std::size_t capacity() const { return owner.size(); }
    std::uint32_t epoch() const noexcept { return epoch_; }
    std::size_t allocatedCount() const { return capacity() - free_.size(); }
    bool allocated(SearchIndex id) const { return id < capacity() && allocated_[id]; }
    void reserveSlots(std::size_t count) {
        if (count <= capacity()) return;
        if (count >= NoSearchIndex) throw std::length_error("Navigation index space exhausted");
        const auto old = capacity();
        g.resize(count); f.resize(count); parent.resize(count, NoSearchIndex);
        x.resize(count); y.resize(count); generation.resize(count); state.resize(count, Unseen);
        owner.resize(count); goalUnit.resize(count); positionUnit.resize(count); goalAircraft.resize(count);
        allocated_.resize(count); closedPosition_.resize(count, NoSearchIndex);
        open.reserveIndices(count);
        free_.reserve(count);
        for (auto i = count; i > old; --i) free_.push_back(static_cast<SearchIndex>(i - 1));
    }
    SearchIndex acquire(Owner* value, std::uint16_t px, std::uint16_t py) {
        if (free_.empty()) reserveSlots(std::max<std::size_t>(64, capacity() * 2));
        const auto id = free_.back();
        free_.pop_back();
        allocated_[id] = 1;
        owner[id] = value; x[id] = px; y[id] = py;
        goalUnit[id] = positionUnit[id] = goalAircraft[id] = 0;
        generation[id] = 0;
        touch(id);
        return id;
    }
    void release(SearchIndex id) {
        assert(allocated(id) && !isOpen(id) && !isClosed(id));
        allocated_[id] = 0;
        owner[id] = nullptr;
        generation[id] = 0;
        free_.push_back(id);
    }
    void beginSearch() {
        assert(open.empty() && closed_.empty());
        open.clear();
        if (++epoch_ == 0) {
            std::fill(generation.begin(), generation.end(), 0);
            epoch_ = 1;
        }
    }
    void touch(SearchIndex id) {
        assert(allocated(id));
        if (generation[id] == epoch_) return;
        generation[id] = epoch_;
        state[id] = Unseen;
        parent[id] = NoSearchIndex;
        g[id] = f[id] = 0;
    }
    bool isOpen(SearchIndex id) const {
        return generation[id] == epoch_ && state[id] == Open;
    }
    bool isClosed(SearchIndex id) const {
        return generation[id] == epoch_ && state[id] == Closed;
    }
    SearchIndex parentOf(SearchIndex id) const {
        return generation[id] == epoch_ ? parent[id] : NoSearchIndex;
    }
    void pushOpen(SearchIndex id) {
        touch(id);
        if (state[id] == Open) {
            // Preserve the planner's re-insertion tie policy with one sift,
            // without an erase followed by a second heap insertion.
            open.update(id, f[id], true);
            return;
        }
        assert(state[id] == Unseen);
        open.push(id, f[id]);
        state[id] = Open;
    }
    void eraseOpen(SearchIndex id) {
        open.erase(id);
        state[id] = Unseen;
    }
    SearchIndex drainOpen() {
        const auto id = open.dropBack();
        state[id] = Unseen;
        return id;
    }
    void close(SearchIndex id) {
        touch(id);
        assert(state[id] == Unseen);
        closedPosition_[id] = static_cast<SearchIndex>(closed_.size());
        closed_.push_back(id);
        state[id] = Closed;
    }
    void eraseClosed(SearchIndex id) {
        assert(isClosed(id));
        const auto position = closedPosition_[id];
        const auto last = closed_.back();
        closed_[position] = last;
        closedPosition_[last] = position;
        closed_.pop_back();
        closedPosition_[id] = NoSearchIndex;
        state[id] = Unseen;
    }
    SearchIndex closedHead() const { return closed_.empty() ? NoSearchIndex : closed_.back(); }
    SearchIndex nextClosed(SearchIndex id) const {
        assert(isClosed(id));
        const auto position = closedPosition_[id];
        return position == 0 ? NoSearchIndex : closed_[position - 1];
    }
    void releaseStorage() { *this = SearchWorkspace{}; }
    std::size_t storageBytes() const {
        return (g.capacity() + f.capacity() + generation.capacity() + goalUnit.capacity()
              + positionUnit.capacity() + goalAircraft.capacity()) * sizeof(std::uint32_t)
             + (parent.capacity() + free_.capacity() + closed_.capacity() + closedPosition_.capacity()) * sizeof(SearchIndex)
             + (x.capacity() + y.capacity()) * sizeof(std::uint16_t)
             + state.capacity() * sizeof(State) + allocated_.capacity()
             + owner.capacity() * sizeof(Owner*) + open.storageBytes();
    }
};
} // namespace navigation
} // extern "C++"
