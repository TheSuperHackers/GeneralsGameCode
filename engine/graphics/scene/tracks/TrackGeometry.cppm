module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>
export module Graphics.Scene.Tracks.Geometry;
export import Graphics.Scene.Surfaces.Geometry;
namespace Graphics
{
export struct TrackEdge final
{
    std::array<std::array<float,3>,2> positions{};
    std::array<std::array<float,2>,2> uv{};
    float alpha = 1;
};

export class TrackGeometry final
{
public:
    std::vector<SurfaceVertex> vertices;
    std::vector<std::uint32_t> indices;

    void Build(std::span<const TrackEdge> edges, int maximum_edges, int opaque_edges,
        const std::array<float,3> &color)
    {
        vertices.clear();
        indices.clear();
        if (edges.size() < 2) return;
        vertices.reserve(edges.size()*2);
        indices.reserve((edges.size()-1)*6);
        for (std::size_t i=0; i<edges.size(); ++i) {
            float fade = 1;
            if (int(edges.size()-1-i) >= opaque_edges && maximum_edges > opaque_edges)
                fade = 1 - float(int(edges.size()-i)-opaque_edges) / float(maximum_edges-opaque_edges);
            const float alpha = float(int(std::clamp(fade * edges[i].alpha,0.0f,1.0f)*255)) / 255;
            for (unsigned side=0; side<2; ++side)
                vertices.push_back({edges[i].positions[side], {color[0],color[1],color[2],alpha}, edges[i].uv[side]});
            if (i > 0) {
                const auto a = static_cast<std::uint32_t>((i-1)*2);
                const std::array<std::uint32_t,6> segment{a,a+1,a+3,a,a+3,a+2};
                indices.insert(indices.end(),segment.begin(),segment.end());
            }
        }
    }
};
}
