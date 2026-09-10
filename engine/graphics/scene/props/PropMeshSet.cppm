module;
#include "../../profiling/Tracy.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Scene.Props.MeshSet;
export import Graphics.Scene.Props.Renderer;

namespace Graphics {
// A nonzero revision promises immutable contents for this source version.
// Zero retains exact-content checks for externally writable source storage.
export struct PropPreparationInput final {
    std::span<const std::byte> bytes;
    std::uint64_t revision = 0;
};
// A source owner keeps one version of each material batch. Compare actual
// contents while adapters still expose mutable arrays without revisions.
// Neither pointer identity nor a frame number proves that geometry is unchanged.
export class PropMeshSet final {
public:
    PropMeshSet() = default;
    PropMeshSet(const PropMeshSet&) = delete;
    PropMeshSet& operator=(const PropMeshSet&) = delete;
    ~PropMeshSet() { Clear(); }

    PropMeshHandle Synchronize(PropRenderer& renderer, std::size_t batch,
        std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        if (batch < m_sources.size()) m_sources[batch] = {};
        return Synchronize_Geometry(renderer,batch,vertices,indices);
    }

    // Input spans describe every value used to build this batch. Own their
    // contents after publication: writable arrays can change at the same address
    // between draws, including two views or instances in the same frame.
    PropMeshHandle Find_Prepared(const PropRenderer& renderer, std::size_t batch,
        std::span<const std::span<const std::byte>> sources) const noexcept
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.FindPrepared");
        if (m_renderer != &renderer || sources.empty() || batch >= m_sources.size()
            || batch >= m_meshes.size()) return {};
        const auto& saved = m_sources[batch];
        if (saved.sizes.size() != sources.size() || !saved.revisions.empty()) return {};
        std::size_t offset = 0;
        for (std::size_t i=0; i<sources.size(); ++i) {
            if (saved.sizes[i] != sources[i].size()
                || (!sources[i].empty() && std::memcmp(saved.bytes.data()+offset,sources[i].data(),sources[i].size()) != 0))
                return {};
            offset += sources[i].size();
        }
        return renderer.Mesh_Geometry(m_meshes[batch]) != nullptr ? m_meshes[batch] : PropMeshHandle{};
    }

    PropMeshHandle Publish_Prepared(PropRenderer& renderer, std::size_t batch,
        std::span<const std::span<const std::byte>> sources,
        std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.PublishPrepared");
        // Allocate the snapshot before replacing geometry. Rejected geometry or
        // allocation failure leaves the previously published version intact.
        PreparedSources saved;
        saved.sizes.reserve(sources.size());
        std::size_t byte_count = 0;
        for (const auto source : sources) {
            if (source.size() > saved.bytes.max_size()-byte_count) return {};
            byte_count += source.size();
            saved.sizes.push_back(source.size());
        }
        saved.bytes.reserve(byte_count);
        for (const auto source : sources)
            if (!source.empty()) saved.bytes.insert(saved.bytes.end(),source.begin(),source.end());
        if (batch >= m_sources.size()) m_sources.resize(batch+1);
        const auto mesh = Synchronize_Geometry(renderer,batch,vertices,indices);
        if (mesh.Is_Valid()) m_sources[batch] = std::move(saved);
        return mesh;
    }

    PropMeshHandle Find_Versioned(const PropRenderer& renderer, std::size_t batch,
        std::span<const PropPreparationInput> sources) const noexcept
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.FindPrepared");
        if (m_renderer != &renderer || sources.empty() || batch >= m_sources.size()
            || batch >= m_meshes.size()) return {};
        const auto& saved = m_sources[batch];
        if (saved.revisions.size() != sources.size()) return {};
        std::size_t offset = 0;
        for (std::size_t i=0; i<sources.size(); ++i) {
            const auto& source = sources[i];
            if (saved.sizes[i] != source.bytes.size()) return {};
            if (source.revision != 0) {
                if (saved.revisions[i] != source.revision || saved.addresses[i] != source.bytes.data()) return {};
            } else {
                if (saved.revisions[i] != 0
                    || (!source.bytes.empty() && std::memcmp(saved.bytes.data()+offset,
                        source.bytes.data(),source.bytes.size()) != 0)) return {};
                offset += source.bytes.size();
            }
        }
        return renderer.Mesh_Geometry(m_meshes[batch]) != nullptr ? m_meshes[batch] : PropMeshHandle{};
    }

    PropMeshHandle Publish_Versioned(PropRenderer& renderer, std::size_t batch,
        std::span<const PropPreparationInput> sources,
        std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.PublishPrepared");
        PreparedSources saved;
        saved.sizes.reserve(sources.size());
        saved.revisions.reserve(sources.size());
        saved.addresses.reserve(sources.size());
        std::size_t byte_count = 0;
        for (const auto& source : sources) {
            if (source.revision == 0) {
                if (source.bytes.size() > saved.bytes.max_size()-byte_count) return {};
                byte_count += source.bytes.size();
            }
            saved.sizes.push_back(source.bytes.size());
            saved.revisions.push_back(source.revision);
            saved.addresses.push_back(source.bytes.data());
        }
        saved.bytes.reserve(byte_count);
        for (const auto& source : sources)
            if (source.revision == 0 && !source.bytes.empty())
                saved.bytes.insert(saved.bytes.end(),source.bytes.begin(),source.bytes.end());
        if (batch >= m_sources.size()) m_sources.resize(batch+1);
        const auto mesh = Synchronize_Geometry(renderer,batch,vertices,indices);
        if (mesh.Is_Valid()) m_sources[batch] = std::move(saved);
        return mesh;
    }

private:
    PropMeshHandle Synchronize_Geometry(PropRenderer& renderer, std::size_t batch,
        std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        if (m_renderer != nullptr && m_renderer != &renderer) return {};
        if (batch >= m_meshes.size()) m_meshes.resize(batch+1);
        const auto old = m_meshes[batch];
        const auto* geometry = renderer.Mesh_Geometry(old);
        if (geometry != nullptr && geometry->Matches(vertices,indices)) return old;
        const auto replacement = renderer.Create_Mesh(vertices,indices);
        if (!replacement.Is_Valid()) return {};
        m_renderer = &renderer;
        m_meshes[batch] = replacement;
        if (old.Is_Valid()) renderer.Destroy_Mesh(old);
        return replacement;
    }

public:
    void Clear() noexcept
    {
        if (m_renderer != nullptr)
            for (const auto mesh : m_meshes) m_renderer->Destroy_Mesh(mesh);
        m_meshes.clear();
        m_sources.clear();
        m_renderer = nullptr;
    }
private:
    struct PreparedSources final {
        std::vector<std::size_t> sizes;
        std::vector<std::byte> bytes;
        std::vector<std::uint64_t> revisions;
        std::vector<const std::byte*> addresses;
    };
    // The source owner releases its set before the renderer is destroyed.
    PropRenderer* m_renderer = nullptr;
    std::vector<PropMeshHandle> m_meshes;
    std::vector<PreparedSources> m_sources;
};
}
