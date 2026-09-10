#pragma once
#include <array>
#include <span>
import Graphics.Scene.Surfaces.Renderer;

class W3DTextureHandle;
class W3DCamera;
class W3DShroud;

// Shared native texture ownership for adapters that consume generated atlases
// and map textures. Clear before destroying the device and when unloading a map.
Graphics::RHITextureHandle Resolve_Graphics_Texture(W3DTextureHandle *texture);
void Release_Graphics_Textures() noexcept;
Graphics::SurfaceParameters Make_Surface_Parameters(W3DCamera &camera);
Graphics::RHITextureHandle Set_Surface_Shroud(Graphics::SurfaceParameters &parameters, W3DShroud *shroud);
