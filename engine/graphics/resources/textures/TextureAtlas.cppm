module;
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
export module Graphics.Resources.Textures.Atlas;
import Graphics.Resources.Textures.Resource;
import Graphics.Resources.Textures.Upload;
export import Assets.Images.PixelEncoding;

namespace Graphics
{
export struct AtlasTile final
{
    Assets::ImageView image;
    unsigned x=0,y=0;
    bool flip_vertical=true;
};
export struct AtlasRepeatBorder final { unsigned x=0,y=0,width=0,height=0,border=0; };
export enum class AtlasAlpha { Source, EdgeMask };
export enum class AtlasBackground { Transparent, Opaque, EdgeGradient };

// Inputs are borrowed for this synchronous upload. Layout belongs to the caller;
// graphics owns conversion, periodic gutters, upload lifetime, and mip creation.
export bool Upload_Texture_Atlas(TextureResource& texture,std::span<const AtlasTile> tiles,
    std::span<const AtlasRepeatBorder> borders,AtlasAlpha alpha,AtlasBackground background,bool generate_mips)
{
    const auto& description=texture.Description();
    const auto encoding=texture.Encoding();
    if (description.dimension!=RHITextureDimension::Texture2D || description.array_size!=1
        || (encoding!=Assets::PixelEncoding::BGRA8 && encoding!=Assets::PixelEncoding::RGBA8)) return false;
    for (const auto& tile : tiles) {
        if (!tile.image.Is_Valid() || tile.image.encoding!=Assets::PixelEncoding::BGRA8
            || tile.x>description.width || tile.image.width>description.width-tile.x
            || tile.y>description.height || tile.image.height>description.height-tile.y) return false;
    }
    for (const auto& border : borders) {
        if (!border.width || !border.height || border.border>border.width || border.border>border.height
            || border.x<border.border || border.y<border.border
            || border.x>description.width || border.width>description.width-border.x
            || border.border>description.width-border.x-border.width
            || border.y>description.height || border.height>description.height-border.y
            || border.border>description.height-border.y-border.height) return false;
    }
    TextureUpload upload;
    if (!upload.Begin(texture.Owner(),texture.Handle(),1)) return false;
    const auto mapping=upload.Mapping(0);
    const unsigned stride=Assets::Pixel_Size(encoding);
    const auto write=[&](unsigned x,unsigned y,std::uint32_t color) {
        return Assets::Write_Image_Pixel(mapping.bytes.subspan(std::size_t(y)*mapping.row_pitch+std::size_t(x)*stride,stride),encoding,color);
    };
    for (unsigned y=0;y<description.height;++y) for (unsigned x=0;x<description.width;++x) {
        std::uint32_t color=background==AtlasBackground::Opaque ? 0xff000000u : 0;
        if (background==AtlasBackground::EdgeGradient)
            color=0x80000000u | (std::uint32_t(std::uint8_t(255-y/2))<<16) | std::uint8_t(x/2);
        write(x,y,color);
    }
    for (const auto& tile : tiles) {
        for (unsigned y=0;y<tile.image.height;++y) {
            const auto row=tile.image.bytes.subspan(std::size_t(tile.flip_vertical ? tile.image.height-1-y : y)*tile.image.row_pitch);
            if (encoding==Assets::PixelEncoding::BGRA8 && alpha!=AtlasAlpha::EdgeMask) {
                std::memcpy(mapping.bytes.data()+std::size_t(tile.y+y)*mapping.row_pitch+std::size_t(tile.x)*4,
                    row.data(),std::size_t(tile.image.width)*4);
                continue;
            }
            for (unsigned x=0;x<tile.image.width;++x) {
                std::uint32_t color=0;
                Assets::Read_Image_Pixel(row.subspan(std::size_t(x)*4,4),Assets::PixelEncoding::BGRA8,color);
                if (alpha==AtlasAlpha::EdgeMask) {
                    const auto rgb=color&0xffffffu;
                    color=rgb | (rgb==0 ? 0x80000000u : rgb==0xffffffu ? 0 : 0xff000000u);
                }
                write(tile.x+x,tile.y+y,color);
            }
        }
    }
    for (const auto& border : borders) {
        const auto pixel=[&](unsigned x,unsigned y) { return mapping.bytes.data()+std::size_t(y)*mapping.row_pitch+std::size_t(x)*stride; };
        for (unsigned y=0;y<border.height;++y) {
            std::memcpy(pixel(border.x-border.border,border.y+y),pixel(border.x+border.width-border.border,border.y+y),std::size_t(border.border)*stride);
            std::memcpy(pixel(border.x+border.width,border.y+y),pixel(border.x,border.y+y),std::size_t(border.border)*stride);
        }
        for (unsigned y=0;y<border.border;++y) {
            const auto bytes=std::size_t(border.width+2*border.border)*stride;
            std::memcpy(pixel(border.x-border.border,border.y-1-y),pixel(border.x-border.border,border.y+border.height-1-y),bytes);
            std::memcpy(pixel(border.x-border.border,border.y+border.height+y),pixel(border.x-border.border,border.y+y),bytes);
        }
    }
    return upload.Finish() && (!generate_mips || description.mip_count==1
        || texture.Owner().Generate_Texture_Mips(texture.Handle()));
}
}
