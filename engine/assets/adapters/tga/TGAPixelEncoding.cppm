module;
export module Assets.Adapters.TGA.PixelEncoding;
export import Assets.Images.PixelEncoding;

namespace Assets
{
// Classify decoded TGA pixels independently of archive access and GPU storage.
export constexpr PixelEncoding TGA_Pixel_Encoding(unsigned bits,unsigned color_map_type,unsigned image_type) noexcept
{
    switch (bits) {
    case 32: return PixelEncoding::BGRA8;
    case 24: return PixelEncoding::BGR8;
    case 16: return image_type == 3 || image_type == 11 ? PixelEncoding::LuminanceAlpha88 : PixelEncoding::BGRA5551;
    case 8:
        if (color_map_type == 1) return PixelEncoding::Indexed8;
        if (image_type == 3 || image_type == 11) return PixelEncoding::Luminance8;
        return PixelEncoding::Alpha8;
    default: return PixelEncoding::Unknown;
    }
}
}
