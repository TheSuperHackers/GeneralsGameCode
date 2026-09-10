module;
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <span>
#include <utility>

export module Graphics.Passes.Bloom;
import Graphics.RHI;
import Graphics.FrameTargets;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Shaders.Library;

namespace Graphics
{
export struct BloomSettings final
{
    float intensity = 0.5f;
    std::array<float,3> exposure{1,1,1};
};
export class BloomRenderer final
{
public:
    BloomRenderer() = default;
    BloomRenderer(const BloomRenderer&) = delete;
    BloomRenderer& operator=(const BloomRenderer&) = delete;
    ~BloomRenderer() { Shutdown(); }

    bool Initialize(Device& device, const std::filesystem::path& directory)
    {
        if (m_device == &device) return true;
        Shutdown();
        ShaderLibrary shaders;
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = 40;
        shader.program.fragment_shader = 40;
        shader.program.source_key = 0x424C4F4F4D303031ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory / "bloom.vso";
        shader.fragment_path = directory / "bloom.pso";
        const auto loaded = shaders.Load_Precompiled(shader);
        if (!shaders.Is_Loaded(loaded)) return false;
        m_device = &device;
        RHIPipeline pipeline;
        pipeline.depth_test = pipeline.depth_write = false;
        pipeline.cull_mode = RHICullMode::None;
        pipeline.blend_mode = RHIBlendMode::Disabled;
        pipeline.samplers[0].address.fill(RHISamplerAddress::Clamp);
        m_pipeline = device.Create_Pipeline(pipeline,
            {shaders.Bytecode(loaded, ShaderStage::Vertex)},
            {shaders.Bytecode(loaded, ShaderStage::Pixel)});
        const std::array<Vertex,3> vertices{{
            {{-1,1,0},{1,1,1,1},{0,0}},
            {{3,1,0},{1,1,1,1},{2,0}},
            {{-1,-3,0},{1,1,1,1},{0,2}}}};
        m_vertices = device.Create_Buffer_Initialized(
            {sizeof(vertices), RHIBufferUsage::Vertex, sizeof(Vertex)}, std::as_bytes(std::span(vertices)));
        m_constants = device.Create_Buffer({sizeof(Parameters), RHIBufferUsage::Constant});
        if (!m_pipeline.Is_Valid() || !m_vertices.Is_Valid() || !m_constants.Is_Valid()) {
            Shutdown();
            return false;
        }
        return true;
    }

    void Shutdown() noexcept
    {
        m_snapshot.Shutdown();
        Release_Targets();
        if (m_device) {
            if (m_pipeline.Is_Valid()) m_device->Destroy_Pipeline(m_pipeline);
            if (m_vertices.Is_Valid()) m_device->Destroy_Buffer(m_vertices);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_device = nullptr;
        m_pipeline = {};
        m_vertices = m_constants = {};
    }

    bool Render(CommandList& commands, const FrameTargets& targets,
        RHITextureFormat format, bool enabled, const BloomSettings& settings = {})
    {
        if (!enabled) return true;
        if (!std::isfinite(settings.intensity) || settings.intensity < 0) return false;
        for (const float value : settings.exposure)
            if (!std::isfinite(value) || value < 0) return false;
        const auto color = targets.backbuffer;
        if (!m_device || !color.texture.Is_Valid() || !targets.depth.texture.Is_Valid()
            || !color.width || !color.height) return false;
        const std::uint32_t width = (std::min)(256u,color.width);
        const std::uint32_t height = (std::max)(1u,
            static_cast<std::uint32_t>(static_cast<std::uint64_t>(color.height)*width/color.width));
        if (!Ensure_Targets(width, height)
            || !m_snapshot.Capture(*m_device, commands, color.texture, color.width, color.height, format)) return false;
        Parameters parameters{1.0f/color.width,1.0f/color.height,0,settings.intensity,settings.exposure};
        parameters.half_kernel_size = std::clamp(static_cast<int>(color.width/static_cast<float>(width)*0.5f-0.5f),0,4)+1;
        bool drawn = Draw(commands, m_targets[0].textures[0], width, height,
            m_snapshot.Texture(), {}, parameters);
        for (unsigned level = 1; drawn && level < m_targets.size(); ++level) {
            const auto& source = m_targets[level-1];
            const auto& target = m_targets[level];
            parameters.texel_x = 1.0f/source.width;
            parameters.texel_y = 1.0f/source.height;
            parameters.operation = 4;
            drawn = Draw(commands,target.textures[0],target.width,target.height,
                source.textures[0],{},parameters);
        }
        for (int level = 2; drawn && level >= 0; --level) {
            const auto& target = m_targets[level];
            parameters.texel_x = 1.0f/target.width;
            parameters.texel_y = 1.0f/target.height;
            unsigned source = 0;
            if (level < 2) {
                parameters.operation = 5;
                drawn = Draw(commands,target.textures[1],target.width,target.height,
                    m_targets[level+1].textures[0],target.textures[0],parameters);
                source = 1;
            }
            parameters.operation = 1;
            drawn = drawn && Draw(commands,target.textures[1-source],target.width,target.height,
                target.textures[source],{},parameters);
            parameters.operation = 2;
            drawn = drawn && Draw(commands,target.textures[source],target.width,target.height,
                target.textures[1-source],{},parameters);
            if (source == 1) std::swap(m_targets[level].textures[0],m_targets[level].textures[1]);
        }
        parameters.operation = 3;
        drawn = drawn && Draw(commands,color.texture,color.width,color.height,
            m_targets[0].textures[0],m_snapshot.Texture(),parameters);
        const bool restored = commands.Reset_State()
            && commands.Set_Render_Targets(color.texture, targets.depth.texture)
            && commands.Set_Viewport({0,0,color.width,color.height});
        return drawn && restored;
    }

private:
    struct Vertex { std::array<float,3> position; std::array<float,4> color; std::array<float,2> uv; };
    struct Parameters {
        float texel_x, texel_y;
        std::uint32_t operation;
        float intensity;
        std::array<float,3> exposure;
        std::uint32_t half_kernel_size = 1;
    };
    static_assert(sizeof(Parameters) == 32);
    struct Level {
        std::array<RHITextureHandle,2> textures{};
        std::uint32_t width = 0, height = 0;
    };
    void Release_Targets() noexcept
    {
        if (m_device) for (const auto& level : m_targets)
            for (const auto target : level.textures)
                if (target.Is_Valid()) m_device->Destroy_Texture(target);
        m_targets = {};
        m_width = m_height = 0;
    }
    bool Ensure_Targets(std::uint32_t width, std::uint32_t height)
    {
        if (m_width == width && m_height == height) return true;
        std::array<Level,3> replacement{};
        auto level_width = width;
        auto level_height = height;
        for (auto& level : replacement) {
            level.width = level_width;
            level.height = level_height;
            const RHITexture description{level_width,level_height,1,RHITextureFormat::RGBA16_Float,
                static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)
                    | static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)};
            for (auto& texture : level.textures) {
                texture = m_device->Create_Texture(description);
                if (!texture.Is_Valid()) {
                    for (const auto& allocated : replacement)
                        for (const auto created : allocated.textures)
                            if (created.Is_Valid()) m_device->Destroy_Texture(created);
                    return false;
                }
            }
            level_width = (std::max)(1u,level_width/4);
            level_height = (std::max)(1u,level_height/4);
        }
        Release_Targets();
        m_targets = replacement;
        m_width = width;
        m_height = height;
        return true;
    }
    bool Draw(CommandList& commands, RHITextureHandle target, std::uint32_t width,
        std::uint32_t height, RHITextureHandle source, RHITextureHandle scene, const Parameters& parameters)
    {
        if (!m_device->Update_Buffer(m_constants,0,std::as_bytes(std::span(&parameters,1)))) return false;
        std::array<RHIBindlessResource,3> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        bindings[1].type = bindings[2].type = RHIResourceType::Texture;
        bindings[1].index = ResourceIndex{0,1};
        bindings[1].texture = source;
        bindings[2].index = ResourceIndex{1,1};
        bindings[2].texture = scene;
        return commands.Reset_State()
            && commands.Set_Color_Target(target)
            && commands.Set_Viewport({0,0,width,height})
            && commands.Bind_Pipeline(m_pipeline)
            && commands.Set_Bindless_Resources(std::span(bindings.data(),scene.Is_Valid() ? 3 : 2))
            && commands.Set_Vertex_Buffer(0,m_vertices,sizeof(Vertex),0)
            && commands.Draw(3,0,1,0);
    }
    Device* m_device = nullptr;
    TextureSnapshot m_snapshot;
    RHIPipelineHandle m_pipeline{};
    RHIBufferHandle m_vertices{}, m_constants{};
    std::array<Level,3> m_targets{};
    std::uint32_t m_width = 0, m_height = 0;
};
namespace { BloomRenderer g_bloom; }
export BloomRenderer& Get_Bloom_Renderer() noexcept { return g_bloom; }
}
