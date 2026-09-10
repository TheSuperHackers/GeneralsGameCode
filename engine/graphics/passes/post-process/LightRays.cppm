module;
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <span>

export module Graphics.Passes.LightRays;
import Graphics.RHI;
import Graphics.FrameTargets;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Shaders.Library;
import Graphics.Scene.Lighting.Environment;

namespace Graphics
{
// Row-major, column-vector inverse of the actual depth-writing view/projection.
// Texture inputs are borrowed through Render; the scene owns their lifetime.
export struct LightRaysInput final
{
    std::array<float,16> inverse_view_projection{};
    std::array<float,4> shroud_projection{};
    RHITextureHandle shroud_texture{};
    // Maximum added RGB in display-color space; the game tints it with sunlight.
    std::array<float,3> brightness{0.048f,0.048f,0.048f};
};

export class LightRaysRenderer final
{
public:
    LightRaysRenderer() = default;
    LightRaysRenderer(const LightRaysRenderer&) = delete;
    LightRaysRenderer& operator=(const LightRaysRenderer&) = delete;
    ~LightRaysRenderer() { Shutdown(); }

    bool Initialize(Device& device, const std::filesystem::path& directory)
    {
        if (m_device == &device) return true;
        Shutdown();
        ShaderLibrary shaders;
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = shader.program.fragment_shader = 41;
        shader.program.source_key = 0x4C49474854524159ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory / "light_rays.vso";
        shader.fragment_path = directory / "light_rays.pso";
        const auto loaded = shaders.Load_Precompiled(shader);
        if (!shaders.Is_Loaded(loaded)) return false;
        m_device = &device;
        RHIPipeline pipeline;
        pipeline.depth_test = pipeline.depth_write = false;
        pipeline.cull_mode = RHICullMode::None;
        pipeline.blend_mode = RHIBlendMode::Disabled;
        pipeline.samplers[0].address.fill(RHISamplerAddress::Clamp);
        pipeline.samplers[15].address.fill(RHISamplerAddress::Wrap);
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
        if (!m_pipeline.Is_Valid() || !m_vertices.Is_Valid() || !m_constants.Is_Valid()
            || !m_environment.Initialize(device)) {
            Shutdown();
            return false;
        }
        return true;
    }

    void Shutdown() noexcept
    {
        m_color.Shutdown();
        m_depth.Shutdown();
        if (m_device) {
            m_environment.Shutdown(*m_device);
            if (m_rays.Is_Valid()) m_device->Destroy_Texture(m_rays);
            if (m_pipeline.Is_Valid()) m_device->Destroy_Pipeline(m_pipeline);
            if (m_vertices.Is_Valid()) m_device->Destroy_Buffer(m_vertices);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_device = nullptr;
        m_rays = {};
        m_pipeline = {};
        m_vertices = m_constants = {};
        m_width = m_height = 0;
    }

    bool Render(CommandList& commands, const FrameTargets& targets,
        RHITextureFormat color_format, RHITextureFormat depth_format,
        const LightRaysInput& input, bool enabled)
    {
        // Lower quality does no capture, allocation, state changes, or drawing.
        if (!enabled) return true;
        const auto& color = targets.backbuffer;
        if (!m_device || !color.texture.Is_Valid() || !targets.depth.texture.Is_Valid()
            || !color.width || !color.height || color.width != targets.depth.width
            || color.height != targets.depth.height) return false;
        for (const float value : input.inverse_view_projection)
            if (!std::isfinite(value)) return false;
        for (const float value : input.brightness)
            if (!std::isfinite(value) || value < 0) return false;
        const std::uint32_t width = color.width / 4 + (color.width % 4 != 0);
        const std::uint32_t height = color.height / 4 + (color.height % 4 != 0);
        if (!Ensure_Target(width, height)) return false;
        // Clear bindings before copying depth or sampling the preceding pass.
        const bool captured = commands.Reset_State()
            && m_color.Capture(*m_device, commands, color.texture, color.width, color.height, color_format)
            && m_depth.Capture(*m_device, commands, targets.depth.texture, color.width, color.height, depth_format);
        Parameters parameters;
        parameters.inverse_view_projection = input.inverse_view_projection;
        parameters.shroud_projection = input.shroud_projection;
        parameters.brightness = input.brightness;
        parameters.shroud_enabled = input.shroud_texture.Is_Valid() ? 1u : 0u;
        const bool traced = captured && Draw(commands, m_rays, width, height, parameters, input.shroud_texture);
        parameters.operation = 1;
        const bool composed = traced && Draw(commands, color.texture, color.width, color.height,
            parameters, input.shroud_texture);
        // Restore even on a failed capture/submission; the frame owner aborts failures.
        const bool restored = commands.Reset_State()
            && commands.Set_Render_Targets(color.texture, targets.depth.texture)
            && commands.Set_Viewport({0,0,color.width,color.height});
        return composed && restored;
    }

private:
    struct Vertex { std::array<float,3> position; std::array<float,4> color; std::array<float,2> uv; };
    struct Parameters {
        std::array<float,16> inverse_view_projection{};
        std::array<float,4> shroud_projection{};
        std::array<float,3> brightness{};
        std::uint32_t operation = 0;
        std::uint32_t shroud_enabled = 0;
        std::array<float,3> padding{};
    };
    static_assert(sizeof(Parameters) == 112);

    bool Ensure_Target(std::uint32_t width, std::uint32_t height)
    {
        if (m_width == width && m_height == height) return true;
        const auto replacement = m_device->Create_Texture({width,height,1,RHITextureFormat::RGBA16_Float,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)
                | static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
        if (!replacement.Is_Valid()) return false;
        if (m_rays.Is_Valid()) m_device->Destroy_Texture(m_rays);
        m_rays = replacement;
        m_width = width;
        m_height = height;
        return true;
    }

    bool Draw(CommandList& commands, RHITextureHandle target, std::uint32_t width,
        std::uint32_t height, const Parameters& parameters, RHITextureHandle shroud)
    {
        if (!m_device->Update_Buffer(m_constants,0,std::as_bytes(std::span(&parameters,1)))) return false;
        std::array<RHIBindlessResource,5> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        unsigned count = 1;
        const auto bind_texture = [&](unsigned slot, RHITextureHandle texture) {
            auto& binding = bindings[count++];
            binding.type = RHIResourceType::Texture;
            binding.index = ResourceIndex{slot,1};
            binding.texture = texture;
        };
        bind_texture(0,m_color.Texture());
        bind_texture(1,m_depth.Texture());
        if (parameters.operation == 1) bind_texture(2,m_rays);
        if (shroud.Is_Valid()) bind_texture(3,shroud);
        return commands.Reset_State()
            && commands.Set_Color_Target(target)
            && commands.Set_Viewport({0,0,width,height})
            && commands.Bind_Pipeline(m_pipeline)
            && commands.Set_Bindless_Resources(std::span(bindings.data(),count))
            && m_environment.Bind(*m_device,commands)
            && commands.Set_Vertex_Buffer(0,m_vertices,sizeof(Vertex),0)
            && commands.Draw(3,0,1,0);
    }

    Device* m_device = nullptr;
    TextureSnapshot m_color, m_depth;
    EnvironmentLightingBinding m_environment;
    RHIPipelineHandle m_pipeline{};
    RHIBufferHandle m_vertices{}, m_constants{};
    RHITextureHandle m_rays{};
    std::uint32_t m_width = 0, m_height = 0;
};
namespace { LightRaysRenderer g_light_rays; }
export LightRaysRenderer& Get_Light_Rays_Renderer() noexcept { return g_light_rays; }
}
