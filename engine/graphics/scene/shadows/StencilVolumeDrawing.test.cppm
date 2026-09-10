module;
#define BOOST_TEST_MODULE StencilVolumeDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Shadows.StencilVolumes.Tests;
import Graphics.Scene.Shadows.StencilVolumes;
import Graphics.Scene.Surfaces.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(volume_winding_stencil_count_and_occluder_mask)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<SurfaceVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f};
    vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f};
    vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> front{0,1,2,0,2,3}, back{0,2,1,0,3,2};
    const auto increment_mesh = renderer.Create_Mesh(vertices,front);
    const auto decrement_mesh = renderer.Create_Mesh(vertices,back);
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D24_UNorm_S8,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
    SurfaceParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 0;
    RHIStencilDescription stencil;
    stencil.reference = 128;
    stencil.read_mask = 128;
    stencil.front.comparison = RHIComparison::NotEqual;
    const auto verify = [&](unsigned value) {
        SurfaceStyle style;
        style.stencil.enabled = true;
        style.stencil.reference = static_cast<std::uint8_t>(value);
        style.stencil.front.comparison = RHIComparison::Equal;
        style.stencil.back = style.stencil.front;
        style.stencil.write_mask = 0;
        BOOST_REQUIRE(renderer.Draw(commands,increment_mesh,style,parameters,{}));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4]),255);
    };
    BOOST_REQUIRE(Draw_Stencil_Volume(renderer,commands,increment_mesh,parameters,true,stencil));
    verify(1);
    SurfaceMeshHandle overlay;
    BOOST_REQUIRE(Draw_Stencil_Shadow(renderer,commands,overlay,{-1,1,1,-1},{0.5f,0.25f,0.75f,1},127));
    std::array<std::byte,16*16*4> shaded{};
    BOOST_REQUIRE(device.Readback_Texture(target,shaded,16*4));
    const std::array<int,4> expected{128,64,191,255};
    for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(shaded[(8*16+8)*4+c])-expected[c],2);
    // Set color to black without clearing stencil, making a failed test visible.
    SurfaceStyle black;
    SurfaceParameters invisible = parameters;
    invisible.opacity = 1;
    for (auto& vertex : vertices) vertex.color = {0,0,0,1};
    const auto black_mesh = renderer.Create_Mesh(vertices,front);
    BOOST_REQUIRE(renderer.Draw(commands,black_mesh,black,invisible,{}));
    BOOST_REQUIRE(Draw_Stencil_Volume(renderer,commands,decrement_mesh,parameters,false,stencil));
    verify(0);
    SurfaceStyle occluder;
    occluder.color_write_mask = 7;
    occluder.stencil.enabled = true;
    occluder.stencil.reference = 128;
    occluder.stencil.front.pass = RHIStencilOperation::Replace;
    occluder.stencil.back = occluder.stencil.front;
    BOOST_REQUIRE(renderer.Draw(commands,black_mesh,occluder,parameters,{}));
    BOOST_REQUIRE(Draw_Stencil_Volume(renderer,commands,increment_mesh,parameters,true,stencil));
    verify(128);
    BOOST_REQUIRE(Draw_Stencil_Shadow(renderer,commands,overlay,{-1,1,1,-1},{0.5f,0.25f,0.75f,1},127));
    BOOST_REQUIRE(device.Readback_Texture(target,shaded,16*4));
    // The player occlusion bit alone must not darken the pixel.
    BOOST_CHECK_EQUAL(std::to_integer<int>(shaded[(8*16+8)*4]),255);
    occluder.stencil.reference=129;
    BOOST_REQUIRE(renderer.Draw(commands,black_mesh,occluder,parameters,{}));
    BOOST_REQUIRE(Draw_Player_Occlusion(renderer,commands,{-1,1,1,-1},{1,0,0,0.5f},129,false,255));
    BOOST_REQUIRE(device.Readback_Texture(target,shaded,16*4));
    BOOST_CHECK_SMALL(std::to_integer<int>(shaded[(8*16+8)*4])-128,2);
    BOOST_CHECK_SMALL(std::to_integer<int>(shaded[(8*16+8)*4+3])-191,2);
    BOOST_REQUIRE(Draw_Player_Occlusion(renderer,commands,{-1,1,1,-1},{1,1,1,1},0,true,255));
    BOOST_REQUIRE(renderer.Draw(commands,black_mesh,black,parameters,{}));
    verify(128);
    // Pixels without an encoded player are cleared to zero.
    occluder.stencil.reference=128;
    BOOST_REQUIRE(renderer.Draw(commands,black_mesh,occluder,parameters,{}));
    BOOST_REQUIRE(Draw_Player_Occlusion(renderer,commands,{-1,1,1,-1},{1,1,1,1},0,true,255));
    verify(0);
    renderer.Destroy_Mesh(overlay);
    for (auto mesh : {increment_mesh,decrement_mesh,black_mesh}) renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}
