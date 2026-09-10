module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Surfaces.Renderer;
export import Graphics.Scene.Surfaces.Geometry;
export import Graphics.RHI;
import Graphics.Resources.Pools.ResourcePool;
import Graphics.Shaders.Library;
import Graphics.Scene.Lighting.Environment;

namespace Graphics
{
export struct SurfaceMeshTag;
export using SurfaceMeshHandle = ResourceHandle<SurfaceMeshTag>;

export struct SurfaceParameters final
{
    std::array<float, 16> view_projection{};
    std::array<float, 4> cloud_projection{};
    std::array<float, 4> lightmap_projection{};
    std::array<float, 4> shroud_projection{};
    float alpha_cutoff = 0;
    float textured = 1;
    float cloud = 0;
    float lightmap = 0;
    float masked_modulation = 0;
    float shroud = 0;
    float opacity = 1;
    float shroud_only = 0;
};
static_assert(sizeof(SurfaceParameters) == 144);

export struct SurfaceStyle final
{
    bool blend_alpha_like_color = false;
    RHIBlendMode blend = RHIBlendMode::Alpha;
    bool depth_write = false;
    bool clamp_texture = false;
    bool linear_filter = true;
    RHIComparison depth_comparison = RHIComparison::LessEqual;
    RHICullMode cull = RHICullMode::None;
    bool front_counter_clockwise = false;
    std::uint8_t color_write_mask = 7;
    RHIStencilDescription stencil{};
    bool operator==(const SurfaceStyle &) const = default;
};

struct SurfaceMesh final
{
    SurfaceGeometry geometry;
    RHIBufferHandle vertices{};
    RHIBufferHandle indices{};
};

struct SurfacePipeline final
{
    SurfaceStyle style;
    RHIPipelineHandle handle;
};

// Owns geometry and GPU resources across map and device lifetimes. CPU geometry
// survives a device reset; upload is deferred until the next draw on that device.
export class SurfaceRenderer final
{
public:
    SurfaceRenderer() = default;
    SurfaceRenderer(const SurfaceRenderer &) = delete;
    SurfaceRenderer &operator=(const SurfaceRenderer &) = delete;
    ~SurfaceRenderer() { Shutdown(); }

    bool Initialize(Device &device, const std::filesystem::path &directory)
    {
        if (m_device == &device && m_constants.Is_Valid()) return true;
        Shutdown();
        m_shaders = ShaderLibrary{};
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = 12;
        shader.program.fragment_shader = 12;
        shader.program.source_key = 0x5355524641434531ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory / "surface.vso";
        shader.fragment_path = directory / "surface.pso";
        m_shader = m_shaders.Load_Precompiled(shader);
        if (!m_shaders.Is_Loaded(m_shader)) return false;
        m_device = &device;
        m_constants = device.Create_Buffer({sizeof(SurfaceParameters), RHIBufferUsage::Constant});
        if (!m_constants.Is_Valid() || !m_environment.Initialize(device)) { Shutdown(); return false; }
        return true;
    }

    void Shutdown() noexcept
    {
        if (m_device != nullptr) {
            m_environment.Shutdown(*m_device);
            m_meshes.For_Each([&](SurfaceMeshHandle handle, const SurfaceMesh &) {
                Release_GPU(*m_meshes.Resolve(handle));
            });
            for (const auto &pipeline : m_pipelines) m_device->Destroy_Pipeline(pipeline.handle);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_pipelines.clear();
        m_constants = {};
        m_device = nullptr;
    }

    SurfaceMeshHandle Create_Mesh(std::span<const SurfaceVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        SurfaceMesh mesh;
        if (!mesh.geometry.Assign(vertices, indices)) return {};
        return m_meshes.Create(std::move(mesh));
    }

    bool Update_Mesh(SurfaceMeshHandle handle, std::span<const SurfaceVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        SurfaceMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr || !mesh->geometry.Assign(vertices, indices)) return false;
        Release_GPU(*mesh);
        return true;
    }

    bool Destroy_Mesh(SurfaceMeshHandle handle) noexcept
    {
        SurfaceMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr) return false;
        Release_GPU(*mesh);
        return m_meshes.Destroy(handle);
    }

    bool Draw(CommandList &commands, SurfaceMeshHandle handle, const SurfaceStyle &style,
        const SurfaceParameters &parameters, std::span<const RHITextureHandle> textures)
    {
        SurfaceMesh *mesh = m_meshes.Resolve(handle);
        if (m_device == nullptr || mesh == nullptr || textures.size() > 4) return false;
        if (mesh->geometry.Indices().empty()) return true;
        const auto has_texture = [&](std::size_t slot) {
            return slot < textures.size() && textures[slot].Is_Valid();
        };
        if ((parameters.textured > 0.5f && !has_texture(0))
            || (parameters.cloud > 0.5f && !has_texture(1))
            || ((parameters.lightmap > 0.5f || parameters.masked_modulation > 0.5f) && !has_texture(2))
            || ((parameters.shroud > 0.5f || parameters.shroud_only > 0.5f) && !has_texture(3))) return false;
        if (!Upload(*mesh)) return false;
        const RHIPipelineHandle pipeline = Pipeline(style);
        if (!pipeline.Is_Valid() || !m_device->Update_Buffer(m_constants, 0,
            std::as_bytes(std::span(&parameters, 1)))) return false;
        std::array<RHIBindlessResource, 5> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        std::size_t count = 1;
        for (std::size_t slot = 0; slot < textures.size(); ++slot) {
            if (!textures[slot].Is_Valid()) continue;
            bindings[count].type = RHIResourceType::Texture;
            bindings[count].index = ResourceIndex{static_cast<std::uint32_t>(slot), 1};
            bindings[count++].texture = textures[slot];
        }
        return commands.Bind_Pipeline(pipeline)
            && commands.Set_Bindless_Resources(std::span(bindings.data(), count))
            && m_environment.Bind(*m_device, commands)
            && commands.Set_Vertex_Buffer(0, mesh->vertices, sizeof(SurfaceVertex), 0)
            && commands.Set_Index_Buffer(mesh->indices, RHIIndexFormat::UInt32, 0)
            && commands.Draw_Indexed(static_cast<std::uint32_t>(mesh->geometry.Indices().size()), 0);
    }

private:
    void Release_GPU(SurfaceMesh &mesh) noexcept
    {
        if (m_device != nullptr) {
            if (mesh.vertices.Is_Valid()) m_device->Destroy_Buffer(mesh.vertices);
            if (mesh.indices.Is_Valid()) m_device->Destroy_Buffer(mesh.indices);
        }
        mesh.vertices = {};
        mesh.indices = {};
    }

    bool Upload(SurfaceMesh &mesh)
    {
        if (mesh.vertices.Is_Valid() && mesh.indices.Is_Valid()) return true;
        const auto vertices = std::as_bytes(mesh.geometry.Vertices());
        const auto indices = std::as_bytes(mesh.geometry.Indices());
        mesh.vertices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(vertices.size()), RHIBufferUsage::Vertex, sizeof(SurfaceVertex)}, vertices);
        mesh.indices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(indices.size()), RHIBufferUsage::Index, sizeof(std::uint32_t)}, indices);
        if (!mesh.vertices.Is_Valid() || !mesh.indices.Is_Valid()) {
            Release_GPU(mesh);
            return false;
        }
        return true;
    }

    RHIPipelineHandle Pipeline(const SurfaceStyle &style)
    {
        for (const auto &entry : m_pipelines) if (entry.style == style) return entry.handle;
        RHIPipeline description;
        description.vertex_format = RHIVertexFormat::Position3Color4UV2;
        description.blend_mode = style.blend;
        description.blend_alpha_like_color = style.blend_alpha_like_color;
        description.depth_write = style.depth_write;
        description.cull_mode = style.cull;
        description.depth_comparison = style.depth_comparison;
        description.front_counter_clockwise = style.front_counter_clockwise;
        description.color_write_mask = style.color_write_mask;
        description.stencil = style.stencil;
        description.sampler_count = 16;
        description.samplers[0].address.fill(style.clamp_texture ? RHISamplerAddress::Clamp : RHISamplerAddress::Wrap);
        description.samplers[0].Set_Filter((style.linear_filter ? Graphics::RHISamplerFilter::Linear : Graphics::RHISamplerFilter::Point));
        const RHIPipelineHandle handle = m_device->Create_Pipeline(description,
            {m_shaders.Bytecode(m_shader, ShaderStage::Vertex)}, {m_shaders.Bytecode(m_shader, ShaderStage::Pixel)});
        if (handle.Is_Valid()) m_pipelines.push_back({style, handle});
        return handle;
    }

    EnvironmentLightingBinding m_environment;
    Device *m_device = nullptr;
    ShaderLibrary m_shaders;
    ShaderHandle m_shader{};
    RHIBufferHandle m_constants{};
    ResourcePool<SurfaceMesh, SurfaceMeshHandle> m_meshes;
    std::vector<SurfacePipeline> m_pipelines;
};

namespace { SurfaceRenderer g_surface_renderer; }
export SurfaceRenderer &Get_Surface_Renderer() noexcept { return g_surface_renderer; }
}
