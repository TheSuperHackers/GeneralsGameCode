module;
#define NOMINMAX
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
export module Graphics.Resources.Textures.Resource;
export import Graphics.RHI;
export import Graphics.Resources.Textures.Storage;

namespace Graphics
{
// A logical texture owner retains its GPU generation and immutable allocation
// metadata. References change on the device thread. Deferred drawing retains
// the GPU handle separately, so it can outlive this owner's publication slot.
export class TextureResource final
{
public:
    TextureResource(const TextureResource&) = delete;
    TextureResource& operator=(const TextureResource&) = delete;

    static TextureResource* Create(Device* device, RHITexture description,
        Assets::PixelEncoding encoding, RHITextureFormat attachment_format = RHITextureFormat::Unknown)
    {
        if (!device || description.width == 0 || description.height == 0 || description.depth == 0) return nullptr;
        const bool depth = (description.usage & static_cast<unsigned>(RHITextureUsage::DepthStencil)) != 0;
        const bool target = (description.usage & static_cast<unsigned>(RHITextureUsage::RenderTarget)) != 0;
        if (target && Assets::Is_Block_Compressed(encoding)) return nullptr;
        if (description.dimension == RHITextureDimension::Cube && description.width != description.height) return nullptr;
        if (depth) {
            if (description.format != RHITextureFormat::D16_UNorm
                && description.format != RHITextureFormat::D24_UNorm_S8
                && description.format != RHITextureFormat::D32_Float) return nullptr;
            encoding = Assets::PixelEncoding::Unknown;
            description.mip_count = 1;
        } else {
            if (Texture_Storage_Format(encoding) == RHITextureFormat::Unknown
                || (description.dimension == RHITextureDimension::Volume && Assets::Is_Block_Compressed(encoding)))
                encoding = Assets::PixelEncoding::BGRA8;
            encoding = Editable_Texture_Encoding(encoding);
            description.format = Texture_Storage_Format(encoding);
            if (description.mip_count == 0)
                description.mip_count = std::bit_width(std::max({description.width, description.height, description.depth}));
            description.generate_mips = description.mip_count > 1 && !Assets::Is_Block_Compressed(encoding);
        }
        auto resource = std::unique_ptr<TextureResource>(new TextureResource(*device,description,encoding));
        resource->m_texture = device->Create_Texture(description);
        if (!resource->m_texture.Is_Valid()) return nullptr;
        if (attachment_format != RHITextureFormat::Unknown) {
            if (!target || depth) return nullptr;
            resource->m_depth_attachment = device->Create_Texture({description.width,description.height,1,
                attachment_format,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            if (!resource->m_depth_attachment.Is_Valid()) return nullptr;
        }
        return resource.release();
    }

    static TextureResource* Create_Placeholder(Device* device)
    {
        auto* resource = Create(device,{2,2,1},Assets::PixelEncoding::BGRA8);
        if (!resource) return nullptr;
        const std::array<std::uint32_t,4> pixels{0xffff00ff,0xff000000,0xff000000,0xffff00ff};
        if (!device->Update_Texture(resource->m_texture,{std::as_bytes(std::span(pixels)),8})) {
            resource->Release();
            return nullptr;
        }
        resource->m_placeholder = true;
        return resource;
    }

    TextureResource* Retain() noexcept { ++m_references; return this; }
    void Release() noexcept { if (--m_references == 0) delete this; }
    unsigned Reference_Count() const noexcept { return m_references; }
    Device& Owner() const noexcept { return *m_device; }
    RHITextureHandle Handle() const noexcept { return m_texture; }
    RHITextureHandle Depth_Attachment() const noexcept { return m_depth_attachment; }
    const RHITexture& Description() const noexcept { return m_description; }
    Assets::PixelEncoding Encoding() const noexcept { return m_encoding; }
    bool Is_Placeholder() const noexcept { return m_placeholder; }
    bool Is_Render_Target() const noexcept
    {
        return (m_description.usage & static_cast<unsigned>(RHITextureUsage::RenderTarget)) != 0;
    }

    ~TextureResource()
    {
        if (m_depth_attachment.Is_Valid()) m_device->Destroy_Texture(m_depth_attachment);
        if (m_texture.Is_Valid()) m_device->Destroy_Texture(m_texture);
    }

private:
    TextureResource(Device& device,RHITexture description,Assets::PixelEncoding encoding)
        : m_device(&device),m_description(description),m_encoding(encoding) {}

    Device* m_device;
    const RHITexture m_description;
    const Assets::PixelEncoding m_encoding;
    RHITextureHandle m_texture{},m_depth_attachment{};
    unsigned m_references = 1;
    bool m_placeholder = false;
};

export TextureResource* Retain_Texture_Resource(TextureResource* texture) noexcept
{
    return texture ? texture->Retain() : nullptr;
}

export void Release_Texture_Resource(TextureResource* texture) noexcept
{
    if (texture) texture->Release();
}
}
