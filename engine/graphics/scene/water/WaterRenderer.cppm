module;
#include "../../profiling/Tracy.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module Graphics.Scene.Water.Renderer;
export import Graphics.Scene.Water.Geometry;
export import Graphics.Scene.Water.View;
export import Graphics.Scene.Water.OceanPatches;
export import Graphics.Scene.Water.Displacement;
export import Graphics.Scene.Water.Waves;
export import Graphics.RHI;
import Graphics.Resources.Pools.ResourcePool;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Resources.Textures.Sampling;
import Graphics.Shaders.Library;
import Graphics.Scene.Lighting.Environment;

namespace Graphics
{
export struct WaterMeshTag;
export using WaterMeshHandle = ResourceHandle<WaterMeshTag>;

export struct WaterParameters final
{
    std::array<float,16> world{};
    std::array<float,16> view{};
    std::array<float,16> projection{};
    std::array<float,4> shroud_projection{};
    std::array<float,4> animation{};
    std::array<float,4> camera_position{};
    std::array<float,4> displacement_domain{};
    std::array<float,4> tint{1,1,1,1};
    std::array<float,4> effects{};
    std::array<float,4> surface_options{};
    std::array<float,4> fog_color{};
    std::array<float,4> fog_state{};
    std::array<float,4> sun_color{1,1,1,1};
    std::array<std::array<float,4>,3> environment_frame{{{1,0,0,0},{0,1,0,0},{0,0,1,0}}};
    std::array<float,16> inverse_view_projection{};
};
static_assert(sizeof(WaterParameters) == 464);
export enum class WaterPass { Ocean, Surface, Sky, Track, Underwater };
export struct WaterStyle final
{
    WaterPass pass = WaterPass::Surface;
    RHIBlendMode blend = RHIBlendMode::Alpha;
    bool clamp_texture = false;
    bool wireframe = false;
    bool operator==(const WaterStyle&) const = default;
};
struct WaterPipeline final {
    WaterStyle style;
    RHISamplerDescription surface_sampler;
    RHIPipelineHandle handle;
    bool instanced = false;
};

struct WaterMesh final
{
    WaterGeometry geometry;
    RHIBufferHandle vertices{};
    RHIBufferHandle indices{};
    OceanPatches patches;
    RHIBufferHandle instances{};
    std::uint64_t uploaded_patch_revision = 0;
};

// Owns geometry and GPU resources across map and device lifetimes. CPU geometry
// survives a device reset; upload is deferred until the next draw on that device.
export class WaterRenderer final
{
public:
    WaterRenderer() = default;
    WaterRenderer(const WaterRenderer &) = delete;
    WaterRenderer &operator=(const WaterRenderer &) = delete;
    ~WaterRenderer() { Shutdown(); }

    bool Initialize(Device &device, const std::filesystem::path &directory)
    {
        if (m_device == &device && m_constants.Is_Valid()) return true;
        Shutdown();
        m_shaders = ShaderLibrary{};
        const std::array<const char*,5> names{"ocean","water_surface","water_sky","water_track","water_underwater"};
        for (unsigned i=0;i<names.size();++i) {
            ShaderPrecompiledDesc shader;
            shader.program.vertex_shader = 14+i;
            shader.program.fragment_shader = 14+i;
            shader.program.source_key = 0x5741544552000000ull+i;
            shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
            shader.vertex_path = directory / (std::string(names[i])+".vso");
            shader.fragment_path = directory / (std::string(names[i])+".pso");
            m_shader[i] = m_shaders.Load_Precompiled(shader);
            if (!m_shaders.Is_Loaded(m_shader[i])) return false;
        }
        ShaderPrecompiledDesc instanced;
        instanced.program.vertex_shader = 19;
        instanced.program.fragment_shader = 14;
        instanced.program.source_key = 0x4F4345414E494E53ull;
        instanced.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        instanced.vertex_path = directory / "ocean_instanced.vso";
        instanced.fragment_path = directory / "ocean.pso";
        m_instanced_shader = m_shaders.Load_Precompiled(instanced);
        if (!m_shaders.Is_Loaded(m_instanced_shader)) return false;
        instanced.program.vertex_shader = 20;
        instanced.program.fragment_shader = 18;
        instanced.program.source_key = 0x554e444552494e53ull;
        instanced.vertex_path = directory / "water_underwater_instanced.vso";
        instanced.fragment_path = directory / "water_underwater.pso";
        m_underwater_instanced_shader = m_shaders.Load_Precompiled(instanced);
        if (!m_shaders.Is_Loaded(m_underwater_instanced_shader)) return false;
        m_device = &device;
        m_constants = device.Create_Buffer({sizeof(WaterParameters), RHIBufferUsage::Constant});
        if (!m_constants.Is_Valid() || !m_environment.Initialize(device)
            || !m_displacement.Initialize(device,directory)) { Shutdown(); return false; }
        const std::array<std::byte,4> empty{};
        m_emptyTexture = device.Create_Texture_Initialized({1,1},{empty,4});
        if (!m_emptyTexture.Is_Valid()) { Shutdown(); return false; }
        return true;
    }

    void Shutdown() noexcept
    {
        m_displacement.Shutdown();
        m_color_snapshot.Shutdown();
        m_depth_snapshot.Shutdown();
        if (m_device != nullptr) {
            m_environment.Shutdown(*m_device);
            m_meshes.For_Each([&](WaterMeshHandle handle, const WaterMesh &) {
                Release_GPU(*m_meshes.Resolve(handle));
            });
            for (const auto& pipeline : m_pipelines) m_device->Destroy_Pipeline(pipeline.handle);
            if (m_emptyTexture.Is_Valid()) m_device->Destroy_Texture(m_emptyTexture);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_pipelines.clear();
        m_emptyTexture = {};
        m_constants = {};
        m_device = nullptr;
    }

    WaterMeshHandle Create_Mesh(std::span<const WaterVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        WaterMesh mesh;
        if (!mesh.geometry.Assign(vertices, indices)) return {};
        return m_meshes.Create(std::move(mesh));
    }

    WaterMeshHandle Create_Surface_Patch(std::span<const std::array<float,3>,4> corners, float spacing)
    {
        WaterMesh mesh;
        if (!mesh.geometry.Assign_Surface_Patch(corners,spacing)) return {};
        return m_meshes.Create(std::move(mesh));
    }

    RHITextureHandle Capture_Color(CommandList& commands, const RHIBackbuffer& source,
        RHITextureFormat format)
    {
        if (m_device == nullptr || !m_color_snapshot.Capture(*m_device, commands,
            source.texture, source.width, source.height, format)) return {};
        return m_color_snapshot.Texture();
    }

    RHITextureHandle Capture_Depth(CommandList& commands, const RHIDepthTarget& source,
        RHITextureFormat format)
    {
        if (m_device == nullptr || !m_depth_snapshot.Capture(*m_device, commands,
            source.texture, source.width, source.height, format)) return {};
        return m_depth_snapshot.Texture();
    }

    bool Update_Mesh(WaterMeshHandle handle, std::span<const WaterVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        WaterMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr || !mesh->geometry.Assign(vertices, indices)) return false;
        Release_Geometry(*mesh);
        return true;
    }

    bool Destroy_Mesh(WaterMeshHandle handle) noexcept
    {
        WaterMesh *mesh = m_meshes.Resolve(handle);
        if (mesh == nullptr) return false;
        Release_GPU(*mesh);
        return m_meshes.Destroy(handle);
    }

    bool Draw(CommandList &commands, WaterMeshHandle handle, const WaterStyle& style,
        const WaterParameters &parameters, std::span<const RHITextureHandle> textures)
    {
        auto* mesh = m_meshes.Resolve(handle);
        if (m_device == nullptr || mesh == nullptr || (textures.size() != 9 && textures.size() != 11)) return false;
        return Draw_Mesh(commands,mesh,style,parameters,textures,false);
    }

    bool Draw_Patches(CommandList& commands, WaterMeshHandle handle, const WaterStyle& style,
        const WaterParameters& parameters, std::span<const RHITextureHandle> textures,
        const OceanPatchGrid& grid)
    {
        auto* mesh = m_meshes.Resolve(handle);
        if (m_device == nullptr || mesh == nullptr || (textures.size() != 9 && textures.size() != 11)
            || (style.pass != WaterPass::Ocean && style.pass != WaterPass::Underwater)) return false;
        if (!mesh->patches.Prepare(grid)) return false;
        const auto worlds = mesh->patches.Worlds();
        if (worlds.empty()) return true;
        if (!mesh->instances.Is_Valid() || mesh->uploaded_patch_revision != mesh->patches.Revision()) {
            const auto bytes = std::as_bytes(worlds);
            const auto buffer = m_device->Create_Buffer_Initialized(
                {static_cast<std::uint32_t>(bytes.size()),RHIBufferUsage::Storage,64},bytes);
            if (!buffer.Is_Valid()) return false;
            if (mesh->instances.Is_Valid()) m_device->Destroy_Buffer(mesh->instances);
            mesh->instances = buffer;
            mesh->uploaded_patch_revision = mesh->patches.Revision();
        }
        return Draw_Mesh(commands,mesh,style,parameters,textures,true);
    }

private:
    bool Draw_Mesh(CommandList &commands, WaterMesh* mesh, const WaterStyle& style,
        const WaterParameters &parameters, std::span<const RHITextureHandle> textures, bool instanced)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Water.Draw");
        assert(m_device != nullptr && mesh != nullptr && (textures.size() == 9 || textures.size() == 11));
        if (mesh->geometry.Indices().empty()) return true;
        if (style.pass == WaterPass::Underwater) {
            if (textures.size() != 11 || !textures[5].Is_Valid() || !textures[8].Is_Valid()
                || !textures[9].Is_Valid() || !textures[10].Is_Valid()) return false;
        } else if (!textures[0].Is_Valid()) return false;
        if (style.pass == WaterPass::Ocean && !textures[1].Is_Valid()) return false;
        if (!Upload(*mesh)) return false;
        const RHIPipelineHandle pipeline = Pipeline(style,instanced);
        if (!pipeline.Is_Valid() || !m_device->Update_Buffer(m_constants, 0,
            std::as_bytes(std::span(&parameters, 1)))) return false;
        std::array<RHIBindlessResource, 14> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        std::size_t count = 1;
        for (std::size_t slot = 0; slot < 11; ++slot) {
            bindings[count].type = RHIResourceType::Texture;
            bindings[count].index = ResourceIndex{static_cast<std::uint32_t>(slot), 1};
            bindings[count++].texture = slot < textures.size() && textures[slot].Is_Valid() ? textures[slot] : m_emptyTexture;
        }
        if (style.pass == WaterPass::Ocean) {
            bindings[count] = bindings[2];
            bindings[count++].stage = RHIShaderStage::Vertex;
        }
        if (instanced) {
            bindings[count].type = RHIResourceType::Buffer;
            bindings[count++].buffer = mesh->instances;
        }
        return commands.Bind_Pipeline(pipeline)
            && commands.Set_Bindless_Resources(std::span(bindings.data(), count))
            && m_environment.Bind(*m_device, commands)
            && commands.Set_Vertex_Buffer(0, mesh->vertices, sizeof(WaterVertex), 0)
            && commands.Set_Index_Buffer(mesh->indices, RHIIndexFormat::UInt32, 0)
            && commands.Draw_Indexed(static_cast<std::uint32_t>(mesh->geometry.Indices().size()), 0, 0,
                instanced ? static_cast<std::uint32_t>(mesh->patches.Worlds().size()) : 1);
    }

    void Release_GPU(WaterMesh &mesh) noexcept
    {
        Release_Geometry(mesh);
        if (m_device != nullptr && mesh.instances.Is_Valid()) m_device->Destroy_Buffer(mesh.instances);
        mesh.instances = {};
        mesh.uploaded_patch_revision = 0;
    }

    void Release_Geometry(WaterMesh &mesh) noexcept
    {
        if (m_device != nullptr) {
            if (mesh.vertices.Is_Valid()) m_device->Destroy_Buffer(mesh.vertices);
            if (mesh.indices.Is_Valid()) m_device->Destroy_Buffer(mesh.indices);
        }
        mesh.vertices = {};
        mesh.indices = {};
    }

    bool Upload(WaterMesh &mesh)
    {
        if (mesh.vertices.Is_Valid() && mesh.indices.Is_Valid()) return true;
        const auto vertices = std::as_bytes(mesh.geometry.Vertices());
        const auto indices = std::as_bytes(mesh.geometry.Indices());
        mesh.vertices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(vertices.size()), RHIBufferUsage::Vertex, sizeof(WaterVertex),RHIBufferUpdateMode::Discard}, vertices);
        mesh.indices = m_device->Create_Buffer_Initialized(
            {static_cast<std::uint32_t>(indices.size()), RHIBufferUsage::Index, sizeof(std::uint32_t),RHIBufferUpdateMode::Discard}, indices);
        if (!mesh.vertices.Is_Valid() || !mesh.indices.Is_Valid()) {
            Release_Geometry(mesh);
            return false;
        }
        return true;
    }

    RHIPipelineHandle Pipeline(const WaterStyle& style, bool instanced)
    {
        // Frame captures, depth, and the displacement field retain their own
        // sampling contracts. Authored surface textures follow the quality setting.
        const auto surface_sampler = Resolve_Texture_Sampling(TextureSampling{}, Get_Texture_Sampling_Settings());
        for (const auto& pipeline : m_pipelines)
            if (pipeline.style == style && pipeline.surface_sampler == surface_sampler && pipeline.instanced == instanced)
                return pipeline.handle;
        const auto pass = static_cast<unsigned>(style.pass);
        if (pass >= m_shader.size()) return {};
        RHIPipeline description;
        description.vertex_format = RHIVertexFormat::Position3Color4UV2UV2Normal3;
        description.blend_mode = style.blend;
        description.blend_alpha_like_color = true;
        description.depth_write = false;
        description.depth_test = style.pass != WaterPass::Sky && style.pass != WaterPass::Underwater;
        description.cull_mode = RHICullMode::None;
        description.color_write_mask = 15;
        description.depth_bias = style.pass == WaterPass::Track ? 8 : 0;
        description.wireframe = style.wireframe;
        description.sampler_count = 16;
        description.samplers[0] = surface_sampler;
        if (style.pass == WaterPass::Ocean || style.pass == WaterPass::Underwater) {
            description.samplers[2] = surface_sampler;
            description.samplers[3] = surface_sampler;
            description.samplers[9] = surface_sampler;
            description.samplers[10].address.fill(RHISamplerAddress::Clamp);
        } else if (style.pass == WaterPass::Surface) {
            description.samplers[1] = surface_sampler;
            description.samplers[2] = surface_sampler;
        }
        if (style.pass == WaterPass::Ocean || style.pass == WaterPass::Surface || style.pass == WaterPass::Underwater) {
            for (unsigned slot : {4u,5u,7u,8u}) description.samplers[slot].address.fill(RHISamplerAddress::Clamp);
            // Slot 1 is a bounded displacement field for the ocean, but a
            // repeating normal map for rivers and standing water.
            if (style.pass == WaterPass::Ocean)
                description.samplers[1].address.fill(RHISamplerAddress::Clamp);
            description.samplers[6].address[1] = RHISamplerAddress::Clamp;
        }
        if (style.pass == WaterPass::Underwater || style.pass == WaterPass::Ocean)
            description.samplers[8].Set_Filter(RHISamplerFilter::Point);
        description.samplers[0].address.fill(style.clamp_texture ? RHISamplerAddress::Clamp : RHISamplerAddress::Wrap);
        const auto vertex_shader = instanced ?
            (style.pass == WaterPass::Underwater ? m_underwater_instanced_shader : m_instanced_shader) : m_shader[pass];
        const auto handle = m_device->Create_Pipeline(description,
            {m_shaders.Bytecode(vertex_shader,ShaderStage::Vertex)},
            {m_shaders.Bytecode(m_shader[pass],ShaderStage::Pixel)});
        if (handle.Is_Valid()) m_pipelines.push_back({style,surface_sampler,handle,instanced});
        return handle;
    }

public:
    OceanDisplacement& Displacement() noexcept { return m_displacement; }
    WaterWaves& Waves() noexcept { return m_waves; }
private:
    WaterWaves m_waves;
    OceanDisplacement m_displacement;
    EnvironmentLightingBinding m_environment;
    TextureSnapshot m_color_snapshot;
    TextureSnapshot m_depth_snapshot;
    Device *m_device = nullptr;
    ShaderLibrary m_shaders;
    std::array<ShaderHandle,5> m_shader{};
    ShaderHandle m_instanced_shader{};
    ShaderHandle m_underwater_instanced_shader{};
    RHIBufferHandle m_constants{};
    ResourcePool<WaterMesh, WaterMeshHandle> m_meshes;
    std::vector<WaterPipeline> m_pipelines;
    RHITextureHandle m_emptyTexture{};
};

namespace { WaterRenderer g_water_renderer; }
export WaterRenderer &Get_Water_Renderer() noexcept { return g_water_renderer; }
}
