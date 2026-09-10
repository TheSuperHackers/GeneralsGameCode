module;
#include <array>
#include <span>
#include <vector>
#include <cstdint>
export module Graphics.Scene.Lines.Drawing;
export import Graphics.Scene.Surfaces.Renderer;
import Graphics.Scene.Props.Geometry;

namespace Graphics {
export bool Update_Navigation_Line(SurfaceRenderer& renderer, SurfaceMeshHandle& mesh,
    std::span<const PropVertex> source, std::span<const std::uint32_t> indices)
{
    std::vector<SurfaceVertex> vertices(source.size());
    for(std::size_t i=0;i<source.size();++i)
        vertices[i]={source[i].position,source[i].color,source[i].uv};
    if(mesh.Is_Valid()) return renderer.Update_Mesh(mesh,vertices,indices);
    mesh=renderer.Create_Mesh(vertices,indices);
    return mesh.Is_Valid();
}
// Navigation overlays use camera-space joined geometry and additive color.
// They remain visible across terrain while leaving the scene depth untouched.
export bool Draw_Navigation_Line(SurfaceRenderer& renderer, CommandList& commands,
    SurfaceMeshHandle mesh, const std::array<float,16>& projection, RHITextureHandle texture)
{
    SurfaceParameters parameters;
    parameters.view_projection = projection;
    parameters.textured = texture.Is_Valid() ? 1.0f : 0.0f;
    SurfaceStyle style;
    style.blend = RHIBlendMode::Additive;
    style.depth_comparison = RHIComparison::Always;
    const std::array<RHITextureHandle,4> textures{texture,{},{},{}};
    return renderer.Draw(commands,mesh,style,parameters,textures);
}
}
