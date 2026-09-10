module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

export module Assets.Images.PixelEncoding;

namespace Assets
{
export enum class PixelEncoding : std::uint8_t
{
    Unknown, RGBA8, BGRA8, BGRX8, RGB8, BGR8, ARGB8,
    BGR565, BGRA5551, BGRX5551, BGRA4444, BGRX4444, RGB332,
    Alpha8, Luminance8, LuminanceAlpha88, LuminanceAlpha44, Indexed8,
    RG8_SNorm, RG5_SNorm_L6, RG8_SNorm_L8X8, RGB332Alpha8,
    IndexedAlpha88, BC1, BC2, BC2Premultiplied, BC3, BC3Premultiplied
};

export unsigned Pixel_Size(PixelEncoding encoding) noexcept
{
    switch (encoding) {
    case PixelEncoding::RGBA8: case PixelEncoding::BGRA8: case PixelEncoding::BGRX8:
    case PixelEncoding::ARGB8: case PixelEncoding::RG8_SNorm_L8X8: return 4;
    case PixelEncoding::RGB8: case PixelEncoding::BGR8: return 3;
    case PixelEncoding::BGR565: case PixelEncoding::BGRA5551: case PixelEncoding::BGRX5551:
    case PixelEncoding::BGRA4444: case PixelEncoding::BGRX4444: case PixelEncoding::LuminanceAlpha88:
    case PixelEncoding::RG8_SNorm: case PixelEncoding::RG5_SNorm_L6:
    case PixelEncoding::RGB332Alpha8: case PixelEncoding::IndexedAlpha88: return 2;
    case PixelEncoding::RGB332: case PixelEncoding::Alpha8: case PixelEncoding::Luminance8:
    case PixelEncoding::LuminanceAlpha44: case PixelEncoding::Indexed8: return 1;
    default: return 0;
    }
}

export constexpr bool Is_Block_Compressed(PixelEncoding encoding) noexcept
{
    return encoding == PixelEncoding::BC1 || encoding == PixelEncoding::BC2
        || encoding == PixelEncoding::BC2Premultiplied || encoding == PixelEncoding::BC3
        || encoding == PixelEncoding::BC3Premultiplied;
}

export constexpr unsigned Pixel_Alpha_Bits(PixelEncoding encoding) noexcept
{
    switch (encoding) {
    case PixelEncoding::RGBA8: case PixelEncoding::BGRA8: case PixelEncoding::ARGB8:
    case PixelEncoding::Alpha8: case PixelEncoding::RGB332Alpha8:
    case PixelEncoding::IndexedAlpha88: case PixelEncoding::LuminanceAlpha88: return 8;
    case PixelEncoding::BGRA4444: case PixelEncoding::LuminanceAlpha44: return 4;
    case PixelEncoding::BGRA5551: return 1;
    default: return 0; // Block and palette encodings do not have scalar alpha bits.
    }
}

export constexpr bool Has_Explicit_Pixel_Alpha(PixelEncoding encoding) noexcept
{
    return Pixel_Alpha_Bits(encoding) != 0 || encoding == PixelEncoding::BC2
        || encoding == PixelEncoding::BC2Premultiplied || encoding == PixelEncoding::BC3
        || encoding == PixelEncoding::BC3Premultiplied;
}

export struct ImagePalette final
{
    std::span<const std::byte> bytes;
    PixelEncoding encoding = PixelEncoding::BGR8;
};

export bool Read_Image_Pixel(std::span<const std::byte> bytes, PixelEncoding encoding,
    std::uint32_t& argb, ImagePalette palette = {}) noexcept
{
    const unsigned size = Pixel_Size(encoding);
    if (size == 0 || bytes.size() < size) return false;
    const auto u8 = [&](unsigned i) { return std::to_integer<unsigned>(bytes[i]); };
    const auto u16 = [&] { return u8(0) | (u8(1) << 8); };
    unsigned r = 0, g = 0, b = 0, a = 255;
    switch (encoding) {
    case PixelEncoding::RGBA8: r = u8(0); g = u8(1); b = u8(2); a = u8(3); break;
    case PixelEncoding::BGRA8: b = u8(0); g = u8(1); r = u8(2); a = u8(3); break;
    case PixelEncoding::BGRX8: case PixelEncoding::BGR8: b = u8(0); g = u8(1); r = u8(2); break;
    case PixelEncoding::RGB8: r = u8(0); g = u8(1); b = u8(2); break;
    case PixelEncoding::ARGB8: a = u8(0); r = u8(1); g = u8(2); b = u8(3); break;
    // Packed texture preparation uses bit shifts without replicated low bits.
    case PixelEncoding::BGR565: {
        const auto value = u16(); r = (value >> 8) & 248; g = (value >> 3) & 252; b = (value << 3) & 248; break;
    }
    case PixelEncoding::BGRA5551: case PixelEncoding::BGRX5551: {
        const auto value = u16(); r = (value >> 7) & 248; g = (value >> 2) & 248; b = (value << 3) & 248;
        a = encoding == PixelEncoding::BGRX5551 || (value & 32768) ? 255 : 0; break;
    }
    case PixelEncoding::BGRA4444: case PixelEncoding::BGRX4444: {
        const auto value = u16(); r = (value >> 4) & 240; g = value & 240; b = (value << 4) & 240;
        a = encoding == PixelEncoding::BGRX4444 ? 255 : (value >> 8) & 240; break;
    }
    case PixelEncoding::RGB332: r = u8(0) & 224; g = (u8(0) << 3) & 224; b = (u8(0) << 6) & 192; break;
    case PixelEncoding::RGB332Alpha8:
        r = u8(0) & 224; g = (u8(0) << 3) & 224; b = (u8(0) << 6) & 192; a = u8(1); break;
    case PixelEncoding::Alpha8: a = u8(0); break;
    case PixelEncoding::Luminance8: r = g = b = u8(0); break;
    case PixelEncoding::LuminanceAlpha88: r = g = b = u8(0); a = u8(1); break;
    case PixelEncoding::LuminanceAlpha44: r = g = b = (u8(0) & 15) << 4; a = u8(0) & 240; break;
    case PixelEncoding::Indexed8: {
        const unsigned entry_size = Pixel_Size(palette.encoding);
        if (entry_size == 0 || palette.encoding == PixelEncoding::Indexed8) return false;
        const std::size_t offset = std::size_t(u8(0)) * entry_size;
        if (offset > palette.bytes.size() || entry_size > palette.bytes.size() - offset) return false;
        return Read_Image_Pixel(palette.bytes.subspan(offset, entry_size), palette.encoding, argb);
    }
    default: return false;
    }
    argb = (a << 24) | (r << 16) | (g << 8) | b;
    return true;
}

export bool Write_Image_Pixel(std::span<std::byte> bytes, PixelEncoding encoding, std::uint32_t argb) noexcept
{
    const unsigned size = Pixel_Size(encoding);
    if (size == 0 || bytes.size() < size) return false;
    const unsigned b = argb & 255, g = (argb >> 8) & 255, r = (argb >> 16) & 255, a = argb >> 24;
    const auto put = [&](unsigned i, unsigned value) { bytes[i] = std::byte(value); };
    const auto put16 = [&](unsigned value) { put(0, value); put(1, value >> 8); };
    const auto luminance = [&] { return (b * 0x1275 + g * 0xb725 + r * 0x3666) >> 16; };
    switch (encoding) {
    case PixelEncoding::RGBA8: put(0, r); put(1, g); put(2, b); put(3, a); break;
    case PixelEncoding::BGRA8: put(0, b); put(1, g); put(2, r); put(3, a); break;
    case PixelEncoding::BGRX8: put(0, b); put(1, g); put(2, r); put(3, 255); break;
    case PixelEncoding::RGB8: put(0, r); put(1, g); put(2, b); break;
    case PixelEncoding::BGR8: put(0, b); put(1, g); put(2, r); break;
    case PixelEncoding::ARGB8: put(0, a); put(1, r); put(2, g); put(3, b); break;
    case PixelEncoding::BGR565: put16(((r & 248) << 8) | ((g & 252) << 3) | (b >> 3)); break;
    case PixelEncoding::BGRA5551: case PixelEncoding::BGRX5551:
        put16(((r & 248) << 7) | ((g & 248) << 2) | (b >> 3)
            | (encoding == PixelEncoding::BGRX5551 || a != 0 ? 32768 : 0)); break;
    case PixelEncoding::BGRA4444: case PixelEncoding::BGRX4444:
        put16(((r & 240) << 4) | (g & 240) | (b >> 4)
            | (encoding == PixelEncoding::BGRX4444 ? 0xf000 : (a & 240) << 8)); break;
    case PixelEncoding::RGB332: put(0, (r & 224) | ((g & 224) >> 3) | (b >> 6)); break;
    case PixelEncoding::RGB332Alpha8:
        put(0, (r & 224) | ((g & 224) >> 3) | (b >> 6)); put(1, a); break;
    case PixelEncoding::Alpha8: put(0, a); break;
    case PixelEncoding::Luminance8: put(0, luminance()); break;
    case PixelEncoding::LuminanceAlpha88: put(0, luminance()); put(1, a); break;
    case PixelEncoding::LuminanceAlpha44: put(0, (luminance() >> 4) | (a & 240)); break;
    default: return false;
    }
    return true;
}

// Pack a solid drawing color into a little-endian pixel word. One-bit alpha
// uses its high bit; luminance uses integer BT.601 weights. Image preparation
// has its own quantization contract and continues to use Write_Image_Pixel.
// Encodings that require a palette or multiple pixels have no single-color word.
export std::uint32_t Pack_Image_Color(PixelEncoding encoding, std::uint32_t argb) noexcept
{
    if (encoding == PixelEncoding::BGRA5551 && (argb >> 24) < 128)
        argb &= 0x00ffffff;
    if (encoding == PixelEncoding::Luminance8 || encoding == PixelEncoding::LuminanceAlpha88
        || encoding == PixelEncoding::LuminanceAlpha44) {
        const auto l = (((argb >> 16) & 255)*77 + ((argb >> 8) & 255)*150 + (argb & 255)*29) >> 8;
        argb = (argb & 0xff000000) | (l*0x010101);
    }
    std::array<std::byte,4> bytes{};
    if (!Write_Image_Pixel(bytes,encoding,argb)) return 0;
    std::uint32_t result=0;
    for (unsigned i=0;i<bytes.size();++i)
        result |= std::to_integer<std::uint32_t>(bytes[i]) << (i*8);
    return result;
}

export bool Convert_Image_Pixel(std::span<std::byte> destination, PixelEncoding destination_encoding,
    std::span<const std::byte> source, PixelEncoding source_encoding, ImagePalette palette = {}) noexcept
{
    const unsigned size = Pixel_Size(source_encoding);
    if (size != 0 && source_encoding == destination_encoding) {
        if (source.size() < size || destination.size() < size) return false;
        std::memmove(destination.data(), source.data(), size);
        return true;
    }
    std::uint32_t color;
    return Read_Image_Pixel(source, source_encoding, color, palette)
        && Write_Image_Pixel(destination, destination_encoding, color);
}

export struct ImageView final
{
    std::span<const std::byte> bytes;
    std::uint32_t width = 0, height = 0;
    std::size_t row_pitch = 0;
    PixelEncoding encoding = PixelEncoding::Unknown;
    ImagePalette palette;

    bool Is_Valid() const noexcept
    {
        const unsigned pixel_size = Pixel_Size(encoding);
        if (width == 0 || height == 0 || pixel_size == 0 || row_pitch / pixel_size < width) return false;
        const auto row_size = std::size_t(width) * pixel_size;
        return row_size <= bytes.size() && height - 1 <= (bytes.size() - row_size) / row_pitch;
    }
};
}
