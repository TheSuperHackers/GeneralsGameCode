module;
#include <cstdint>

export module Graphics.Resources.Textures.Snapshot;
import Graphics.RHI;

namespace Graphics
{
// Retains a sampleable GPU copy independently of subsequent source writes.
// The device must outlive this owner, or Shutdown must be called first.
export class TextureSnapshot final
{
public:
    TextureSnapshot() = default;
    TextureSnapshot(const TextureSnapshot&) = delete;
    TextureSnapshot& operator=(const TextureSnapshot&) = delete;
    ~TextureSnapshot() { Shutdown(); }

    bool Capture(Device& device, CommandList& commands, RHITextureHandle source,
        std::uint32_t width, std::uint32_t height, RHITextureFormat format)
    {
        if (!source.Is_Valid() || width == 0 || height == 0) return false;
        if (m_device == &device && m_width == width && m_height == height && m_format == format)
            return commands.Copy_Texture(source, m_texture);

        const auto replacement = device.Create_Texture({width, height, 1, format,
            static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
        if (!replacement.Is_Valid()) return false;
        if (!commands.Copy_Texture(source, replacement)) {
            device.Destroy_Texture(replacement);
            return false;
        }
        Shutdown();
        m_device = &device;
        m_texture = replacement;
        m_width = width;
        m_height = height;
        m_format = format;
        return true;
    }

    void Shutdown() noexcept
    {
        if (m_device != nullptr && m_texture.Is_Valid()) m_device->Destroy_Texture(m_texture);
        m_device = nullptr;
        m_texture = {};
        m_width = m_height = 0;
    }

    RHITextureHandle Texture() const noexcept { return m_texture; }

private:
    Device* m_device = nullptr;
    RHITextureHandle m_texture{};
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    RHITextureFormat m_format = RHITextureFormat::RGBA8_UNorm;
};
}
