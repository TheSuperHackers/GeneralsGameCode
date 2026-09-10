module;
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
export module Graphics.Scene.Dazzles.Layer;

namespace Graphics {
// Membership is independent of draw state: assignment and re-preparation must
// not invalidate the ownership held by a queued layer.
export struct DazzleMembership final { bool queued = false; };
export template<class Owner>
class DazzleLayer final {
    struct Entry final { DazzleMembership* membership; Owner owner; };
public:
    explicit DazzleLayer(std::size_t type_count) : m_types(type_count) {}
    ~DazzleLayer() { Clear(); }
    DazzleLayer(const DazzleLayer&) = delete;
    DazzleLayer& operator=(const DazzleLayer&) = delete;
    template<class Retain>
    void Queue(std::size_t type, DazzleMembership& membership, Retain&& retain) {
        assert(type < m_types.size());
        if (membership.queued) return;
        m_types[type].push_back({&membership, retain()});
        membership.queued = true;
    }
    template<class Draw>
    void Draw_All(Draw&& draw) {
        for (auto& entries : m_types) {
            for (auto entry = entries.rbegin(); entry != entries.rend(); ++entry) draw(entry->owner);
            Clear_Type(entries);
        }
    }
    void Clear() { for (auto& entries : m_types) Clear_Type(entries); }
private:
    static void Clear_Type(std::vector<Entry>& entries) {
        // Match the head-first release order and never access a source after
        // releasing the last reference that keeps its membership alive.
        while (!entries.empty()) {
            entries.back().membership->queued = false;
            entries.pop_back();
        }
    }
    std::vector<std::vector<Entry>> m_types;
};
}
