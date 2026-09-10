module;
#include <array>
#include <span>
export module Graphics.Scene.Bridges.Renderer;
export import Graphics.Scene.Surfaces.Renderer;

namespace Graphics
{
export struct BridgeDraw final
{
    SurfaceMeshHandle mesh;
    RHITextureHandle texture;
};

export bool Draw_Bridges(SurfaceRenderer &renderer, CommandList &commands,
    std::span<const BridgeDraw> bridges, SurfaceParameters parameters,
    RHITextureHandle cloud, RHITextureHandle shroud)
{
    const bool use_shroud = parameters.textured > 0.5f && parameters.shroud > 0.5f;
    SurfaceStyle base_style;
    base_style.depth_write = true;
    parameters.alpha_cutoff = 96.0f / 255.0f;
    parameters.shroud = 0;
    parameters.shroud_only = 0;
    for (const BridgeDraw &bridge : bridges) {
        const std::array<RHITextureHandle, 4> textures{bridge.texture, cloud, {}, shroud};
        if (!renderer.Draw(commands, bridge.mesh, base_style, parameters, textures)) return false;
    }
    if (!use_shroud) return true;
    SurfaceStyle shroud_style;
    shroud_style.blend = RHIBlendMode::Multiply;
    shroud_style.depth_comparison = RHIComparison::Equal;
    // Match the two-sided base coverage, including reflected views.
    shroud_style.cull = RHICullMode::None;
    parameters.textured = 0;
    parameters.cloud = 0;
    parameters.lightmap = 0;
    parameters.shroud_only = 1;
    const std::array<RHITextureHandle, 4> textures{RHITextureHandle{}, {}, {}, shroud};
    for (const BridgeDraw &bridge : bridges)
        if (!renderer.Draw(commands, bridge.mesh, shroud_style, parameters, textures)) return false;
    return true;
}
}
