#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DShroud.h"
import Graphics.Frame.Runtime;
import Graphics.Resources.Textures.References;

namespace
{
Graphics::TextureReferences texture_references;
}

Graphics::RHITextureHandle Resolve_Graphics_Texture(W3DTextureHandle *texture)
{
    auto* device = Graphics::Shared_Frame_Device();
    if (device == nullptr || texture == nullptr || !texture->Ensure_Render_Backend_Texture()
        || texture->Is_Missing_Texture()) return {};
    return texture_references.Retain(*device, texture->Peek_Graphics_Texture());
}

void Release_Graphics_Textures() noexcept
{
    texture_references.Clear();
}

Graphics::SurfaceParameters Make_Surface_Parameters(W3DCamera &camera)
{
    Matrix3D view;
    Matrix4x4 projection;
    camera.Get_View_Matrix(&view);
    camera.Get_Backend_Projection_Matrix(&projection);
    const Matrix4x4 transform = projection * Matrix4x4(view);
    Graphics::SurfaceParameters parameters;
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            parameters.view_projection[row * 4 + column] = transform[row][column];
    return parameters;
}

Graphics::RHITextureHandle Set_Surface_Shroud(Graphics::SurfaceParameters &parameters, W3DShroud *shroud)
{
    parameters.shroud = 0;
    if (shroud == nullptr || shroud->getCellWidth() <= 0 || shroud->getCellHeight() <= 0
        || shroud->getTextureWidth() <= 0 || shroud->getTextureHeight() <= 0) return {};
    const auto texture = Resolve_Graphics_Texture(shroud->getShroudTexture());
    if (!texture.Is_Valid()) return {};
    const float sx = 1.0f / (shroud->getCellWidth() * shroud->getTextureWidth());
    const float sy = 1.0f / (shroud->getCellHeight() * shroud->getTextureHeight());
    parameters.shroud_projection = {sx, sy, (-shroud->getDrawOriginX() + shroud->getCellWidth()) * sx,
        (-shroud->getDrawOriginY() + shroud->getCellHeight()) * sy};
    parameters.shroud = 1;
    return texture;
}
