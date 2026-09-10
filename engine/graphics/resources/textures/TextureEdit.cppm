module;
#define NOMINMAX
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Resources.Textures.Edit;
export import Assets.Images.Buffer;
export import Graphics.Resources.Textures.Resource;
import Assets.Images.Preparation;

namespace Graphics
{
export struct ImageMapping final
{
    std::span<std::byte> bytes{};
    unsigned row_pitch=0;
};

// CPU edits retain an optional GPU generation independently of its logical owner.
export class TextureEdit final
{
public:
    TextureEdit(const TextureEdit&)=delete;
    TextureEdit& operator=(const TextureEdit&)=delete;

    static TextureEdit* Create(unsigned width,unsigned height,Assets::PixelEncoding encoding)
    {
        auto image=Assets::ImageBuffer::Create(width,height,encoding);
        return image ? new TextureEdit(std::move(*image)) : nullptr;
    }

    static TextureEdit* Readback(TextureResource& texture,unsigned mip)
    {
        const auto& description=texture.Description();
        if (mip>=description.mip_count || description.dimension!=RHITextureDimension::Texture2D
            || description.array_size!=1 || Assets::Pixel_Size(texture.Encoding())==0
                && !Assets::Is_Block_Compressed(texture.Encoding())) return nullptr;
        std::unique_ptr<TextureEdit> edit(Create(std::max(1u,description.width>>mip),
            std::max(1u,description.height>>mip),texture.Encoding()));
        if (!edit || !texture.Owner().Readback_Texture_Subresource(texture.Handle(),
            {edit->m_image.Bytes(),edit->m_image.Row_Pitch(),0,mip})
            || !texture.Owner().Retain_Texture(texture.Handle())) return nullptr;
        edit->m_device=&texture.Owner(); edit->m_texture=texture.Handle(); edit->m_mip=mip;
        return edit.release();
    }

    static TextureEdit* Placeholder()
    {
        auto* edit=Create(2,2,Assets::PixelEncoding::BGRA8);
        if (!edit) return nullptr;
        const std::array<unsigned,4> pixels{0xffff00ff,0xff000000,0xff000000,0xffff00ff};
        std::ranges::copy(std::as_bytes(std::span(pixels)),edit->m_image.Bytes().begin());
        return edit;
    }

    ~TextureEdit()
    {
        if (m_write_pending) Commit();
        if (m_device) m_device->Destroy_Texture(m_texture);
    }

    Assets::ImageBuffer& Image() noexcept { return m_image; }
    const Assets::ImageBuffer& Image() const noexcept { return m_image; }

    ImageMapping Map(const Assets::ImageRegion* region=nullptr,bool read_only=false) noexcept
    {
        if (m_locked || Assets::Is_Block_Compressed(m_image.Encoding())) return {};
        if (region && !m_image.Contains(*region)) return {};
        m_locked=true; m_read_only=read_only;
        m_write_pending |= !read_only;
        auto bytes=m_image.Bytes();
        if (region) {
            const unsigned stride=Assets::Pixel_Size(m_image.Encoding());
            const std::size_t start=std::size_t(region->top)*m_image.Row_Pitch()+std::size_t(region->left)*stride;
            const std::size_t size=std::size_t(region->bottom-region->top-1)*m_image.Row_Pitch()
                +std::size_t(region->right-region->left)*stride;
            bytes=bytes.subspan(start,size);
        }
        return {bytes,m_image.Row_Pitch()};
    }

    bool Unmap() noexcept
    {
        if (!m_locked) return false;
        m_locked=false;
        return m_read_only || Commit();
    }

    bool Commit() noexcept
    {
        if (m_device && !m_device->Update_Texture(m_texture,
            {m_image.Bytes(),m_image.Row_Pitch(),0,m_mip})) return false;
        m_write_pending=false;
        return true;
    }

    bool Copy_From(const TextureEdit& source,Assets::ImageRegion from,Assets::ImageRegion to)
    {
        if (m_locked || !Assets::Copy_Image_Region(source.Image(),from,m_image,to)) return false;
        m_write_pending=true;
        return Commit();
    }

    TextureResource* Create_Texture(Device* device,unsigned mip_count) const
    {
        std::unique_ptr<TextureResource> texture(TextureResource::Create(device,
            {m_image.Width(),m_image.Height(),mip_count},m_image.Encoding()));
        if (!texture) return nullptr;
        auto pixels=m_image.Bytes();
        unsigned pitch=m_image.Row_Pitch();
        std::vector<Assets::PreparedImage> converted;
        if (texture->Encoding()!=m_image.Encoding()) {
            if (!Assets::Prepare_Image_Levels(m_image.View(),texture->Encoding(),m_image.Width(),m_image.Height(),1,{},converted))
                return nullptr;
            pixels=converted.front().bytes; pitch=static_cast<unsigned>(converted.front().row_pitch);
        }
        if (!device->Update_Texture(texture->Handle(),{pixels,pitch})) return nullptr;
        if (texture->Description().generate_mips && !device->Generate_Texture_Mips(texture->Handle())) return nullptr;
        return texture.release();
    }

private:
    explicit TextureEdit(Assets::ImageBuffer image) : m_image(std::move(image)) {}
    Assets::ImageBuffer m_image;
    Device* m_device=nullptr;
    RHITextureHandle m_texture{};
    unsigned m_mip=0;
    bool m_locked=false,m_read_only=false,m_write_pending=false;
};
}
