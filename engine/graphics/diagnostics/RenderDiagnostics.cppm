export module Graphics.Diagnostics.Render;

namespace Graphics
{
// Device-thread diagnostic controls consumed during scene and overlay traversal.
// Resource recreation must retain them; a new application session resets them.
export struct RenderDiagnostics final
{
    bool collecting_statistics = false;
    bool disable_water = false;
    bool disable_objects = false;
    bool disable_overhead = false;
    bool disable_console = false;
    int console_line_limit = -1;
};

export RenderDiagnostics& Get_Render_Diagnostics() noexcept
{
    static RenderDiagnostics diagnostics;
    return diagnostics;
}
}
