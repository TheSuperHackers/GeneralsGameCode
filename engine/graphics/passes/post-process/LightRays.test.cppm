module;
#define BOOST_TEST_MODULE LightRaysTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Passes.LightRays.Tests;
import Graphics.Passes.LightRays;
import Graphics.RHI;
import Graphics.FrameTargets;
import Graphics.Tests.Device;
import Graphics.Capture.FrameCapture;
import Graphics.Scene.Lighting.Environment;

using namespace Graphics;

namespace
{
constexpr auto ColorFormat = RHITextureFormat::RGBA8_UNorm;
constexpr auto DepthFormat = RHITextureFormat::D24_UNorm_S8;
const std::filesystem::path Shaders(Graphics::Test_Shader_Directory(GRAPHICS_LIGHT_RAYS_SHADER_DIRECTORY));

LightRaysInput OrthographicInput()
{
    LightRaysInput input;
    input.inverse_view_projection = {1,0,0,0, 0,1,0,0, 0,0,100,0, 0,0,0,1};
    input.brightness = {3,3,3};
    input.shroud_projection = {0.5f,-0.5f,0.5f,0.5f};
    return input;
}

struct Fixture final
{
    GraphicsTestDevice device;
    LightRaysRenderer rays;
    FrameCapture capture;
    FrameTargets targets{};
    RHITextureFormat format = ColorFormat;
    explicit Fixture(bool warp) : device({warp})
    {
        BOOST_REQUIRE(device.Is_Valid());
        Get_Environment_Lighting() = {};
        BOOST_REQUIRE(rays.Initialize(device,Shaders));
    }
    ~Fixture()
    {
        Get_Environment_Lighting() = {};
        rays.Shutdown();
        ReleaseTargets();
    }
    void ReleaseTargets()
    {
        device.Immediate_Command_List().Reset_State();
        if (targets.backbuffer.texture.Is_Valid()) device.Destroy_Texture(targets.backbuffer.texture);
        if (targets.depth.texture.Is_Valid()) device.Destroy_Texture(targets.depth.texture);
        targets = {};
    }
    void Reset(unsigned width, unsigned height, float depth = 0.5f)
    {
        ReleaseTargets();
        std::vector<std::byte> pixels(width*height*4);
        for (unsigned i=0;i<width*height;++i) {
            pixels[i*4] = pixels[i*4+1] = pixels[i*4+2] = std::byte{16};
            pixels[i*4+3] = std::byte{73};
        }
        const auto color = device.Create_Texture_Initialized({width,height,1,format,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)}, {pixels,width*4});
        const auto depth_target = device.Create_Texture({width,height,1,DepthFormat,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(color.Is_Valid());
        BOOST_REQUIRE(depth_target.Is_Valid());
        targets = {{color,width,height},{depth_target,width,height}};
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(color,depth_target));
        BOOST_REQUIRE(commands.Clear_Depth(depth));
    }
    void Render(const LightRaysInput& input, bool enabled = true)
    {
        BOOST_REQUIRE(rays.Render(device.Immediate_Command_List(),targets,format,DepthFormat,input,enabled));
    }
    auto Read() { return capture.Read(device,targets.backbuffer.texture,
        targets.backbuffer.width,targets.backbuffer.height,format); }
};
}

BOOST_AUTO_TEST_CASE(depth_integral_disabled_background_alpha_resize_and_restart)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        LightRaysRenderer uninitialized;
        BOOST_CHECK(uninitialized.Render(f.device.Immediate_Command_List(),{},ColorFormat,DepthFormat,{},false));
        for (const auto format : {ColorFormat,RHITextureFormat::BGRA8_UNorm}) {
            f.format = format;
            for (const auto width : {32u,37u,1u}) {
                for (const float depth : {0.0f,0.25f,0.5f,1.0f}) {
                    f.Reset(width,width == 1 ? 1 : width-3,depth);
                    f.Render(OrthographicInput(),false);
                    auto frame = f.Read();
                    BOOST_REQUIRE(frame.Is_Valid());
                    for (unsigned y=0;y<frame.height;++y) for (unsigned x=0;x<frame.width;++x) {
                        const auto offset = y*frame.row_pitch+x*4;
                        BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[offset]),16);
                        BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[offset+3]),73);
                    }
                    f.Render(OrthographicInput());
                    frame = f.Read();
                    BOOST_REQUIRE(frame.Is_Valid());
                    // Independent integral: 3/400 * (100*depth) * 255 + 16.
                    // Clear depth 1 has no surface and must remain untouched.
                    const int expected = depth == 0.25f ? 64 : depth == 0.5f ? 112 : 16;
                    for (unsigned y=0;y<frame.height;++y) for (unsigned x=0;x<frame.width;++x) {
                        const auto offset = y*frame.row_pitch+x*4;
                        for (unsigned c=0;c<3;++c)
                            BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[offset+c])-expected,2);
                        BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[offset+3]),73);
                    }
                    BOOST_REQUIRE(f.device.Immediate_Command_List().Clear({0,1,0,1},1));
                    frame = f.Read();
                    BOOST_REQUIRE(frame.Is_Valid());
                    BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[1]),255);
                }
            }
        }
        f.rays.Shutdown();
        BOOST_REQUIRE(f.rays.Initialize(f.device,Shaders));
        f.Reset(41,29);
        f.Render(OrthographicInput());
        const auto frame = f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[0])-112,2);
    }
}

BOOST_AUTO_TEST_CASE(cloud_color_shadow_depth_and_cascade_selection)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        const std::array<std::byte,4> cloud_pixel{std::byte{64},std::byte{128},std::byte{255},std::byte{255}};
        const auto cloud = f.device.Create_Texture_Initialized({1,1,1,ColorFormat,
            static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)}, {cloud_pixel,4});
        BOOST_REQUIRE(cloud.Is_Valid());
        auto& env = Get_Environment_Lighting();
        env.cloud_texture = cloud;
        env.parameters.cloud_offset_strength = {0,0,1,1};
        auto input = OrthographicInput();
        for (const bool shadowed : {false,true}) {
            // A shadow plane at z=25 blocks the latter half of the 0..50 ray.
            const auto shadow = f.device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
                static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)
                    | static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
            BOOST_REQUIRE(shadow.Is_Valid());
            auto& commands = f.device.Immediate_Command_List();
            BOOST_REQUIRE(commands.Set_Depth_Target(shadow));
            BOOST_REQUIRE(commands.Clear_Depth(0.25f));
            env.shadow_textures[0] = shadow;
            env.parameters.shadow_options = {shadowed ? 1.0f : 0.0f,0.00001f,0,0};
            env.parameters.shadow_view_projection[0] = {1,0,0,0, 0,1,0,0, 0,0,0.01f,0, 0,0,0,1};
            env.parameters.shadow_view_depth = {0,0,1,0};
            env.parameters.shadow_splits[0] = 100;
            f.Reset(32,32);
            f.Render(input);
            const auto frame = f.Read();
            BOOST_REQUIRE(frame.Is_Valid());
            // Cloud RGB (64,128,255) times 0.375, halved by the shadow plane.
            const std::array<int,3> expected = shadowed ? std::array<int,3>{28,40,64} : std::array<int,3>{40,64,112};
            for (unsigned c=0;c<3;++c)
                BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[16*frame.row_pitch+16*4+c])-expected[c],2);
            env.parameters.shadow_options[0] = 0;
            env.shadow_textures = {};
            commands.Reset_State();
            f.device.Destroy_Texture(shadow);
        }
        // Both cascades must participate: the near half is lit, the far half dark.
        std::array<RHITextureHandle,2> shadows{};
        for (unsigned i=0;i<2;++i) {
            shadows[i] = f.device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
                static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)
                    | static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
            BOOST_REQUIRE(shadows[i].Is_Valid());
            BOOST_REQUIRE(f.device.Immediate_Command_List().Set_Depth_Target(shadows[i]));
            BOOST_REQUIRE(f.device.Immediate_Command_List().Clear_Depth(i == 0 ? 1.0f : 0.0f));
            env.shadow_textures[i] = shadows[i];
            env.parameters.shadow_view_projection[i] = {1,0,0,0, 0,1,0,0, 0,0,0.01f,0, 0,0,0,1};
        }
        env.parameters.shadow_options[0] = 2;
        env.parameters.shadow_splits = {25,100,0,0};
        f.Reset(32,32);
        f.Render(input);
        const auto frame = f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[16*frame.row_pitch+16*4+2])-64,2);
        env = {};
        f.device.Immediate_Command_List().Reset_State();
        for (const auto shadow : shadows) f.device.Destroy_Texture(shadow);
        f.device.Destroy_Texture(cloud);
    }
}

BOOST_AUTO_TEST_CASE(shroud_is_applied_at_full_resolution_and_does_not_change_scene_alpha)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        // A boundary inside a coarse ray pixel catches shroud leakage on upscale.
        constexpr unsigned size = 37;
        std::vector<std::byte> pixels(size*4);
        for (unsigned x=0;x<size;++x) for (unsigned c=0;c<4;++c)
            pixels[x*4+c] = std::byte(x >= 19 ? 255 : 0);
        const auto shroud = f.device.Create_Texture_Initialized({size,1,1,ColorFormat,
            static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)}, {pixels,size*4});
        BOOST_REQUIRE(shroud.Is_Valid());
        auto input = OrthographicInput();
        input.shroud_texture = shroud;
        f.Reset(size,29);
        f.Render(input);
        const auto frame = f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        for (unsigned x=0;x<size;++x) {
            const auto offset = 14*frame.row_pitch+x*4;
            BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[offset])-(x >= 19 ? 112 : 16),2);
            BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[offset+3]),73);
        }
        f.device.Immediate_Command_List().Reset_State();
        f.device.Destroy_Texture(shroud);
    }
}

BOOST_AUTO_TEST_CASE(perspective_divide_camera_translation_and_height_dependent_cloud_integral)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        auto input = OrthographicInput();
        // At the center: near z=100 and depth 0.5 -> z=200. Translation
        // changes the endpoints equally. These matrices use column vectors.
        for (const auto& inverse : {
            std::array<float,16>{1,0,0,0, 0,1,0,0, 0,0,0,100, 0,0,-1,1},
            std::array<float,16>{1,0,-10,10, 0,1,-20,20, 0,0,-30,130, 0,0,-1,1},
            std::array<float,16>{0,0,0,100, 1,0,0,0, 0,1,0,0, 0,0,-1,1}}) {
            input.inverse_view_projection = inverse;
            f.Reset(1,1);
            f.Render(input);
            const auto frame = f.Read();
            BOOST_REQUIRE(frame.Is_Valid());
            // 100 world units * 3/400 * 255 + 16 = 207.25.
            BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[0])-207,2);
        }

        // Two dark and two white texels. Height moves the ray through the
        // cloud's bilinear transition; sampling only its endpoint would be dark.
        const std::array<std::byte,16> pixels{
            std::byte{0},std::byte{0},std::byte{0},std::byte{255},
            std::byte{0},std::byte{0},std::byte{0},std::byte{255},
            std::byte{255},std::byte{255},std::byte{255},std::byte{255},
            std::byte{255},std::byte{255},std::byte{255},std::byte{255}};
        const auto cloud = f.device.Create_Texture_Initialized({4,1,1,ColorFormat,
            static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)}, {pixels,16});
        BOOST_REQUIRE(cloud.Is_Valid());
        auto& env = Get_Environment_Lighting();
        env.cloud_texture = cloud;
        env.parameters.cloud_offset_strength = {0.75f,0,1,1};
        input = OrthographicInput();
        f.Reset(1,1);
        f.Render(input);
        auto frame = f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[0])-112,2);
        env.parameters.cloud_multiplier[2] = 0.01f;
        f.Reset(1,1);
        f.Render(input);
        frame = f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        // 100 endpoint steps from U=.745 to .25 integrate to 0.495 visibility.
        // 16 + .495 * 95.625 = 63.334375.
        BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[0])-63,2);
        env = {};
        f.device.Immediate_Command_List().Reset_State();
        f.device.Destroy_Texture(cloud);
    }
}

BOOST_AUTO_TEST_CASE(distant_rays_remain_bounded_and_preserve_sunlight_tint)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        for (const float extent : {800.0f,8000.0f,60000.0f}) {
            auto input = OrthographicInput();
            input.inverse_view_projection[10] = extent;
            // Runtime-strength warm sunlight: at most 4.8%, 2.4%, 1.2% added RGB.
            input.brightness = LightRaysInput{}.brightness;
            input.brightness[1] *= 0.5f;
            input.brightness[2] *= 0.25f;
            f.Reset(37,29);
            f.Render(input);
            const auto frame = f.Read();
            BOOST_REQUIRE(frame.Is_Valid());
            const std::array<int,3> expected{28,22,19};
            for (unsigned y=0;y<frame.height;++y) for (unsigned x=0;x<frame.width;++x) {
                const auto offset = y*frame.row_pitch+x*4;
                for (unsigned c=0;c<3;++c)
                    BOOST_CHECK_SMALL(std::to_integer<int>(frame.pixels[offset+c])-expected[c],1);
                BOOST_CHECK_EQUAL(std::to_integer<int>(frame.pixels[offset+3]),73);
            }
        }
    }
}
