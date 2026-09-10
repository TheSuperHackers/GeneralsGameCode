module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <numeric>
#include <span>
#include <vector>

export module Graphics.Scene.Shadows.DirectionalRenderer;
export import Graphics.Scene.Shadows;
export import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Lighting.Environment;
import Graphics.Passes.Shadow;
import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{
export struct ShadowCasterTag;
export using ShadowCasterHandle = ResourceHandle<ShadowCasterTag>;
// Casters are extracted once with the same geometry, UVs, and opacity as their
// visible material. Imported textures remain owned by the submitting scene
// until Render completes. No gameplay object or asset-format type crosses here.
export class DirectionalShadowRenderer final
{
public:
    DirectionalShadowRenderer() = default;
    DirectionalShadowRenderer(const DirectionalShadowRenderer&) = delete;
    DirectionalShadowRenderer& operator=(const DirectionalShadowRenderer&) = delete;
    ~DirectionalShadowRenderer() { Shutdown(); }
    bool Initialize(Device& device,const std::filesystem::path& shaders)
    {
        if (m_device == &device) return true;
        Shutdown();
        if (!m_renderer.Initialize(device,shaders)) return false;
        m_device = &device;
        return true;
    }

    void Clear_Casters() noexcept
    {
        for (const auto& caster : m_casters)
            if (caster.source != nullptr) {
                caster.source->Instances().Release(caster.instance);
                caster.source->Destroy_Mesh(caster.source_mesh);
            }
        m_casters.clear();
        m_caster_order.clear(); m_batches.clear(); m_instance_worlds.clear(); m_instance_indices.clear();
        if (m_transient_mesh.Is_Valid()) m_renderer.Update_Mesh(m_transient_mesh,{},{});
        m_transient_index_count = 0;
    }

    ShadowCasterHandle Create_Caster(std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        if (m_device == nullptr) return {};
        const auto mesh = m_renderer.Create_Mesh(vertices,indices);
        if (!mesh.Is_Valid()) return {};
        Mesh geometry;
        geometry.handle = mesh;
        if (!vertices.empty()) {
            geometry.minimum = geometry.maximum = vertices.front().position;
            for (const auto& vertex : vertices) for (std::size_t axis=0;axis<3;++axis) {
                geometry.minimum[axis] = std::min(geometry.minimum[axis],vertex.position[axis]);
                geometry.maximum[axis] = std::max(geometry.maximum[axis],vertex.position[axis]);
            }
        }
        return m_meshes.Create(geometry);
    }

    bool Is_Caster_Valid(ShadowCasterHandle handle) const noexcept
    {
        return m_meshes.Resolve(handle) != nullptr;
    }

    bool Destroy_Caster(ShadowCasterHandle handle) noexcept
    {
        const auto* mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr) return false;
        m_renderer.Destroy_Mesh(mesh->handle);
        return m_meshes.Destroy(handle);
    }

    void Shutdown() noexcept
    {
        Clear_Casters();
        if (m_transient_mesh.Is_Valid()) m_renderer.Destroy_Mesh(m_transient_mesh);
        m_transient_mesh = {};
        m_meshes.For_Each([&](ShadowCasterHandle,const Mesh& mesh) {
            m_renderer.Destroy_Mesh(mesh.handle);
        });
        m_meshes.Clear();
        if (m_device != nullptr) {
            auto& environment = Get_Environment_Lighting();
            environment.parameters.shadow_options[0] = 0;
            environment.shadow_textures = {};
            m_maps.Shutdown(*m_device);
        }
        m_renderer.Shutdown();
        m_graph = {};
        m_plan = {};
        m_device = nullptr;
    }

    bool Add_Caster(std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices, PropParameters parameters,
        std::span<const RHITextureHandle> textures, const PropStyle& material_style = {})
    {
        if (m_device == nullptr || textures.size() > PropTextureCount) return false;
        if (!m_transient_mesh.Is_Valid()) m_transient_mesh = m_renderer.Create_Mesh({},{});
        if (!m_transient_mesh.Is_Valid() || !m_renderer.Append_Mesh(m_transient_mesh,vertices,indices)) return false;
        auto caster = Make_Caster(parameters,textures,material_style);
        caster.first_index = m_transient_index_count;
        caster.index_count = static_cast<std::uint32_t>(indices.size());
        m_transient_index_count += caster.index_count;
        if (!vertices.empty()) {
            caster.bounds.minimum = caster.bounds.maximum = vertices.front().position;
            for (const auto& vertex : vertices) for (std::size_t axis=0;axis<3;++axis) {
                caster.bounds.minimum[axis] = std::min(caster.bounds.minimum[axis],vertex.position[axis]);
                caster.bounds.maximum[axis] = std::max(caster.bounds.maximum[axis],vertex.position[axis]);
            }
        }
        caster.bounds = Transform_Bounds(caster.bounds,parameters.world);
        m_casters.push_back(caster);
        return true;
    }

    // Retained geometry remains valid across frame-list clears. Its owner
    // releases it when geometry changes, before this renderer is destroyed.
    bool Add_Caster(ShadowCasterHandle mesh,PropParameters parameters,
        std::span<const RHITextureHandle> textures,const PropStyle& material_style = {})
    {
        if (!Is_Caster_Valid(mesh) || textures.size() > PropTextureCount) return false;
        auto caster = Make_Caster(parameters,textures,material_style);
        caster.mesh = mesh;
        caster.bounds = Transform_Bounds(*m_meshes.Resolve(mesh),parameters.world);
        m_casters.push_back(caster);
        return true;
    }

    // Share the exact mesh version used by color/reflection passes. Retain it
    // through all cascades even if its source publishes new geometry meanwhile.
    bool Add_Caster(PropRenderer& source,PropMeshHandle mesh,PropParameters parameters,
        std::span<const RHITextureHandle> textures,const PropStyle& material_style = {}, PropInstanceHandle instance = {})
    {
        if (m_device == nullptr || textures.size() > PropTextureCount) return false;
        const auto* geometry = source.Mesh_Geometry(mesh);
        if (geometry == nullptr || !source.Retain_Mesh(mesh)) return false;
        auto caster = Make_Caster(parameters,textures,material_style);
        caster.source = &source;
        caster.source_mesh = mesh;
        if (!instance.Is_Valid()) instance=source.Instances().Update({},parameters);
        else if (!source.Instances().Retain(instance)) { source.Destroy_Mesh(mesh); return false; }
        caster.instance=instance;
        {
            GRAPHICS_PROFILE_SCOPE("Graphics.Shadows.MeshBounds");
            if (!source.Mesh_Bounds(mesh,instance,caster.bounds.minimum,caster.bounds.maximum)) {
                source.Instances().Release(instance); source.Destroy_Mesh(mesh); return false;
            }
        }
        caster.bounds = Transform_Bounds(caster.bounds,source.Instances().Resolve(instance)->world);
        m_casters.push_back(caster);
        return true;
    }

    bool Render(CommandList& commands,const View& view,const RenderLight& light,
        const ShadowSettings& settings,RHITextureHandle color_target,
        RHITextureHandle depth_target,RHIViewport viewport)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Shadows.Render");
        if (m_device == nullptr || !color_target.Is_Valid() || !depth_target.Is_Valid()) return false;
        ShadowCascades cascades;
        if (!Build_Shadow_Cascades(view,LightHandle(0,1),light,settings,cascades)) return false;
        if (!Prepare_Maps(settings)) return false;
        if (!Prepare_Batches()) return false;
        auto& environment = Get_Environment_Lighting();
        const auto saved = environment;
        environment.parameters.shadow_options[0] = 0;
        environment.parameters.cloud_offset_strength[3] = 0;
        environment.parameters.clip_plane = {};
        const bool rendered = m_plan.Execute(m_graph,commands,
            [&](GraphPassHandle pass,CommandList& list,const PassResources& resources) {
                std::uint32_t cascade = 0;
                while (cascade<cascades.count && m_passes[cascade]!=pass) ++cascade;
                if (cascade == cascades.count) return false;
                if (!list.Set_Depth_Target(resources.Texture(m_maps.Target(cascade)))
                    || !list.Set_Viewport({0,0,settings.map_size,settings.map_size})
                    || !list.Clear_Depth(1)) return false;
                const auto planes = Cascade_Planes(cascades.views[cascade].view_projection);
                for (const auto& batch : m_batches) {
                    const auto& first = m_casters[m_caster_order[batch.first]];
                    std::size_t instance_count = 0;
                    for (auto index=batch.first; index<batch.end; ++index) {
                        const auto& caster = m_casters[m_caster_order[index]];
                        if (Intersects_Cascade(caster.bounds,planes)) {
                            m_instance_worlds[instance_count] = caster.parameters.world;
                            m_instance_indices[instance_count] = caster.instance.Get_Index();
                            ++instance_count;
                        }
                    }
                    if (instance_count == 0) continue;
                    auto parameters = first.parameters;
                    parameters.view_projection = cascades.views[cascade].view_projection.values;
                    parameters.world = m_instance_worlds.front();
                    const auto records = first.source ? std::span<const std::uint32_t>(m_instance_indices.data(),instance_count)
                        : std::span<const std::uint32_t>{};
                    const auto worlds = first.source || instance_count == 1 ? std::span<const std::array<float,16>>{}
                        : std::span<const std::array<float,16>>(m_instance_worlds.data(),instance_count);
                    if (!batch.renderer->Draw_Range(list,batch.mesh,first.style,parameters,
                        std::span(first.textures.data(),first.texture_count),batch.first_index,batch.index_count,worlds,records)) return false;
                }
                return true;
            });
        environment = saved;
        // Restoration is attempted even if a caster submission failed.
        const bool restored = commands.Set_Render_Targets(color_target,depth_target)
            && commands.Set_Viewport(viewport);
        environment.parameters.shadow_options[0] = 0;
        if (!rendered || !restored) return false;
        for (std::uint32_t cascade=0;cascade<cascades.count;++cascade) {
            environment.parameters.shadow_view_projection[cascade] = cascades.views[cascade].view_projection.values;
            environment.parameters.shadow_splits[cascade] = cascades.views[cascade].split_far;
            environment.shadow_textures[cascade] = m_maps.Texture(cascade);
        }
        for (std::size_t column=0;column<4;++column)
            environment.parameters.shadow_view_depth[column] = -view.view_matrix(2,column);
        environment.parameters.shadow_options[0] = static_cast<float>(cascades.count);
        return true;
    }

private:
    struct Mesh final
    {
        PropMeshHandle handle{};
        std::array<float,3> minimum{};
        std::array<float,3> maximum{};
    };

    static Mesh Transform_Bounds(const Mesh& local,const std::array<float,16>& world) noexcept
    {
        Mesh result;
        result.handle = local.handle;
        for (unsigned corner=0;corner<8;++corner) {
            const std::array<float,3> point{
                corner&1 ? local.maximum[0] : local.minimum[0],
                corner&2 ? local.maximum[1] : local.minimum[1],
                corner&4 ? local.maximum[2] : local.minimum[2]};
            for (unsigned axis=0;axis<3;++axis) {
                const auto offset=axis*4;
                const auto value=world[offset]*point[0]+world[offset+1]*point[1]
                    +world[offset+2]*point[2]+world[offset+3];
                if (corner==0) result.minimum[axis]=result.maximum[axis]=value;
                else {
                    result.minimum[axis]=std::min(result.minimum[axis],value);
                    result.maximum[axis]=std::max(result.maximum[axis],value);
                }
            }
        }
        return result;
    }

    static std::array<std::array<float,4>,6> Cascade_Planes(const Matrix4x4& matrix) noexcept
    {
        std::array<std::array<float,4>,6> planes{};
        for (std::size_t column=0;column<4;++column) {
            planes[0][column] = matrix(3,column) + matrix(0,column);
            planes[1][column] = matrix(3,column) - matrix(0,column);
            planes[2][column] = matrix(3,column) + matrix(1,column);
            planes[3][column] = matrix(3,column) - matrix(1,column);
            planes[4][column] = matrix(2,column);
            planes[5][column] = matrix(3,column) - matrix(2,column);
        }
        return planes;
    }

    static bool Intersects_Cascade(const Mesh& mesh,
        const std::array<std::array<float,4>,6>& planes) noexcept
    {
        // Test the padded light volume, never the main camera frustum. Only
        // reject when the entire box is outside one plane. This also retains
        // geometry crossing a volume with every vertex outside that volume.
        for (const auto& plane : planes) {
            float distance = plane[3];
            float magnitude = std::abs(plane[3]);
            for (std::size_t axis=0;axis<3;++axis) {
                const float support = plane[axis] >= 0 ? mesh.maximum[axis] : mesh.minimum[axis];
                const float term = plane[axis] * support;
                distance += term;
                magnitude += std::abs(term);
            }
            // Expand at the boundary to account for floating-point rounding.
            if (distance < -0.00001f * (1.0f + magnitude)) return false;
        }
        return true;
    }

    struct Caster final
    {
        PropRenderer* source = nullptr;
        PropMeshHandle source_mesh{};
        PropInstanceHandle instance{};
        ShadowCasterHandle mesh{};
        Mesh bounds;
        std::uint32_t first_index = 0;
        std::uint32_t index_count = 0;
        PropParameters parameters;
        PropStyle style;
        std::array<RHITextureHandle,PropTextureCount> textures{};
        std::uint32_t texture_count = 0;
    };

    struct Batch final {
        PropRenderer* renderer;
        PropMeshHandle mesh;
        std::uint32_t first_index, index_count;
        std::size_t first, end;
    };

    bool Prepare_Batches()
    {
        // Every caster uses LessEqual depth writes with color and stencil
        // writes disabled. Equal-depth fragments therefore commute as well.
        // Group retained geometry once, before any cascade visibility loops.
        m_caster_order.resize(m_casters.size());
        std::iota(m_caster_order.begin(),m_caster_order.end(),std::size_t{0});
        const auto less = [&](std::size_t a, std::size_t b) {
            const auto& left = m_casters[a]; const auto& right = m_casters[b];
            if (left.source != right.source) return std::less<PropRenderer*>{}(left.source,right.source);
            if (left.source_mesh.Get_Index() != right.source_mesh.Get_Index())
                return left.source_mesh.Get_Index() < right.source_mesh.Get_Index();
            if (left.source_mesh.Get_Generation() != right.source_mesh.Get_Generation())
                return left.source_mesh.Get_Generation() < right.source_mesh.Get_Generation();
            if (left.mesh.Get_Index() != right.mesh.Get_Index()) return left.mesh.Get_Index() < right.mesh.Get_Index();
            if (left.mesh.Get_Generation() != right.mesh.Get_Generation()) return left.mesh.Get_Generation() < right.mesh.Get_Generation();
            return left.first_index < right.first_index;
        };
        std::sort(m_caster_order.begin(),m_caster_order.end(),less);
        m_batches.clear();
        m_batches.reserve(m_casters.size());
        m_instance_worlds.resize(m_casters.size());
        m_instance_indices.resize(m_casters.size());
        for (std::size_t first=0; first<m_caster_order.size();) {
            const auto& caster = m_casters[m_caster_order[first]];
            assert(caster.style.color_write_mask == 0 && !caster.style.stencil.enabled);
            auto* renderer = caster.source != nullptr ? caster.source : &m_renderer;
            auto mesh = caster.source_mesh;
            std::uint32_t first_index = 0, index_count = 0;
            if (caster.source == nullptr) {
                if (caster.mesh.Is_Valid()) {
                    const auto* stored = m_meshes.Resolve(caster.mesh);
                    if (stored == nullptr) return false;
                    mesh = stored->handle;
                } else {
                    mesh = m_transient_mesh;
                    first_index = caster.first_index; index_count = caster.index_count;
                }
            }
            if (caster.source != nullptr || caster.mesh.Is_Valid()) {
                const auto* geometry = renderer->Mesh_Geometry(mesh);
                if (geometry == nullptr) return false;
                index_count = static_cast<std::uint32_t>(geometry->Indices().size());
            }
            auto end = first+1;
            while (end < m_caster_order.size()) {
                const auto& next = m_casters[m_caster_order[end]];
                if (next.source != caster.source || next.source_mesh != caster.source_mesh || next.mesh != caster.mesh
                    || next.first_index != caster.first_index || next.index_count != caster.index_count
                    || next.style != caster.style || next.texture_count != caster.texture_count || next.textures != caster.textures
                    || !Same_Depth_Parameters(next.parameters,caster.parameters)) break;
                ++end;
            }
            m_batches.push_back({renderer,mesh,first_index,index_count,first,end});
            first = end;
        }
        return true;
    }

    static bool Same_Depth_Parameters(const PropParameters& left,const PropParameters& right) noexcept
    {
        // Local lights and ambient affect RGB only. Preserve all view, alpha,
        // mapping and material inputs; the caster pipeline masks every color write.
        constexpr auto lighting = offsetof(PropParameters,scene_ambient);
        constexpr auto material = offsetof(PropParameters,textured);
        constexpr auto world = offsetof(PropParameters,world);
        return std::memcmp(&left,&right,lighting) == 0
            && std::memcmp(reinterpret_cast<const std::byte*>(&left)+material,
                reinterpret_cast<const std::byte*>(&right)+material,world-material) == 0;
    }

    static Caster Make_Caster(PropParameters parameters,std::span<const RHITextureHandle> textures,
        const PropStyle& material_style)
    {
        Caster caster;
        caster.parameters = parameters;
        caster.parameters.shroud = caster.parameters.shroud_only = 0;
        caster.parameters.fog_state = {};
        caster.style = material_style;
        caster.style.blend = RHIBlendMode::Disabled;
        caster.style.source_blend = RHIBlendFactor::One;
        caster.style.destination_blend = RHIBlendFactor::Zero;
        caster.style.depth_test = caster.style.depth_write = true;
        caster.style.depth_comparison = RHIComparison::LessEqual;
        caster.style.color_write_mask = 0;
        caster.style.stencil = {};
        caster.style.wireframe = false;
        caster.style.cull = RHICullMode::None;
        caster.style.depth_bias = 0;
        caster.texture_count = static_cast<std::uint32_t>(textures.size());
        for (std::size_t index=0;index<textures.size();++index) caster.textures[index] = textures[index];
        return caster;
    }

    bool Prepare_Maps(const ShadowSettings& settings)
    {
        if (m_maps.Count() == settings.cascade_count && m_maps.Map_Size() == settings.map_size && m_plan.Is_Valid()) return true;
        Get_Environment_Lighting().parameters.shadow_options[0] = 0;
        Get_Environment_Lighting().shadow_textures = {};
        m_maps.Shutdown(*m_device);
        m_graph = {};
        m_plan = {};
        if (!m_maps.Initialize(*m_device,m_graph,settings.cascade_count,settings.map_size)
            || !ShadowPass::Add_Cascades_To_Graph(m_graph,m_maps,m_passes,20)) return false;
        for (std::uint32_t cascade=0;cascade<settings.cascade_count;++cascade)
            m_bindings[cascade] = GraphResourceBinding::Texture(m_maps.Target(cascade),m_maps.Texture(cascade));
        return m_plan.Compile(m_graph,std::span(m_bindings.data(),settings.cascade_count));
    }

    Device* m_device = nullptr;
    PropRenderer m_renderer;
    ResourcePool<Mesh,ShadowCasterHandle> m_meshes;
    PropMeshHandle m_transient_mesh{};
    std::uint32_t m_transient_index_count = 0;
    ShadowMapResources m_maps;
    RenderGraph m_graph;
    ExecutionPlan m_plan;
    std::array<GraphPassHandle,4> m_passes{};
    std::array<GraphResourceBinding,4> m_bindings{};
    std::vector<Caster> m_casters;
    std::vector<std::size_t> m_caster_order;
    std::vector<Batch> m_batches;
    std::vector<std::array<float,16>> m_instance_worlds;
    std::vector<std::uint32_t> m_instance_indices;
};

export DirectionalShadowRenderer& Get_Directional_Shadow_Renderer() noexcept
{
    static DirectionalShadowRenderer renderer;
    return renderer;
}
}
