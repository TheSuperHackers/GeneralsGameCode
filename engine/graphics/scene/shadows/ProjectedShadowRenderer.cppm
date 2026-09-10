module;
#include <span>
export module Graphics.Scene.Shadows.Projected;
export import Graphics.Scene.Props.Renderer;

namespace Graphics
{
export enum class DecalBlend { Multiply, Alpha, Additive };

// Receiver geometry is already conformed to the terrain by the map adapter.
// This component owns the shared decal and projected-shadow draw policy.
export bool Draw_Decal(PropRenderer& renderer, CommandList& commands,
    PropMeshHandle mesh, PropParameters parameters, RHITextureHandle texture,
    DecalBlend blend)
{
    PropStyle style;
    style.depth_write = false;
    style.blend = RHIBlendMode::Alpha;
    style.source_blend = blend == DecalBlend::Alpha ? RHIBlendFactor::SourceAlpha
        : blend == DecalBlend::Additive ? RHIBlendFactor::One : RHIBlendFactor::Zero;
    style.destination_blend = blend == DecalBlend::Alpha ? RHIBlendFactor::InverseSourceAlpha
        : blend == DecalBlend::Additive ? RHIBlendFactor::One : RHIBlendFactor::SourceColor;
    for (auto& sampler : style.samplers) sampler.address.fill(RHISamplerAddress::Clamp);
    return renderer.Draw(commands, mesh, style, parameters, std::span(&texture, 1));
}

export bool Draw_Projected_Shadow(PropRenderer& renderer, CommandList& commands,
    PropMeshHandle mesh, PropParameters parameters, std::span<const RHITextureHandle> textures)
{
    PropStyle style;
    style.depth_write = false;
    style.blend = RHIBlendMode::Alpha;
    style.source_blend = RHIBlendFactor::DestinationColor;
    style.destination_blend = RHIBlendFactor::Zero;
    for (auto& sampler : style.samplers) sampler.address.fill(RHISamplerAddress::Clamp);
    style.stencil.enabled = true;
    style.stencil.reference = 1;
    style.stencil.front.pass = RHIStencilOperation::Increment;
    style.stencil.back = style.stencil.front;
    return renderer.Draw(commands, mesh, style, parameters, textures);
}
}

