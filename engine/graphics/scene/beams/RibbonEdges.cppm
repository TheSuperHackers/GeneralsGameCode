module;
#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include <vector>
export module Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonMath;

namespace Graphics {
using namespace RibbonMath;
export struct RibbonEdgePlanes {
    std::array<float,3> direction{};
    std::array<float,3> top{};
    std::array<float,3> bottom{};
    bool fold = false;
};

// Eye-space silhouette planes pass through the eye, so only their normals
// are needed. Owned points retain the duplicate-point adjustment for tessellation.
export class RibbonEdges {
public:
    bool Build(std::span<const RibbonPoint> points, float width)
    {
        m_points.clear();
        m_edges.clear();
        if (!std::isfinite(width)) return false;
        for (const auto& point : points) if (!Finite(point.position)) return false;
        m_points.assign(points.begin(),points.end());
        if (points.size() < 2) return true;
        m_edges.resize(points.size()-1);
        bool switched = false;
        const float radius = width * 0.5f;
        for (std::size_t i=0; i<m_edges.size(); ++i) {
            const auto current = m_points[i].position;
            auto& next = m_points[i+1].position;
            const auto delta = Sub(current,next);
            if (std::abs(delta[0]) < 0.0001f && std::abs(delta[1]) < 0.0001f && std::abs(delta[2]) < 0.0001f)
                next[0] += 0.001f;
            auto& edge = m_edges[i];
            edge.direction = Normalize(Sub(next,current));
            const auto nearest = Add(current,Mul(edge.direction,-Dot(edge.direction,current)));
            const auto offset = Normalize(Cross(edge.direction,nearest));
            const auto top = Normalize(Cross(Add(current,Mul(offset,radius)),edge.direction));
            const auto bottom = Normalize(Cross(edge.direction,Add(current,Mul(offset,-radius))));
            edge.fold = false;
            if (i) {
                const auto previous_plane = Normalize(Cross(m_points[i-1].position,current));
                const auto current_plane = Normalize(Cross(current,next));
                edge.fold = Dot(previous_plane,current_plane) < 0;
                if (edge.fold) switched = !switched;
            }
            edge.top = switched ? Mul(bottom,-1) : top;
            edge.bottom = switched ? Mul(top,-1) : bottom;
            if (!Finite(edge.direction) || !Finite(edge.top) || !Finite(edge.bottom)) {
                m_edges.clear();
                m_points.clear();
                return false;
            }
        }
        return true;
    }
    std::span<const RibbonPoint> Points() const noexcept { return m_points; }
    std::span<const RibbonEdgePlanes> Edges() const noexcept { return m_edges; }
private:
    std::vector<RibbonPoint> m_points;
    std::vector<RibbonEdgePlanes> m_edges;
};
}
