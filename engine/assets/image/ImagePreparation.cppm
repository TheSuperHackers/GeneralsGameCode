module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Assets.Images.Preparation;
export import Assets.Images.PixelEncoding;
export import Assets.Math;

namespace Assets
{
export struct PreparedImage final
{
    std::uint32_t width = 0, height = 0;
    std::size_t row_pitch = 0;
    PixelEncoding encoding = PixelEncoding::Unknown;
    std::vector<std::byte> bytes;

    ImageView View() const noexcept { return {bytes, width, height, row_pitch, encoding, {}}; }
};

namespace ImagePreparationDetail
{
bool Is_Gradient(PixelEncoding encoding)
{
    return encoding == PixelEncoding::RG8_SNorm || encoding == PixelEncoding::RG5_SNorm_L6
        || encoding == PixelEncoding::RG8_SNorm_L8X8;
}

bool Encode_Level(std::span<const std::uint32_t> colors, PreparedImage& image)
{
    const unsigned pixel_size = Pixel_Size(image.encoding);
    if (!Is_Gradient(image.encoding)) {
        for (std::size_t i = 0; i < colors.size(); ++i)
            if (!Write_Image_Pixel(std::span(image.bytes).subspan(i * pixel_size, pixel_size), image.encoding, colors[i]))
                return false;
        return true;
    }
    const auto luminance = [&](unsigned x, unsigned y) {
        const auto color = colors[std::size_t(y) * image.width + x];
        return int(((color & 255) * 0x1275 + ((color >> 8) & 255) * 0xb725
            + ((color >> 16) & 255) * 0x3666) >> 16);
    };
    for (unsigned y = 0; y < image.height; ++y) {
        for (unsigned x = 0; x < image.width; ++x) {
            const int center = luminance(x, y);
            const int left = luminance(x == 0 ? 0 : x - 1, y);
            const int right = luminance(std::min(x + 1, image.width - 1), y);
            const int up = luminance(x, y == 0 ? 0 : y - 1);
            const int down = luminance(x, std::min(y + 1, image.height - 1));
            int du = left - right;
            const int dv = down - up;
            if (center < left && center < right) du = std::max(left - center, center - right);
            const unsigned intensity = center > 1 ? 63 : 127;
            auto* destination = image.bytes.data() + std::size_t(y) * image.row_pitch + x * pixel_size;
            if (image.encoding == PixelEncoding::RG5_SNorm_L6) {
                const unsigned value = ((du >> 3) & 31) | (((dv >> 3) & 31) << 5) | ((intensity >> 2) << 10);
                destination[0] = std::byte(value); destination[1] = std::byte(value >> 8);
            } else {
                destination[0] = std::byte(du); destination[1] = std::byte(dv);
                if (pixel_size == 4) { destination[2] = std::byte(intensity); destination[3] = std::byte{0}; }
            }
        }
    }
    return true;
}
}

// Source storage is immutable. Resizing uses point samples; subsequent levels
// use a box filter with per-sample byte truncation, preserving authored load behavior.
// Gradient encodings retain signed derivatives and the two-level luminance channel.
export bool Prepare_Image_Levels(const ImageView& source, PixelEncoding encoding,
    std::uint32_t width, std::uint32_t height, unsigned level_count, Vector3f hsv_shift,
    std::vector<PreparedImage>& output)
{
    const auto pixel_size = Pixel_Size(encoding);
    if (!source.Is_Valid() || width == 0 || height == 0 || level_count == 0 || pixel_size == 0
        || encoding == PixelEncoding::Indexed8 || !std::isfinite(hsv_shift.x)
        || !std::isfinite(hsv_shift.y) || !std::isfinite(hsv_shift.z)) return false;
    unsigned max_levels = 1;
    for (unsigned dimension = std::max(width, height); dimension > 1; dimension >>= 1) ++max_levels;
    if (level_count > max_levels || width > (std::numeric_limits<std::size_t>::max() / 4) / height) return false;
    std::vector<std::uint32_t> colors(std::size_t(width) * height);
    const unsigned source_pixel_size = Pixel_Size(source.encoding);
    const bool recolor = hsv_shift.x != 0 || hsv_shift.y != 0 || hsv_shift.z != 0;
    for (unsigned y = 0; y < height; ++y) {
        const auto source_y = std::uint64_t(y) * source.height / height;
        for (unsigned x = 0; x < width; ++x) {
            const auto source_x = std::uint64_t(x) * source.width / width;
            const std::size_t offset = std::size_t(source_y) * source.row_pitch + std::size_t(source_x) * source_pixel_size;
            auto& color = colors[std::size_t(y) * width + x];
            if (!Read_Image_Pixel(source.bytes.subspan(offset, source_pixel_size), source.encoding, color, source.palette))
                return false;
            if (recolor) color = Shift_Color_ARGB(color, hsv_shift);
        }
    }
    std::vector<PreparedImage> result;
    result.reserve(level_count);
    for (unsigned level = 0; level < level_count; ++level) {
        PreparedImage image;
        image.width = width; image.height = height; image.encoding = encoding;
        image.row_pitch = std::size_t(width) * pixel_size;
        image.bytes.resize(image.row_pitch * height);
        if (!ImagePreparationDetail::Encode_Level(colors, image)) return false;
        result.push_back(std::move(image));
        if (level + 1 == level_count) break;
        const unsigned next_width = std::max(1u, width / 2), next_height = std::max(1u, height / 2);
        // Earlier output positions cannot overwrite samples needed by later blocks.
        for (unsigned y = 0; y < next_height; ++y) {
            for (unsigned x = 0; x < next_width; ++x) {
                const unsigned x0 = x * 2, x1 = std::min(x0 + 1, width - 1);
                const unsigned y0 = y * 2, y1 = std::min(y0 + 1, height - 1);
                const auto quarter = [](std::uint32_t color) { return (color & 0xfcfcfcfc) >> 2; };
                colors[std::size_t(y) * next_width + x] = quarter(colors[std::size_t(y0) * width + x0])
                    + quarter(colors[std::size_t(y0) * width + x1]) + quarter(colors[std::size_t(y1) * width + x0])
                    + quarter(colors[std::size_t(y1) * width + x1]);
            }
        }
        width = next_width; height = next_height;
        colors.resize(std::size_t(width) * height);
    }
    output = std::move(result);
    return true;
}

export bool Copy_Prepared_Image(const PreparedImage& image, std::span<std::byte> destination,
    std::size_t row_pitch) noexcept
{
    // A mapped span ends at the last occupied byte, excluding trailing row padding.
    if (!image.View().Is_Valid()) return false;
    const std::size_t row_bytes = std::size_t(image.width) * Pixel_Size(image.encoding);
    if (row_pitch < row_bytes || row_bytes > destination.size()
        || image.height - 1 > (destination.size() - row_bytes) / row_pitch) return false;
    for (unsigned y = 0; y < image.height; ++y)
        std::memcpy(destination.data() + y * row_pitch, image.bytes.data() + y * image.row_pitch, row_bytes);
    return true;
}
}
