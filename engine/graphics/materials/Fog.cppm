module;
#include <array>
export module Graphics.Materials.Fog;

namespace Graphics
{
export enum class MaterialFogMode { Disabled, Scene, Black, White };

export struct SceneFog final
{
    bool enabled = false;
    float start = 0;
    float end = 1;
    std::array<float,4> color{};
};

export struct MaterialFog final
{
    std::array<float,4> state{};
    std::array<float,4> color{};
};

// Each draw resolves its own fog. Additive and multiplicative materials choose
// neutral black/white fog without changing the scene input for later draws.
export constexpr MaterialFog Resolve_Material_Fog(const SceneFog& scene, MaterialFogMode mode) noexcept
{
    if (!scene.enabled || mode == MaterialFogMode::Disabled) return {};
    MaterialFog result{{scene.start,scene.end,1,0},scene.color};
    if (mode == MaterialFogMode::Black) result.color = {0,0,0,1};
    if (mode == MaterialFogMode::White) result.color = {1,1,1,1};
    return result;
}
}
