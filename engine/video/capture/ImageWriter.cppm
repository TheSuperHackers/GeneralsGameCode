module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
export module Video.Capture.ImageWriter;
export import Video.Frame;
namespace Engine::Video
{
export enum class FrameImageFormat : std::uint8_t { PNG, JPEG, TGA, BMP };

export struct FrameImageOptions final
{
    FrameImageFormat format = FrameImageFormat::PNG;
    int jpeg_quality = 90;
    float gamma = 1.0f;
};

// Screenshot output is opaque RGB. The supplied frame remains unchanged.
export bool Write_Frame_Image(const std::filesystem::path& path,
    const DecodedVideoFrame& frame, const FrameImageOptions& options = {})
{
    if (!frame.Is_Valid() || !std::isfinite(options.gamma)
        || options.format > FrameImageFormat::BMP
        || (frame.format != PixelFormat::RGB8 && frame.format != PixelFormat::RGBA8
            && frame.format != PixelFormat::BGRA8 && frame.format != PixelFormat::BGRX8))
        return false;
    const std::uint64_t byte_count = std::uint64_t(frame.width) * frame.height * 3;
    if (byte_count > std::numeric_limits<int>::max())
        return false;
    if ((options.format == FrameImageFormat::TGA || options.format == FrameImageFormat::JPEG)
        && (frame.width > 65535 || frame.height > 65535))
        return false;
    std::array<unsigned char, 256> gamma{};
    const float reciprocal = options.gamma > 0 ? 1.0f / options.gamma : 1.0f;
    for (unsigned value = 0; value != gamma.size(); ++value)
        gamma[value] = static_cast<unsigned char>(std::clamp(256.0f * std::pow(value / 256.0f, reciprocal), 0.0f, 255.0f));
    std::vector<unsigned char> rgb(static_cast<std::size_t>(byte_count));
    const unsigned channels = Bytes_Per_Pixel(frame.format);
    const bool bgr = frame.format == PixelFormat::BGRA8 || frame.format == PixelFormat::BGRX8;
    for (std::uint32_t y = 0; y != frame.height; ++y) {
        const auto* source = frame.pixels.data() + std::size_t(y) * frame.row_pitch;
        auto* destination = rgb.data() + std::size_t(y) * frame.width * 3;
        for (std::uint32_t x = 0; x != frame.width; ++x) {
            destination[x * 3] = gamma[std::to_integer<unsigned>(source[x * channels + (bgr ? 2 : 0)])];
            destination[x * 3 + 1] = gamma[std::to_integer<unsigned>(source[x * channels + 1])];
            destination[x * 3 + 2] = gamma[std::to_integer<unsigned>(source[x * channels + (bgr ? 0 : 2)])];
        }
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open())
        return false;
    const auto write = [](void* context, void* bytes, int count) {
        static_cast<std::ofstream*>(context)->write(static_cast<const char*>(bytes), count);
    };
    const int width = static_cast<int>(frame.width), height = static_cast<int>(frame.height);
    int result = 0;
    switch (options.format) {
    case FrameImageFormat::PNG:
        result = stbi_write_png_to_func(write, &output, width, height, 3, rgb.data(), width * 3);
        break;
    case FrameImageFormat::JPEG:
        result = stbi_write_jpg_to_func(write, &output, width, height, 3, rgb.data(), std::clamp(options.jpeg_quality, 1, 100));
        break;
    case FrameImageFormat::TGA:
        result = stbi_write_tga_to_func(write, &output, width, height, 3, rgb.data());
        break;
    case FrameImageFormat::BMP:
        result = stbi_write_bmp_to_func(write, &output, width, height, 3, rgb.data());
        break;
    }
    output.close();
    return result != 0 && !output.fail();
}
}
