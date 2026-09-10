module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <utility>
#include <vector>
export module Assets.Adapters.TGA.Image;
export import Assets.Adapters.TGA.PixelEncoding;

namespace Assets
{
export struct TGAImageInfo final
{
    unsigned width = 0, height = 0;
    PixelEncoding encoding = PixelEncoding::Unknown;
    PixelEncoding palette_encoding = PixelEncoding::Unknown;
    unsigned palette_first = 0, palette_count = 0;
    std::size_t palette_offset = 0, pixel_offset = 0;
    bool run_length_encoded = false, top_origin = false, right_origin = false;
    unsigned row_interleave = 1;
};

// Header-only callers supply the full file size. Packet contents and palette
// indices are validated by decoding before any image is published.
export bool Read_TGA_Info(std::span<const std::byte> header, std::size_t file_size, TGAImageInfo& output)
{
    if (header.size() < 18 || file_size < 18) return false;
    const auto u8 = [&](unsigned offset) { return std::to_integer<unsigned>(header[offset]); };
    const auto u16 = [&](unsigned offset) { return u8(offset) | (u8(offset + 1) << 8); };
    const unsigned type = u8(2), color_map = u8(1), bits = u8(16), descriptor = u8(17);
    const bool indexed = type == 1 || type == 9;
    const bool grayscale = type == 3 || type == 11;
    if ((!indexed && !grayscale && type != 2 && type != 10) || color_map > 1
        || (descriptor >> 6) == 3 || (indexed && (color_map != 1 || bits != 8))
        || (grayscale && bits != 8 && bits != 16)) return false;
    TGAImageInfo result;
    result.width = u16(12); result.height = u16(14);
    result.encoding = TGA_Pixel_Encoding(bits, color_map, type);
    const unsigned pixel_size = Pixel_Size(result.encoding);
    if (result.width == 0 || result.height == 0 || pixel_size == 0
        || result.width > std::numeric_limits<std::size_t>::max() / result.height / pixel_size) return false;
    result.run_length_encoded = type >= 9;
    result.top_origin = (descriptor & 32) != 0;
    result.right_origin = (descriptor & 16) != 0;
    result.row_interleave = 1u << (descriptor >> 6);
    result.palette_offset = 18 + u8(0);
    if (result.palette_offset > file_size) return false;
    std::size_t palette_bytes = 0;
    if (color_map == 1) {
        result.palette_first = u16(3); result.palette_count = u16(5);
        const unsigned palette_bits = u8(7);
        if ((palette_bits != 16 && palette_bits != 24 && palette_bits != 32)
            || result.palette_count == 0 || result.palette_first + result.palette_count > 256) return false;
        result.palette_encoding = TGA_Pixel_Encoding(palette_bits, 0, 2);
        palette_bytes = std::size_t(result.palette_count) * Pixel_Size(result.palette_encoding);
    }
    if (palette_bytes > file_size - result.palette_offset) return false;
    result.pixel_offset = result.palette_offset + palette_bytes;
    const std::size_t pixels = std::size_t(result.width) * result.height;
    const std::size_t minimum_bytes = result.run_length_encoded
        ? ((pixels + 127) / 128) * (pixel_size + 1) : pixels * pixel_size;
    if (minimum_bytes > file_size - result.pixel_offset) return false;
    output = result;
    return true;
}

export struct TGAImage final
{
    TGAImageInfo info;
    std::vector<std::byte> pixels, palette;
    ImageView View() const noexcept
    {
        return {pixels, info.width, info.height, std::size_t(info.width) * Pixel_Size(info.encoding),
            info.encoding, {palette, info.palette_encoding}};
    }
};

namespace TGAImageDetail
{
template<class Visitor>
bool Visit_Packets(std::span<const std::byte> source, const TGAImageInfo& info, Visitor visit)
{
    const unsigned pixel_size = Pixel_Size(info.encoding);
    std::size_t remaining = std::size_t(info.width) * info.height;
    std::size_t offset = info.pixel_offset;
    while (remaining != 0) {
        std::size_t count = remaining;
        bool repeated = false;
        if (info.run_length_encoded) {
            if (offset == source.size()) return false;
            const unsigned packet = std::to_integer<unsigned>(source[offset++]);
            count = (packet & 127) + 1;
            repeated = (packet & 128) != 0;
        }
        if (count > remaining) return false;
        const std::size_t encoded_count = repeated ? 1 : count;
        if (encoded_count > (source.size() - offset) / pixel_size) return false;
        const auto bytes = source.subspan(offset, encoded_count * pixel_size);
        if (info.encoding == PixelEncoding::Indexed8) {
            for (const auto encoded : bytes) {
                const unsigned index = std::to_integer<unsigned>(encoded);
                if (index < info.palette_first || index - info.palette_first >= info.palette_count) return false;
            }
        }
        visit(bytes, count, repeated);
        offset += bytes.size();
        remaining -= count;
    }
    return true;
}
}

// Keep authored bytes and palette precision. The output origin is always top
// left, so image preparation needs neither a file reader nor format-specific flips.
export bool Decode_TGA_Image(std::span<const std::byte> source, TGAImage& output)
{
    TGAImage result;
    if (!Read_TGA_Info(source, source.size(), result.info)) return false;
    const auto& info = result.info;
    if (!TGAImageDetail::Visit_Packets(source, info, [](auto, auto, auto) {})) return false;
    const unsigned pixel_size = Pixel_Size(info.encoding);
    result.pixels.resize(std::size_t(info.width) * info.height * pixel_size);
    if (info.palette_count != 0) {
        const unsigned palette_size = Pixel_Size(info.palette_encoding);
        result.palette.resize(std::size_t(info.palette_first + info.palette_count) * palette_size);
        std::memcpy(result.palette.data() + std::size_t(info.palette_first) * palette_size,
            source.data() + info.palette_offset, std::size_t(info.palette_count) * palette_size);
    }
    std::size_t pixel_index = 0;
    TGAImageDetail::Visit_Packets(source, info, [&](std::span<const std::byte> bytes, std::size_t count, bool repeated) {
        for (std::size_t i = 0; i < count; ++i, ++pixel_index) {
            unsigned x = unsigned(pixel_index % info.width), y = unsigned(pixel_index / info.width);
            // Interleaved files store each row group consecutively.
            for (unsigned pass = 0; pass < info.row_interleave; ++pass) {
                const unsigned rows = (info.height + info.row_interleave - 1 - pass) / info.row_interleave;
                if (y < rows) { y = pass + y * info.row_interleave; break; }
                y -= rows;
            }
            if (info.right_origin) x = info.width - 1 - x;
            if (!info.top_origin) y = info.height - 1 - y;
            std::memcpy(result.pixels.data() + (std::size_t(y) * info.width + x) * pixel_size,
                bytes.data() + (repeated ? 0 : i * pixel_size), pixel_size);
        }
    });
    output = std::move(result);
    return true;
}
}
