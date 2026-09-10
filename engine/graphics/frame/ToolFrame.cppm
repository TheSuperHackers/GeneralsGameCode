module;
#include <filesystem>

export module Graphics.Frame.ToolFrame;

import Graphics.Frame.Runtime;
import Graphics.Frame.SceneRenderers;
import Graphics.Renderer2D;
import Graphics.Resources.Recreation;

namespace Graphics
{
namespace
{
bool initialized = false;
std::filesystem::path shader_directory{"GraphicsShaders"};
ResourceRecreationRegistration resource_registration;

void Release_Tool_Renderers() noexcept
{
    Shutdown_Scene_Renderers();
    initialized = false;
}

bool Initialize_Tool_Renderers(Device& device)
{
    return Initialize_Scene_Renderers(device, shader_directory);
}

bool Draw_Tool_Overlays(Device& device, CommandList& commands,
    const FrameTargets& targets) noexcept
{
    return Get_Renderer2D().Execute(device, commands,
        targets.backbuffer.texture, targets.depth.texture,
        {0, 0, targets.backbuffer.width, targets.backbuffer.height, 0, 1});
}
}

// Tool applications may supply an installed shader directory before their
// first frame. Recovery reuses that directory and initializes on the next begin.
export bool Initialize_Tool_Frame(const std::filesystem::path& shaders)
{
    if (initialized || !Frame_Device_Ready()) return false;
    shader_directory = shaders;
    if (!Register_Frame_Draw_Executor(&Initialize_Tool_Renderers, &Draw_Tool_Overlays))
        return false;
    resource_registration = Get_Resource_Recreation_Registry().Register(
        &Release_Tool_Renderers, {});
    initialized = true;
    return true;
}

export bool Begin_Tool_Frame()
{
    if (!Frame_Device_Ready() && !Recover_Frame_Device()) return false;
    if (!initialized && !Initialize_Tool_Frame(shader_directory)) return false;
    if (!Graphics_Begin_Frame()) return false;
    if (!initialized) {
        // A device loss discovered during begin released the renderers. Retry
        // initialization while idle on the next frame.
        Graphics_Abort_Frame();
        return false;
    }
    // Begin can recover the device. Borrow the drawable extent afterwards.
    const auto resources = Shared_Frame_Device()->Get_Swap_Chain().Backbuffer();
    Get_Renderer2D().Begin(resources.width, resources.height);
    return true;
}

export void Abort_Tool_Frame() noexcept
{
    Get_Renderer2D().Discard();
    Graphics_Abort_Frame();
}

export bool End_Tool_Frame()
{
    if (Graphics_Execute_Queued_Draws()
        && Graphics_End_Frame() && Graphics_Present()) return true;
    Abort_Tool_Frame();
    return false;
}

export void Shutdown_Tool_Frame() noexcept
{
    Detach_Frame_Draw_Executor();
    resource_registration.Reset();
    Release_Tool_Renderers();
}
}
