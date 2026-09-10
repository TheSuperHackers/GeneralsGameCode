module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

export module Graphics.Resources.Textures.Sampling;
import Graphics.RHI;

namespace Graphics
{
export enum class SamplingFilter : std::uint8_t { Disabled, Fast, Best, Default };
export enum class TextureSamplingMode : unsigned { None, Point, Bilinear, Trilinear, Anisotropic };
export constexpr std::array<const char*, 5> TextureSamplingModeNames{
    "None", "Point", "Bilinear", "Trilinear", "Anisotropic"};

export struct TextureSampling final
{
    SamplingFilter minification = SamplingFilter::Default;
    SamplingFilter magnification = SamplingFilter::Default;
    SamplingFilter mipmap = SamplingFilter::Default;
    std::array<RHISamplerAddress, 2> address{RHISamplerAddress::Wrap, RHISamplerAddress::Wrap};
};

export constexpr TextureSampling Make_Texture_Sampling(bool mipmaps) noexcept
{
    TextureSampling sampling;
    if (!mipmaps) sampling.mipmap = SamplingFilter::Disabled;
    return sampling;
}

export struct TextureSamplingSettings final
{
    TextureSamplingMode mode = TextureSamplingMode::Bilinear;
    unsigned anisotropy = 2;
};

namespace { TextureSamplingSettings sampling_settings; }

export TextureSamplingSettings Get_Texture_Sampling_Settings() noexcept { return sampling_settings; }
export void Set_Texture_Sampling_Mode(int mode) noexcept
{
    sampling_settings.mode = static_cast<TextureSamplingMode>(std::clamp(mode, 0, 4));
}
export void Set_Texture_Anisotropy(int level) noexcept
{
    const unsigned limit = static_cast<unsigned>(std::clamp(level, 2, 16));
    unsigned power = 2;
    while (power * 2 <= limit) power *= 2;
    sampling_settings.anisotropy = power;
}

export TextureSamplingMode Parse_Texture_Sampling_Mode(std::string_view text) noexcept
{
    for (unsigned mode = 0; mode < TextureSamplingModeNames.size(); ++mode) {
        const std::string_view name = TextureSamplingModeNames[mode];
        if (name.size() != text.size()) continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size(); ++i) {
            const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
            if (lower(name[i]) != lower(text[i])) { equal = false; break; }
        }
        if (equal) return static_cast<TextureSamplingMode>(mode);
    }
    return TextureSamplingMode::None;
}

// Resolve authored choices against rendering quality settings before submission.
export RHISamplerDescription Resolve_Texture_Sampling(const TextureSampling& sampling,
    TextureSamplingSettings settings, bool allow_anisotropy = true) noexcept
{
    const auto spatial_filter = [&](SamplingFilter filter) {
        return filter == SamplingFilter::Disabled || settings.mode <= TextureSamplingMode::Point
            ? RHISamplerFilter::Point : RHISamplerFilter::Linear;
    };
    RHISamplerDescription result;
    result.minification = spatial_filter(sampling.minification);
    result.magnification = spatial_filter(sampling.magnification);
    result.mipmap = sampling.mipmap == SamplingFilter::Disabled || sampling.mipmap == SamplingFilter::Fast
        || settings.mode <= TextureSamplingMode::Bilinear ? RHISamplerFilter::Point : RHISamplerFilter::Linear;
    if (sampling.mipmap == SamplingFilter::Disabled || settings.mode == TextureSamplingMode::None) result.max_lod = 0;
    if (allow_anisotropy && settings.mode == TextureSamplingMode::Anisotropic
        && sampling.minification >= SamplingFilter::Best && sampling.magnification >= SamplingFilter::Best)
        result.anisotropy = static_cast<std::uint8_t>(std::clamp(settings.anisotropy, 2u, 16u));
    std::copy(sampling.address.begin(), sampling.address.end(), result.address.begin());
    return result;
}
}
