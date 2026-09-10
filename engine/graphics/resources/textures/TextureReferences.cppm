module;
#include <cstddef>
#include <cstdint>
#include <unordered_map>

export module Graphics.Resources.Textures.References;
import Graphics.RHI;

namespace Graphics
{
// A resource scope retains each texture version once. Handles returned from
// Retain are borrowed until Clear; the device must outlive this scope.
export class TextureReferences final
{
public:
    TextureReferences() = default;
    TextureReferences(const TextureReferences&) = delete;
    TextureReferences& operator=(const TextureReferences&) = delete;
    ~TextureReferences() { Clear(); }

    RHITextureHandle Retain(Device& device, RHITextureHandle texture)
    {
        if (!texture.Is_Valid() || (m_device != nullptr && m_device != &device)) return {};
        const auto key = (static_cast<std::uint64_t>(texture.Get_Generation()) << 32)
            | texture.Get_Index();
        if (m_textures.contains(key)) return texture;
        if (!device.Retain_Texture(texture)) return {};
        try {
            m_textures.emplace(key, texture);
        } catch (...) {
            device.Destroy_Texture(texture);
            throw;
        }
        m_device = &device;
        return texture;
    }

    void Clear() noexcept
    {
        if (m_device != nullptr)
            for (const auto& entry : m_textures) m_device->Destroy_Texture(entry.second);
        m_textures.clear();
        m_device = nullptr;
    }

    std::size_t Size() const noexcept { return m_textures.size(); }

private:
    Device* m_device = nullptr;
    std::unordered_map<std::uint64_t, RHITextureHandle> m_textures;
};
}
