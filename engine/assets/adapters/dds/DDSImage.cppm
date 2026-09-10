module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <span>
#include <vector>

export module Assets.Adapters.DDS;
import Assets.Math;
export import Assets.Images.PixelEncoding;

namespace Assets
{
export enum class DDSCompression { BC1, BC2, BC3 };
export enum class DDSDimension { Texture, Cube, Volume };

export struct DDSSurface final
{
    std::uint32_t width = 0, height = 0, depth = 1;
    std::size_t offset = 0, row_pitch = 0, slice_pitch = 0, size = 0;
};

export struct DDSLayout final
{
    DDSCompression compression = DDSCompression::BC1;
    DDSDimension dimension = DDSDimension::Texture;
    bool premultiplied_alpha = false;
    std::uint32_t mip_count = 0;
    std::uint32_t face_mask = 0;
    // Face-major, then mip-major. Absent cube faces have empty surfaces.
    std::vector<DDSSurface> surfaces;

    const DDSSurface* Surface(std::uint32_t level, std::uint32_t face = 0) const noexcept
    {
        if (level >= mip_count || face >= (dimension == DDSDimension::Cube ? 6u : 1u)) return nullptr;
        const std::size_t index = std::size_t(face) * mip_count + level;
        return index < surfaces.size() && surfaces[index].size != 0 ? &surfaces[index] : nullptr;
    }
};

export constexpr PixelEncoding DDS_Pixel_Encoding(const DDSLayout& layout) noexcept
{
    switch (layout.compression) {
    case DDSCompression::BC1: return PixelEncoding::BC1;
    case DDSCompression::BC2:
        return layout.premultiplied_alpha ? PixelEncoding::BC2Premultiplied : PixelEncoding::BC2;
    case DDSCompression::BC3:
        return layout.premultiplied_alpha ? PixelEncoding::BC3Premultiplied : PixelEncoding::BC3;
    }
    return PixelEncoding::Unknown;
}

namespace DDSDetail
{
bool Destination_Fits(std::size_t capacity, std::size_t row_bytes, std::size_t rows,
    unsigned depth, std::size_t row_pitch, std::size_t slice_pitch) noexcept
{
    if (row_bytes == 0 || rows == 0 || depth == 0 || row_pitch < row_bytes || row_bytes > capacity
        || rows - 1 > (capacity - row_bytes) / row_pitch) return false;
    const std::size_t slice_bytes = (rows - 1) * row_pitch + row_bytes;
    return slice_pitch >= slice_bytes && depth - 1 <= (capacity - slice_bytes) / slice_pitch;
}

std::uint16_t Shift_Endpoint(std::uint16_t color, const std::array<float, 3>& shift)
{
    // Preserve the source recoloring convention: expand by shifting, then round
    // transformed channels to bytes before quantizing the endpoints again.
    const unsigned r5 = color >> 11, g6 = (color >> 5) & 63, b5 = color & 31;
    const auto shifted = Shift_Color_HSV(Vector3f{float(r5 << 3) / 255,
        float(g6 << 2) / 255, float(b5 << 3) / 255}, {shift[0], shift[1], shift[2]});
    const auto r = unsigned(shifted.x * 255 + 0.5f), g = unsigned(shifted.y * 255 + 0.5f), b = unsigned(shifted.z * 255 + 0.5f);
    return std::uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

std::uint32_t U32(std::span<const std::byte> bytes, std::size_t offset)
{
    return std::to_integer<std::uint32_t>(bytes[offset])
        | (std::to_integer<std::uint32_t>(bytes[offset + 1]) << 8)
        | (std::to_integer<std::uint32_t>(bytes[offset + 2]) << 16)
        | (std::to_integer<std::uint32_t>(bytes[offset + 3]) << 24);
}
}

// Metadata-only readers supply the complete file size without reading its payload.
// Every subresource is checked before publication; failure leaves output untouched.
export bool Read_DDS_Layout(std::span<const std::byte> header, std::size_t file_size, DDSLayout& output)
{
    if (header.size() < 128 || file_size < 128) return false;
    using DDSDetail::U32;
    if (U32(header, 0) != 0x20534444 || U32(header, 4) != 124 || U32(header, 76) != 32
        || (U32(header, 80) & 4) == 0) return false;
    DDSLayout result;
    switch (U32(header, 84)) {
    case 0x31545844: result.compression = DDSCompression::BC1; break;
    case 0x32545844: result.premultiplied_alpha = true; [[fallthrough]];
    case 0x33545844: result.compression = DDSCompression::BC2; break;
    case 0x34545844: result.premultiplied_alpha = true; [[fallthrough]];
    case 0x35545844: result.compression = DDSCompression::BC3; break;
    default: return false;
    }
    const auto width = U32(header, 16), height = U32(header, 12), caps = U32(header, 112);
    const bool cube = (caps & 0x200) != 0, volume = (caps & 0x200000) != 0;
    if (width == 0 || height == 0 || (cube && volume)) return false;
    result.dimension = cube ? DDSDimension::Cube : volume ? DDSDimension::Volume : DDSDimension::Texture;
    result.face_mask = cube ? (caps >> 10) & 63 : 1;
    const auto depth = volume ? U32(header, 24) : 1u;
    if (depth == 0 || result.face_mask == 0 || (cube && width != height)) return false;
    result.mip_count = std::max(1u, U32(header, 28));
    unsigned max_levels = 1;
    for (auto dimension = std::max({width, height, depth}); dimension > 1; dimension >>= 1) ++max_levels;
    if (result.mip_count > max_levels) return false;
    result.surfaces.resize(std::size_t(cube ? 6 : 1) * result.mip_count);
    const std::size_t block_size = result.compression == DDSCompression::BC1 ? 8 : 16;
    std::size_t offset = 128;
    for (unsigned face = 0; face < (cube ? 6u : 1u); ++face) {
        if ((result.face_mask & (1u << face)) == 0) continue;
        for (unsigned level = 0; level < result.mip_count; ++level) {
            DDSSurface surface;
            surface.width = std::max(1u, width >> level);
            surface.height = std::max(1u, height >> level);
            surface.depth = std::max(1u, depth >> level);
            const std::uint64_t rows = (std::uint64_t(surface.height) + 3) / 4;
            const std::uint64_t pitch = ((std::uint64_t(surface.width) + 3) / 4) * block_size;
            // Divide the available byte count before multiplying untrusted dimensions.
            const std::size_t available = file_size - offset;
            if (pitch > available / rows / surface.depth) return false;
            surface.row_pitch = std::size_t(pitch);
            surface.slice_pitch = std::size_t(pitch * rows);
            surface.size = surface.slice_pitch * surface.depth;
            surface.offset = offset;
            offset += surface.size;
            result.surfaces[std::size_t(face) * result.mip_count + level] = surface;
        }
    }
    output = std::move(result);
    return true;
}

export std::span<const std::byte> DDS_Surface_Bytes(std::span<const std::byte> bytes, const DDSSurface& surface)
{
    if (surface.offset > bytes.size() || surface.size > bytes.size() - surface.offset) return {};
    return bytes.subspan(surface.offset, surface.size);
}

// Preserve authored block bytes, with independent destination row and slice strides.
export bool Copy_DDS_Blocks(std::span<const std::byte> source, const DDSSurface& surface,
    std::span<std::byte> destination, std::size_t row_pitch, std::size_t slice_pitch,
    DDSCompression compression, const std::array<float, 3>& hsv_shift = {})
{
    const auto bytes = DDS_Surface_Bytes(source, surface);
    if (bytes.empty() || surface.row_pitch == 0 || surface.slice_pitch % surface.row_pitch != 0) return false;
    const auto rows = surface.slice_pitch / surface.row_pitch;
    if (!DDSDetail::Destination_Fits(destination.size(), surface.row_pitch, rows,
        surface.depth, row_pitch, slice_pitch)) return false;
    for (const auto value : hsv_shift) if (!std::isfinite(value)) return false;
    const std::size_t block_size = compression == DDSCompression::BC1 ? 8 : 16;
    if (surface.row_pitch % block_size != 0) return false;
    const bool recolor = hsv_shift != std::array<float, 3>{};
    for (unsigned z = 0; z < surface.depth; ++z)
        for (std::size_t row = 0; row < rows; ++row) {
            auto* destination_row = destination.data() + z * slice_pitch + row * row_pitch;
            std::memcpy(destination_row,
                bytes.data() + z * surface.slice_pitch + row * surface.row_pitch, surface.row_pitch);
            if (!recolor) continue;
            for (std::size_t block = 0; block < surface.row_pitch; block += block_size) {
                auto* endpoints = destination_row + block + (block_size == 8 ? 0 : 8);
                for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
                    auto* encoded = endpoints + endpoint * 2;
                    const auto color = std::uint16_t(std::to_integer<unsigned>(encoded[0])
                        | (std::to_integer<unsigned>(encoded[1]) << 8));
                    const auto shifted = DDSDetail::Shift_Endpoint(color, hsv_shift);
                    encoded[0] = std::byte(shifted); encoded[1] = std::byte(shifted >> 8);
                }
            }
        }
    return true;
}

namespace DDSDetail
{
std::array<std::uint8_t, 4> Decode_RGB565(std::uint16_t value) noexcept
{
	const std::uint8_t red = static_cast<std::uint8_t>(((value >> 11) & 0x1f) * 255 / 31);
	const std::uint8_t green = static_cast<std::uint8_t>(((value >> 5) & 0x3f) * 255 / 63);
	const std::uint8_t blue = static_cast<std::uint8_t>((value & 0x1f) * 255 / 31);
	return {red, green, blue, 255};
}

}

// Decode one selected slice to RGBA8. DXT2/4 retain their authored
// premultiplication; callers select matching blend behavior from the layout.
export bool Decode_DDS_Surface(std::span<const std::byte> source, const DDSLayout& layout,
    unsigned level, unsigned face, unsigned slice, std::vector<std::byte>& output)
{
    const auto* surface = layout.Surface(level, face);
    if (!surface || slice >= surface->depth) return false;
    auto encoded = DDS_Surface_Bytes(source, *surface);
    if (encoded.empty()) return false;
    encoded = encoded.subspan(std::size_t(slice) * surface->slice_pitch, surface->slice_pitch);
    const auto width = surface->width, height = surface->height;
    if (width > (std::numeric_limits<std::size_t>::max() / 4) / height) return false;
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(encoded.data());
    const std::size_t block_width = (std::size_t(width) + 3) / 4;
    const std::size_t block_height = (std::size_t(height) + 3) / 4;
    const std::size_t block_size = layout.compression == DDSCompression::BC1 ? 8 : 16;
    if (block_width > encoded.size() / block_size / block_height) return false;
    constexpr auto dxt1 = DDSCompression::BC1, dxt3 = DDSCompression::BC2, dxt5 = DDSCompression::BC3;
    const auto four_cc = layout.compression;
    auto read_u32 = [encoded](std::size_t offset) { return DDSDetail::U32(encoded, offset); };
    using DDSDetail::Decode_RGB565;
	std::vector<std::byte> pixels(static_cast<std::size_t>(width) * height * 4);
	for (std::size_t block_y = 0; block_y < block_height; ++block_y) {
		for (std::size_t block_x = 0; block_x < block_width; ++block_x) {
			const std::size_t block_offset = (block_y * block_width + block_x) * block_size;
            const std::size_t color_offset=block_offset+(four_cc==dxt1 ? 0 : 8);
			const std::uint16_t color0 = static_cast<std::uint16_t>(bytes[color_offset])
				| static_cast<std::uint16_t>(bytes[color_offset + 1] << 8);
			const std::uint16_t color1 = static_cast<std::uint16_t>(bytes[color_offset + 2])
				| static_cast<std::uint16_t>(bytes[color_offset + 3] << 8);
			const std::array<std::uint8_t, 4> first = Decode_RGB565(color0);
			const std::array<std::uint8_t, 4> second = Decode_RGB565(color1);
			std::array<std::array<std::uint8_t, 4>, 4> colors = {first, second, {}, {}};
			if (four_cc == dxt1 && color0 <= color1) {
				for (std::size_t channel = 0; channel < 3; ++channel) {
					colors[2][channel] = static_cast<std::uint8_t>((first[channel] + second[channel]) / 2);
					colors[3][channel] = 0;
				}
				colors[2][3] = 255;
			}
			else {
				for (std::size_t channel = 0; channel < 3; ++channel) {
					colors[2][channel] = static_cast<std::uint8_t>((2 * first[channel] + second[channel]) / 3);
					colors[3][channel] = static_cast<std::uint8_t>((first[channel] + 2 * second[channel]) / 3);
				}
				colors[2][3] = colors[3][3] = 255;
			}

			const std::uint32_t color_indices = read_u32(color_offset + 4);
			std::array<std::uint8_t, 16> alpha_indices{};
			if (four_cc == dxt3) {
				for (std::size_t pixel = 0; pixel < 16; ++pixel) {
					const std::uint8_t value = bytes[block_offset + (pixel / 2)];
					alpha_indices[pixel] = static_cast<std::uint8_t>((pixel & 1) == 0 ? value & 0x0f : value >> 4);
				}
			}
			else if (four_cc == dxt5) {
				const std::uint8_t alpha0 = bytes[block_offset + 0];
				const std::uint8_t alpha1 = bytes[block_offset + 1];
				std::array<std::uint8_t, 8> alpha_values = {alpha0, alpha1, 0, 0, 0, 0, 0, 0};
				if (alpha0 > alpha1) {
					for (std::size_t index = 2; index < 8; ++index)
						alpha_values[index] = static_cast<std::uint8_t>(((8 - index) * alpha0 + (index - 1) * alpha1) / 7);
				}
				else {
					for (std::size_t index = 2; index < 6; ++index)
						alpha_values[index] = static_cast<std::uint8_t>(((6 - index) * alpha0 + (index - 1) * alpha1) / 5);
					alpha_values[6] = 0;
					alpha_values[7] = 255;
				}
				std::uint64_t alpha_bits = 0;
				for (std::size_t index = 0; index < 6; ++index)
					alpha_bits |= static_cast<std::uint64_t>(bytes[block_offset + 2 + index]) << (index * 8);
				for (std::size_t pixel = 0; pixel < 16; ++pixel)
					alpha_indices[pixel] = alpha_values[(alpha_bits >> (pixel * 3)) & 0x7];
			}

			for (std::size_t local_y = 0; local_y < 4; ++local_y) {
				for (std::size_t local_x = 0; local_x < 4; ++local_x) {
					const std::size_t x = block_x * 4 + local_x;
					const std::size_t y = block_y * 4 + local_y;
					if (x >= width || y >= height)
						continue;
					const std::size_t pixel = local_y * 4 + local_x;
					const std::uint32_t color_index = (color_indices >> (pixel * 2)) & 0x3;
					std::byte *destination = pixels.data() + (y * width + x) * 4;
					for (std::size_t channel = 0; channel < 4; ++channel)
						destination[channel] = static_cast<std::byte>(colors[color_index][channel]);
					if (four_cc == dxt3)
						destination[3] = static_cast<std::byte>(alpha_indices[pixel] * 17);
					else if (four_cc == dxt5)
						destination[3] = static_cast<std::byte>(alpha_indices[pixel]);
				}
			}
		}
	}
    output = std::move(pixels);
    return true;
}

// Copy a selected face/mip, retaining compression or expanding to a scalar pixel
// encoding. Dimensions are authored or padded to one block; padding repeats edge
// texels. Destination storage and its row/slice strides belong to the caller.
export bool Copy_DDS_Image(std::span<const std::byte> source, const DDSLayout& layout,
    unsigned level, unsigned face, PixelEncoding encoding, unsigned width, unsigned height,
    unsigned depth, std::span<std::byte> destination, std::size_t row_pitch,
    std::size_t slice_pitch, const std::array<float, 3>& hsv_shift = {})
{
    const auto* surface = layout.Surface(level, face);
    if (!surface || surface->width == 0 || surface->height == 0 || depth == 0
        || depth != surface->depth || DDS_Pixel_Encoding(layout) == PixelEncoding::Unknown
        || (width != surface->width && width != std::max(4u, surface->width))
        || (height != surface->height && height != std::max(4u, surface->height))) return false;
    for (const float value : hsv_shift) if (!std::isfinite(value)) return false;

    const std::size_t block_size = layout.compression == DDSCompression::BC1 ? 8 : 16;
    const std::uint64_t source_row = ((std::uint64_t(surface->width) + 3) / 4) * block_size;
    const std::uint64_t source_rows = (std::uint64_t(surface->height) + 3) / 4;
    const auto bytes = DDS_Surface_Bytes(source, *surface);
    if (source_row > bytes.size() / source_rows / depth
        || surface->row_pitch != source_row || surface->slice_pitch != source_row * source_rows
        || surface->size != surface->slice_pitch * depth) return false;

    if (encoding == DDS_Pixel_Encoding(layout))
        return Copy_DDS_Blocks(source, *surface, destination, row_pitch, slice_pitch,
            layout.compression, hsv_shift);

    const unsigned pixel_size = Pixel_Size(encoding);
    std::array<std::byte, 4> probe{};
    if (pixel_size == 0 || !Write_Image_Pixel(probe, encoding, 0) || width > row_pitch / pixel_size
        || !DDSDetail::Destination_Fits(destination.size(), std::size_t(width) * pixel_size,
            height, depth, row_pitch, slice_pitch)) return false;

    // Recolor encoded endpoints before expansion so compressed and expanded
    // images use the same quantization and retain authored alpha.
    DDSLayout selected;
    selected.compression = layout.compression;
    selected.premultiplied_alpha = layout.premultiplied_alpha;
    selected.mip_count = 1;
    selected.surfaces.push_back(*surface);
    selected.surfaces[0].offset = 0;
    std::vector<std::byte> blocks(surface->size);
    if (!Copy_DDS_Blocks(source, *surface, blocks, surface->row_pitch,
        surface->slice_pitch, layout.compression, hsv_shift)) return false;
    std::vector<std::byte> pixels;
    for (unsigned slice = 0; slice < depth; ++slice) {
        if (!Decode_DDS_Surface(blocks, selected, 0, 0, slice, pixels)) return false;
        for (unsigned y = 0; y < height; ++y) {
            auto row = destination.subspan(std::size_t(slice) * slice_pitch + std::size_t(y) * row_pitch);
            for (unsigned x = 0; x < width; ++x) {
                const auto* pixel = pixels.data() + (std::size_t(std::min(y, surface->height - 1))
                    * surface->width + std::min(x, surface->width - 1)) * 4;
                const unsigned argb = (std::to_integer<unsigned>(pixel[3]) << 24)
                    | (std::to_integer<unsigned>(pixel[0]) << 16)
                    | (std::to_integer<unsigned>(pixel[1]) << 8) | std::to_integer<unsigned>(pixel[2]);
                Write_Image_Pixel(row.subspan(std::size_t(x) * pixel_size, pixel_size), encoding, argb);
            }
        }
    }
    return true;
}

}
