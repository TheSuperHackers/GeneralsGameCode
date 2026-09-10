module;
#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include <vector>
export module Graphics.Scene.Beams.RibbonIntersections;
import Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonMath;

namespace Graphics {
using namespace RibbonMath;

export struct RibbonIntersection {
    std::size_t point_count = 1;
    std::array<float,3> direction{};
    RibbonPoint point;
    bool fold = false;
    bool parallel = false;
private:
    std::size_t next_segment = 0;
    friend class RibbonIntersections;
};

export class RibbonIntersections {
    using Vec = std::array<float,3>;
    struct Segment { Vec start{}; std::array<Vec,2> planes{}; };
    static Vec Project(Vec point, Vec plane) { return Sub(point,Mul(plane,Dot(plane,point))); }
    static Vec Oriented(Vec direction, Vec point)
    {
        return Dot(direction,point)<0 ? Mul(direction,-1) : direction;
    }
    static Vec AveragePlane(Vec a, Vec b, float dot)
    {
        return Normalize(dot>0 ? Add(a,b) : Sub(a,b));
    }
    static Vec Intersect(Vec a, Vec b, Vec point, bool& parallel)
    {
        const float dot=Dot(a,b);
        parallel=std::abs(dot)>=0.9f;
        return parallel ? Normalize(Project(point,AveragePlane(a,b,dot)))
            : Oriented(Normalize(Cross(a,b)),point);
    }
    bool Merge(unsigned side, float limit)
    {
        auto& joints=m_joints[side];
        bool merged=false;
        std::size_t read=1, write=1;
        const auto count=joints.size()-1;
        while(read<count) {
            auto current=joints[read];
            const auto& previous_segment=m_segments[joints[write-1].next_segment];
            // Keep the original adjacent segment for repeated merges in this pass.
            const auto& current_segment=m_segments[current.next_segment];
            while(read<count) {
                const auto& next=joints[read+1];
                const auto& next_segment=m_segments[next.next_segment];
                const bool overlaps_next=!next.fold && Dot(current.direction,next_segment.start)>0
                    && Dot(current.direction,next_segment.planes[side])>0;
                const bool overlaps_previous=!current.fold && Dot(next.direction,Mul(current_segment.start,-1))>0
                    && Dot(next.direction,previous_segment.planes[side])>0;
                if(!overlaps_next && !overlaps_previous) break;
                const auto combined_count=current.point_count+next.point_count;
                // Retain the existing authored weighting, including successive merges.
                const float factor=(1.0f/static_cast<float>(combined_count))*static_cast<float>(current.point_count);
                RibbonPoint point;
                point.position=Add(Mul(current.point.position,factor),Mul(next.point.position,factor));
                point.color=Add(Mul(current.point.color,factor),Mul(next.point.color,factor));
                point.v=current.point.v*factor+next.point.v*factor;
                const auto a=previous_segment.planes[side], b=next_segment.planes[side];
                const float dot=Dot(a,b);
                const bool parallel=std::abs(dot)>=0.9f;
                Vec direction;
                if(!parallel) direction=Oriented(Normalize(Cross(a,b)),point.position);
                else {
                    const auto plane=AveragePlane(a,b,dot);
                    // Project the existing direction, not uninitialized scratch state.
                    direction=current.parallel ? Normalize(Project(current.direction,plane))
                        : Normalize(Cross(current_segment.planes[side],plane));
                }
                if(limit>=0) {
                    const auto current_distance=Project(current.point.position,direction);
                    const auto next_distance=Project(next.point.position,direction);
                    if(Dot(current_distance,current_distance)>limit || Dot(next_distance,next_distance)>limit) break;
                }
                current.direction=direction;
                current.parallel=parallel;
                current.point=point;
                current.point_count=combined_count;
                current.next_segment=next.next_segment;
                current.fold=current.fold || next.fold;
                ++read;
                merged=true;
            }
            // Assignment retains every field, including the parallel flag.
            joints[write++]=current;
            ++read;
        }
        if(read==count) joints[write++]=joints[read];
        joints.resize(write);
        return merged;
    }
public:
    bool Build(const RibbonEdges& edges, bool merge, float width, float abort_factor)
    {
        for(auto& joints:m_joints) joints.clear();
        m_segments.clear();
        const auto points=edges.Points();
        const auto planes=edges.Edges();
        if(!std::isfinite(width) || !std::isfinite(abort_factor)) return false;
        if(points.size()<2) return true;
        const auto count=points.size();
        if(planes.size()!=count-1) return false;
        m_segments.resize(count+1);
        for(auto& joints:m_joints) {
            joints.resize(count+1);
            joints[0].point_count=0;
            for(std::size_t i=1;i<=count;++i) {
                auto& joint=joints[i];
                joint.point=points[i-1];
                joint.point_count=1;
                joint.next_segment=i;
                joint.fold=i==1 || i==count || planes[i-1].fold;
                joint.parallel=false;
            }
        }
        for(std::size_t i=1;i<count;++i)
            m_segments[i]={planes[i-1].direction,{planes[i-1].top,planes[i-1].bottom}};
        for(unsigned side=0;side<2;++side) {
            m_joints[side][1].direction=Normalize(Project(points.front().position,m_segments[1].planes[side]));
            m_joints[side][count].direction=Normalize(Project(points.back().position,m_segments[count-1].planes[side]));
            for(std::size_t i=2;i<count;++i) {
                auto& joint=m_joints[side][i];
                joint.direction=Intersect(m_segments[i-1].planes[side],m_segments[i].planes[side],
                    joint.point.position,joint.parallel);
            }
        }
        for(std::size_t i=1;i<=count;++i) {
            Vec start=Normalize(Cross(m_joints[0][i].direction,m_joints[1][i].direction));
            const auto direction=i==count ? planes.back().direction : planes[i-1].direction;
            if(Dot(direction,start)<=0) start=Mul(start,-1);
            if(i==1) {
                m_segments[0]={start,{start,start}};
                m_segments[1].start=start;
            } else if(i==count) m_segments[count]={start,{start,start}};
            else m_segments[i].start=start;
        }
        if(merge) {
            const float distance=width*0.5f*abort_factor;
            const float limit=abort_factor>0 ? distance*distance : -1;
            bool changed;
            do {
                changed=Merge(0,limit);
                changed=Merge(1,limit) || changed;
            } while(changed);
        }
        for(const auto& joints:m_joints) for(const auto& joint:joints)
            if(!Finite(joint.direction) || !Finite(joint.point.position) || !Finite(joint.point.color) || !std::isfinite(joint.point.v)) {
                for(auto& output:m_joints) output.clear();
                return false;
            }
        return true;
    }
    std::span<const RibbonIntersection> Top() const noexcept
    {
        return m_joints[0].empty() ? std::span<const RibbonIntersection>{} : std::span(m_joints[0]).subspan(1);
    }
    std::span<const RibbonIntersection> Bottom() const noexcept
    {
        return m_joints[1].empty() ? std::span<const RibbonIntersection>{} : std::span(m_joints[1]).subspan(1);
    }
private:
    std::vector<Segment> m_segments;
    std::array<std::vector<RibbonIntersection>,2> m_joints;
};
}
