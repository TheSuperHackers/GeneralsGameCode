module;

#include <cstdint>
#include <span>

export module Graphics.Resources.Textures.CachePolicy;

import Graphics.Resources.Textures.Residency;

namespace Graphics
{

// Texture eviction remains a policy over owners supplied by the asset layer;
// the graphics resource layer only applies residency transitions.
export class TextureCachePolicy final
{
public:
    static constexpr std::uint32_t Default_Inactivation_Time = 20000;

    static void Invalidate_Old_Unused_Texture(
        TextureResidency& texture,
        std::uint32_t now,
        std::uint32_t inactive_time_override = 0) noexcept
    {
        texture.Evict_If_Old(now, inactive_time_override);
    }

    static void Invalidate_Old_Unused_Textures(
        std::span<TextureResidency* const> textures,
        std::uint32_t now,
        std::uint32_t inactive_time_override = 0) noexcept
    {
        for (TextureResidency* texture : textures) {
            if (texture)
                texture->Evict_If_Old(now, inactive_time_override);
        }
    }
};

}
