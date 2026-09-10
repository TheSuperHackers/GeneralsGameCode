module;
#define BOOST_TEST_MODULE TextureMappingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <stdexcept>
#include <variant>
export module Graphics.Materials.TextureMapping.Tests;
import Assets.Math;
import Assets.Materials.TextureMapping;
import Assets.Adapters.W3D.TextureMapping;
import Graphics.Materials.TextureCoordinates;
import Graphics.Materials.TextureProjection;
import Graphics.Materials.TextureMapping;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Geometry;
import Graphics.Tests.Device;
import Graphics.RHI;
using namespace Graphics;
using namespace Assets;
namespace {
void Near(float actual,float expected) { BOOST_CHECK_SMALL(actual-expected,0.00001f); }
unsigned samples=0;
float Sample() { ++samples; return 0.25f; }
float Sine(float angle) { return std::sin(angle); }
float Cosine(float angle) { return std::cos(angle); }
constexpr std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
}

BOOST_AUTO_TEST_CASE(authored_mapping_flags_and_argument_semantics)
{
    const auto decoded=W3D::W3DRead_Texture_Mapping(0x00040000,0,
        "UScale=50%\nUScale=9\nuscale=8\nVScale=2\nUPerSec=-25%\nVOffset=.75;comment\nClampFix=Yes\n");
    BOOST_REQUIRE(decoded);
    const auto* scroll=std::get_if<TextureScrollMapping>(&*decoded);
    BOOST_REQUIRE(scroll);
    Near(scroll->scale.x,.5f); Near(scroll->scale.y,2); Near(scroll->rate_per_second.x,-.25f);
    Near(scroll->start_offset.y,.75f); BOOST_CHECK(scroll->clamp); BOOST_CHECK(!scroll->screen_projection);
    const auto grid=W3D::W3DRead_Texture_Mapping(0x00000700,1,"Log2Width=$2\nLast=Ah\nOffset=3\nFPS=-4");
    BOOST_REQUIRE(grid);
    const auto& g=std::get<TextureGridMapping>(*grid);
    BOOST_CHECK_EQUAL(g.width_log2,2); BOOST_CHECK_EQUAL(g.last_frame,10); BOOST_CHECK_EQUAL(g.start_frame,3);
    Near(g.frames_per_second,-4);
    BOOST_CHECK(!W3D::W3DRead_Texture_Mapping(0,0,""));
    BOOST_CHECK(!W3D::W3DRead_Texture_Mapping(0x00050000,0,""));
    BOOST_CHECK(!W3D::W3DRead_Texture_Mapping(0x00150000,0,""));
    BOOST_CHECK(!W3D::W3DRead_Texture_Mapping(0x00040000,2,""));
    for (unsigned code=1;code<=20;++code) {
        if (code==5) continue;
        const auto description=W3D::W3DRead_Texture_Mapping(code<<16,0,"");
        BOOST_REQUIRE(description);
        const auto mapping=TextureMapping::Create(*description,0,Sample,Sine,Cosine);
        const auto result=mapping->Evaluate(100);
        for (const auto value:result.transform) BOOST_CHECK(std::isfinite(value));
        const bool normal=code==1 || code==2 || code==12 || code==13 || code==14 || code==15 || code==17 || code==19 || code==20;
        BOOST_CHECK_EQUAL(mapping->Needs_Normals(),normal);
        BOOST_CHECK_EQUAL(mapping->Is_Time_Variant(),code!=1 && code!=2 && code!=6 && code!=12 && code!=13);
    }
}

BOOST_AUTO_TEST_CASE(scroll_clock_wrap_copy_reset_and_external_override)
{
    TextureScrollMapping description{{2,3},{.5f,-.5f},{.75f,.25f}};
    auto mapping=TextureMapping::Create(description,0xfffffff0u);
    auto result=mapping->Evaluate(984); // 1000 milliseconds across uint32 wrap.
    Near(result.transform[3],.25f); Near(result.transform[7],.75f);
    BOOST_CHECK(mapping->Evaluate(984).transform==result.transform);
    auto clone=mapping->Clone(984);
    Near(clone->Evaluate(984).transform[3],.75f);
    mapping->Reset(984);
    Near(mapping->Evaluate(984).transform[3],0);
    auto* linear=mapping->Linear_Scroll(); BOOST_REQUIRE(linear);
    linear->rate_per_millisecond={}; linear->offset={.625f,.375f}; linear->last_time=2000;
    Near(mapping->Evaluate(2000).transform[3],.625f);
    Near(clone->Evaluate(1984).transform[3],.25f);
    std::weak_ptr<TextureMapping> weak=mapping;
    auto retained=mapping; mapping.reset(); BOOST_CHECK(!weak.expired());
    retained.reset(); BOOST_CHECK(weak.expired());
}

BOOST_AUTO_TEST_CASE(scroll_clamps_and_screen_coordinates_keep_homogeneous_translation)
{
    TextureScrollMapping description{{2,3},{20,-20},{},true};
    auto clamped=TextureMapping::Create(description,0);
    auto result=clamped->Evaluate(1000);
    Near(result.transform[3],-2); Near(result.transform[7],3);
    description={{2,3},{},{.25f,.5f},false,true};
    auto screen=TextureMapping::Create(description,0);
    BOOST_CHECK(!screen->Linear_Scroll());
    const std::array<float,16> projection{2,0,0,4, 0,5,0,6, 0,0,1,0, 0,0,2,1};
    result=screen->Evaluate(0,identity,projection);
    Near(result.transform[0],4); Near(result.transform[2],.5f); Near(result.transform[3],8.25f);
    Near(result.transform[5],15); Near(result.transform[6],1); Near(result.transform[7],18.5f);
    BOOST_CHECK(result.coordinates.source==TextureCoordinateSource::CameraPosition);
    BOOST_CHECK(result.coordinates.projected);
}

BOOST_AUTO_TEST_CASE(grid_offsets_retain_frame_remainders_and_backward_unsigned_modulus)
{
    TextureGridMapping description{4,2,10,0};
    auto forward=TextureMapping::Create(description,0);
    auto result=forward->Evaluate(750);
    Near(result.transform[0],1); Near(result.transform[3],.75f); Near(result.transform[7],0);
    result=forward->Evaluate(1000);
    Near(result.transform[3],0); Near(result.transform[7],.25f);
    description.frames_per_second=-4;
    auto backward=TextureMapping::Create(description,0);
    result=backward->Evaluate(2500); // 9 - 10 -> UINT_MAX % 10 == 5.
    Near(result.transform[3],.25f); Near(result.transform[7],.25f);
    backward->Reset(2500);
    result=backward->Evaluate(2500);
    Near(result.transform[3],.25f); Near(result.transform[7],.5f);
    description.frames_per_second=0;
    auto frozen=TextureMapping::Create(description,0);
    Near(frozen->Evaluate(99999).transform[3],0);
    description.frames_per_second=2000;
    BOOST_CHECK_THROW(TextureMapping::Create(description,0),std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rotation_sine_step_and_zigzag_preserve_authored_units)
{
    auto rotate=TextureMapping::Create(TextureRotateMapping{{2,3},{.25f,.5f},.25f},0);
    auto result=rotate->Evaluate(1000);
    Near(result.transform[0],0); Near(result.transform[1],-2); Near(result.transform[3],1.5f);
    Near(result.transform[4],3); Near(result.transform[5],0); Near(result.transform[7],.75f);
    Near(rotate->Clone(1000)->Evaluate(1000).transform[0],2);
    auto sine=TextureMapping::Create(TextureSineMapping{{2,3},{2,1,0},{3,2,.5f}},0);
    result=sine->Evaluate(250);
    Near(result.transform[3],2); Near(result.transform[7],-3);
    auto step=TextureMapping::Create(TextureStepMapping{{1,1},{.25f,-.25f},2,false},0);
    Near(step->Evaluate(499).transform[3],0);
    result=step->Evaluate(500); Near(result.transform[3],.25f); Near(result.transform[7],.75f);
    result=step->Evaluate(1500); Near(result.transform[3],.75f); Near(result.transform[7],.25f);
    auto zigzag=TextureMapping::Create(TextureZigZagMapping{{1,1},{2,-4},2},0);
    result=zigzag->Evaluate(1000); Near(result.transform[3],2); Near(result.transform[7],-4);
    result=zigzag->Evaluate(1500); Near(result.transform[3],1); Near(result.transform[7],-2);
    result=zigzag->Evaluate(4500); Near(result.transform[3],1); Near(result.transform[7],-2);
}

BOOST_AUTO_TEST_CASE(environment_axes_use_inverse_view_rotation_and_ignore_translation)
{
    const std::array<float,16> view{0,-1,0,11, 1,0,0,22, 0,0,1,33, 0,0,0,1};
    const std::array<std::array<float,8>,3> expected{{
        {-.5f,0,0,.5f,0,0,.5f,.5f},
        {0,.5f,0,.5f,0,0,.5f,.5f},
        {0,.5f,0,.5f,-.5f,0,0,.5f}}};
    for (unsigned axis=0;axis<3;++axis) {
        const TextureEnvironmentMapping description{TextureEnvironmentSource::Reflection,true,static_cast<TextureMappingAxis>(axis)};
        auto mapping=TextureMapping::Create(description,0);
        const auto result=mapping->Evaluate(123,view);
        for (unsigned i=0;i<8;++i) Near(result.transform[i],expected[axis][i]);
        BOOST_CHECK(result.coordinates.source==TextureCoordinateSource::CameraReflection);
    }
    TextureGridMapping grid{1,1,4,0,{TextureEnvironmentSource::Normal,true,TextureMappingAxis::Z}};
    const auto result=TextureMapping::Create(grid,0)->Evaluate(1000,view);
    Near(result.transform[1],.25f); Near(result.transform[4],-.25f);
    Near(result.transform[3],.75f); Near(result.transform[7],.25f);
}

BOOST_AUTO_TEST_CASE(edge_copy_keeps_current_offset_but_reset_clears_it)
{
    auto edge=TextureMapping::Create(TextureEdgeMapping{.5f,.25f,true},0);
    auto result=edge->Evaluate(1000); Near(result.transform[7],.75f);
    auto clone=edge->Clone(1000);
    Near(clone->Evaluate(1000).transform[7],.75f);
    edge->Reset(1000); Near(edge->Evaluate(1000).transform[7],0);
    BOOST_CHECK(result.coordinates.source==TextureCoordinateSource::CameraReflection);
    Near(result.transform[2],.5f); Near(result.transform[3],.5f);
}

BOOST_AUTO_TEST_CASE(random_sampling_count_copy_and_reset_keep_stream_order)
{
    samples=0;
    auto mapping=TextureMapping::Create(TextureRandomMapping{{2,3},{-1,1},2},0,Sample);
    BOOST_CHECK_EQUAL(samples,3);
    auto result=mapping->Evaluate(250); BOOST_CHECK_EQUAL(samples,3);
    Near(result.transform[0],0); Near(result.transform[1],-3); Near(result.transform[4],2);
    Near(result.transform[3],0); Near(result.transform[7],.5f);
    result=mapping->Evaluate(1750); BOOST_CHECK_EQUAL(samples,6); // Three elapsed frames, one sample triplet.
    mapping->Evaluate(1750); BOOST_CHECK_EQUAL(samples,6);
    auto clone=mapping->Clone(1750); BOOST_CHECK_EQUAL(samples,9);
    mapping->Reset(1750); BOOST_CHECK_EQUAL(samples,9);
    Near(mapping->Evaluate(1750).transform[3],.25f);
    auto continuous=TextureMapping::Create(TextureRandomMapping{{1,1},{-1,0},0},0,Sample);
    Near(continuous->Evaluate(500).transform[3],-.25f); // fmod keeps the negative sign.
}

BOOST_AUTO_TEST_CASE(bump_reset_keeps_its_rotation_clock_while_copy_restarts_it)
{
    auto mapping=TextureMapping::Create(TextureBumpMapping{{{1,1},{},{.25f,.5f}},.25f,2},0,nullptr,Sine,Cosine);
    auto result=mapping->Evaluate(1000); BOOST_REQUIRE(result.bump);
    Near((*result.bump)[0],0); Near((*result.bump)[1],-2); Near((*result.bump)[2],2);
    mapping->Reset(1000);
    result=mapping->Evaluate(2000);
    Near(result.transform[3],0); Near((*result.bump)[0],-2);
    auto clone=mapping->Clone(2000); result=clone->Evaluate(2000);
    Near(result.transform[3],.25f); Near((*result.bump)[0],2);
}

BOOST_AUTO_TEST_CASE(projector_padding_depth_normal_and_homogeneous_coordinates)
{
    auto mapping=TextureMapping::Create_Projection();
    auto* projection=mapping->Projection(); BOOST_REQUIRE(projection);
    const std::array<float,16> source{2,0,0,1,0,4,0,-1,0,0,2,0,0,0,1,1};
    projection->Set_Texture_Transform(source,4);
    auto result=mapping->Evaluate(0);
    Near(result.transform[0],.5f); Near(result.transform[2],.25f); Near(result.transform[3],.5f);
    Near(result.transform[5],-1); Near(result.transform[6],.25f); Near(result.transform[7],.5f);
    const auto coordinate=projection->Compute_Texture_Coordinate({2,3,4});
    Near(coordinate[0],2.5f); Near(coordinate[1],-1.5f); Near(coordinate[2],5);
    projection->type=TextureProjection::Perspective;
    BOOST_CHECK(mapping->Evaluate(0).coordinates.projected);
    projection->type=TextureProjection::DepthGradient;
    projection->invert_depth=true; projection->Set_Texture_Transform(source,4);
    result=mapping->Evaluate(0); Near(result.transform[3],.5f); Near(result.transform[6],-.5f); Near(result.transform[7],.5f);
    projection->type=TextureProjection::NormalGradient; projection->gradient_u=.25f;
    result=mapping->Evaluate(0); Near(result.transform[3],.25f); Near(result.transform[6],-1);
    BOOST_CHECK(result.coordinates.source==TextureCoordinateSource::CameraNormal);
    BOOST_CHECK(!mapping->Needs_Normals()); BOOST_CHECK(!mapping->Is_Time_Variant());
    BOOST_CHECK(projection->Get_Texture_Transform()==source);
}

BOOST_AUTO_TEST_CASE(mapped_materials_draw_authored_texels_and_alpha_on_both_triangles)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<std::uint8_t,64> texels{};
    for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
        const unsigned i=(y*4+x)*4;
        texels[i]=static_cast<std::uint8_t>(32+48*x); texels[i+1]=static_cast<std::uint8_t>(32+48*y);
        texels[i+2]=96; texels[i+3]=static_cast<std::uint8_t>(32+16*(y*4+x));
    }
    const auto texture=device.Create_Texture_Initialized({4,4},{std::as_bytes(std::span(texels)),16});
    BOOST_REQUIRE(texture.Is_Valid());
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,.5f}; vertices[1].position={1,-1,.5f};
    vertices[2].position={1,1,.5f}; vertices[3].position={-1,1,.5f};
    for (auto& vertex:vertices) { vertex.uv={.125f,.125f}; vertex.normal={0,0,1}; vertex.color={1,1,1,1}; }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth)); BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    PropStyle style; style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    style.samplers[0].Set_Filter(RHISamplerFilter::Point);
    struct Case { std::shared_ptr<TextureMapping> mapping; unsigned time,x,y; };
    auto projected=TextureMapping::Create_Projection();
    projected->Projection()->type=TextureProjection::Perspective;
    projected->Projection()->Set_Texture_Transform({0,0,0,1,0,0,0,-1,0,0,1,0,0,0,0,2},4);
    const Case cases[]{
        {TextureMapping::Create(TextureScaleMapping{},0),0,0,0},
        {TextureMapping::Create(TextureScrollMapping{{1,1},{-1,0}},0),500,2,0},
        {TextureMapping::Create(TextureGridMapping{1,1,4,0},0),2000,0,2},
        {TextureMapping::Create(TextureEnvironmentMapping{},0),0,2,2},
        {projected,0,1,1}};
    for (const auto& value:cases) {
        BOOST_REQUIRE(commands.Clear_Color_Target(target,{0,0,0,0}));
        PropParameters parameters; parameters.view_projection=identity;
        const auto mapping=value.mapping->Evaluate(value.time);
        parameters.uv_transform[0]=mapping.transform;
        parameters.uv_sources[0]=static_cast<float>(mapping.coordinates.source);
        parameters.uv_sources[1]=mapping.coordinates.projected ? 1.0f : 0.0f;
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
        std::array<std::byte,256> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32));
        const std::array<int,4> expected{static_cast<int>(32+48*value.x),static_cast<int>(32+48*value.y),96,
            static_cast<int>(32+16*(4*value.y+value.x))};
        for (unsigned pixel:{2u*8+2,5u*8+5}) for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[pixel*4+channel])-expected[channel],2);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
