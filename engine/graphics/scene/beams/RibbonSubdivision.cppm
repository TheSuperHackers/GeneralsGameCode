module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Beams.RibbonSubdivision;

namespace Graphics {
export struct RibbonPoint final {
    std::array<float,3> position{};
    std::array<float,4> color{1,1,1,1};
    float v=0;
};

export class RibbonSubdivision final {
public:
    // Midpoints retain their source segment's starting color. This is the
    // authored ribbon path's existing step-color behavior; UVs interpolate.
    // The caller supplies visual randomness, including seed/reset policy.
    template<class ReadPoint,class RandomVector>
    bool Build(std::size_t count,unsigned levels,float noise,ReadPoint&& read_point,RandomVector&& random)
    {
        m_points.clear();m_stack.clear();
        if (!std::isfinite(noise)||levels>=std::numeric_limits<std::size_t>::digits) return false;
        if (!count) return true;
        const auto subdivisions=std::size_t{1}<<levels;
        if (count-1>(m_points.max_size()-1)/subdivisions) return false;
        m_points.reserve((count-1)*subdivisions+1);
        m_stack.reserve(levels+1);
        RibbonPoint head=read_point(0);
        if (!Valid(head)) return false;
        for(std::size_t segment=1;segment<count;++segment) {
            const RibbonPoint tail=read_point(segment);
            if (!Valid(tail)) { m_points.clear();return false; }
            m_stack.push_back({head,tail,noise,levels});
            while(!m_stack.empty()) {
                const auto task=m_stack.back();m_stack.pop_back();
                if(!task.levels) { m_points.push_back(task.head);continue; }
                const std::array<float,3> offset=random();
                RibbonPoint midpoint=task.head;
                for(unsigned axis=0;axis<3;++axis)
                    midpoint.position[axis]=(task.head.position[axis]+task.tail.position[axis])*.5f+offset[axis]*task.noise;
                midpoint.v=(task.head.v+task.tail.v)*.5f;
                if (!Finite(offset)||!Valid(midpoint)) { m_points.clear();m_stack.clear();return false; }
                m_stack.push_back({midpoint,task.tail,task.noise*.5f,task.levels-1});
                m_stack.push_back({task.head,midpoint,task.noise*.5f,task.levels-1});
            }
            head=tail;
        }
        m_points.push_back(head);
        return true;
    }
    std::span<const RibbonPoint> Points() const noexcept { return m_points; }
private:
    template<std::size_t N> static bool Finite(const std::array<float,N>& data) noexcept {
        return std::all_of(data.begin(),data.end(),[](float value){return std::isfinite(value);});
    }
    static bool Valid(const RibbonPoint& point) noexcept { return Finite(point.position)&&Finite(point.color)&&std::isfinite(point.v); }
    struct Task { RibbonPoint head,tail;float noise;unsigned levels; };
    std::vector<RibbonPoint> m_points;
    std::vector<Task> m_stack;
};
}
