module;
#include <cstdint>
export module Graphics.Scene.DrawParameters;
export import Graphics.Materials.Fog;
export import Graphics.RHI;

namespace Graphics
{
// Inputs to scene traversal, copied into each material submission. They do not
// bind GPU state, and queued draws never consult the current traversal again.
export struct SceneDrawParameters final
{
    SceneFog fog{};
    RHIStencilDescription stencil{};
    std::int32_t depth_bias = 0;
    std::uint8_t color_write_mask = 15;
    bool wireframe = false;
};

export SceneDrawParameters& Get_Scene_Draw_Parameters() noexcept
{
    static SceneDrawParameters parameters;
    return parameters;
}

// Device-thread scene traversal uses lexical scopes. Each scope owns its
// predecessor value, so nested views have no shared stack or capacity limit.
export class SceneDrawScope final
{
public:
    explicit SceneDrawScope(SceneDrawParameters& parameters) noexcept
        : m_parameters(parameters), m_saved(parameters) {}
    SceneDrawScope(const SceneDrawScope&) = delete;
    SceneDrawScope& operator=(const SceneDrawScope&) = delete;
    ~SceneDrawScope() { m_parameters = m_saved; }
private:
    SceneDrawParameters& m_parameters;
    SceneDrawParameters m_saved;
};
}
