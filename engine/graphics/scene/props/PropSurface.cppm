module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
export module Graphics.Scene.Props.Surface;
import Assets.Materials;

namespace Graphics {
// Slots 0/1 remain the legacy stages, 3 the shroud. Environment shadows and
// clouds own 11..15. Surface maps occupy 4..10 without aliasing those bindings.
export inline constexpr std::size_t PropSurfaceTextureFirst = 4;
export inline constexpr std::size_t PropSurfaceTextureCount = 7;
export inline constexpr std::size_t PropTextureCount = 11;
export inline constexpr std::uint32_t PropSurfaceVertexAlphaUVOffset = 128;

export struct PropSurfaceParameters final {
    float shading_model = 0;
    std::uint32_t maps = 0;
    float normal_scale = 1;
    float normal_flip_green = 0;
    float specular_scale = 1;
    float emissive_scale = 1;
    float roughness = .5f;
    float metallic = 0;
    float occlusion_strength = 1;
    std::uint32_t specular_channel = 4;
    std::uint32_t team_color_channel = 0;
    float team_color_multiplier = 1;
    // Alpha enables tint; player colour is instance state, not texture data.
    std::array<float,4> team_color{1,1,1,0};
};
static_assert(sizeof(PropSurfaceParameters) == 64);

export bool Configure_Prop_Surface(const Assets::MaterialSurfaceParameters& source,
    std::uint32_t map_mask, PropSurfaceParameters& result) noexcept
{
    if (!Assets::Validate_Material_Surface(source) || (map_mask & ~127u) != 0) return false;
    PropSurfaceParameters parameters;
    parameters.shading_model = source.shading_model == Assets::MaterialShadingModel::Legacy ? 0
        : source.shading_model == Assets::MaterialShadingModel::SpecularGlossiness ? 1 : 2;
    parameters.maps = map_mask | (source.uv_offset_from_vertex_alpha ? PropSurfaceVertexAlphaUVOffset : 0u);
    parameters.normal_scale = source.normal_scale;
    parameters.normal_flip_green = source.normal_flip_green ? 1.0f : 0.0f;
    parameters.specular_scale = source.specular_scale;
    parameters.emissive_scale = source.emissive_scale;
    parameters.roughness = source.roughness;
    parameters.metallic = source.metallic;
    parameters.occlusion_strength = source.occlusion_strength;
    parameters.specular_channel = static_cast<std::uint32_t>(source.specular_channel);
    parameters.team_color_channel = static_cast<std::uint32_t>(source.team_color_channel);
    parameters.team_color_multiplier = source.team_color_multiplier;
    result = parameters;
    return true;
}
}
