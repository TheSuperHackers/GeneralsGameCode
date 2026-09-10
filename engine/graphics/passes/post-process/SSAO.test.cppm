module;
#define BOOST_TEST_MODULE SSAOTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Passes.SSAO.Tests;
import Graphics.Passes.SSAO;
import Graphics.RHI;
import Graphics.FrameTargets;
import Graphics.Tests.Device;
import Graphics.Capture.FrameCapture;
import Graphics.Shaders.Library;
using namespace Graphics;
namespace
{
const std::filesystem::path Shaders(Graphics::Test_Shader_Directory(GRAPHICS_SSAO_SHADER_DIRECTORY));
SSAOInput Camera(bool perspective)
{
    SSAOInput input;
    if (perspective) {
        // 90 degree FOV, near=1, far=101, -Z forward.
        input.projection = {1,0,0,0, 0,1,0,0, 0,0,-1.01f,-1.01f, 0,0,-1,0};
        input.inverse_projection = {1,0,0,0, 0,1,0,0, 0,0,0,-1, 0,0,-100.0f/101.0f,1};
    } else {
        // 100 by 100 orthographic view, near=1, far=101.
        input.projection = {0.02f,0,0,0, 0,0.02f,0,0, 0,0,-0.01f,-0.01f, 0,0,0,1};
        input.inverse_projection = {50,0,0,0, 0,50,0,0, 0,0,-100,-1, 0,0,0,1};
    }
    input.bias = 0.1f;
    input.strength = 1;
    return input;
}
float DepthAt(float z, bool perspective)
{
    return perspective ? 1.01f+1.01f/z : (-z-1.0f)/100.0f;
}
struct Vertex { std::array<float,3> position; std::array<float,4> color{1,1,1,1}; std::array<float,2> uv{}; };
struct Fixture final
{
    GraphicsTestDevice device;
    SSAORenderer ssao;
    FrameCapture capture;
    FrameTargets targets{};
    RHIPipelineHandle pipeline{};
    RHIBufferHandle vertices{}, constants{};
    RHITextureFormat color_format = RHITextureFormat::RGBA8_UNorm;
    RHITextureFormat depth_format = RHITextureFormat::D24_UNorm_S8;
    explicit Fixture(bool warp) : device({warp})
    {
        BOOST_REQUIRE(device.Is_Valid());
        BOOST_REQUIRE(ssao.Initialize(device,Shaders));
        ShaderLibrary shaders;
        ShaderPrecompiledDesc desc;
        desc.program.vertex_shader = desc.program.fragment_shader = 501;
        desc.program.source_key = 0x5353414F54455354ull;
        desc.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
        desc.vertex_path = Shaders/"fullscreen_overlay.vso";
        desc.fragment_path = Shaders/"fullscreen_overlay.pso";
        const auto shader = shaders.Load_Precompiled(desc);
        BOOST_REQUIRE(shaders.Is_Loaded(shader));
        RHIPipeline state;
        state.depth_test = state.depth_write = true;
        state.cull_mode = RHICullMode::None;
        state.color_write_mask = 0;
        pipeline = device.Create_Pipeline(state,
            {shaders.Bytecode(shader,ShaderStage::Vertex)},{shaders.Bytecode(shader,ShaderStage::Pixel)});
        vertices = device.Create_Buffer({6*sizeof(Vertex),RHIBufferUsage::Vertex,sizeof(Vertex)});
        const std::array<float,16> data{1,1,1,1};
        constants = device.Create_Buffer_Initialized({sizeof(data),RHIBufferUsage::Constant},std::as_bytes(std::span(data)));
        BOOST_REQUIRE(pipeline.Is_Valid()); BOOST_REQUIRE(vertices.Is_Valid()); BOOST_REQUIRE(constants.Is_Valid());
    }
    ~Fixture()
    {
        ssao.Shutdown();
        Release();
        device.Destroy_Pipeline(pipeline);
        device.Destroy_Buffer(vertices);
        device.Destroy_Buffer(constants);
    }
    void Release()
    {
        device.Immediate_Command_List().Reset_State();
        if (targets.backbuffer.texture.Is_Valid()) device.Destroy_Texture(targets.backbuffer.texture);
        if (targets.depth.texture.Is_Valid()) device.Destroy_Texture(targets.depth.texture);
        targets = {};
    }
    void Reset(unsigned width, unsigned height)
    {
        Release();
        std::vector<std::byte> pixels(width*height*4);
        for (unsigned y=0;y<height;++y) for (unsigned x=0;x<width;++x) {
            const unsigned offset=(y*width+x)*4;
            pixels[offset]=std::byte{200}; pixels[offset+1]=std::byte{120}; pixels[offset+2]=std::byte{60};
            pixels[offset+3]=std::byte(x<width/2 ? 73 : 173);
        }
        const auto color = device.Create_Texture_Initialized({width,height,1,color_format,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)},{pixels,width*4});
        const auto depth = device.Create_Texture({width,height,1,depth_format,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(color.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
        targets={{color,width,height},{depth,width,height}};
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(color,depth));
        BOOST_REQUIRE(commands.Clear_Depth(1));
    }
    void DepthRect(float left, float right, float bottom, float top, float left_depth, float right_depth)
    {
        // Real rasterized depth. Color writes are disabled to isolate occlusion.
        const std::array<Vertex,6> data{{
            {{left,top,left_depth}},{{right,top,right_depth}},{{left,bottom,left_depth}},
            {{left,bottom,left_depth}},{{right,top,right_depth}},{{right,bottom,right_depth}}}};
        BOOST_REQUIRE(device.Update_Buffer(vertices,0,std::as_bytes(std::span(data))));
        RHIBindlessResource binding;
        binding.type=RHIResourceType::Material; binding.buffer=constants;
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Reset_State());
        BOOST_REQUIRE(commands.Set_Render_Targets(targets.backbuffer.texture,targets.depth.texture));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,targets.backbuffer.width,targets.backbuffer.height}));
        BOOST_REQUIRE(commands.Bind_Pipeline(pipeline));
        BOOST_REQUIRE(commands.Set_Bindless_Resources(std::span(&binding,1)));
        BOOST_REQUIRE(commands.Set_Vertex_Buffer(0,vertices,sizeof(Vertex),0));
        BOOST_REQUIRE(commands.Draw(6,0,1,0));
    }
    void Render(const SSAOInput& input, bool enabled=true)
    {
        BOOST_REQUIRE(ssao.Render(device.Immediate_Command_List(),targets,color_format,depth_format,input,enabled));
    }
    auto Read() { return capture.Read(device,targets.backbuffer.texture,targets.backbuffer.width,targets.backbuffer.height,color_format); }
};
}

BOOST_AUTO_TEST_CASE(flat_sloped_and_clear_surfaces_disabled_resize_and_reinitialization)
{
    for (const bool warp : {true,false}) {
        Fixture f(warp);
        SSAORenderer uninitialized;
        BOOST_CHECK(uninitialized.Render(f.device.Immediate_Command_List(),{},f.color_format,f.depth_format,{},false));
        for (const bool perspective : {false,true}) for (const auto width : {32u,37u,1u}) {
            for (const auto depth_format : {RHITextureFormat::D24_UNorm_S8,RHITextureFormat::D32_Float}) {
                f.depth_format=depth_format;
                f.color_format=perspective ? RHITextureFormat::BGRA8_UNorm : RHITextureFormat::RGBA8_UNorm;
                f.ssao.Shutdown();
                BOOST_REQUIRE(f.ssao.Initialize(f.device,Shaders));
                for (unsigned surface=0;surface<3;++surface) {
                    f.Reset(width,width==1 ? 1 : width-3);
                    if (surface!=0) f.DepthRect(-1,1,-1,1,0.45f,surface==1 ? 0.45f : 0.65f);
                    const auto original=f.Read();
                    BOOST_REQUIRE(original.Is_Valid());
                    const std::vector<std::byte> expected(original.pixels.begin(),original.pixels.end());
                    f.Render(Camera(perspective),false);
                    auto frame=f.Read();
                    BOOST_CHECK(std::equal(expected.begin(),expected.end(),frame.pixels.begin(),frame.pixels.end()));
                    f.Render(Camera(perspective));
                    frame=f.Read();
                    BOOST_REQUIRE(frame.Is_Valid());
                    BOOST_CHECK(std::equal(expected.begin(),expected.end(),frame.pixels.begin(),frame.pixels.end()));
                    // The following UI pass must receive the restored attachments.
                    BOOST_REQUIRE(f.device.Immediate_Command_List().Clear({0,1,0,1},1));
                    frame=f.Read();
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(frame.pixels[1]),255u);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(nearby_geometry_occludes_but_foreground_sky_and_distant_geometry_do_not)
{
    for (const bool warp : {true,false}) for (const bool perspective : {false,true}) {
        Fixture f(warp);
        const auto input=Camera(perspective);
        const auto draw=[&](float obstacle_z,bool floor) {
            f.Reset(128,128);
            if (floor) f.DepthRect(-1,1,-1,1,DepthAt(-51,perspective),DepthAt(-51,perspective));
            f.DepthRect(-0.25f,0.25f,-0.25f,0.25f,DepthAt(obstacle_z,perspective),DepthAt(obstacle_z,perspective));
            f.Render(input);
        };
        draw(-47,true);
        auto frame=f.Read();
        BOOST_REQUIRE(frame.Is_Valid());
        const std::vector<std::byte> first(frame.pixels.begin(),frame.pixels.end());
        const auto pixel=[&](unsigned x,unsigned y,unsigned channel=0) {
            return std::to_integer<int>(frame.pixels[y*frame.row_pitch+x*4+channel]);
        };
        // Both sides of the raised square darken within the world-space radius.
        for (const unsigned edge : {42u,80u}) {
            unsigned sum=0;
            for (unsigned x=edge;x<edge+6;++x) for (unsigned y=56;y<72;++y) sum+=pixel(x,y);
            BOOST_CHECK_LT(sum,200u*96u);
            BOOST_CHECK_GT(sum,140u*96u);
        }
        BOOST_CHECK_EQUAL(pixel(12,64),200);
        BOOST_CHECK_EQUAL(pixel(64,64),200);
        BOOST_CHECK_EQUAL(pixel(63,64,3),73);
        BOOST_CHECK_EQUAL(pixel(64,64,3),173);
        // Filtering preserves the source hue, within byte rounding.
        BOOST_CHECK_SMALL(pixel(46,64)*120-pixel(46,64,1)*200,201);
        BOOST_CHECK_SMALL(pixel(46,64)*60-pixel(46,64,2)*200,201);
        draw(-47,true);
        frame=f.Read();
        BOOST_CHECK(std::equal(first.begin(),first.end(),frame.pixels.begin(),frame.pixels.end()));
        for (const bool floor : {false,true}) {
            draw(-20,floor); // 31 units of separation exceeds the 12-unit AO radius.
            frame=f.Read();
            for (unsigned x=0;x<128;++x) BOOST_CHECK_EQUAL(pixel(x,64),200);
        }
    }
}

BOOST_AUTO_TEST_CASE(invalid_parameters_are_rejected_before_drawing)
{
    BOOST_CHECK(!SSAOInput{}.Is_Valid());
    auto input=Camera(true);
    BOOST_CHECK(input.Is_Valid());
    input.radius=std::numeric_limits<float>::quiet_NaN();
    BOOST_CHECK(!input.Is_Valid());
    input=Camera(false); input.strength=2;
    BOOST_CHECK(!input.Is_Valid());
    input=Camera(false); input.inverse_projection[0]=0;
    BOOST_CHECK(!input.Is_Valid());
    Fixture f(true);
    f.Reset(16,16);
    BOOST_CHECK(!f.ssao.Render(f.device.Immediate_Command_List(),f.targets,f.color_format,f.depth_format,input,true));
    auto targets=f.targets; targets.depth.width=15;
    BOOST_CHECK(!f.ssao.Render(f.device.Immediate_Command_List(),targets,f.color_format,f.depth_format,Camera(false),true));
    input=Camera(false); input.strength=0;
    f.Render(input);
    const auto frame=f.Read();
    BOOST_REQUIRE(frame.Is_Valid());
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(frame.pixels[0]),200u);
}
