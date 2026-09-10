module;
#include <algorithm>
#include <bit>
#include <cstdint>
export module Graphics.Resources.Textures.Storage;
export import Assets.Images.PixelEncoding;
import Graphics.RHI;

namespace Graphics
{
export struct TextureExtent final
{
    unsigned width = 0, height = 0, depth = 1;
    bool operator==(const TextureExtent&) const = default;
};

// Image preparation retains power-of-two sizing; GPU limits come directly from
// the device. A missing device limit produces an invalid extent without looping.
export TextureExtent Select_Texture_Extent(TextureExtent source, RHITextureLimits limits) noexcept
{
    const auto round_extent = [](unsigned value, unsigned limit) {
        if (limit == 0) return 0u;
        unsigned result = 1;
        while (result < value && result < limit) {
            if (result > limit / 2) return limit;
            result *= 2;
        }
        return result;
    };
    return {round_extent(source.width, limits.max_2d_extent),
        round_extent(source.height, limits.max_2d_extent),
        round_extent(source.depth, limits.max_3d_extent)};
}

export struct TextureMipPreferences final
{
    unsigned requested_count = 0; // Zero selects all available levels.
    unsigned reduction = 0;
    unsigned minimum_dimension = 1;
    bool reducible = true;
};

export struct TextureMipSelection final
{
    TextureExtent extent;
    unsigned first_mip = 0, mip_count = 0;
};

// Preserve the configured compressed-image policy: reserve the last two authored
// mip levels, stop at a 4x4 footprint, and respect both quality and per-resource
// reduction choices. Hardware fitting can require a further source-mip skip.
export bool Select_Compressed_Texture_Mips(TextureExtent source, unsigned source_mips,
    TextureMipPreferences preferences, RHITextureLimits limits, TextureMipSelection& output) noexcept
{
    if (source.width == 0 || source.height == 0 || source.depth == 0 || source_mips == 0
        || source_mips > std::bit_width(std::max({source.width, source.height, source.depth}))) return false;
    const unsigned available = source_mips > 2 ? source_mips - 2 : 1;
    const unsigned requested_reduction = std::min(preferences.reduction, available - 1);
    unsigned reduction = 0;
    unsigned width = source.width, height = source.height;
    while (reduction < requested_reduction && width > preferences.minimum_dimension
        && height > preferences.minimum_dimension) {
        width >>= 1; height >>= 1; ++reduction;
    }
    if (!preferences.reducible || preferences.requested_count == 1) reduction = 0;
    else if (preferences.requested_count != 0)
        reduction = std::min(reduction, preferences.requested_count - 1);

    for (; reduction < available; ++reduction) {
        const TextureExtent extent{std::max(source.width >> reduction, 4u),
            std::max(source.height >> reduction, 4u), std::max(source.depth >> reduction, 1u)};
        if (Select_Texture_Extent(extent, limits) != extent) continue;
        unsigned count = preferences.requested_count == 0 ? available : std::min(preferences.requested_count, available);
        if (reduction >= count) return false;
        count -= reduction;
        unsigned extent_mips = 1;
        for (std::uint64_t dimension = 4; dimension < extent.width && dimension < extent.height; dimension *= 2)
            ++extent_mips;
        output = {extent, reduction, std::min(count, extent_mips)};
        return true;
    }
    return false;
}

export constexpr RHITextureFormat Texture_Storage_Format(Assets::PixelEncoding encoding) noexcept
{
    using E=Assets::PixelEncoding;
    using F=RHITextureFormat;
    switch (encoding) {
    case E::RGBA8: return F::RGBA8_UNorm;
    case E::BGRA8: return F::BGRA8_UNorm;
    case E::BGRX8: return F::BGRX8_UNorm;
    case E::BGR565: return F::BGR565_UNorm;
    case E::BGRX5551: case E::BGRA5551: return F::BGRA5551_UNorm;
    case E::BGRA4444: return F::BGRA4444_UNorm;
    case E::Alpha8: return F::A8_UNorm;
    case E::Luminance8: return F::R8_UNorm;
    case E::LuminanceAlpha88: return F::RG8_UNorm;
    case E::RG8_SNorm: return F::RG8_SNorm;
    case E::BC1: return F::BC1_UNorm;
    case E::BC2: case E::BC2Premultiplied: return F::BC2_UNorm;
    case E::BC3: case E::BC3Premultiplied: return F::BC3_UNorm;
    default: return F::Unknown;
    }
}

export constexpr Assets::PixelEncoding Select_Texture_Encoding(Assets::PixelEncoding source,
    bool allow_compression,bool prefer_16_bits) noexcept
{
    using E=Assets::PixelEncoding;
    if (!allow_compression && Assets::Is_Block_Compressed(source))
        source=source == E::BC1 ? E::BGRX8 : E::BGRA8;
    if (source == E::BGR8) source=E::BGRX8;
    if (prefer_16_bits) {
        if (source == E::BGRA8 || source == E::RGBA8) return E::BGRA4444;
        if (source == E::BGRX8) return E::BGR565;
    }
    return Texture_Storage_Format(source) == RHITextureFormat::Unknown ? E::BGRA8 : source;
}

// CPU-editable surfaces retain the byte layout expected by existing terrain
// and image-editing consumers. Sampling precision is selected independently.
export constexpr Assets::PixelEncoding Editable_Texture_Encoding(Assets::PixelEncoding encoding) noexcept
{
    using E=Assets::PixelEncoding;
    return encoding == E::BGRA5551 || encoding == E::BGRA4444 || encoding == E::Luminance8
        ? E::BGRA8 : encoding;
}
}
