module;
#include <atomic>
#include <cstdint>
#include <memory>
export module Graphics.Scene.Models.SourceRevision;

namespace Graphics {
// Copies share a mutation domain, just as their source arrays share storage.
// Tokens identify a complete version, not an address or a simulation frame.
// A domain may conservatively cover several arrays that are copied together.
export class SourceRevision final {
    static std::uint64_t New_Token() noexcept
    {
        static std::atomic<std::uint64_t> next{1};
        auto token = next.load(std::memory_order_relaxed);
        while (token != 0) {
            if (next.compare_exchange_weak(token,token+1,std::memory_order_relaxed)) return token;
        }
        return 0; // Exhaustion permanently disables version-based reuse.
    }
    struct State final {
        std::uint64_t token = New_Token();
        bool writable = false;
    };
public:
    SourceRevision() : m_state(std::make_shared<State>()) {}
    SourceRevision(const SourceRevision&) noexcept = default;
    SourceRevision& operator=(const SourceRevision&) noexcept = default;

    std::uint64_t Token() const noexcept { return m_state->writable ? 0 : m_state->token; }

    // Call before a controlled write. Readers must not run concurrently with
    // source mutation; this tracks the existing single-threaded source owner.
    void Invalidate() noexcept { if (!m_state->writable) m_state->token = New_Token(); }

    // A returned writable pointer may outlive this call and subsequent draws.
    // Every alias therefore needs content checks for the rest of this domain.
    void Expose_Writable() noexcept { m_state->writable = true; }

    // Only use when ALL storage represented by this domain has been replaced.
    // Detaching one array from a shared group must keep the old domain instead.
    void Reset() { m_state = std::make_shared<State>(); }
private:
    std::shared_ptr<State> m_state;
};
}
