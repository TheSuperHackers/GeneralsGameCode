module;
#define BOOST_TEST_MODULE TreeDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Trees.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Trees.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(sway_pivot_cutout_darkening_overbright_and_shroud)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    TreeRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto leaves = texture({200,100,50,128});
    const auto transparent = texture({200,100,50,127});
    const auto shroud = texture({128,128,128,255});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<TreeVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f};
    vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f};
    vertices[3].position = {-1,1,0.5f};
    for (auto& vertex : vertices) vertex.sway = {10,0.5f,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    TreeParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.options = {1,0.5f,2,0};
    parameters.sway[9] = {8,0,0,0};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    std::array<RHITextureHandle,2> textures{transparent,shroud};
    const auto check = [&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    };
    BOOST_REQUIRE(renderer.Draw(commands,mesh,parameters,textures));
    check({0,0,255,255});
    textures[0] = leaves;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,parameters,textures));
    check({100,50,25,128});
    // A vertex at its base height stays anchored despite the large sway.
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    for (auto& vertex : vertices) vertex.sway[2] = 0;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,parameters,textures));
    check({0,0,255,255});
    parameters.sway[9] = {};
    BOOST_REQUIRE(renderer.Draw(commands,mesh,parameters,textures));
    check({100,50,25,128});
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,parameters,textures));
    check({100,50,25,128});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for(auto handle : {leaves,transparent,shroud,target,depth}) device.Destroy_Texture(handle);
}
