module;
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Water.Displacement;
import Graphics.RHI;
import Graphics.Frame.AttachmentBindings;
import Graphics.Shaders.Library;

namespace Graphics
{
export struct OceanWaveParticle final
{
    std::array<float,2> position{};
    std::array<float,2> velocity{};
    float amplitude = 0;
    float creation_time = 0;
    std::array<float,2> padding{};
};
static_assert(sizeof(OceanWaveParticle) == 32);

export class OceanDisplacement final
{
public:
    OceanDisplacement() = default;
    OceanDisplacement(const OceanDisplacement&) = delete;
    OceanDisplacement& operator=(const OceanDisplacement&) = delete;
    ~OceanDisplacement() { Shutdown(); }

    bool Initialize(Device& device, const std::filesystem::path& directory)
    {
        if (m_device == &device) return true;
        Shutdown();
        ShaderLibrary shaders;
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = shader.program.fragment_shader = 16;
        shader.program.source_key = 0x4f4345414e444953ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory / "water_displacement.vso";
        shader.fragment_path = directory / "water_displacement.pso";
        const auto loaded = shaders.Load_Precompiled(shader);
        if (!shaders.Is_Loaded(loaded)) return false;
        m_device = &device;
        RHIPipeline pipeline;
        pipeline.depth_test = pipeline.depth_write = false;
        pipeline.cull_mode = RHICullMode::None;
        pipeline.blend_mode = RHIBlendMode::Disabled;
        for (unsigned i = 0; i < m_pipelines.size(); ++i) {
            pipeline.samplers[0].address.fill(i == 1 ? RHISamplerAddress::Clamp : RHISamplerAddress::Wrap);
            pipeline.blend_mode = i == 2 ? RHIBlendMode::Additive : RHIBlendMode::Disabled;
            m_pipelines[i] = device.Create_Pipeline(pipeline,
                {shaders.Bytecode(loaded,ShaderStage::Vertex)},
                {shaders.Bytecode(loaded,ShaderStage::Pixel)});
            if (!m_pipelines[i].Is_Valid()) { Shutdown(); return false; }
        }
        const std::array<Vertex,6> vertices{{
            {{-1,1,0},{1,1,1,1},{0,0}}, {{1,1,0},{1,1,1,1},{1,0}},
            {{-1,-1,0},{1,1,1,1},{0,1}}, {{-1,-1,0},{1,1,1,1},{0,1}},
            {{1,1,0},{1,1,1,1},{1,0}}, {{1,-1,0},{1,1,1,1},{1,1}}}};
        m_vertices = device.Create_Buffer_Initialized(
            {sizeof(vertices),RHIBufferUsage::Vertex,sizeof(Vertex)},std::as_bytes(std::span(vertices)));
        m_constants = device.Create_Buffer({sizeof(Parameters),RHIBufferUsage::Constant});
        const RHITexture description{Resolution,Resolution,1,RHITextureFormat::RGBA16_Float,
            static_cast<unsigned>(RHITextureUsage::RenderTarget) | static_cast<unsigned>(RHITextureUsage::ShaderResource)};
        for (auto& target : m_targets) target = device.Create_Texture(description);
        if (!m_vertices.Is_Valid() || !m_constants.Is_Valid()
            || !m_targets[0].Is_Valid() || !m_targets[1].Is_Valid()) {
            Shutdown();
            return false;
        }
        return true;
    }

    void Shutdown() noexcept
    {
        if (m_device) {
            for (const auto pipeline : m_pipelines)
                if (pipeline.Is_Valid()) m_device->Destroy_Pipeline(pipeline);
            for (const auto texture : m_targets)
                if (texture.Is_Valid()) m_device->Destroy_Texture(texture);
            for (const auto buffer : {m_vertices,m_constants,m_particles})
                if (buffer.Is_Valid()) m_device->Destroy_Buffer(buffer);
        }
        m_device = nullptr;
        m_pipelines = {};
        m_targets = {};
        m_vertices = m_constants = m_particles = {};
        m_particle_capacity = 0;
        m_cached = false;
        m_cached_particles.clear();
    }

    bool Render(CommandList& commands, const AttachmentSelection& restore,
        RHITextureHandle static_texture, const std::array<float,4>& domain,
        float time, std::span<const OceanWaveParticle> particles = {}, std::uint64_t source_revision = 0)
    {
        if (!m_device || !static_texture.Is_Valid() || !std::isfinite(time)) return false;
        for (const float value : domain) if (!std::isfinite(value)) return false;
        if (domain[2] <= 0 || domain[3] <= 0) return false;
        if (particles.size() > MaximumParticles) return false;
        const auto bytes = std::as_bytes(particles);
        if (source_revision && m_cached && m_cached_source == static_texture
            && m_source_revision == source_revision && m_cached_time == time && m_cached_domain == domain
            && m_cached_particles.size() == particles.size()
            && (bytes.empty() || std::memcmp(bytes.data(),m_cached_particles.data(),bytes.size()) == 0))
            return commands.Set_Render_Targets(restore.color,restore.depth) && commands.Set_Viewport(restore.viewport);
        m_cached = false;
        if (!particles.empty()) {
            if (particles.size() > m_particle_capacity) {
                const auto capacity = std::bit_ceil(static_cast<std::uint32_t>(particles.size()));
                const auto byte_size = capacity * sizeof(OceanWaveParticle);
                if (byte_size > std::numeric_limits<std::uint32_t>::max()) return false;
                const auto replacement = m_device->Create_Buffer({static_cast<std::uint32_t>(byte_size),
                    RHIBufferUsage::Storage,sizeof(OceanWaveParticle)});
                if (!replacement.Is_Valid()) return false;
                if (m_particles.Is_Valid()) m_device->Destroy_Buffer(m_particles);
                m_particles = replacement;
                m_particle_capacity = capacity;
            }
            if (!m_device->Update_Buffer(m_particles,0,bytes)) return false;
        }
        Parameters parameters{domain,time,1.0f/Resolution,0,0};
        bool drawn = Draw(commands,m_targets[0],static_texture,parameters,0,1);
        if (drawn && !particles.empty()) {
            parameters.operation = 3;
            drawn = Draw(commands,m_targets[0],static_texture,parameters,2,static_cast<std::uint32_t>(particles.size()));
        }
        parameters.operation = 1;
        drawn = drawn && Draw(commands,m_targets[1],m_targets[0],parameters,1,1);
        parameters.operation = 2;
        drawn = drawn && Draw(commands,m_targets[0],m_targets[1],parameters,1,1);
        const bool restored = commands.Reset_State()
            && commands.Set_Render_Targets(restore.color,restore.depth)
            && commands.Set_Viewport(restore.viewport);
        if (drawn && source_revision) {
            m_cached_source = static_texture;
            m_source_revision = source_revision;
            m_cached_time = time;
            m_cached_domain = domain;
            m_cached_particles.assign(particles.begin(),particles.end());
            m_cached = true;
        }
        return drawn && restored;
    }

    RHITextureHandle Texture() const noexcept { return m_targets[0]; }
    static constexpr std::uint32_t Resolution = 1024;
    static constexpr std::uint32_t MaximumParticles = 1000000;

private:
    struct Vertex { std::array<float,3> position; std::array<float,4> color; std::array<float,2> uv; };
    struct Parameters {
        std::array<float,4> domain;
        float time;
        float texel;
        std::uint32_t operation;
        float padding;
    };
    bool Draw(CommandList& commands, RHITextureHandle target, RHITextureHandle source,
        const Parameters& parameters, unsigned pipeline, std::uint32_t instances)
    {
        if (!m_device->Update_Buffer(m_constants,0,std::as_bytes(std::span(&parameters,1)))) return false;
        std::array<RHIBindlessResource,3> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        bindings[1].type = RHIResourceType::Texture;
        bindings[1].index = ResourceIndex{0,1};
        bindings[1].texture = source;
        bindings[2].type = RHIResourceType::Buffer;
        bindings[2].buffer = m_particles;
        return commands.Reset_State()
            && commands.Set_Color_Target(target)
            && commands.Set_Viewport({0,0,Resolution,Resolution})
            && commands.Bind_Pipeline(m_pipelines[pipeline])
            && commands.Set_Bindless_Resources(std::span(bindings.data(),pipeline == 2 ? 3 : 2))
            && commands.Set_Vertex_Buffer(0,m_vertices,sizeof(Vertex),0)
            && commands.Draw(6,0,instances,0);
    }
    Device* m_device = nullptr;
    std::array<RHIPipelineHandle,3> m_pipelines{};
    std::array<RHITextureHandle,2> m_targets{};
    RHIBufferHandle m_vertices{}, m_constants{}, m_particles{};
    std::size_t m_particle_capacity = 0;
    std::vector<OceanWaveParticle> m_cached_particles;
    RHITextureHandle m_cached_source{};
    std::array<float,4> m_cached_domain{};
    std::uint64_t m_source_revision = 0;
    float m_cached_time = 0;
    bool m_cached = false;
};
}
