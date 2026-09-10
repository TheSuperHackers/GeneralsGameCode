module;
#include <filesystem>

export module Graphics.Frame.SceneRenderers;

export import Graphics.RHI;
import Graphics.Renderer2D;
import Graphics.Capture.FramePreview;
import Graphics.Passes.Bloom;
import Graphics.Passes.LightRays;
import Graphics.Passes.SSAO;
import Graphics.Scene.Beams;
import Graphics.Scene.Beams.Laser;
import Graphics.Scene.Lighting.Renderer;
import Graphics.Scene.Particles.Renderer;
import Graphics.Scene.Screen.Distortion;
import Graphics.Scene.Screen.FullscreenOverlay;
import Graphics.Scene.Screen.Filters;
import Graphics.Scene.Ring;
import Graphics.Scene.WorldQuads;
import Graphics.Scene.Terrain.Renderer;
import Graphics.Scene.Trees.Renderer;
import Graphics.Scene.Water.Renderer;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Extraction;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Surfaces.Renderer;
import Graphics.Scene.Lighting.Environment;
import Graphics.Scene.Shadows.DirectionalRenderer;

namespace Graphics
{
// Applications release their imported resources before shutting down these
// renderers. The caller retains device, target, and presentation ownership.
export void Shutdown_Scene_Renderers() noexcept
{
    Get_Frame_Preview().Shutdown();
    Get_Bloom_Renderer().Shutdown();
    Get_Light_Rays_Renderer().Shutdown();
    Get_SSAO_Renderer().Shutdown();
    Get_Prop_Submission().Shutdown();
    Get_Environment_Lighting() = {};
    Get_Renderer2D().Shutdown();
    GetLightRenderer().Shutdown();
    GetScreenDistortionRenderer().Shutdown();
    GetParticleRenderer().Shutdown();
    GetLaserRenderer().Shutdown();
    GetBeamRenderer().Shutdown();
    GetRingRenderer().Shutdown();
    GetFullscreenOverlayRenderer().Shutdown();
    GetWorldQuadRenderer().Shutdown();
    Shutdown_Terrain_Renderers();
    Get_Directional_Shadow_Renderer().Shutdown();
    Get_Screen_Filter_Renderer().Shutdown();
    Get_Prop_Renderer().Shutdown();
    Get_Prop_Extraction_Cache().Clear();
    Get_Water_Renderer().Shutdown();
    Get_Tree_Renderer().Shutdown();
    Get_Surface_Renderer().Shutdown();
}

export bool Initialize_Scene_Renderers(Device& device, const std::filesystem::path& shaders)
{
    const bool initialized = Get_Frame_Preview().Initialize(device, shaders)
        && Get_Screen_Filter_Renderer().Initialize(device, shaders)
        && Get_Directional_Shadow_Renderer().Initialize(device, shaders)
        && Get_Prop_Renderer().Initialize(device, shaders)
        && Get_Water_Renderer().Initialize(device, shaders)
        && Get_Tree_Renderer().Initialize(device, shaders)
        && Get_Surface_Renderer().Initialize(device, shaders)
        && Initialize_Terrain_Renderers(device, shaders)
        && (GetLaserRenderer().Is_Initialized() || GetLaserRenderer().Initialize(device, shaders, 4096))
        && (GetBeamRenderer().Is_Initialized() || GetBeamRenderer().Initialize(device, shaders, 4096))
        && (GetLightRenderer().Is_Initialized() || GetLightRenderer().Initialize(device, 4096))
        && (GetParticleRenderer().Is_Initialized() || GetParticleRenderer().Initialize(device, shaders, 1024, 65536))
        && (GetScreenDistortionRenderer().Is_Initialized() || GetScreenDistortionRenderer().Initialize(device, shaders, 512))
        && (GetRingRenderer().Is_Initialized() || GetRingRenderer().Initialize(device, shaders))
        && (GetFullscreenOverlayRenderer().Is_Initialized() || GetFullscreenOverlayRenderer().Initialize(device, shaders))
        && (GetWorldQuadRenderer().Is_Initialized() || GetWorldQuadRenderer().Initialize(device, shaders, 1000, 8))
        && Get_Renderer2D().Initialize(device, shaders);
    if (!initialized) Shutdown_Scene_Renderers();
    else Get_Prop_Submission().Initialize(device,Get_Prop_Renderer(),Get_Directional_Shadow_Renderer());
    return initialized;
}
}
