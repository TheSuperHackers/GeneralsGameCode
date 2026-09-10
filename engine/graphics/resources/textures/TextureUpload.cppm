module;
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
export module Graphics.Resources.Textures.Upload;
import Graphics.RHI;

namespace Graphics
{
// Begin and Finish run on the device thread. Workers may fill disjoint mapped
// subresources between them; the caller must join workers before Finish.
// The device outlives this owner. No partially prepared image is published here.
export class TextureUpload final
{
public:
    TextureUpload() = default;
    TextureUpload(const TextureUpload&) = delete;
    TextureUpload& operator=(const TextureUpload&) = delete;
    ~TextureUpload() { Finish(); }

    bool Begin(Device& device, RHITextureHandle texture, std::uint32_t mip_count,
        std::uint32_t layer_count = 1)
    {
        if (Active() || mip_count==0 || layer_count==0
            || layer_count>std::numeric_limits<std::size_t>::max()/mip_count)
            return false;
        m_mappings.resize(std::size_t(mip_count)*layer_count);
        if (!device.Retain_Texture(texture)) {
            m_mappings.clear();
            return false;
        }
        m_device = &device;
        m_texture = texture;
        m_mip_count = mip_count;
        for (std::uint32_t layer=0;layer<layer_count;++layer) {
            for (std::uint32_t mip=0;mip<mip_count;++mip) {
                auto& mapping = m_mappings[m_mapped_count];
                if (!device.Map_Texture(texture,mip,layer,false,mapping)) {
                    Finish();
                    return false;
                }
                ++m_mapped_count;
                if (mapping.bytes.empty()) {
                    Finish();
                    return false;
                }
            }
        }
        return true;
    }

    bool Finish() noexcept
    {
        if (!Active()) return true;
        bool complete = true;
        for (std::size_t index=0;index<m_mapped_count;++index)
            if (!m_device->Unmap_Texture(m_texture,static_cast<std::uint32_t>(index%m_mip_count),
                static_cast<std::uint32_t>(index/m_mip_count))) complete = false;
        m_device->Destroy_Texture(m_texture);
        m_device = nullptr;
        m_texture = {};
        m_mappings.clear();
        m_mapped_count = 0;
        m_mip_count = 0;
        return complete;
    }

    RHITextureMapping Mapping(std::uint32_t mip, std::uint32_t layer = 0) const noexcept
    {
        if (mip>=m_mip_count || layer>=m_mapped_count/m_mip_count) return {};
        return m_mappings[std::size_t(layer)*m_mip_count+mip];
    }

    bool Active() const noexcept { return m_device!=nullptr; }

private:
    Device* m_device = nullptr;
    RHITextureHandle m_texture{};
    std::vector<RHITextureMapping> m_mappings;
    std::size_t m_mapped_count = 0;
    std::uint32_t m_mip_count = 0;
};
}
