module;
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>
#include <utility>
export module Graphics.Scene.Props.Skinning;
export import Graphics.Scene.Props.Geometry;
import Graphics.Scene.RenderScene;

namespace Graphics {
export struct PropSkinInfluences final {
    std::array<std::uint16_t,4> indices{};
    std::array<float,4> weights{1,0,0,0};
};
namespace PropSkinDetail {
using V=std::array<float,3>;
float Dot(const V& a,const V& b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
V Cross(const V& a,const V& b) { return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]}; }
V Unit(V value,const V& fallback) {
    const float length=std::sqrt(Dot(value,value));
    if(length<1e-8f) return fallback;
    for(auto& v:value) v/=length;
    return value;
}
V Transform(const RenderTransform& transform,const V& value,bool point) {
    V result{};
    for(unsigned r=0;r<3;++r) {
        for(unsigned c=0;c<3;++c) result[r]+=transform.matrix[r*4+c]*value[c];
        if(point) result[r]+=transform.matrix[r*4+3];
    }
    return result;
}
}

// Produces a new immutable geometry version; queued poses never share mutable vertices.
export bool Pose_Prop_Vertices(std::span<const PropVertex> source,
    std::span<const PropSkinInfluences> influences,std::span<const RenderTransform> bones,
    std::vector<PropVertex>& output) {
    using namespace PropSkinDetail;
    if(source.size()!=influences.size() || source.empty()) return false;
    std::vector<PropVertex> result(source.begin(),source.end());
    for(std::size_t i=0;i<source.size();++i) {
        const auto& original=source[i]; const auto& skin=influences[i];
        V p{},n{},t{},b{}; float total=0;
        const V tangent{original.tangent[0],original.tangent[1],original.tangent[2]};
        V bitangent=Cross(original.normal,tangent);
        for(auto& value:bitangent) value*=original.tangent[3];
        for(unsigned j=0;j<4;++j) {
            const float weight=skin.weights[j];
            if(!std::isfinite(weight) || weight<0 || weight>1) return false;
            if(weight==0) continue;
            if(skin.indices[j]>=bones.size()) return false;
            total+=weight;
            const auto& matrix=bones[skin.indices[j]];
            const auto sp=Transform(matrix,original.position,true);
            const auto sn=Transform(matrix,original.normal,false);
            const auto st=Transform(matrix,tangent,false);
            const auto sb=Transform(matrix,bitangent,false);
            for(unsigned k=0;k<3;++k) {p[k]+=weight*sp[k];n[k]+=weight*sn[k];t[k]+=weight*st[k];b[k]+=weight*sb[k];}
        }
        if(std::abs(total-1)>0.00001f) return false;
        n=Unit(n,Unit(original.normal,{0,0,1}));
        const float dot=Dot(t,n);
        for(unsigned k=0;k<3;++k) t[k]-=dot*n[k];
        const V axis=std::abs(n[0])<.8f ? V{1,0,0} : V{0,1,0};
        t=Unit(t,Unit(Cross(axis,n),{1,0,0}));
        result[i].position=p;result[i].normal=n;
        result[i].tangent={t[0],t[1],t[2],Dot(Cross(n,t),b)<0 ? -1.0f : 1.0f};
    }
    if(!Finite_Prop_Vertices(result)) return false;
    output=std::move(result); return true;
}
}
