module;
#include <cstdint>
#include <functional>
#include <map>
#include <utility>
#include <vector>
export module Graphics.Scene.OrderedDraws;

namespace Graphics {

// Authored layers run from back to front (higher values first). Within a
// layer, submission order is significant for alpha blending. Entries retain
// their source until extraction and the caller flushes each completed layer.
export class OrderedDrawQueue final {
    struct Entry final {
        void* object;
        bool (*draw)(void*, void*);
        void (*release)(void*);

        Entry(void* object, bool (*draw)(void*, void*), void (*release)(void*))
            : object(object), draw(draw), release(release) {}
        Entry(const Entry&) = delete;
        Entry& operator=(const Entry&) = delete;
        Entry(Entry&& other) noexcept
            : object(std::exchange(other.object, nullptr)), draw(other.draw), release(other.release) {}
        ~Entry() { if (object) release(object); }
    };
public:
    OrderedDrawQueue() = default;
    OrderedDrawQueue(const OrderedDrawQueue&) = delete;
    OrderedDrawQueue& operator=(const OrderedDrawQueue&) = delete;

    void Set_Enabled(bool enabled) noexcept { m_enabled = enabled; }
    bool Is_Enabled() const noexcept { return m_enabled && !m_draining; }

    // Draw translates caller-owned source data into renderer submissions. The
    // borrowed context is supplied at drain time, never captured from a draw's
    // temporary stack. Objects use the existing intrusive lifetime contract.
    template<auto Draw, class Object>
    bool Enqueue(std::uint32_t order, Object& object) {
        if (!order || !Is_Enabled()) return false;
        object.Add_Ref();
        Entry entry(&object,
            [](void* source, void* context) { return Draw(*static_cast<Object*>(source), context); },
            [](void* source) { static_cast<Object*>(source)->Release_Ref(); });
        m_layers[order].push_back(std::move(entry));
        return true;
    }

    template<class Flush>
    bool Drain(void* context, Flush&& flush) {
        if (m_draining) return false;
        m_draining = true;
        struct Scope final {
            bool& draining;
            ~Scope() { draining = false; }
        } scope{m_draining};
        for (auto& [order, entries] : m_layers) {
            bool submitted = false;
            for (auto& stored : entries) {
                // Release immediately after extraction, before flushing the
                // layer. The local owner also releases if a callback throws.
                Entry entry(std::move(stored));
                if (entry.object) submitted = entry.draw(entry.object, context) || submitted;
            }
            entries.clear();
            if (submitted) flush();
        }
        return true;
    }

    bool Clear() noexcept {
        if (m_draining) return false;
        for (auto& [order, entries] : m_layers) entries.clear();
        return true;
    }
private:
    // Sparse authored orders and growable per-layer storage impose no object
    // budget. Reuse the allocations on subsequent frames.
    std::map<std::uint32_t, std::vector<Entry>, std::greater<std::uint32_t>> m_layers;
    bool m_enabled = false;
    bool m_draining = false;
};

export OrderedDrawQueue& Get_Scene_Draw_Queue() {
    static OrderedDrawQueue queue;
    return queue;
}
}
