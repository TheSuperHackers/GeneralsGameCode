module;
#define BOOST_TEST_MODULE LineMaterialDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Lines.MaterialDrawing.Tests;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(textured_lines_retain_tiling_scrolling_blend_and_scene_depth)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) { vertex.color={0.5f,0.25f,1,0.5f}; vertex.uv={1.25f,0}; }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    const std::array<std::uint8_t,8> texels{255,0,0,255,0,255,0,255};
    const auto texture=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.depth_write=false;
    style.source_blend=RHIBlendFactor::One;
    style.destination_blend=RHIBlendFactor::One;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    const std::array textures{texture};
    const auto check=[&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for(int channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
    };
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.25f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({191,64,64,191});
    // Advancing the CPU UV offset by half a tile selects green.
    for(auto& vertex : vertices) vertex.uv[0]+=0.5f;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.25f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({64,128,64,191});
    // Projectile streams multiply the source by alpha before adding it.
    style.source_blend=RHIBlendFactor::SourceAlpha;
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.25f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({64,96,64,128});
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0.25f},0.2f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({64,64,64,64});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for(auto handle : {texture,target,depth}) device.Destroy_Texture(handle);
}
