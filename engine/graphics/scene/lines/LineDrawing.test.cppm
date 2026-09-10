module;
#define BOOST_TEST_MODULE LineDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Lines.Drawing.Tests;
import Graphics.Scene.Lines.Drawing;
import Graphics.Scene.Props.Geometry;
import Graphics.Tests.Device;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(navigation_line_adds_color_across_terrain_without_changing_depth)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.8f}; vertices[1].position={1,-1,0.8f};
    vertices[2].position={1,1,0.8f}; vertices[3].position={-1,1,0.8f};
    for (auto& vertex : vertices) vertex.color={0.25f,0.5f,1,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    SurfaceMeshHandle mesh{};
    BOOST_REQUIRE(Update_Navigation_Line(renderer,mesh,vertices,indices));
    BOOST_CHECK(mesh.Is_Valid());
    const std::array<std::uint8_t,4> texel{128,128,128,128};
    const auto texture=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(texel)),4});
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.75f},0.2f));
    const std::array<float,16> projection{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    vertices = {};
    BOOST_REQUIRE(Draw_Navigation_Line(renderer,commands,mesh,projection,texture));
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    const std::array<int,4> expected{96,128,192,191};
    for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    // A subsequent ordinary surface at 0.5 must remain hidden by terrain at 0.2.
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) { vertex.position[2]=0.5f; vertex.color={1,0,0,1}; }
    BOOST_REQUIRE(Update_Navigation_Line(renderer,mesh,vertices,indices));
    vertices = {};
    SurfaceParameters parameters; parameters.view_projection=projection; parameters.textured=0;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,{},parameters,{}));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4])-96,2);
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(Draw_Navigation_Line(renderer,commands,mesh,projection,{}));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    BOOST_CHECK_GT(std::to_integer<unsigned>(pixels[(8*16+8)*4]),0u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*16+8)*4+1]),0u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*16+8)*4+2]),0u);
    // The navigation surface writes RGB only (SurfaceStyle's default mask is
    // 0b0111), so its additive draw preserves the cleared destination alpha.
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*16+8)*4+3]),0u);
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    for (auto handle : {texture,target,depth}) device.Destroy_Texture(handle);
}
