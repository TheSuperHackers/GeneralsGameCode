#pragma once

#include <memory>
import Graphics.Materials.ProceduralPass;
import Graphics.Scene.DrawContext;
#include "WWLib/ref_ptr.h"

class W3DCamera;
class W3DTextureHandle;
class OBBoxClass;
using NativeMaterialPass = Graphics::ProceduralMaterialPass<RefCountPtr<W3DTextureHandle>, OBBoxClass>;

class W3DRenderContext final : public Graphics::SceneDrawContext<NativeMaterialPass>
{
public:
    explicit W3DRenderContext(W3DCamera &camera) : Camera(camera) {}
    W3DCamera &Camera;
};
