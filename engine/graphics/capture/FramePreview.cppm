module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>

export module Graphics.Capture.FramePreview;
export import Graphics.Capture.FrameCapture;
import Graphics.FrameTargets;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Scene.Screen.Filters;

namespace Graphics
{
// GPU downsampling precedes readback. Returned pixels remain valid until the
// next fresh capture or Shutdown; throttled calls return the same frame.
// Dimensions are multiples of four for block-compressed preview consumers.
export class FramePreview final
{
public:
    ~FramePreview() { Shutdown(); }

    bool Initialize(Device& device, const std::filesystem::path& shaders)
    {
        if (m_device == &device) return true;
        Shutdown();
        if (!m_renderer.Initialize(device, shaders)) return false;
        m_device = &device;
        return true;
    }

    void Shutdown() noexcept
    {
        m_snapshot.Shutdown();
        m_renderer.Shutdown();
        if (m_device && m_target.Is_Valid()) m_device->Destroy_Texture(m_target);
        m_device = nullptr;
        m_target = {};
        m_frame = {};
        m_source = {};
        m_source_identity = 0;
        m_width = m_height = 0;
    }

    Engine::Video::DecodedVideoFrame Read(const FrameTargets& source, RHITextureFormat format,
        std::uint32_t maximum_extent, std::uint32_t time_ms, std::uint32_t interval_ms)
    {
        const auto color = source.backbuffer;
        if (!m_device || !color.texture.Is_Valid() || !source.depth.texture.Is_Valid()
            || !color.width || !color.height || maximum_extent < 4
            || (format != RHITextureFormat::RGBA8_UNorm && format != RHITextureFormat::BGRA8_UNorm)) return {};
        const std::uint32_t width = maximum_extent & ~3u;
        const auto height = static_cast<std::uint32_t>(std::clamp<std::uint64_t>(
            (std::uint64_t(width) * color.height + color.width / 2) / color.width, 4, width)) & ~3u;
        const bool same_source = source.identity != 0
            ? m_source_identity == source.identity
            : m_source_identity == 0 && m_source == color.texture;
        if (same_source && m_source_width == color.width && m_source_height == color.height
            && m_source_format == format && m_width == width && m_height == height
            && !m_frame.pixels.empty() && time_ms - m_last_time < interval_ms) return m_frame;

        auto& commands = m_device->Immediate_Command_List();
        if (!m_snapshot.Capture(*m_device, commands, color.texture, color.width, color.height, format)) return {};
        if (m_width != width || m_height != height) {
            const auto replacement = m_device->Create_Texture({width, height, 1,
                RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
            if (!replacement.Is_Valid()) return {};
            if (m_target.Is_Valid()) m_device->Destroy_Texture(m_target);
            m_target = replacement;
            m_width = width;
            m_height = height;
            m_frame = {};
        }
        std::array<ScreenFilterVertex,4> vertices{};
        vertices[0].position={1,-1,0}; vertices[0].uv={1,1};
        vertices[1].position={1,1,0}; vertices[1].uv={1,0};
        vertices[2].position={-1,-1,0}; vertices[2].uv={0,1};
        vertices[3].position={-1,1,0}; vertices[3].uv={0,0};
        const bool drawn = commands.Set_Color_Target(m_target)
            && commands.Set_Viewport({0,0,m_width,m_height})
            && m_renderer.Draw(commands,vertices,{}, {},m_snapshot.Texture());
        // Capture runs at a frame boundary. Restore the caller's frame targets
        // and viewport; subsequent renderers bind their own pipeline/resources.
        const bool restored = commands.Reset_State()
            && commands.Set_Render_Targets(color.texture,source.depth.texture)
            && commands.Set_Viewport({0,0,color.width,color.height});
        if (!drawn || !restored) return {};
        m_frame = m_readback.Read(*m_device,m_target,m_width,m_height,RHITextureFormat::RGBA8_UNorm);
        if (!m_frame.pixels.empty()) {
            m_source = color.texture;
            m_source_identity = source.identity;
            m_source_width = color.width;
            m_source_height = color.height;
            m_source_format = format;
            m_last_time = time_ms;
        }
        return m_frame;
    }

private:
    Device* m_device = nullptr;
    TextureSnapshot m_snapshot;
    ScreenFilterRenderer m_renderer;
    FrameCapture m_readback;
    Engine::Video::DecodedVideoFrame m_frame{};
    RHITextureHandle m_target{}, m_source{};
    std::uint64_t m_source_identity = 0;
    std::uint32_t m_width=0, m_height=0, m_source_width=0, m_source_height=0, m_last_time=0;
    RHITextureFormat m_source_format = RHITextureFormat::RGBA8_UNorm;
};
namespace { FramePreview g_frame_preview; }
export FramePreview& Get_Frame_Preview() noexcept { return g_frame_preview; }
}
