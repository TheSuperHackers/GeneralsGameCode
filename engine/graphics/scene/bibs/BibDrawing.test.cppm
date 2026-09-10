module;
#define BOOST_TEST_MODULE BibDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Bibs.Drawing.Tests;
import Graphics.Scene.Bibs.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(bibs_blend_before_models_and_draw_into_the_current_target)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<BibQuad,1> quads{{{{{{-1,-1,0.8f},{1,-1,0.8f},{1,1,0.8f},{-1,1,0.8f}}}}}};
    SurfaceMeshHandle mesh;
    const std::array<std::uint8_t,4> texel{128,128,128,128};
    const auto texture=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(texel)),4});
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.75f},0.2f));
    SurfaceParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    BOOST_REQUIRE(Draw_Bibs(renderer,commands,mesh,quads,{0.5f,1,0,1},parameters,texture));
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    const std::array<int,4> expected{64,96,32,191};
    for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    const auto reflection=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(commands.Set_Render_Targets(reflection,depth));
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.2f));
    BOOST_REQUIRE(Draw_Bibs(renderer,commands,mesh,quads,{0.5f,1,0,1},parameters,texture));
    BOOST_REQUIRE(device.Readback_Texture(reflection,pixels,16*4));
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+1])-64,2);
    // The original target must retain its own color, and later models can cover bibs.
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    std::array<SurfaceVertex,4> model_vertices{};
    for (unsigned i=0;i<4;++i) { model_vertices[i].position=quads[0].corners[i]; model_vertices[i].position[2]=0.1f; model_vertices[i].color={1,0,0,1}; }
    const std::array<std::uint32_t,6> topology{0,1,2,0,2,3};
    const auto model=renderer.Create_Mesh(model_vertices,topology);
    parameters.textured=0;
    BOOST_REQUIRE(renderer.Draw(commands,model,{},parameters,{}));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4]),255);
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4+1]),0);
    renderer.Destroy_Mesh(model);
    device.Destroy_Texture(reflection);
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    for (auto handle : {texture,target,depth}) device.Destroy_Texture(handle);
}
