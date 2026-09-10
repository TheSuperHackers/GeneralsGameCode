module;
#define BOOST_TEST_MODULE ScreenFilterDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Screen.Filters.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Screen.Filters;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(copy_tint_crossfade_motion_accumulation_and_capture)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ScreenFilterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,4> pixel{128,64,32,128},mask_pixel{128,255,255,128};
    const auto texture=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
    const auto mask=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(mask_pixel)),4});
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<ScreenFilterVertex,4> vertices{};
    vertices[0].position={1,-1,0}; vertices[1].position={1,1,0};
    vertices[2].position={-1,-1,0}; vertices[3].position={-1,1,0};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    ScreenFilterParameters parameters;
    ScreenFilterStyle style;
    const auto check=[&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    };
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture));
    check({128,64,32,128});
    parameters.operation=1;
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture));
    check({80,80,80,80});
    parameters.tint={1,0,0,1}; parameters.fade=0.5f;
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture));
    check({104,32,16,80});
    parameters.operation=2; style.blend=true;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture,mask));
    check({16,16,199,207});
    parameters.operation=0; parameters.vertex_alpha=1;
    for(auto& vertex:vertices) vertex.color[3]=0.5f;
    style.destination=RHIBlendFactor::One;
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture));
    check({128,96,80,64});
    style.blend=false; parameters.vertex_alpha=0; parameters.operation=3;
    for(auto& vertex:vertices) vertex.color[3]=1;
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,texture));
    check({32,64,128,128});
    parameters.textured=0; parameters.operation=0; style.color_write_mask=8;
    for(auto& vertex:vertices) vertex.color[3]=0.25f;
    BOOST_REQUIRE(renderer.Draw(commands,vertices,parameters,style,{}));
    check({32,64,128,64});
    renderer.Shutdown();
    for(auto handle:{texture,mask,target,depth}) device.Destroy_Texture(handle);
}

