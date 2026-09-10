module;
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
export module Graphics.Capture.FrameCapture;
export import Graphics.RHI;
export import Video.Frame;
namespace Graphics
{
// The returned frame borrows storage until the next capture or destruction.
// The caller supplies the source layout and retains ownership of the texture.
export class FrameCapture final
{
public:
    Engine::Video::DecodedVideoFrame Read(Device& device, RHITextureHandle texture,
        std::uint32_t width, std::uint32_t height, RHITextureFormat format)
    {
        using Engine::Video::PixelFormat;
        if (width == 0 || height == 0 || width > std::numeric_limits<std::uint32_t>::max() / 4
            || (format != RHITextureFormat::RGBA8_UNorm && format != RHITextureFormat::BGRA8_UNorm))
            return {};
        const std::uint32_t pitch = width * 4;
        const std::uint64_t size = std::uint64_t(pitch) * height;
        if (size > m_pixels.max_size())
            return {};
        m_pixels.resize(static_cast<std::size_t>(size));
        if (!device.Readback_Texture(texture, m_pixels, pitch))
            return {};
        return {width, height, pitch,
            format == RHITextureFormat::BGRA8_UNorm ? PixelFormat::BGRA8 : PixelFormat::RGBA8,
            0, 0, m_pixels};
    }
private:
    std::vector<std::byte> m_pixels;
};
}
