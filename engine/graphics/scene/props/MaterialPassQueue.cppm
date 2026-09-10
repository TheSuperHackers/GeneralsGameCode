module;
#include <algorithm>
#include <array>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Props.MaterialPassQueue;
export import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.BatchBounds;
namespace Graphics {
// Procedural passes whose base geometry is omitted execute after the opaque
// depth pass. Retain submission order; these are not transparency-sorted draws.
export class MaterialPassQueue final {
public:
    MaterialPassQueue()=default;
    MaterialPassQueue(const MaterialPassQueue&)=delete;
    MaterialPassQueue& operator=(const MaterialPassQueue&)=delete;
    void Set_Viewport(RHIViewport viewport) noexcept { m_viewport=viewport; }
    bool Submit(PropRenderer& renderer,std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices,const PropStyle& style,
        const PropParameters& parameters,std::span<const RHITextureHandle> textures)
    {
        const auto mesh=renderer.Create_Mesh(vertices,indices);
        if (!mesh.Is_Valid()) return false;
        const bool submitted = Submit(renderer,mesh,style,parameters,textures);
        renderer.Destroy_Mesh(mesh);
        return submitted;
    }
    bool Submit(PropRenderer& renderer,PropMeshHandle mesh,const PropStyle& style,
        const PropParameters& parameters,std::span<const RHITextureHandle> textures,
        PropInstanceHandle instance = {})
    {
        if (textures.size()>PropTextureCount || (m_renderer && m_renderer!=&renderer)) return false;
        if (instance.Is_Valid() && !renderer.Valid_Instance(mesh,instance)) return false;
        if (m_instances.size()>=(std::numeric_limits<std::uint32_t>::max)()/sizeof(std::uint32_t)) return false;
        std::array<RHITextureHandle,PropTextureCount> bindings{};
        std::copy(textures.begin(),textures.end(),bindings.begin());
        const bool temporary = !instance.Is_Valid();
        if (temporary) instance = renderer.Instances().Update({},parameters);
        else if (!renderer.Instances().Retain(instance)) return false;
        // Instancing retains index order within each instance and instance
        // order within this run, including deferred alpha/additive passes.
        // Immediate-draw eligibility is decided separately by the submission.
        if (!m_group_boundary && !m_batches.empty()) {
            auto& last = m_batches.back();
            if (last.mesh == mesh && last.style == style && last.textures == bindings
                && last.parameters.Matches(parameters)) {
                Append(last,instance);
                return true;
            }
        }
        // Only the opaque queue enables this search. Preserve all explicit
        // groups, and never cross an intervening draw with overlapping pixels.
        if (!m_group_boundary && m_groups.empty() && m_viewport.width && m_viewport.height
            && Can_Batch_Opaque(style) && m_batches.size()>1) {
            const auto first=m_batches.size()>32 ? m_batches.size()-32 : 0;
            for (auto index=m_batches.size()-1; index>first;) {
                auto& candidate=m_batches[--index];
                if (candidate.mesh!=mesh || candidate.style!=style || candidate.textures!=bindings
                    || !candidate.parameters.Matches(parameters)) continue;
                const auto incoming=Instance_Bounds(renderer,mesh,instance,parameters.view_projection);
                bool disjoint=incoming.known;
                for (auto intervening=index+1; disjoint && intervening<m_batches.size(); ++intervening)
                    disjoint=incoming.Disjoint(Bounds(m_batches[intervening]));
                if (disjoint) {
                    Append(candidate,instance);
                    return true;
                }
                // Earlier candidates cross the same conflicting draw.
                break;
            }
        }
        if (!renderer.Retain_Mesh(mesh)) { renderer.Instances().Release(instance); return false; }
        Batch batch{mesh,style,PropSharedParameters(parameters),bindings};
        Append(batch,instance);
        m_batches.push_back(batch); m_renderer=&renderer; m_group_boundary=false;
        return true;
    }
    static bool Can_Batch_Opaque(const PropStyle& style) noexcept {
        return style.source_blend == RHIBlendFactor::One && style.destination_blend == RHIBlendFactor::Zero
            && style.depth_write && !style.stencil.enabled;
    }
    bool Flush(CommandList& commands)
    {
        bool success=true;
        for (const auto& batch : m_batches)
            if (!Draw_Batch(commands,batch)) success=false;
        Clear();
        return success;
    }
    void Begin_Group() { m_groups.push_back(m_batches.size()); m_group_boundary=true; }
    bool Flush_Groups_Reversed(CommandList& commands)
    {
        if (m_groups.empty()) return Flush(commands);
        bool success = true;
        std::size_t end = m_batches.size();
        for (auto group = m_groups.rbegin(); group != m_groups.rend(); ++group) {
            for (auto index = *group; index < end; ++index) {
                const auto& batch = m_batches[index];
                if (!Draw_Batch(commands,batch)) success = false;
            }
            end = *group;
        }
        // Batches submitted before the first explicit group form the oldest group.
        for (std::size_t index=0; index<end; ++index) {
            const auto& batch = m_batches[index];
            if (!Draw_Batch(commands,batch)) success = false;
        }
        Clear();
        return success;
    }
    void Clear() noexcept
    {
        for (const auto& batch : m_batches) m_renderer->Destroy_Mesh(batch.mesh);
        for (const auto& instance : m_instances) m_renderer->Instances().Release(instance.handle);
        m_instances.clear();
        m_batches.clear(); m_indices.clear(); m_groups.clear(); m_renderer=nullptr; m_group_boundary=false;
    }
    bool Empty() const noexcept { return m_batches.empty(); }
private:
    struct Batch {
        PropMeshHandle mesh;
        PropStyle style;
        PropSharedParameters parameters;
        std::array<RHITextureHandle,PropTextureCount> textures{};
        std::uint32_t first=0,last=0,count=0;
        PropBatchBounds bounds;
        bool bounds_ready=false;
    };
    struct Instance { PropInstanceHandle handle; std::uint32_t next=0; };
    void Append(Batch& batch,PropInstanceHandle instance) {
        const auto index=static_cast<std::uint32_t>(m_instances.size());
        if (batch.count==0) batch.first=index;
        else m_instances[batch.last].next=index;
        m_instances.push_back({instance});
        batch.last=index; ++batch.count; batch.bounds_ready=false;
    }
    PropBatchBounds Instance_Bounds(PropRenderer& renderer,PropMeshHandle mesh,PropInstanceHandle instance,
        const std::array<float,16>& view_projection) const noexcept {
        const auto* record=renderer.Instances().Resolve(instance);
        // Animated instances still batch adjacently. Their conservative posed
        // bounds are used for shadows; this ordering proof covers rigid draws.
        if (!record || record->skin[1]!=0) return {};
        return Project_Prop_Batch_Bounds(*renderer.Mesh_Geometry(mesh),record->world,view_projection,m_viewport);
    }
    const PropBatchBounds& Bounds(Batch& batch) {
        if (batch.bounds_ready) return batch.bounds;
        auto index=batch.first;
        batch.bounds=Instance_Bounds(*m_renderer,batch.mesh,m_instances[index].handle,batch.parameters.view.view_projection);
        for (std::uint32_t count=1; batch.bounds.known && count<batch.count; ++count) {
            index=m_instances[index].next;
            batch.bounds.Include(Instance_Bounds(*m_renderer,batch.mesh,m_instances[index].handle,batch.parameters.view.view_projection));
        }
        batch.bounds_ready=true;
        return batch.bounds;
    }
    bool Draw_Batch(CommandList& commands, const Batch& batch) {
        m_indices.clear();
        auto index=batch.first;
        for (std::uint32_t count=0; count<batch.count; ++count) {
            m_indices.push_back(m_instances[index].handle.Get_Index());
            index=m_instances[index].next;
        }
        return m_renderer->Draw_Records(commands,batch.mesh,batch.style,batch.parameters,batch.textures,
            m_indices);
    }
    std::vector<Instance> m_instances;
    std::vector<std::uint32_t> m_indices;
    RHIViewport m_viewport{};
    bool m_group_boundary=false;
    // Explicitly clear at frame cancellation or renderer shutdown. Texture
    // ownership remains with the caller until this queue has flushed/cleared.
    PropRenderer* m_renderer=nullptr;
    std::vector<Batch> m_batches;
    std::vector<std::size_t> m_groups;
};
}
