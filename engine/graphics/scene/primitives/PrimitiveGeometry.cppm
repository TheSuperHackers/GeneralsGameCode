module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Primitives.Geometry;
export import Graphics.Scene.Props.Geometry;

namespace Graphics
{
namespace PrimitiveDetail
{
constexpr float Pi=3.14159265358979323846f;
template<std::size_t Size>
bool Finite(const std::array<float,Size>& values)
{
    return std::all_of(values.begin(),values.end(),[](float value) { return std::isfinite(value); });
}
}

export class AnnulusGeometry final
{
public:
    bool Generate(std::uint32_t segments)
    {
        if (segments<3 || std::uint64_t(segments)*6>std::numeric_limits<std::uint32_t>::max()) return false;
        m_unit.resize((std::size_t(segments)+1)*2);
        m_vertices.assign(m_unit.size(),{});
        m_indices.resize(std::size_t(segments)*6);
        const float step=2*PrimitiveDetail::Pi/static_cast<float>(segments);
        float angle=0;
        for (std::size_t vertex=0;vertex<m_unit.size();vertex+=2,angle+=step) {
            m_unit[vertex]=m_unit[vertex+1]={-std::sin(angle),std::cos(angle)};
            m_vertices[vertex].normal=m_vertices[vertex+1].normal={0,0,1};
        }
        // Keep the source strip's triangle order, including alternate winding.
        for (std::uint32_t triangle=0;triangle<segments*2;++triangle) {
            m_indices[triangle*3]=triangle;
            m_indices[triangle*3+1]=triangle+1;
            m_indices[triangle*3+2]=triangle+2;
        }
        m_segments=segments;
        m_color={1,1,1,1};
        m_uv_valid=false;
        m_inner=m_outer={1,1};
        Update_Scale(m_inner,m_outer);
        Set_Tiling(m_tiles);
        return true;
    }

    bool Scale(std::array<float,2> inner,std::array<float,2> outer)
    {
        if (!PrimitiveDetail::Finite(inner) || !PrimitiveDetail::Finite(outer)) return false;
        if (inner!=m_inner || outer!=m_outer) Update_Scale(inner,outer);
        return true;
    }

    bool Set_Tiling(float tiles)
    {
        if (!std::isfinite(tiles) || m_segments==0) return false;
        if (m_uv_valid && tiles==m_tiles) return true;
        m_tiles=tiles;
        m_uv_valid=true;
        const float step=tiles/static_cast<float>(m_segments);
        float u=0;
        for (std::size_t vertex=0;vertex<m_vertices.size();vertex+=2,u+=step) {
            m_vertices[vertex].uv={u,0};
            m_vertices[vertex+1].uv={u,1};
        }
        return true;
    }

    void Set_Color(const std::array<float,4>& color)
    {
        if (m_color==color) return;
        m_color=color;
        for (auto& vertex : m_vertices) vertex.color=color;
    }
    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }
    std::uint32_t Triangle_Count() const noexcept { return static_cast<std::uint32_t>(m_indices.size()/3); }

private:
    void Update_Scale(std::array<float,2> inner,std::array<float,2> outer)
    {
        m_inner=inner; m_outer=outer;
        for (std::size_t vertex=0;vertex<m_vertices.size();++vertex) {
            const auto& scale=vertex%2==0 ? inner : outer;
            m_vertices[vertex].position={m_unit[vertex][0]*scale[0],m_unit[vertex][1]*scale[1],0};
        }
    }
    std::vector<std::array<float,2>> m_unit;
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
    std::array<float,2> m_inner{1,1},m_outer{1,1};
    std::uint32_t m_segments=0;
    float m_tiles=5;
    std::array<float,4> m_color{1,1,1,1};
    bool m_uv_valid=false;
};

export class SphereGeometry final
{
public:
    bool Generate(float radius,std::uint32_t slices,std::uint32_t stacks)
    {
        if (!std::isfinite(radius) || radius<=0 || slices<3 || stacks==0) return false;
        const std::uint64_t count=(std::uint64_t(slices)+1)*stacks+2;
        const std::uint64_t triangles=std::uint64_t(slices)*stacks*2;
        if (count>std::numeric_limits<std::uint32_t>::max()
            || triangles>std::numeric_limits<std::uint32_t>::max()/3) return false;
        m_vertices.assign(static_cast<std::size_t>(count),{});
        m_opacity_valid=false;
        m_indices.clear(); m_indices.reserve(static_cast<std::size_t>(triangles)*3);
        m_vertices.front().position={0,0,radius}; m_vertices.front().normal={0,0,1};
        m_vertices.front().uv={0.5f,0};
        m_vertices.back().position={0,0,-radius}; m_vertices.back().normal={0,0,-1};
        m_vertices.back().uv={0.5f,1};
        for (std::uint32_t stack=0;stack<stacks;++stack) {
            const float v=static_cast<float>(stack+1)/static_cast<float>(stacks+1);
            const float latitude=PrimitiveDetail::Pi*v;
            for (std::uint32_t slice=0;slice<=slices;++slice) {
                const float u=static_cast<float>(slice)/static_cast<float>(slices);
                const float longitude=2*PrimitiveDetail::Pi*u;
                auto& vertex=m_vertices[1+std::size_t(stack)*(slices+1)+slice];
                vertex.normal={std::sin(latitude)*std::sin(longitude),
                    -std::sin(latitude)*std::cos(longitude),std::cos(latitude)};
                for (unsigned axis=0;axis<3;++axis) vertex.position[axis]=radius*vertex.normal[axis];
                vertex.uv={u,v};
            }
        }
        const auto triangle=[&](std::uint32_t a,std::uint32_t b,std::uint32_t c) {
            m_indices.insert(m_indices.end(),{a,b,c});
        };
        for (std::uint32_t stack=0;stack+1<stacks;++stack)
            for (std::uint32_t slice=0;slice<slices;++slice) {
                const auto upper=1+stack*(slices+1)+slice;
                const auto lower=upper+slices+1;
                triangle(lower,upper,lower+1);
                triangle(upper,upper+1,lower+1);
            }
        for (std::uint32_t slice=0;slice<slices;++slice) triangle(0,slice+2,slice+1);
        const auto south=static_cast<std::uint32_t>(count-1);
        for (std::uint32_t slice=0;slice<slices;++slice) triangle(south,south-2-slice,south-1-slice);
        return true;
    }

    bool Set_Directional_Opacity(std::array<float,3> direction,float intensity,bool inverse,bool additive)
    {
        if (!PrimitiveDetail::Finite(direction) || !std::isfinite(intensity) || intensity<0) return false;
        if (m_opacity_valid && direction==m_direction && intensity==m_intensity
            && inverse==m_inverse && additive==m_additive) return true;
        m_direction=direction; m_intensity=intensity;
        m_inverse=inverse; m_additive=additive; m_opacity_valid=true;
        for (auto& vertex : m_vertices) {
            float value=std::min(1.0f,std::abs((direction[0]*vertex.normal[0]
                +direction[1]*vertex.normal[1]+direction[2]*vertex.normal[2])*intensity));
            if (!inverse) value=1-value;
            vertex.color=additive ? std::array{value,value,value,0.0f} : std::array{1.0f,1.0f,1.0f,value};
        }
        return true;
    }
    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }
    std::uint32_t Triangle_Count() const noexcept { return static_cast<std::uint32_t>(m_indices.size()/3); }
private:
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
    std::array<float,3> m_direction{};
    float m_intensity=0;
    bool m_inverse=false,m_additive=false,m_opacity_valid=false;
};
}
