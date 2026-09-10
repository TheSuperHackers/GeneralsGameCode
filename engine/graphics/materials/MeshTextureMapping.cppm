module;
#include <array>
#include <cstdint>
#include <optional>
export module Graphics.Materials.MeshTextureMapping;
import Assets.Math;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.TextureMapping;
import Graphics.Scene.Props.Renderer;

namespace Graphics {
export void Extract_Mesh_Texture_Mappings(PropParameters& parameters, const MeshMaterial* material,
    std::uint32_t milliseconds, const std::array<float, 16>& view, const std::array<float, 16>& projection,
    std::optional<std::array<float, 2>> offset_override = std::nullopt)
{
    if (!material) return;
    for (unsigned stage = 0; stage < parameters.uv_transform.size(); ++stage) {
        auto* mapping = material->mappings[stage].get();
        if (!mapping) continue;
        auto* scroll = stage == 0 && offset_override ? mapping->Linear_Scroll() : nullptr;
        const auto saved_offset = scroll ? scroll->offset : Assets::Vector2f{};
        const auto saved_time = scroll ? scroll->last_time : 0;
        if (scroll) {
            scroll->offset = {(*offset_override)[0], (*offset_override)[1]};
            scroll->last_time = milliseconds;
        }
        const auto result = mapping->Evaluate(milliseconds, view, projection);
        if (scroll) {
            scroll->offset = saved_offset;
            scroll->last_time = saved_time;
        }
        parameters.uv_transform[stage] = result.transform;
        parameters.uv_sources[stage * 2] = static_cast<float>(result.coordinates.source);
        parameters.uv_sources[stage * 2 + 1] = result.coordinates.projected ? 1.0f : 0.0f;
        if (result.bump) parameters.bump_matrix = *result.bump;
    }
}
}
