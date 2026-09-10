module;
#define BOOST_TEST_MODULE ScreenDistortionDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
export module Graphics.Scene.Screen.DistortionDrawing.Tests;
import Graphics.Scene.Screen.Distortion;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(distortion_samples_current_background_and_preserves_edges_and_depth)
{
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    ScreenDistortionRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_SCREEN_SHADER_DIRECTORY),1));
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{},{0,0,32,32,0,1}}));
    std::array<std::byte,32*32*4> source{};
    for (unsigned y=0;y<32;++y) for (unsigned x=0;x<32;++x) {
        source[(y*32+x)*4]=static_cast<std::byte>(x*8);
        // Scene alpha carries coverage/masks, not the distortion's opacity.
        source[(y*32+x)*4+3]=std::byte{0};
    }
    const auto background=device.Create_Texture_Initialized({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)},{source,32*4});
    const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(background.Is_Valid()); BOOST_REQUIRE(target.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
    auto& commands=device.Immediate_Command_List();
    const std::array<float,1> zero{0}, z{0.5f}, size{0.8f}, offset{0.25f}, one{1};
    ScreenDistortionData data{zero,zero,z,offset,zero,size,zero};
    const auto draw=[&](float depth_clear) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear_Depth(depth_clear));
        BOOST_REQUIRE(commands.Copy_Texture(background,target));
        BOOST_REQUIRE(renderer.Render(commands,target,depth,{0,0,32,32},data,RHITextureFormat::RGBA8_UNorm));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        return pixels;
    };
    const auto invisible=draw(1);
    BOOST_CHECK(invisible==source);
    data.opacities=one;
    const auto occluded=draw(0);
    BOOST_CHECK(occluded==source);
    const auto displaced=draw(1);
    const unsigned center=(16*32+16)*4;
    BOOST_CHECK_GT(std::to_integer<int>(displaced[center]),std::to_integer<int>(source[center])+30);
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK(displaced[channel]==source[channel]);
    BOOST_CHECK_EQUAL(std::to_integer<int>(displaced[center+1]),0);
    BOOST_CHECK_EQUAL(std::to_integer<int>(displaced[center+2]),0);
    // Replacing the copy texture must retain a valid logical binding and the
    // execution plan must observe the current targets after each resize.
    data.opacities=zero;
    std::array<std::byte,16*16*4> resized_source{};
    for (unsigned i=0;i<16*16;++i) resized_source[i*4+2]=std::byte{200};
    const auto resized=device.Create_Texture_Initialized({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)},{resized_source,16*4});
    const auto resized_depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(commands.Set_Render_Targets(resized,resized_depth));
    BOOST_REQUIRE(commands.Clear_Depth(1));
    BOOST_REQUIRE(renderer.Render(commands,resized,resized_depth,{0,0,16,16},data,RHITextureFormat::RGBA8_UNorm));
    std::array<std::byte,16*16*4> resized_pixels{};
    BOOST_REQUIRE(device.Readback_Texture(resized,resized_pixels,16*4));
    BOOST_CHECK(resized_pixels==resized_source);
    const auto bgra=device.Create_Texture({16,16,1,RHITextureFormat::BGRA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(commands.Set_Render_Targets(bgra,resized_depth));
    BOOST_REQUIRE(commands.Clear({0.1f,0.2f,0.3f,0},1));
    BOOST_REQUIRE(renderer.Render(commands,bgra,resized_depth,{0,0,16,16},data,RHITextureFormat::BGRA8_UNorm));
    BOOST_CHECK(draw(1)==source);
    renderer.Shutdown();
    device.Destroy_Texture(resized); device.Destroy_Texture(resized_depth); device.Destroy_Texture(bgra);
    device.Destroy_Texture(background); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
