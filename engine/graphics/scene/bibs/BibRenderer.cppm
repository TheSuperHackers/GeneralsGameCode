module;
#include <array>
#include <cstdint>
#include <span>
#include <vector>
export module Graphics.Scene.Bibs.Renderer;
export import Graphics.Scene.Surfaces.Renderer;
namespace Graphics {
export struct BibQuad final {
    std::array<std::array<float,3>,4> corners{};
};
export bool Draw_Bibs(SurfaceRenderer& renderer, CommandList& commands, SurfaceMeshHandle& mesh,
    std::span<const BibQuad> quads, const std::array<float,4>& color,
    const SurfaceParameters& parameters, RHITextureHandle texture)
{
    if (quads.empty()) return true;
    std::vector<SurfaceVertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(quads.size()*4); indices.reserve(quads.size()*6);
    constexpr std::array<std::array<float,2>,4> uv{{{0,1},{1,1},{1,0},{0,0}}};
    for (const auto& quad : quads) {
        const auto first=static_cast<std::uint32_t>(vertices.size());
        for (unsigned i=0;i<4;++i) vertices.push_back({quad.corners[i],color,uv[i]});
        for (unsigned index : {0,1,2,0,2,3}) indices.push_back(first+index);
    }
    if (!mesh.Is_Valid()) mesh=renderer.Create_Mesh(vertices,indices);
    else if (!renderer.Update_Mesh(mesh,vertices,indices)) return false;
    SurfaceStyle style;
    style.clamp_texture=true;
    style.depth_comparison=RHIComparison::Always;
    const std::array<RHITextureHandle,4> textures{texture,{},{},{}};
    return renderer.Draw(commands,mesh,style,parameters,textures);
}
}
