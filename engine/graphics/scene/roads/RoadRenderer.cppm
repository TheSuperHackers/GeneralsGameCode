module;
#include <array>
#include <span>
export module Graphics.Scene.Roads.Renderer;
export import Graphics.Scene.Surfaces.Renderer;

namespace Graphics
{
// Roads have ordered alpha composition. With both projected maps enabled,
// light-map multiplication applies to the composited destination, masked by the
// road texture's alpha. Keep this policy out of game-side adapters.
export bool Draw_Road(SurfaceRenderer &renderer, CommandList &commands,
    SurfaceMeshHandle mesh, SurfaceParameters parameters,
    const std::array<RHITextureHandle, 3> &textures, bool linear_filter)
{
    SurfaceStyle style;
    style.linear_filter = linear_filter;
    const bool masked_lightmap = parameters.textured > 0.5f
        && parameters.cloud > 0.5f && parameters.lightmap > 0.5f;
    if (masked_lightmap) parameters.lightmap = 0;
    parameters.masked_modulation = 0;
    if (!renderer.Draw(commands, mesh, style, parameters, textures)) return false;
    if (!masked_lightmap) return true;
    parameters.masked_modulation = 1;
    style.blend = RHIBlendMode::Multiply;
    return renderer.Draw(commands, mesh, style, parameters, textures);
}
}
