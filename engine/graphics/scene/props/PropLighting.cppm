module;
#include <algorithm>
#include <array>
#include <cmath>
export module Graphics.Scene.Props.Lighting;
namespace Graphics
{
export struct PropLight final
{
    std::array<float,3> direction{};
    std::array<float,3> diffuse{};
    std::array<float,3> specular{};
};
export struct PropLighting final
{
    std::array<float,3> ambient{};
    std::array<PropLight,4> lights{};
};
}
