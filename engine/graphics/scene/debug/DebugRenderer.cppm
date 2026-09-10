module;
#include <span>
#include <cstdint>
export module Graphics.Scene.Debug.Renderer;
export import Graphics.Scene.Surfaces.Renderer;
namespace Graphics
{
export bool Draw_Debug_Geometry(SurfaceRenderer& renderer, CommandList& commands,
    SurfaceMeshHandle& mesh, std::span<const SurfaceVertex> vertices,
    std::span<const std::uint32_t> indices, SurfaceParameters parameters)
{
    if (!mesh.Is_Valid()) mesh=renderer.Create_Mesh(vertices,indices);
    else if (!renderer.Update_Mesh(mesh,vertices,indices)) return false;
    SurfaceStyle style;
    style.depth_comparison=RHIComparison::Always;
    style.blend_alpha_like_color=true;
    style.color_write_mask=15;
    parameters.textured=0;
    return renderer.Draw(commands,mesh,style,parameters,{});
}
}
