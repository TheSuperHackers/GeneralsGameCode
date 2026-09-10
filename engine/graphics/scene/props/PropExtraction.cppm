module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Props.Extraction;
export import Graphics.Scene.Props.Geometry;

namespace Graphics
{
// Transformed geometry is shared by a mesh's material batches. Material and
// texture attributes are populated only when a batch references a vertex.
export struct PropSourceVertex final
{
    std::array<float,3> position{};
    std::array<float,3> normal{0,0,1};

    inline PropVertex Make_Vertex() const noexcept
    {
        PropVertex vertex;
        vertex.position = position;
        vertex.normal = normal;
        return vertex;
    }
};
static_assert(sizeof(PropSourceVertex) == 24);

export class PropExtractionWorkspace final
{
public:
    std::span<PropSourceVertex> Prepare_Source(std::size_t count)
    {
        m_source.assign(count,PropSourceVertex{});
        return m_source;
    }

    // Populate an owned snapshot once, in source order. The callback writes
    // both position and normal, including defaults for absent source channels.
    template<class Populate>
    std::span<PropSourceVertex> Prepare_Source(std::size_t count, Populate&& populate)
    {
        if (m_source.size() < count) m_source.resize(count);
        for (std::size_t index=0;index<count;++index) populate(m_source[index],index);
        return std::span(m_source).first(count);
    }

    PropBatchBuilder& Batch() noexcept { return m_batch; }

    std::size_t Allocated_Bytes() const noexcept
    {
        return sizeof(*this)+m_source.capacity()*sizeof(PropSourceVertex)+m_batch.Allocated_Bytes();
    }

private:
    std::vector<PropSourceVertex> m_source;
    PropBatchBuilder m_batch;
};

// Render-thread scratch storage. Each active extraction owns a distinct
// workspace, including nested extractions. Only idle storage is budgeted;
// exhaustion of the cache never limits mesh size or the number of leases.
export class PropExtractionCache final
{
public:
    PropExtractionCache() = default;
    PropExtractionCache(const PropExtractionCache&) = delete;
    PropExtractionCache& operator=(const PropExtractionCache&) = delete;

    class Lease final
    {
    public:
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;
        Lease(Lease&& other) noexcept
            : m_owner(other.m_owner),m_generation(other.m_generation),m_workspace(std::move(other.m_workspace)) {}
        ~Lease()
        {
            if (m_workspace) m_owner->Release(m_generation,std::move(m_workspace));
        }
        PropExtractionWorkspace& Workspace() noexcept { return *m_workspace; }

    private:
        friend class PropExtractionCache;
        Lease(PropExtractionCache& owner,std::unique_ptr<PropExtractionWorkspace> workspace) noexcept
            : m_owner(&owner),m_generation(owner.m_generation),m_workspace(std::move(workspace)) {}
        PropExtractionCache* m_owner;
        std::uint64_t m_generation;
        std::unique_ptr<PropExtractionWorkspace> m_workspace;
    };

    Lease Acquire()
    {
        if (m_idle.empty()) return Lease(*this,std::make_unique<PropExtractionWorkspace>());
        auto workspace = std::move(m_idle.back());
        m_idle.pop_back();
        m_idle_bytes -= workspace->Allocated_Bytes();
        return Lease(*this,std::move(workspace));
    }

    // Call before releasing the application's allocator. Outstanding leases
    // stay usable, but their old storage will not re-enter the cleared cache.
    void Clear() noexcept
    {
        ++m_generation;
        std::vector<std::unique_ptr<PropExtractionWorkspace>>{}.swap(m_idle);
        m_idle_bytes = 0;
    }

private:
    void Release(std::uint64_t generation,std::unique_ptr<PropExtractionWorkspace> workspace) noexcept
    {
        constexpr std::size_t maximum_idle_bytes = 64u*1024u*1024u;
        const auto bytes = workspace->Allocated_Bytes();
        if (generation != m_generation || bytes > maximum_idle_bytes-m_idle_bytes) return;
        try {
            m_idle.push_back(std::move(workspace));
            m_idle_bytes += bytes;
        } catch (...) {
            // Failure to cache idle storage must not make a completed draw fail.
        }
    }

    std::uint64_t m_generation = 0;
    std::size_t m_idle_bytes = 0;
    std::vector<std::unique_ptr<PropExtractionWorkspace>> m_idle;
};

export PropExtractionCache& Get_Prop_Extraction_Cache() noexcept
{
    static PropExtractionCache cache;
    return cache;
}
}
