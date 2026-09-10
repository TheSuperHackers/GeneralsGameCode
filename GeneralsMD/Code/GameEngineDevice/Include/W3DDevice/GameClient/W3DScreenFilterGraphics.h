#pragma once
#include "W3DDevice/GameClient/W3DGraphicsResources.h"

#include "W3DDevice/GameClient/W3DTextureHandle.h"
import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.Runtime;
import Graphics.Scene.Screen.Filters;

// Translate the existing viewport quad and texture ownership into graphics data.
template<typename QuadVertex>
bool Draw_Screen_Filter_Quad(const QuadVertex* source, W3DTextureHandle* texture,
    const Graphics::ScreenFilterParameters& parameters={},
    const Graphics::ScreenFilterStyle& style={}, W3DTextureHandle* mask=nullptr)
{
    auto* device=Graphics::Shared_Frame_Device();
    Graphics::RHIViewport viewport{};
    viewport = Graphics::Get_Attachment_Bindings().Current().viewport;
    if (!device || !viewport.width || !viewport.height) return false;
    std::array<Graphics::ScreenFilterVertex,4> vertices{};
    for (unsigned i=0;i<4;++i) {
        const auto& src=source[i]; auto& dst=vertices[i];
        dst.position={2*(src.p.X+0.5f-viewport.x)/viewport.width-1,
            1-2*(src.p.Y+0.5f-viewport.y)/viewport.height,src.p.Z};
        dst.color={float((src.color>>16)&255)/255,float((src.color>>8)&255)/255,
            float(src.color&255)/255,float(src.color>>24)/255};
        dst.uv={src.u,src.v};
        if constexpr(requires { src.u1; src.v1; }) dst.mask_uv={src.u1,src.v1};
    }
    const bool drawn=Graphics::Get_Screen_Filter_Renderer().Draw(device->Immediate_Command_List(),
        vertices,parameters,style,Resolve_Graphics_Texture(texture),Resolve_Graphics_Texture(mask));
    return drawn;
}


