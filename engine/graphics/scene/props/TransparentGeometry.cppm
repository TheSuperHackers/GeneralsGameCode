module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Scene.Props.TransparentGeometry;
export import Graphics.Scene.Props.Renderer;

namespace Graphics {
// Deferred triangle ordering across material batches. Textures remain owned by
// the caller until Flush or Clear; each batch retains its mesh version.
export class TransparentGeometry final {
public:
    bool Submit(PropRenderer& renderer, std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices, const PropStyle& style,
        const PropParameters& parameters, std::span<const RHITextureHandle> textures,
        const std::array<float,4>& camera_depth)
    {
        const auto mesh = renderer.Create_Mesh(vertices,indices);
        if (!mesh.Is_Valid()) return false;
        const bool submitted = Submit(renderer,mesh,style,parameters,textures,camera_depth);
        renderer.Destroy_Mesh(mesh);
        return submitted;
    }

    bool Submit(PropRenderer& renderer, PropMeshHandle mesh, const PropStyle& style,
        const PropParameters& parameters, std::span<const RHITextureHandle> textures,
        const std::array<float,4>& camera_depth, PropInstanceHandle instance = {})
    {
        if (textures.size()>PropTextureCount) return false;
        for (float value : camera_depth) if (!std::isfinite(value)) return false;
        const auto* record=renderer.Instances().Resolve(instance);
        if (instance.Is_Valid() && !record) return false;
        const auto& world=record ? record->world : parameters.world;
        // Evaluate camera depth directly in the geometry's local space.
        std::array<float,4> local_depth{};
        for (unsigned column=0;column<4;++column)
            for (unsigned row=0;row<4;++row)
                local_depth[column]+=camera_depth[row]*world[row*4+column];
        for (float value : local_depth) if (!std::isfinite(value)) return false;
        const auto* geometry = renderer.Mesh_Geometry(mesh);
        if (geometry == nullptr || !renderer.Retain_Mesh(mesh)) return false;
        if (instance.Is_Valid() && !renderer.Instances().Retain(instance)) {
            renderer.Destroy_Mesh(mesh); return false;
        }
        const auto pose=renderer.Instances().Pose(instance);
        if (!pose.empty() && geometry->Maximum_Bone_Index()>=pose.size()) {
            renderer.Instances().Release(instance); renderer.Destroy_Mesh(mesh); return false;
        }
        const auto vertices = geometry->Vertices();
        const auto indices = geometry->Indices();
        const auto batch_index=m_batches.size();
        Batch batch{mesh,style,parameters};
        batch.instance=instance;
        std::copy(textures.begin(),textures.end(),batch.textures.begin());
        m_batches.push_back(batch);
        const auto append = [&]<bool Skinned>() {
            for (std::size_t index=0; index<indices.size(); index+=3) {
                float depth=0;
                for (std::size_t corner=0; corner<3; ++corner) {
                    const auto& vertex=vertices[indices[index+corner]];
                    auto position=vertex.position;
                    if constexpr (Skinned)
                        position=Transform_Prop_Skin_Position(position,pose[static_cast<std::uint32_t>(vertex.bone_index)]);
                    depth+=position[0]*local_depth[0]+position[1]*local_depth[1]
                        +position[2]*local_depth[2]+local_depth[3];
                }
                m_triangles.push_back({batch_index,static_cast<std::uint32_t>(index),depth/3});
            }
        };
        if (pose.empty()) append.template operator()<false>();
        else append.template operator()<true>();
        return true;
    }

    bool Flush(PropRenderer& renderer, CommandList& commands)
    {
        // Camera space looks down -Z. Stable ties retain submission order.
        std::stable_sort(m_triangles.begin(),m_triangles.end(),
            [](const Triangle& left,const Triangle& right) { return left.depth<right.depth; });
        bool success=true;
        for (std::size_t i=0; i<m_triangles.size();) {
            const auto triangle=m_triangles[i];
            const auto& batch=m_batches[triangle.batch];
            std::uint32_t count=3;
            ++i;
            while (i<m_triangles.size() && m_triangles[i].batch==triangle.batch
                && m_triangles[i].first==triangle.first+count) { count+=3; ++i; }
            const auto instance=batch.instance.Get_Index();
            const auto records=batch.instance.Is_Valid() ? std::span(&instance,1) : std::span<const std::uint32_t>{};
            if (!renderer.Draw_Range(commands,batch.mesh,batch.style,batch.parameters,
                batch.textures,triangle.first,count,{},records)) success=false;
        }
        Clear(renderer);
        return success;
    }

    void Clear(PropRenderer& renderer) noexcept
    {
        for (const auto& batch : m_batches) {
            renderer.Instances().Release(batch.instance);
            renderer.Destroy_Mesh(batch.mesh);
        }
        m_batches.clear(); m_triangles.clear();
    }
    bool Empty() const noexcept { return m_triangles.empty(); }
private:
    struct Batch {
        PropMeshHandle mesh;
        PropStyle style;
        PropParameters parameters;
        std::array<RHITextureHandle,PropTextureCount> textures{};
        PropInstanceHandle instance{};
    };
    struct Triangle { std::size_t batch; std::uint32_t first; float depth; };
    std::vector<Batch> m_batches;
    std::vector<Triangle> m_triangles;
};
}
