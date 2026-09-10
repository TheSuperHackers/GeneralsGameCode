module;
#include "../../profiling/Tracy.h"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Props.Renderer;
export import Graphics.Scene.Props.Geometry;
export import Graphics.Scene.Props.Constants;
export import Graphics.Scene.Props.Instances;
export import Graphics.RHI;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.SkinBounds;
import Graphics.Resources.Pools.ResourcePool;
import Graphics.Shaders.Library;
import Graphics.Scene.Lighting.Environment;

namespace Graphics
{
export struct PropMeshTag;
export using PropMeshHandle = ResourceHandle<PropMeshTag>;

export struct PropStyle final
{
    RHIBlendMode blend = RHIBlendMode::Alpha;
    bool depth_write = true;
    bool depth_test = true;
    RHIBlendFactor source_blend = RHIBlendFactor::One;
    RHIBlendFactor destination_blend = RHIBlendFactor::Zero;
    std::array<RHISamplerDescription,2> samplers{};
    RHIComparison depth_comparison = RHIComparison::LessEqual;
    RHICullMode cull = RHICullMode::None;
    bool front_counter_clockwise = false;
    std::uint8_t color_write_mask = 15;
    RHIStencilDescription stencil{};
    std::int32_t depth_bias=0;
    bool wireframe=false;
    bool operator==(const PropStyle &) const = default;
};

export PropStyle Resolve_Prop_Style(PropStyle authored, const SceneDrawParameters& scene) noexcept
{
    authored.color_write_mask &= scene.color_write_mask;
    authored.depth_bias = scene.depth_bias;
    authored.wireframe = scene.wireframe;
    authored.stencil = scene.stencil;
    return authored;
}

struct PropMesh final
{
    PropGeometry geometry;
    PropSkinBounds skin_bounds;
    PropMaterialBinding material;
    PropInstanceBinding instances;
    PropInstanceIndexBinding instance_indices;
    RHIBufferHandle vertices{};
    RHIBufferHandle indices{};
    std::size_t references = 1;
};

struct PropPipeline final
{
    PropStyle style;
    RHIPipelineHandle handle;
    bool instanced = false;
    bool records = false;
    bool shared_lighting = false;
};

// Owns geometry and GPU resources across map and device lifetimes. CPU geometry
// survives a device reset; upload is deferred until the next draw on that device.
export class PropRenderer final
{
public:
    PropRenderer() = default;
    PropRenderer(const PropRenderer &) = delete;
    PropRenderer &operator=(const PropRenderer &) = delete;
    ~PropRenderer() { Shutdown(); }

    bool Initialize(Device &device, const std::filesystem::path &directory)
    {
        if (m_device == &device) return true;
        Shutdown();
        m_shaders = ShaderLibrary{};
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = 20;
        shader.program.fragment_shader = 20;
        shader.program.source_key = 0x50524F504D415431ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory / "prop.vso";
        shader.fragment_path = directory / "prop.pso";
        m_shader = m_shaders.Load_Precompiled(shader);
        if (!m_shaders.Is_Loaded(m_shader)) return false;
        shader.program.vertex_shader = 21;
        shader.program.source_key = 0x50524F50494E5354ull;
        shader.vertex_path = directory / "prop_instanced.vso";
        m_instanced_shader = m_shaders.Load_Precompiled(shader);
        if (!m_shaders.Is_Loaded(m_instanced_shader)) return false;
        shader.program.vertex_shader = shader.program.fragment_shader = 22;
        shader.program.source_key = 0x50524F5052454344ull;
        shader.vertex_path = directory / "prop_records.vso";
        shader.fragment_path = directory / "prop_records.pso";
        m_record_shader = m_shaders.Load_Precompiled(shader);
        if (!m_shaders.Is_Loaded(m_record_shader)) return false;
        m_device = &device;
        if (!m_constants.Initialize(device) || !m_environment.Initialize(device)) { Shutdown(); return false; }
        for (std::size_t index=0; index<m_bindings.size(); ++index)
            m_bindings[index].type = RHIResourceType::Texture;
        return true;
    }

    void Shutdown() noexcept
    {
        if (m_device != nullptr) {
            m_environment.Shutdown(*m_device);
            m_meshes.For_Each([&](PropMeshHandle handle, const PropMesh &) {
                Release_GPU(*m_meshes.Resolve(handle));
            });
            for (const auto &pipeline : m_pipelines) m_device->Destroy_Pipeline(pipeline.handle);
            m_constants.Shutdown(*m_device);
            m_instances.Shutdown(*m_device);
        }
        m_pipelines.clear();
        m_bindings = {};
        m_device = nullptr;
    }

    PropMeshHandle Create_Mesh(std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        PropMesh mesh;
        if (!mesh.geometry.Assign(vertices, indices)) return {};
        return m_meshes.Create(std::move(mesh));
    }

    bool Update_Mesh(PropMeshHandle handle, std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        PropMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr || mesh->references != 1 || !mesh->geometry.Assign(vertices, indices)) return false;
        mesh->skin_bounds.Clear();
        Release_Geometry(*mesh);
        return true;
    }

    bool Destroy_Mesh(PropMeshHandle handle) noexcept
    {
        PropMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr) return false;
        if (mesh->references > 1) { --mesh->references; return true; }
        Release_GPU(*mesh);
        return m_meshes.Destroy(handle);
    }

    bool Append_Mesh(PropMeshHandle handle, std::span<const PropVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        PropMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr || mesh->references != 1 || !mesh->geometry.Append(vertices, indices)) return false;
        mesh->skin_bounds.Clear();
        Release_Geometry(*mesh);
        return true;
    }

    // Deferred consumers own a reference until they draw or cancel. Shared
    // geometry cannot be mutated; publish a replacement version instead.
    bool Retain_Mesh(PropMeshHandle handle) noexcept
    {
        auto* mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr || mesh->references == std::numeric_limits<std::size_t>::max()) return false;
        ++mesh->references;
        return true;
    }

    const PropGeometry* Mesh_Geometry(PropMeshHandle handle) const noexcept
    {
        const auto* mesh = m_meshes.Resolve(handle);
        return mesh == nullptr ? nullptr : &mesh->geometry;
    }

    PropInstances& Instances() noexcept { return m_instances; }
    bool Valid_Instance(PropMeshHandle mesh,PropInstanceHandle instance) const noexcept {
        const auto* geometry=Mesh_Geometry(mesh);
        const auto* record=m_instances.Resolve(instance);
        return geometry && record && (record->skin[1]==0 || geometry->Maximum_Bone_Index()<record->skin[1]);
    }
    std::uint64_t Geometry_Uploaded_Bytes() const noexcept { return m_geometry_uploaded_bytes; }
    bool Mesh_Bounds(PropMeshHandle handle,PropInstanceHandle instance,
        std::array<float,3>& minimum,std::array<float,3>& maximum) {
        auto* mesh=m_meshes.Resolve(handle);
        return mesh && mesh->skin_bounds.Evaluate(mesh->geometry,m_instances.Pose(instance),minimum,maximum);
    }
    bool Draw_Record(CommandList& commands,PropMeshHandle mesh,const PropStyle& style,
        const PropParameters& parameters,std::span<const RHITextureHandle> textures,PropInstanceHandle instance) {
        if (!Valid_Instance(mesh,instance)) return false;
        const auto index=instance.Get_Index();
        return Draw_Records(commands,mesh,style,parameters,textures,std::span(&index,1));
    }

    bool Draw(CommandList &commands, PropMeshHandle handle, const PropStyle &style,
        const PropParameters &parameters, std::span<const RHITextureHandle> textures)
    {
        const PropMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr) return false;
        return Draw_Range(commands, handle, style, parameters, textures, 0,
            static_cast<std::uint32_t>(mesh->geometry.Indices().size()));
    }

    bool Draw_Range(CommandList &commands, PropMeshHandle handle, const PropStyle &style,
        const PropParameters &parameters, std::span<const RHITextureHandle> textures,
        std::uint32_t first_index, std::uint32_t index_count,
        std::span<const std::array<float,16>> worlds = {},
        std::span<const std::uint32_t> records = {})
    {
        return Draw_Range_With_Constants(commands,handle,style,parameters,textures,first_index,index_count,worlds,records);
    }

private:
    static const PropParameters& Material_Parameters(const PropParameters& parameters) noexcept { return parameters; }
    static const PropMaterialConstants& Material_Parameters(const PropSharedParameters& parameters) noexcept { return parameters.material; }

    template<class Parameters>
    bool Draw_Range_With_Constants(CommandList &commands, PropMeshHandle handle, const PropStyle &style,
        const Parameters &parameters, std::span<const RHITextureHandle> textures,
        std::uint32_t first_index, std::uint32_t index_count,
        std::span<const std::array<float,16>> worlds,
        std::span<const std::uint32_t> records)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.Draw");
        PropMesh *mesh = m_meshes.Resolve(handle);
        if (m_device == nullptr || mesh == nullptr || textures.size() > PropTextureCount) return false;
        if ((!worlds.empty() && !records.empty())
            || records.size()>(std::numeric_limits<std::uint32_t>::max)()/sizeof(std::uint32_t)) return false;
        const auto size = mesh->geometry.Indices().size();
        if (first_index > size || index_count > size - first_index || index_count % 3 != 0) return false;
        if (index_count == 0) return true;
        const auto has_texture = [&](std::size_t slot) {
            return slot < textures.size() && textures[slot].Is_Valid();
        };
        const auto& material = Material_Parameters(parameters);
        if ((material.textured > 0.5f && !has_texture(0))
            || (material.secondary_texture > 0.5f && !has_texture(1))
            || ((material.shroud > 0.5f || material.shroud_only > 0.5f) && !has_texture(3))) return false;
        if (material.surface.shading_model > .5f)
            for (std::size_t role=0; role<PropSurfaceTextureCount; ++role)
                if ((material.surface.maps & (1u << role)) != 0 && !has_texture(PropSurfaceTextureFirst+role)) return false;
        if (!Upload(*mesh)) return false;
        if (worlds.size() > std::numeric_limits<std::uint32_t>::max() / 64u) return false;
        const bool instanced = !worlds.empty();
        if (!records.empty() && (!m_instances.Prepare(*m_device)
            || !mesh->instance_indices.Prepare(*m_device,records))) return false;
        if (instanced && !mesh->instances.Prepare(*m_device,worlds)) return false;
        const bool shared_lighting=records.size()==1;
        const RHIPipelineHandle pipeline = Pipeline(style, instanced, !records.empty(),shared_lighting);
        if (!pipeline.Is_Valid()) return false;
        {
            GRAPHICS_PROFILE_SCOPE("Graphics.Props.BindResources");
            if (!m_constants.Prepare_Resources(*m_device, parameters, mesh->material,
                std::span(m_bindings).first<4>(), !records.empty())) return false;
            if (shared_lighting && !m_constants.Prepare_Lighting(*m_device,m_instances.At_Index(records.front()).lighting)) return false;
            std::size_t count = 4;
            for (std::size_t slot = 0; slot < textures.size(); ++slot) {
                if (!textures[slot].Is_Valid()) continue;
                m_bindings[count] = {};
                m_bindings[count].type = RHIResourceType::Texture;
                m_bindings[count].index = ResourceIndex{static_cast<std::uint32_t>(slot), 1};
                m_bindings[count++].texture = textures[slot];
            }
            if (instanced) {
                m_bindings[count] = {};
                m_bindings[count].type = RHIResourceType::Buffer;
                m_bindings[count++].buffer = mesh->instances.Buffer();
            }
            if (!records.empty()) {
                m_bindings[count] = {};
                m_bindings[count].type = RHIResourceType::Buffer;
                m_bindings[count++].buffer = m_instances.Buffer();
                m_bindings[count] = {};
                m_bindings[count].type = RHIResourceType::Buffer;
                m_bindings[count++].buffer = mesh->instance_indices.Buffer();
                m_bindings[count] = {};
                m_bindings[count].type = RHIResourceType::Buffer;
                m_bindings[count++].buffer = m_instances.Palettes().Buffer();
            }
            unsigned environment_count=0;
            if (!m_environment.Prepare_Resources(*m_device, std::span(m_bindings).subspan(count), environment_count)) return false;
            count += environment_count;
            return commands.Bind_Pipeline(pipeline)
                && commands.Set_Bindless_Resources(std::span(m_bindings.data(), count))
                && commands.Set_Vertex_Buffer(0, mesh->vertices, sizeof(PropVertex), 0)
                && commands.Set_Index_Buffer(mesh->indices, RHIIndexFormat::UInt32, 0)
                && commands.Draw_Indexed(index_count, first_index, 0, !records.empty() ? static_cast<std::uint32_t>(records.size())
                    : instanced ? static_cast<std::uint32_t>(worlds.size()) : 1);
        }
    }

public:
    bool Draw_Instances(CommandList& commands, PropMeshHandle mesh, const PropStyle& style,
        const PropParameters& parameters, std::span<const RHITextureHandle> textures,
        std::span<const std::array<float,16>> worlds)
    {
        const auto* geometry = Mesh_Geometry(mesh);
        if (!geometry || worlds.empty() || worlds.size() > std::numeric_limits<std::uint32_t>::max() / 64u) return false;
        return Draw_Range(commands, mesh, style, parameters, textures, 0,
            static_cast<std::uint32_t>(geometry->Indices().size()), worlds);
    }

    bool Draw_Records(CommandList& commands, PropMeshHandle mesh, const PropStyle& style,
        const PropParameters& parameters, std::span<const RHITextureHandle> textures,
        std::span<const std::uint32_t> records)
    {
        const auto* geometry = Mesh_Geometry(mesh);
        if (!geometry || records.empty() || records.size() > (std::numeric_limits<std::uint32_t>::max)()) return false;
        return Draw_Range(commands,mesh,style,parameters,textures,0,
            static_cast<std::uint32_t>(geometry->Indices().size()),{},records);
    }

    bool Draw_Records(CommandList& commands, PropMeshHandle mesh, const PropStyle& style,
        const PropSharedParameters& parameters, std::span<const RHITextureHandle> textures,
        std::span<const std::uint32_t> records)
    {
        const auto* geometry = Mesh_Geometry(mesh);
        if (!geometry || records.empty() || records.size() > (std::numeric_limits<std::uint32_t>::max)()) return false;
        return Draw_Range_With_Constants(commands,mesh,style,parameters,textures,0,
            static_cast<std::uint32_t>(geometry->Indices().size()),{},records);
    }

private:
    void Release_GPU(PropMesh &mesh) noexcept
    {
        Release_Geometry(mesh);
        if (m_device != nullptr) mesh.material.Shutdown(*m_device);
        if (m_device != nullptr) mesh.instances.Shutdown(*m_device);
        if (m_device != nullptr) mesh.instance_indices.Shutdown(*m_device);
    }

    void Release_Geometry(PropMesh &mesh) noexcept
    {
        if (m_device != nullptr) {
            if (mesh.vertices.Is_Valid()) m_device->Destroy_Buffer(mesh.vertices);
            if (mesh.indices.Is_Valid()) m_device->Destroy_Buffer(mesh.indices);
        }
        mesh.vertices = {};
        mesh.indices = {};
    }

    bool Upload(PropMesh &mesh)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.Upload");
        if (mesh.vertices.Is_Valid() && mesh.indices.Is_Valid()) return true;
        const auto vertices = std::as_bytes(mesh.geometry.Vertices());
        const auto indices = std::as_bytes(mesh.geometry.Indices());
        mesh.vertices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(vertices.size()), RHIBufferUsage::Vertex, sizeof(PropVertex),RHIBufferUpdateMode::Discard}, vertices);
        mesh.indices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(indices.size()), RHIBufferUsage::Index, sizeof(std::uint32_t),RHIBufferUpdateMode::Discard}, indices);
        if (!mesh.vertices.Is_Valid() || !mesh.indices.Is_Valid()) {
            Release_Geometry(mesh);
            return false;
        }
        m_geometry_uploaded_bytes += vertices.size()+indices.size();
        return true;
    }

    RHIPipelineHandle Pipeline(const PropStyle &style, bool instanced, bool records, bool shared_lighting)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.Pipeline");
        for (const auto &entry : m_pipelines) if (entry.style == style && entry.instanced == instanced
            && entry.records == records && entry.shared_lighting == shared_lighting) return entry.handle;
        RHIPipeline description;
        description.vertex_format = RHIVertexFormat::Position3Color4UV2UV2Normal3;
        description.vertex_element_count = 12;
        description.vertex_elements[0] = {RHIVertexSemantic::Position,0,RHIVertexElementFormat::Float3,offsetof(PropVertex,position)};
        description.vertex_elements[1] = {RHIVertexSemantic::Color,0,RHIVertexElementFormat::Float4,offsetof(PropVertex,color)};
        description.vertex_elements[2] = {RHIVertexSemantic::TexCoord,0,RHIVertexElementFormat::Float2,offsetof(PropVertex,uv)};
        description.vertex_elements[3] = {RHIVertexSemantic::TexCoord,1,RHIVertexElementFormat::Float2,offsetof(PropVertex,secondary_uv)};
        description.vertex_elements[4] = {RHIVertexSemantic::Normal,0,RHIVertexElementFormat::Float3,offsetof(PropVertex,normal)};
        description.vertex_elements[5] = {RHIVertexSemantic::Color,1,RHIVertexElementFormat::Float4,offsetof(PropVertex,material_ambient)};
        description.vertex_elements[6] = {RHIVertexSemantic::Color,2,RHIVertexElementFormat::Float4,offsetof(PropVertex,material_diffuse)};
        description.vertex_elements[7] = {RHIVertexSemantic::Color,3,RHIVertexElementFormat::Float4,offsetof(PropVertex,material_emissive)};
        description.vertex_elements[8] = {RHIVertexSemantic::Color,4,RHIVertexElementFormat::Float4,offsetof(PropVertex,material_specular)};
        description.vertex_elements[9] = {RHIVertexSemantic::Color,5,RHIVertexElementFormat::Float4,offsetof(PropVertex,secondary_color)};
        description.vertex_elements[10] = {RHIVertexSemantic::TexCoord,2,RHIVertexElementFormat::Float4,offsetof(PropVertex,tangent)};
        description.vertex_elements[11] = {RHIVertexSemantic::TexCoord,3,RHIVertexElementFormat::Float1,offsetof(PropVertex,bone_index)};

        description.blend_mode = style.blend;
        description.depth_write = style.depth_write;
        description.depth_test = style.depth_test;
        description.blend_alpha_like_color = true;
        description.custom_blend_factors = true;
        description.source_blend = style.source_blend;
        description.destination_blend = style.destination_blend;
        description.cull_mode = style.cull;
        description.depth_comparison = style.depth_comparison;
        description.front_counter_clockwise = style.front_counter_clockwise;
        description.color_write_mask = style.color_write_mask;
        description.stencil = style.stencil;
        description.depth_bias=style.depth_bias;
        description.wireframe=style.wireframe;
        description.sampler_count = 16;
        description.samplers[0] = style.samplers[0];
        description.samplers[1] = style.samplers[1];
        for (std::size_t slot=PropSurfaceTextureFirst; slot<PropTextureCount; ++slot)
            description.samplers[slot] = style.samplers[0];
        description.samplers[3].address.fill(RHISamplerAddress::Clamp);
        const RHIPipelineHandle handle = m_device->Create_Pipeline(description,
            {m_shaders.Bytecode(records ? m_record_shader : instanced ? m_instanced_shader : m_shader, ShaderStage::Vertex)},
            {m_shaders.Bytecode(records && !shared_lighting ? m_record_shader : m_shader, ShaderStage::Pixel)});
        if (handle.Is_Valid()) m_pipelines.push_back({style, handle, instanced, records,shared_lighting});
        return handle;
    }

    EnvironmentLightingBinding m_environment;
    Device *m_device = nullptr;
    ShaderLibrary m_shaders;
    ShaderHandle m_shader{}, m_instanced_shader{}, m_record_shader{};
    PropInstances m_instances;
    PropConstantBindings m_constants;
    std::uint64_t m_geometry_uploaded_bytes=0;
    ResourcePool<PropMesh, PropMeshHandle> m_meshes;
    std::vector<PropPipeline> m_pipelines;
    // Borrowed submission scratch. Every used texture entry is overwritten
    // before submission; the command list consumes the supplied span at once.
    std::array<RHIBindlessResource, PropTextureCount+14> m_bindings{};
};

export bool Draw_Prop(PropRenderer& renderer, CommandList& commands, PropMeshHandle mesh,
    const PropStyle& base_style, PropParameters parameters, std::span<const RHITextureHandle> textures,
    bool apply_shroud)
{
    if (!renderer.Draw(commands,mesh,base_style,parameters,textures)) return false;
    if (!apply_shroud) return true;
    PropStyle style = base_style;
    style.source_blend = RHIBlendFactor::Zero;
    style.destination_blend = RHIBlendFactor::SourceColor;
    style.depth_write = false;
    style.cull = RHICullMode::None;
    style.depth_comparison = RHIComparison::Equal;
    parameters.shroud_only = 1;
    parameters.textured = parameters.secondary_texture = 0;
    return renderer.Draw(commands,mesh,style,parameters,textures);
}

namespace { PropRenderer g_prop_renderer; }
export PropRenderer &Get_Prop_Renderer() noexcept { return g_prop_renderer; }
}
