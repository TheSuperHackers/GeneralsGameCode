module;
#define NOMINMAX
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>
export module Graphics.Resources.Textures.Load;
export import Graphics.Resources.Loading.Queue;
export import Graphics.Resources.Textures.Resource;
import Graphics.Resources.Textures.Upload;
import Assets.Adapters.DDS;
import Assets.Adapters.TGA.Image;
import Assets.Images.Preparation;

namespace Graphics
{
// Read a prefix, or the entire source when prefix_size is zero, and report the
// complete source size. Archive/path policy stays with the supplying adapter.
// Header reads run during Prepare; payload reads run during Decode.
export using TextureImageReader = std::function<bool(std::size_t prefix_size,
    std::vector<std::byte>& bytes, std::size_t& source_size)>;

export struct TextureLoadRequest final
{
    Device* device = nullptr;
    RHITextureDimension dimension = RHITextureDimension::Texture2D;
    Assets::PixelEncoding encoding = Assets::PixelEncoding::Unknown;
    TextureMipPreferences mips;
    bool allow_compression = true;
    bool prefer_16_bits = false;
    std::array<float,3> hsv_shift{};
    TextureImageReader read_dds, read_tga;
};

// The queue owns this job. Only Decode runs on its worker. Publication takes
// ownership of the completed resource on the device thread and must not throw.
// The callback also retains any caller lifetime needed through completion.
export class TextureLoadJob final : public ResourceLoadJob
{
public:
    using Publish = std::function<void(TextureResource*)>;
    TextureLoadJob(TextureLoadRequest request, Publish publish)
        : m_request(std::move(request)), m_publish(std::move(publish)) {}
    ~TextureLoadJob() override
    {
        m_upload.Finish();
        Release_Texture_Resource(m_resource);
    }

    bool Prepare() override
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Texture.Prepare");
        if (m_started || !m_request.device) return false;
        m_started = true;
        if (!(m_request.allow_compression && Allocate_DDS()) && !Allocate_TGA()) return false;
        const auto& description = m_resource->Description();
        m_prepared = m_upload.Begin(m_resource->Owner(), m_resource->Handle(),
            description.mip_count, description.array_size);
        return m_prepared;
    }

    bool Decode() override
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Texture.Decode");
        if (!m_prepared || m_completed) return false;
        m_decoded = (m_request.allow_compression && Decode_DDS()) || Decode_TGA();
        return m_decoded;
    }

    void Complete(bool decoded) noexcept override
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Texture.Complete");
        if (m_completed) return;
        m_completed = true;
        const bool uploaded = m_upload.Finish();
        if (!decoded || !m_decoded || !uploaded) {
            Release_Texture_Resource(std::exchange(m_resource,nullptr));
            try { m_resource = TextureResource::Create_Placeholder(m_request.device); }
            catch (...) { m_resource = nullptr; }
        }
        if (m_publish) m_publish(std::exchange(m_resource,nullptr));
    }

private:
    bool Matches_Dimension(Assets::DDSDimension dimension) const noexcept
    {
        if (m_request.dimension == RHITextureDimension::Cube) return dimension == Assets::DDSDimension::Cube;
        if (m_request.dimension == RHITextureDimension::Volume) return dimension == Assets::DDSDimension::Volume;
        return dimension == Assets::DDSDimension::Texture;
    }

    bool Allocate(TextureExtent extent, unsigned mips, Assets::PixelEncoding encoding)
    {
        RHITexture description{extent.width,extent.height,mips};
        description.dimension = m_request.dimension;
        description.array_size = m_request.dimension == RHITextureDimension::Cube ? 6u : 1u;
        description.depth = m_request.dimension == RHITextureDimension::Volume ? extent.depth : 1u;
        m_resource = TextureResource::Create(m_request.device,description,encoding);
        return m_resource != nullptr;
    }

    bool Allocate_DDS()
    {
        if (!m_request.read_dds) return false;
        std::vector<std::byte> bytes;
        std::size_t source_size = 0;
        Assets::DDSLayout layout;
        if (!m_request.read_dds(128,bytes,source_size)
            || !Assets::Read_DDS_Layout(bytes,source_size,layout)
            || !Matches_Dimension(layout.dimension)) return false;
        const auto* surface = layout.Surface(0);
        if (!surface) return false;
        TextureMipSelection selection;
        if (!Select_Compressed_Texture_Mips({surface->width,surface->height,surface->depth},
            layout.mip_count,m_request.mips,m_request.device->Texture_Limits(),selection)) return false;
        const auto encoding = Select_Texture_Encoding(Assets::DDS_Pixel_Encoding(layout),
            m_request.allow_compression,m_request.prefer_16_bits);
        if (!Allocate(selection.extent,selection.mip_count,encoding)) return false;
        m_first_mip = selection.first_mip;
        return true;
    }

    bool Allocate_TGA()
    {
        if (!m_request.read_tga) return false;
        std::vector<std::byte> bytes;
        std::size_t source_size = 0;
        Assets::TGAImageInfo info;
        if (!m_request.read_tga(18,bytes,source_size)
            || !Assets::Read_TGA_Info(bytes,source_size,info)) return false;
        const auto extent = Select_Texture_Extent({info.width,info.height,1},m_request.device->Texture_Limits());
        if (!extent.width || !extent.height || !extent.depth) return false;
        const auto encoding = Select_Texture_Encoding(m_request.encoding == Assets::PixelEncoding::Unknown
            ? info.encoding : m_request.encoding,false,m_request.prefer_16_bits);
        m_first_mip = 0;
        return Allocate(extent,m_request.mips.requested_count,encoding);
    }

    bool Decode_DDS()
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Texture.DecodeDDS");
        if (!m_request.read_dds) return false;
        std::vector<std::byte> bytes;
        std::size_t source_size = 0;
        Assets::DDSLayout layout;
        if (!m_request.read_dds(0,bytes,source_size)
            || !Assets::Read_DDS_Layout(bytes,source_size,layout)
            || !Matches_Dimension(layout.dimension)) return false;
        const auto& description = m_resource->Description();
        const auto encoding = m_resource->Encoding();
        const unsigned first = std::min(m_first_mip,layout.mip_count-1);
        for (unsigned layer=0;layer<description.array_size;++layer) {
            unsigned width=description.width, height=description.height, depth=description.depth;
            for (unsigned mip=0;mip<description.mip_count;++mip) {
                const auto mapping = m_upload.Mapping(mip,layer);
                const unsigned slice_pitch = description.dimension == RHITextureDimension::Volume
                    ? mapping.slice_pitch : mapping.row_pitch * (Assets::Is_Block_Compressed(encoding) ? (height+3)/4 : height);
                if (!Assets::Copy_DDS_Image(bytes,layout,first+mip,layer,encoding,width,height,depth,
                    mapping.bytes,mapping.row_pitch,slice_pitch,m_request.hsv_shift)) return false;
                width=std::max(1u,width>>1); height=std::max(1u,height>>1); depth=std::max(1u,depth>>1);
            }
        }
        return true;
    }

    bool Decode_TGA()
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Texture.DecodeTGA");
        if (!m_request.read_tga) return false;
        std::vector<std::byte> bytes;
        std::size_t source_size = 0;
        Assets::TGAImage image;
        if (!m_request.read_tga(0,bytes,source_size) || !Assets::Decode_TGA_Image(bytes,image)) return false;
        const auto& description = m_resource->Description();
        std::vector<Assets::PreparedImage> levels;
        if (!Assets::Prepare_Image_Levels(image.View(),m_resource->Encoding(),description.width,
            description.height,description.mip_count,
            {m_request.hsv_shift[0],m_request.hsv_shift[1],m_request.hsv_shift[2]},levels)) return false;
        // A single image supplies the same pixels to every requested face/slice.
        for (unsigned layer=0;layer<description.array_size;++layer) {
            for (unsigned mip=0;mip<levels.size();++mip) {
                const auto mapping = m_upload.Mapping(mip,layer);
                const unsigned depth = std::max(1u,description.depth>>mip);
                for (unsigned slice=0;slice<depth;++slice) {
                    const std::size_t offset = std::size_t(slice)*mapping.slice_pitch;
                    if (offset > mapping.bytes.size() || !Assets::Copy_Prepared_Image(levels[mip],
                        mapping.bytes.subspan(offset),mapping.row_pitch)) return false;
                }
            }
        }
        return true;
    }

    const TextureLoadRequest m_request;
    const Publish m_publish;
    TextureResource* m_resource = nullptr;
    TextureUpload m_upload;
    unsigned m_first_mip = 0;
    bool m_started = false, m_prepared = false, m_decoded = false, m_completed = false;
};
}
