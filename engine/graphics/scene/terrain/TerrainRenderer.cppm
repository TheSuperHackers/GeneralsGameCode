module;
#include "../../profiling/Tracy.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

export module Graphics.Scene.Terrain.Renderer;

export import Graphics.Scene.Terrain.Geometry;
export import Graphics.RHI;
import Graphics.Shaders.Library;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.Lighting.Environment;
import Graphics.Scene.Terrain.Visibility;
import Graphics.Scene.Shadows.DirectionalRenderer;

namespace Graphics
{

export enum class TerrainSurfacePass : std::uint8_t
{
    Surface,
    Overlay,
    Shroud,
    Shoreline,
    Mask
};

export struct TerrainLight final
{
    std::array<float, 4> position_range{};
    std::array<float, 4> diffuse_inner{};
    std::array<float, 4> ambient_kind{};
    std::array<float, 4> direction{};
};

export struct TerrainDrawParameters final
{
    std::array<float, 16> view_projection{};
    std::array<float, 4> cloud_projection{};
    std::array<float, 4> lightmap_projection{};
    std::array<float, 4> shroud_projection{};
    std::array<float, 4> features{};
    std::array<float, 4> lighting{1.0f, 1.0f, 1.0f, 1.0f};
    std::array<float, 4> options{};
    std::array<float, 4> light_options{};
    std::array<TerrainLight, 20> lights{};
};
static_assert(sizeof(TerrainDrawParameters) == 1456);

struct TerrainPipeline final
{
    std::size_t pass;
    bool wireframe;
    RHISamplerDescription sampler;
    RHIPipelineHandle handle;
};

// One instance owns one surface batch. Adapters supply cells and resolved
// texture handles; all topology construction and GPU lifetime stays here.
export class TerrainRenderer final
{
public:
    TerrainRenderer() = default;
    TerrainRenderer(const TerrainRenderer &) = delete;
    TerrainRenderer &operator=(const TerrainRenderer &) = delete;
    ~TerrainRenderer() { Shutdown(); }

    bool Add_Shadow_Caster(DirectionalShadowRenderer& shadows,
        const std::array<float,16>& world)
    {
        if (m_geometry.Indices().empty()) return true;
        PropParameters parameters;
        parameters.textured = 0;
        if (m_shadow_owner == &shadows && world == m_shadow_world
            && shadows.Is_Caster_Valid(m_shadow_mesh))
            return shadows.Add_Caster(m_shadow_mesh,parameters,{});
        Release_Shadow_Caster();
        std::vector<PropVertex> vertices;
        vertices.reserve(m_geometry.Vertices().size());
        for (const auto& source : m_geometry.Vertices()) {
            PropVertex vertex;
            for (unsigned row=0;row<3;++row)
                vertex.position[row] = world[row*4]*source.position[0]
                    +world[row*4+1]*source.position[1]+world[row*4+2]*source.position[2]+world[row*4+3];
            vertices.push_back(vertex);
        }
        m_shadow_mesh = shadows.Create_Caster(vertices,m_geometry.Indices());
        m_shadow_owner = &shadows;
        m_shadow_world = world;
        return shadows.Add_Caster(m_shadow_mesh,parameters,{});
    }

    bool Initialize(Device &device, const std::filesystem::path &shader_directory)
    {
        if (m_device == &device && m_constants.Is_Valid()) return true;
        Shutdown();
        m_device = &device;
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = 11;
        shader.program.fragment_shader = 11;
        shader.program.source_key = 0x5445525241494e31ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = shader_directory / "terrain.vso";
        shader.fragment_path = shader_directory / "terrain.pso";
        m_shaders = ShaderLibrary{};
        m_shader = m_shaders.Load_Precompiled(shader);
        if (!m_shaders.Is_Loaded(m_shader)) {
            Shutdown();
            return false;
        }
        m_constants = device.Create_Buffer({sizeof(TerrainDrawParameters), RHIBufferUsage::Constant});
        if (!m_constants.Is_Valid() || !m_environment.Initialize(device)) {
            Shutdown();
            return false;
        }
        if (!Upload_Surface()) { Shutdown(); return false; }
        return true;
    }

    void Shutdown() noexcept
    {
        Release_GPU_Surface();
        if (m_device != nullptr) {
            m_environment.Shutdown(*m_device);
            for (const auto &pipeline : m_pipelines)
                m_device->Destroy_Pipeline(pipeline.handle);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_device = nullptr;
        m_pipelines = {};
        m_vertices = {};
        m_indices = {};
        m_constants = {};
        m_index_count = 0;
        m_vertex_bytes = m_index_bytes = 0;
    }

    void Release_Surface() noexcept
    {
        Release_GPU_Surface();
        m_geometry = {};
    }

private:
    void Release_GPU_Surface() noexcept
    {
        Release_Shadow_Caster();
        if (m_device != nullptr) {
            if (m_vertices.Is_Valid()) m_device->Destroy_Buffer(m_vertices);
            if (m_indices.Is_Valid()) m_device->Destroy_Buffer(m_indices);
        }
        m_vertices = {};
        m_indices = {};
        m_index_count = 0;
        m_vertex_bytes = m_index_bytes = 0;
    }

public:
    bool Set_Cells(std::span<const TerrainCell> cells)
    {
        if (!m_geometry.Build(cells)) return false;
        Release_Shadow_Caster();
        m_index_count = 0;
        return m_device == nullptr || Upload_Surface();
    }

private:
    bool Upload_Surface()
    {
        if (m_geometry.Indices().empty()) {
            m_index_count = 0;
            return true;
        }
        const auto vertices = std::as_bytes(m_geometry.Vertices());
        const auto indices = std::as_bytes(m_geometry.Indices());
        if (m_vertices.Is_Valid() && m_indices.Is_Valid()
            && vertices.size() == m_vertex_bytes && indices.size() == m_index_bytes) {
            m_index_count = 0;
            if (!m_device->Update_Buffer(m_vertices, 0, vertices) || !m_device->Update_Buffer(m_indices, 0, indices))
                return false;
            m_index_count = static_cast<std::uint32_t>(m_geometry.Indices().size());
            return true;
        }
        const RHIBufferHandle vertex_buffer = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(vertices.size()), RHIBufferUsage::Vertex, sizeof(TerrainVertex)}, vertices);
        if (!vertex_buffer.Is_Valid()) return false;
        const RHIBufferHandle index_buffer = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(indices.size()), RHIBufferUsage::Index, sizeof(std::uint32_t)}, indices);
        if (!index_buffer.Is_Valid()) {
            m_device->Destroy_Buffer(vertex_buffer);
            return false;
        }
        if (m_vertices.Is_Valid()) m_device->Destroy_Buffer(m_vertices);
        if (m_indices.Is_Valid()) m_device->Destroy_Buffer(m_indices);
        m_vertex_bytes = vertices.size();
        m_index_bytes = indices.size();
        m_vertices = vertex_buffer;
        m_indices = index_buffer;
        m_index_count = static_cast<std::uint32_t>(m_geometry.Indices().size());
        return true;
    }

public:
    // Targets and viewport are established by the frame/pass owner. This also
    // permits drawing into a reflection target without assuming a swap chain.
    bool Render(CommandList &commands, TerrainSurfacePass pass,
        const TerrainDrawParameters &parameters, std::span<const RHITextureHandle> textures,
        bool linear_filter = true, bool wireframe = false) noexcept
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Terrain.Render");
        const std::size_t pass_index = static_cast<std::size_t>(pass);
        if (m_device == nullptr || pass_index >= 5 || textures.size() > 5)
            return false;
        if (m_index_count == 0) return true;
        const auto has_texture = [&](std::size_t slot) {
            return slot < textures.size() && textures[slot].Is_Valid();
        };
        if (pass == TerrainSurfacePass::Shoreline) {
            if (!has_texture(0)) return false;
        } else if (pass == TerrainSurfacePass::Shroud || pass == TerrainSurfacePass::Mask) {
            if (!has_texture(4)) return false;
        } else if (parameters.features[2] <= 0.5f) {
            if (!has_texture(0) || (pass == TerrainSurfacePass::Surface && !has_texture(1))
                || (parameters.features[0] > 0.5f && !has_texture(2))
                || (parameters.features[1] > 0.5f && !has_texture(3))
                || (parameters.options[1] > 0.5f && !has_texture(4))) return false;
        }
        if (!m_device->Update_Buffer(m_constants, 0,
            std::as_bytes(std::span<const TerrainDrawParameters>(&parameters, 1))))
            return false;
        std::array<RHIBindlessResource, 6> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        std::size_t count = 1;
        for (std::size_t index = 0; index < textures.size(); ++index) {
            if (!textures[index].Is_Valid()) continue;
            bindings[count].type = RHIResourceType::Texture;
            bindings[count].index = ResourceIndex{static_cast<std::uint32_t>(index), 1};
            bindings[count].texture = textures[index];
            ++count;
        }
        const auto pipeline = Pipeline(pass_index, linear_filter, wireframe);
        if (!pipeline.Is_Valid()) return false;
        if (!(commands.Bind_Pipeline(pipeline)
            && commands.Set_Bindless_Resources(std::span<const RHIBindlessResource>(bindings.data(), count))
            && m_environment.Bind(*m_device, commands)
            && commands.Set_Vertex_Buffer(0, m_vertices, sizeof(TerrainVertex), 0)
            && commands.Set_Index_Buffer(m_indices, RHIIndexFormat::UInt32, 0))) return false;
        for (const TerrainGeometryBatch &batch : m_geometry.Batches()) {
            if (Is_Terrain_Batch_Visible(batch, parameters.view_projection)
                && !commands.Draw_Indexed(batch.index_count, batch.first_index)) return false;
        }
        return true;
    }

private:
    RHIPipelineHandle Pipeline(std::size_t pass, bool linear_filter, bool wireframe)
    {
        auto sampler = Resolve_Texture_Sampling(TextureSampling{}, Get_Texture_Sampling_Settings());
        sampler.address.fill(RHISamplerAddress::Clamp);
        if (!linear_filter) {
            sampler.Set_Filter(RHISamplerFilter::Point);
            sampler.anisotropy = 1;
        }
        for (const auto &pipeline : m_pipelines)
            if (pipeline.pass == pass && pipeline.wireframe == wireframe && pipeline.sampler == sampler)
                return pipeline.handle;

        RHIPipeline description;
        description.vertex_format = RHIVertexFormat::Position3Color4UV2UV2Normal3;
        description.cull_mode = RHICullMode::None;
        description.wireframe = wireframe;
        description.depth_write = pass == 0 || pass == 4;
        description.color_write_mask = pass >= 3 ? 8 : 7;
        description.blend_mode = pass == 1 ? RHIBlendMode::Alpha
            : pass == 2 ? RHIBlendMode::Multiply : RHIBlendMode::Disabled;
        description.sampler_count = 16;
        description.samplers[0] = description.samplers[1] = sampler;
        const auto handle = m_device->Create_Pipeline(description,
            {m_shaders.Bytecode(m_shader, ShaderStage::Vertex)},
            {m_shaders.Bytecode(m_shader, ShaderStage::Pixel)});
        if (handle.Is_Valid()) m_pipelines.push_back({pass, wireframe, sampler, handle});
        return handle;
    }

    void Release_Shadow_Caster() noexcept
    {
        if (m_shadow_owner != nullptr) m_shadow_owner->Destroy_Caster(m_shadow_mesh);
        m_shadow_owner = nullptr;
        m_shadow_mesh = {};
    }

    DirectionalShadowRenderer* m_shadow_owner = nullptr;
    ShadowCasterHandle m_shadow_mesh{};
    std::array<float,16> m_shadow_world{};
    EnvironmentLightingBinding m_environment;
    Device *m_device = nullptr;
    TerrainGeometry m_geometry;
    ShaderLibrary m_shaders;
    ShaderHandle m_shader{};
    std::vector<TerrainPipeline> m_pipelines;
    RHIBufferHandle m_vertices{};
    RHIBufferHandle m_indices{};
    RHIBufferHandle m_constants{};
    std::uint32_t m_index_count = 0;
    std::size_t m_vertex_bytes = 0;
    std::size_t m_index_bytes = 0;
};

namespace
{
std::array<TerrainRenderer, 3> g_terrain_renderers;
}

export TerrainRenderer &Get_Terrain_Renderer() noexcept { return g_terrain_renderers[0]; }
export TerrainRenderer &Get_Terrain_Overlay_Renderer() noexcept { return g_terrain_renderers[1]; }
export TerrainRenderer &Get_Terrain_Shoreline_Renderer() noexcept { return g_terrain_renderers[2]; }

export bool Initialize_Terrain_Renderers(Device &device, const std::filesystem::path &directory)
{
    for (TerrainRenderer &renderer : g_terrain_renderers) {
        if (!renderer.Initialize(device, directory)) {
            for (TerrainRenderer &created : g_terrain_renderers) created.Shutdown();
            return false;
        }
    }
    return true;
}

export void Shutdown_Terrain_Renderers() noexcept
{
    for (TerrainRenderer &renderer : g_terrain_renderers) renderer.Shutdown();
}

}
