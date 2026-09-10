module;
#include <array>
#include <cstdint>
export module Graphics.Scene.Shadows.StencilVolumes;
export import Graphics.Scene.Surfaces.Renderer;
namespace Graphics
{
export bool Draw_Stencil_Volume(SurfaceRenderer& renderer, CommandList& commands,
    SurfaceMeshHandle mesh, SurfaceParameters parameters, bool increment,
    const RHIStencilDescription& stencil)
{
    SurfaceStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.color_write_mask = 0;
    style.cull = RHICullMode::Back;
    style.front_counter_clockwise = increment;
    style.stencil = stencil;
    style.stencil.enabled = true;
    style.stencil.front.pass = increment ? RHIStencilOperation::Increment : RHIStencilOperation::DecrementSaturate;
    style.stencil.back = style.stencil.front;
    parameters.textured = 0;
    return renderer.Draw(commands,mesh,style,parameters,{});
}

export bool Draw_Stencil_Shadow(SurfaceRenderer& renderer, CommandList& commands,
    SurfaceMeshHandle& mesh, const std::array<float,4>& bounds,
    const std::array<float,4>& color, std::uint8_t read_mask)
{
    std::array<SurfaceVertex,4> vertices{};
    vertices[0].position={bounds[0],bounds[1],0};
    vertices[1].position={bounds[2],bounds[1],0};
    vertices[2].position={bounds[2],bounds[3],0};
    vertices[3].position={bounds[0],bounds[3],0};
    for (auto& vertex : vertices) vertex.color=color;
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    if (!mesh.Is_Valid()) mesh=renderer.Create_Mesh(vertices,indices);
    else if (!renderer.Update_Mesh(mesh,vertices,indices)) return false;
    SurfaceStyle style;
    style.blend=RHIBlendMode::Multiply;
    style.depth_comparison=RHIComparison::Always;
    style.stencil.enabled=true;
    style.stencil.reference=1;
    style.stencil.read_mask=read_mask;
    style.stencil.write_mask=0;
    style.stencil.front.comparison=RHIComparison::LessEqual;
    style.stencil.back=style.stencil.front;
    SurfaceParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    return renderer.Draw(commands,mesh,style,parameters,{});
}

export bool Draw_Player_Occlusion(SurfaceRenderer& renderer, CommandList& commands,
    const std::array<float,4>& bounds, const std::array<float,4>& color,
    std::uint8_t reference, bool clear, std::uint8_t occluded_mask)
{
    std::array<SurfaceVertex,4> vertices{};
    vertices[0].position={bounds[0],bounds[1],0};
    vertices[1].position={bounds[2],bounds[1],0};
    vertices[2].position={bounds[2],bounds[3],0};
    vertices[3].position={bounds[0],bounds[3],0};
    for (auto& vertex : vertices) vertex.color=color;
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    SurfaceStyle style;
    style.color_write_mask=clear ? 0 : 15;
    style.blend_alpha_like_color=true;
    style.depth_comparison=clear ? RHIComparison::Never : RHIComparison::Always;
    style.stencil.enabled=true;
    style.stencil.reference=clear ? 128 : reference;
    style.stencil.read_mask=clear ? occluded_mask : 255;
    style.stencil.front.comparison=clear ? RHIComparison::Less : RHIComparison::Equal;
    if (clear) {
        style.stencil.front.depth_fail=RHIStencilOperation::Replace;
        style.stencil.front.pass=RHIStencilOperation::Replace;
        style.stencil.front.fail=RHIStencilOperation::Zero;
    }
    style.stencil.back=style.stencil.front;
    SurfaceParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    const bool drawn=renderer.Draw(commands,mesh,style,parameters,{});
    renderer.Destroy_Mesh(mesh);
    return drawn;
}
}
