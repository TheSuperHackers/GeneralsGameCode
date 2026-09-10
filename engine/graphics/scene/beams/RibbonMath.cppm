module;
#include <array>
#include <cmath>
#include <cstddef>
export module Graphics.Scene.Beams.RibbonMath;

export namespace Graphics::RibbonMath {
template<std::size_t N> std::array<float,N> Add(std::array<float,N> a, std::array<float,N> b)
{
    for(std::size_t i=0;i<N;++i) a[i]+=b[i];
    return a;
}
template<std::size_t N> std::array<float,N> Mul(std::array<float,N> a, float b)
{
    for(auto& value:a) value*=b;
    return a;
}
inline std::array<float,3> Sub(std::array<float,3> a, std::array<float,3> b)
{
    return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};
}
inline float Dot(std::array<float,3> a, std::array<float,3> b)
{
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
inline std::array<float,3> Cross(std::array<float,3> a, std::array<float,3> b)
{
    return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
}
inline std::array<float,3> Normalize(std::array<float,3> value)
{
    const float length2=Dot(value,value);
    return length2==0 ? value : Mul(value,1.0f/static_cast<float>(std::sqrt(length2)));
}
template<std::size_t N> bool Finite(std::array<float,N> value)
{
    for(float component:value) if(!std::isfinite(component)) return false;
    return true;
}
}
