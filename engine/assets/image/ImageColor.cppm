module;
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
export module Assets.Images.Color;
export import Assets.Images.PixelEncoding;
export import Assets.Images.Buffer;

namespace Assets
{
export bool Fill_Packed_Image_Region(ImageBuffer& image,ImageRegion region,std::uint32_t packed) noexcept
{
    const unsigned stride=Pixel_Size(image.Encoding());
    if (!stride || !image.Contains(region)) return false;
    for (int y=region.top;y<region.bottom;++y) for (int x=region.left;x<region.right;++x) {
        auto pixel=image.Bytes().subspan(std::size_t(y)*image.Row_Pitch()+std::size_t(x)*stride,stride);
        for (unsigned i=0;i<stride;++i) pixel[i]=std::byte(packed>>(i*8));
    }
    return true;
}

export bool Write_Packed_Image_Pixel(ImageBuffer& image,int x,int y,std::uint32_t packed) noexcept
{
    if (x<0 || y<0 || static_cast<unsigned>(x)>=image.Width() || static_cast<unsigned>(y)>=image.Height()) return false;
    const unsigned stride=Pixel_Size(image.Encoding());
    if (!stride) return false;
    auto pixel=image.Bytes().subspan(std::size_t(y)*image.Row_Pitch()+std::size_t(x)*stride,stride);
    for (unsigned i=0;i<stride;++i) pixel[i]=std::byte(packed>>(i*8));
    return true;
}

// Replace RGB with normalized color while retaining the authored alpha bits.
// Channel truncation matches packed image editing, independently of GPU UNORM expansion.
export bool Replace_Image_RGB(std::span<std::byte> pixel,PixelEncoding encoding,std::array<float,3> rgb) noexcept
{
    for (float channel : rgb) if (!std::isfinite(channel) || channel<0 || channel>1) return false;
    const unsigned size=Pixel_Size(encoding);
    std::uint32_t argb=0;
    if (!Read_Image_Pixel(pixel,encoding,argb)) return false;
    const auto retained_last=pixel[size-1];
    argb=(argb&0xff000000u) | (static_cast<unsigned>(rgb[0]*255.0f)<<16)
        | (static_cast<unsigned>(rgb[1]*255.0f)<<8) | static_cast<unsigned>(rgb[2]*255.0f);
    if (!Write_Image_Pixel(pixel,encoding,argb)) return false;
    if (encoding==PixelEncoding::BGRX8) pixel[3]=retained_last;
    return true;
}
}
