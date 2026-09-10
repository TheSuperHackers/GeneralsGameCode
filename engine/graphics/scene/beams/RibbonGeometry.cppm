module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Beams.RibbonGeometry;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonIntersections;
import Graphics.Scene.Beams.RibbonMath;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
export import Graphics.Scene.Props.Geometry;
import Assets.Math;

namespace Graphics {
export class RibbonGeometry {
    bool Fail() { m_vertices.clear(); m_indices.clear(); return false; }
    static bool Covers(std::span<const RibbonIntersection> joints, std::size_t count)
    {
        std::size_t covered=0;
        for(const auto& joint:joints) {
            if(!joint.point_count || joint.point_count>count-covered) return false;
            covered+=joint.point_count;
        }
        return joints.size()>=2 && covered==count;
    }
public:
    bool Build(std::span<const RibbonPoint> points, std::span<const RibbonIntersection> top,
        std::span<const RibbonIntersection> bottom, RibbonTextureMapping mapping,
        std::array<float,2> offset)
    {
        m_vertices.clear(); m_indices.clear();
        if(points.size()<2) return top.empty() && bottom.empty();
        constexpr auto maximum=(std::numeric_limits<std::uint32_t>::max)();
        if(top.size()>maximum/sizeof(PropVertex) || bottom.size()>maximum/sizeof(PropVertex)-top.size()
            || points.size()-1>maximum/6 || !Covers(top,points.size()) || !Covers(bottom,points.size())) return false;
        m_vertices.reserve(top.size()+bottom.size());
        m_indices.reserve((points.size()-1)*6);
        const auto u=Ribbon_Texture_U(mapping);
        const auto emit=[&](const RibbonIntersection& joint,std::size_t point,unsigned side) {
            PropVertex vertex;
            vertex.position=RibbonMath::Mul(joint.direction,RibbonMath::Dot(points[point].position,joint.direction));
            const auto& source=joint.point.color;
            const auto color=Assets::Color_From_ARGB(Assets::Color_To_ARGB({source[0],source[1],source[2],source[3]}));
            vertex.color={color.r,color.g,color.b,color.a};
            vertex.uv={u[side]+offset[0],joint.point.v+offset[1]};
            m_vertices.push_back(vertex);
        };
        emit(top.front(),0,0); emit(bottom.front(),0,1);
        std::size_t top_index=0,bottom_index=0,point=0;
        std::size_t top_remaining=top[0].point_count,bottom_remaining=bottom[0].point_count;
        std::uint32_t last_top=0,last_bottom=1;
        const auto skip=[&] {
            const auto delta=std::min(top_remaining,bottom_remaining)-1;
            top_remaining-=delta; bottom_remaining-=delta; point+=delta;
        };
        skip();
        for(;;) {
            const auto next=static_cast<std::uint32_t>(m_vertices.size());
            if(++point>=points.size()) return Fail();
            if(top_remaining==1 && bottom_remaining==1) {
                if(++top_index>=top.size() || ++bottom_index>=bottom.size()) return Fail();
                m_indices.insert(m_indices.end(),{last_top,last_bottom,next,last_bottom,next+1,next});
                last_top=next; last_bottom=next+1;
                top_remaining=top[top_index].point_count; bottom_remaining=bottom[bottom_index].point_count;
                emit(top[top_index],point,0); emit(bottom[bottom_index],point,1);
            } else if(top_remaining>1) {
                if(++bottom_index>=bottom.size()) return Fail();
                m_indices.insert(m_indices.end(),{last_top,last_bottom,next});
                last_bottom=next;
                --top_remaining; bottom_remaining=bottom[bottom_index].point_count;
                emit(bottom[bottom_index],point,1);
            } else {
                if(++top_index>=top.size()) return Fail();
                m_indices.insert(m_indices.end(),{last_top,last_bottom,next});
                last_top=next;
                --bottom_remaining; top_remaining=top[top_index].point_count;
                emit(top[top_index],point,0);
            }
            skip();
            if((top_index+1==top.size() && top_remaining==1)
                || (bottom_index+1==bottom.size() && bottom_remaining==1)) {
                if(top_index+1!=top.size() || bottom_index+1!=bottom.size() || point+1!=points.size()) return Fail();
                break;
            }
        }
        return Finite_Prop_Vertices(m_vertices) ? true : Fail();
    }
    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }
private:
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};
}
