module;
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <span>

export module Graphics.Passes.SSAO;
import Graphics.RHI;
import Graphics.FrameTargets;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Shaders.Library;

namespace Graphics
{
// Actual depth-writing projection, row-major, column vectors, -Z view forward.
export struct SSAOInput final
{
    std::array<float,16> projection{};
    std::array<float,16> inverse_projection{};
    float radius = 12.0f;
    float bias = 0.35f;
    float strength = 0.8f;

    bool Is_Valid() const noexcept
    {
        if (!std::isfinite(radius) || !std::isfinite(bias) || !std::isfinite(strength)
            || radius<=0 || bias<0 || bias>=radius || strength<0 || strength>1) return false;
        for (const float value : projection) if (!std::isfinite(value)) return false;
        for (const float value : inverse_projection) if (!std::isfinite(value)) return false;
        for (unsigned row=0;row<4;++row) for (unsigned col=0;col<4;++col) {
            float value=0;
            for (unsigned k=0;k<4;++k) value+=projection[row*4+k]*inverse_projection[k*4+col];
            if (!std::isfinite(value) || std::abs(value-(row==col ? 1.0f : 0.0f))>0.001f) return false;
        }
        return true;
    }
};

export class SSAORenderer final
{
public:
    SSAORenderer() = default;
    SSAORenderer(const SSAORenderer&) = delete;
    SSAORenderer& operator=(const SSAORenderer&) = delete;
    ~SSAORenderer() { Shutdown(); }
    bool Initialize(Device& device, const std::filesystem::path& directory)
    {
        if (m_device==&device) return true;
        Shutdown();
        ShaderLibrary shaders;
        ShaderPrecompiledDesc shader;
        shader.program.vertex_shader = shader.program.fragment_shader = 42;
        shader.program.source_key = 0x5353414F30303031ull;
        shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        shader.vertex_path = directory/"ssao.vso";
        shader.fragment_path = directory/"ssao.pso";
        const auto loaded = shaders.Load_Precompiled(shader);
        if (!shaders.Is_Loaded(loaded)) return false;
        m_device = &device;
        RHIPipeline pipeline;
        pipeline.depth_test = pipeline.depth_write = false;
        pipeline.cull_mode = RHICullMode::None;
        pipeline.blend_mode = RHIBlendMode::Disabled;
        m_pipeline = device.Create_Pipeline(pipeline,
            {shaders.Bytecode(loaded,ShaderStage::Vertex)},
            {shaders.Bytecode(loaded,ShaderStage::Pixel)});
        const std::array<Vertex,3> vertices{{
            {{-1,1,0},{1,1,1,1},{0,0}},
            {{3,1,0},{1,1,1,1},{2,0}},
            {{-1,-3,0},{1,1,1,1},{0,2}}}};
        m_vertices = device.Create_Buffer_Initialized(
            {sizeof(vertices),RHIBufferUsage::Vertex,sizeof(Vertex)},std::as_bytes(std::span(vertices)));
        m_constants = device.Create_Buffer({sizeof(Parameters),RHIBufferUsage::Constant});
        if (!m_pipeline.Is_Valid() || !m_vertices.Is_Valid() || !m_constants.Is_Valid()) {
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
            if (m_occlusion.Is_Valid()) m_device->Destroy_Texture(m_occlusion);
            if (m_pipeline.Is_Valid()) m_device->Destroy_Pipeline(m_pipeline);
            if (m_vertices.Is_Valid()) m_device->Destroy_Buffer(m_vertices);
            if (m_constants.Is_Valid()) m_device->Destroy_Buffer(m_constants);
        }
        m_device = nullptr;
        m_pipeline = {};
        m_vertices = m_constants = {};
        m_occlusion = {};
        m_width = m_height = 0;
    }
    bool Render(CommandList& commands, const FrameTargets& targets,
        RHITextureFormat color_format, RHITextureFormat depth_format,
        const SSAOInput& input, bool enabled)
    {
        if (!enabled) return true;
        const auto& color = targets.backbuffer;
        if (!m_device || !input.Is_Valid() || !color.texture.Is_Valid()
            || !targets.depth.texture.Is_Valid() || !color.width || !color.height
            || color.width!=targets.depth.width || color.height!=targets.depth.height) return false;
        if (input.strength==0) return true;
        const std::uint32_t width = color.width/2+color.width%2;
        const std::uint32_t height = color.height/2+color.height%2;
        if (!Ensure_Target(width,height)) return false;
        const bool captured = commands.Reset_State()
            && m_color.Capture(*m_device,commands,color.texture,color.width,color.height,color_format)
            && m_depth.Capture(*m_device,commands,targets.depth.texture,color.width,color.height,depth_format);
        Parameters parameters{input.projection,input.inverse_projection,input.radius,input.bias,input.strength,0};
        const bool occluded = captured && Draw(commands,m_occlusion,width,height,parameters);
        parameters.operation = 1;
        const bool composed = occluded && Draw(commands,color.texture,color.width,color.height,parameters);
        const bool restored = commands.Reset_State()
            && commands.Set_Render_Targets(color.texture,targets.depth.texture)
            && commands.Set_Viewport({0,0,color.width,color.height});
        return composed && restored;
    }
private:
    struct Vertex { std::array<float,3> position; std::array<float,4> color; std::array<float,2> uv; };
    struct Parameters {
        std::array<float,16> projection, inverse_projection;
        float radius, bias, strength;
        std::uint32_t operation;
    };
    static_assert(sizeof(Parameters)==144);
    bool Ensure_Target(std::uint32_t width, std::uint32_t height)
    {
        if (m_width==width && m_height==height) return true;
        // Retain precise view depth for bilateral rejection at long distances.
        const auto target = m_device->Create_Texture({width,height,1,RHITextureFormat::RGBA32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)
                | static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
        if (!target.Is_Valid()) return false;
        if (m_occlusion.Is_Valid()) m_device->Destroy_Texture(m_occlusion);
        m_occlusion = target;
        m_width = width;
        m_height = height;
        return true;
    }
    bool Draw(CommandList& commands, RHITextureHandle target, std::uint32_t width,
        std::uint32_t height, const Parameters& parameters)
    {
        if (!m_device->Update_Buffer(m_constants,0,std::as_bytes(std::span(&parameters,1)))) return false;
        std::array<RHIBindlessResource,4> bindings{};
        bindings[0].type = RHIResourceType::Material;
        bindings[0].buffer = m_constants;
        const std::array textures{m_color.Texture(),m_depth.Texture(),m_occlusion};
        const unsigned count = parameters.operation==0 ? 2 : 3;
        for (unsigned i=0;i<count;++i) {
            bindings[i+1].type = RHIResourceType::Texture;
            bindings[i+1].index = ResourceIndex{i,1};
            bindings[i+1].texture = textures[i];
        }
        return commands.Reset_State()
            && commands.Set_Color_Target(target)
            && commands.Set_Viewport({0,0,width,height})
            && commands.Bind_Pipeline(m_pipeline)
            && commands.Set_Bindless_Resources(std::span(bindings.data(),count+1))
            && commands.Set_Vertex_Buffer(0,m_vertices,sizeof(Vertex),0)
            && commands.Draw(3,0,1,0);
    }
    Device* m_device = nullptr;
    TextureSnapshot m_color, m_depth;
    RHIPipelineHandle m_pipeline{};
    RHIBufferHandle m_vertices{}, m_constants{};
    RHITextureHandle m_occlusion{};
    std::uint32_t m_width = 0, m_height = 0;
};
namespace { SSAORenderer g_ssao; }
export SSAORenderer& Get_SSAO_Renderer() noexcept { return g_ssao; }
}
