module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <span>
#include <vector>
export module Graphics.Scene.Particles.LineGroupGeometry;
import Assets.Math;
export import Graphics.Scene.Props.Geometry;

namespace Graphics {
export enum class LineGroupShape { Tetrahedron, Prism };
export struct LineGroupPoint final {
    std::array<float,3> head{};
    std::array<float,3> tail{};
    std::array<float,4> head_color{1,1,1,1};
    std::array<float,4> tail_color{};
    float size=0;
    float u=0;
};

// Owns generated geometry. The caller supplies active lines in draw order and
// the rotation from the authored cross-section plane into position space.
export class LineGroupGeometry final {
public:
    template<class ReadPoint>
    bool Build(std::size_t count, LineGroupShape shape, const std::array<float,9>& rotation,
        ReadPoint&& read_point)
    {
        Clear();
        if (shape!=LineGroupShape::Tetrahedron && shape!=LineGroupShape::Prism) return false;
        const std::size_t corners=shape==LineGroupShape::Tetrahedron?4:6;
        const std::size_t indices=shape==LineGroupShape::Tetrahedron?12:24;
        if (count>(std::numeric_limits<std::uint32_t>::max)()/indices || !Finite(rotation)) return false;
        constexpr std::array<unsigned,12> tetra_indices{0,2,1,0,3,2,0,1,3,1,2,3};
        constexpr std::array<unsigned,24> prism_indices{0,1,2,0,3,1,1,3,4,1,4,5,1,5,2,0,2,5,0,5,3,3,5,4};
        const std::array<float,3> angles{std::numbers::pi_v<float>/2,
            7*std::numbers::pi_v<float>/6,11*std::numbers::pi_v<float>/6};
        std::array<std::array<float,3>,3> offsets{};
        for (unsigned i=0;i<3;++i) for (unsigned row=0;row<3;++row)
            offsets[i][row]=rotation[row*3]*std::cos(angles[i])+rotation[row*3+1]*std::sin(angles[i]);
        m_vertices.resize(count*corners);
        m_indices.resize(count*indices);
        for (std::size_t i=0;i<count;++i) {
            const LineGroupPoint point=read_point(i);
            if (!Finite(point.head)||!Finite(point.tail)||!Finite(point.head_color)||!Finite(point.tail_color)
                ||!std::isfinite(point.size)||!std::isfinite(point.u)) { Clear();return false; }
            const auto head_color=Quantize(point.head_color), tail_color=Quantize(point.tail_color);
            for (std::size_t corner=0;corner<corners;++corner) {
                const bool tail=shape==LineGroupShape::Tetrahedron?corner==0:corner>=3;
                auto& vertex=m_vertices[i*corners+corner];
                vertex.position=tail?point.tail:point.head;
                if (shape==LineGroupShape::Prism || corner!=0) {
                    const auto& offset=offsets[shape==LineGroupShape::Prism?corner%3:corner-1];
                    for(unsigned axis=0;axis<3;++axis) vertex.position[axis]+=point.size*offset[axis];
                }
                vertex.color=tail?tail_color:head_color;
                vertex.uv={point.u,tail?1.0f:0.0f};
                if (!Finite(vertex.position)) { Clear();return false; }
            }
            for(std::size_t index=0;index<indices;++index)
                m_indices[i*indices+index]=static_cast<std::uint32_t>(i*corners+
                    (shape==LineGroupShape::Tetrahedron?tetra_indices[index]:prism_indices[index]));
        }
        return true;
    }
    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }
private:
    template<std::size_t N> static bool Finite(const std::array<float,N>& values) noexcept {
        return std::all_of(values.begin(),values.end(),[](float value){return std::isfinite(value);});
    }
    static std::array<float,4> Quantize(const std::array<float,4>& color) noexcept {
        const auto packed=Assets::Color_To_ARGB({color[0],color[1],color[2],color[3]});
        return {((packed>>16)&255)/255.0f,((packed>>8)&255)/255.0f,(packed&255)/255.0f,((packed>>24)&255)/255.0f};
    }
    void Clear() noexcept { m_vertices.clear();m_indices.clear(); }
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
}
