module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module Graphics.Cursors.Load;
export import Graphics.Cursors.Cursor;
import Assets.Adapters.DDS;
import Assets.Adapters.TGA.Image;

namespace Graphics
{
export struct CursorSource final
{
    std::string image_name;
    int frame_count = 0;
    double frames_per_second = 0;
    int hotspot_x = 0, hotspot_y = 0;
};
export using CursorImageReader = std::function<bool(std::string_view, std::vector<std::byte>&)>;

namespace CursorLoadDetail
{
bool Read_Image(const CursorImageReader& reader, const std::string& name, Assets::PreparedImage& image)
{
    std::vector<std::byte> bytes;
    Assets::DDSLayout layout;
    if (reader(name + ".dds", bytes) && Assets::Read_DDS_Layout(bytes, bytes.size(), layout)
        && layout.dimension == Assets::DDSDimension::Texture
        && Assets::Decode_DDS_Surface(bytes, layout, 0, 0, 0, image.bytes)) {
        image.width = layout.Surface(0)->width;
        image.height = layout.Surface(0)->height;
        image.row_pitch = std::size_t(image.width) * 4;
        image.encoding = Assets::PixelEncoding::RGBA8;
        // Native cursor creation expects straight alpha, including DXT2/4 input.
        if (layout.premultiplied_alpha) {
            for (std::size_t offset = 0; offset < image.bytes.size(); offset += 4) {
                const auto alpha = std::to_integer<unsigned>(image.bytes[offset + 3]);
                for (unsigned channel = 0; channel < 3; ++channel) {
                    const auto color = std::to_integer<unsigned>(image.bytes[offset + channel]);
                    image.bytes[offset + channel] = std::byte(alpha
                        ? std::min(255u, (color * 255 + alpha / 2) / alpha) : 0u);
                }
            }
        }
        return true;
    }
    bytes.clear();
    Assets::TGAImage tga;
    std::vector<Assets::PreparedImage> prepared;
    if (!reader(name + ".tga", bytes) || !Assets::Decode_TGA_Image(bytes, tga)
        || !Assets::Prepare_Image_Levels(tga.View(), Assets::PixelEncoding::RGBA8,
            tga.info.width, tga.info.height, 1, {}, prepared)) return false;
    image = std::move(prepared.front());
    return true;
}
}

// Archive access is supplied by the application. Decode complete authored
// images independently of GPU texture reduction and device recreation.
export bool Load_Cursor(Cursor& cursor, const CursorSource& source, const CursorImageReader& reader)
{
    if (!reader || source.image_name.empty() || source.frame_count <= 0
        || !std::isfinite(source.frames_per_second) || source.frames_per_second < 0) return false;
    const auto duration = source.frames_per_second == 0 ? 0u : static_cast<std::uint32_t>(
        std::clamp(std::round(1000.0 / source.frames_per_second), 1.0,
            double(std::numeric_limits<std::uint32_t>::max())));
    std::vector<Assets::PreparedImage> images(source.frame_count);
    std::vector<CursorFrame> frames;
    frames.reserve(source.frame_count);
    for (int frame = 0; frame < source.frame_count; ++frame) {
        std::string name = source.image_name;
        if (source.frame_count > 1) {
            auto suffix = std::to_string(frame);
            if (suffix.size() < 4) suffix.insert(0, 4 - suffix.size(), '0');
            name += suffix;
        }
        if (!CursorLoadDetail::Read_Image(reader, name, images[frame])) return false;
        frames.push_back({images[frame].View(), duration});
    }
    return cursor.Initialize(frames, source.hotspot_x, source.hotspot_y);
}
}
