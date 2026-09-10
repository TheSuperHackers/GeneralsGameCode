module;
#define BOOST_TEST_MODULE PrimitiveGeometryTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <span>
export module Graphics.Scene.Primitives.Geometry.Tests;
import Graphics.Scene.Primitives.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(annulus_retains_orientation_strip_order_scaling_and_uv_seam)
{
    AnnulusGeometry geometry;
    BOOST_REQUIRE(geometry.Generate(4));
    BOOST_REQUIRE(geometry.Scale({0.5f,1},{2,3}));
    BOOST_REQUIRE(geometry.Set_Tiling(3));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),10u);
    BOOST_CHECK_EQUAL(geometry.Triangle_Count(),8u);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].position[1],1);
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].position[1],3);
    BOOST_CHECK_SMALL(geometry.Vertices()[2].position[0]+0.5f,1e-6f);
    BOOST_CHECK_SMALL(geometry.Vertices()[3].position[0]+2,1e-6f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[2].uv[0],0.75f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[8].uv[0],3);
    BOOST_CHECK_EQUAL(geometry.Vertices()[8].uv[1],0);
    BOOST_CHECK_EQUAL(geometry.Vertices()[9].uv[1],1);
    for (unsigned triangle=0;triangle<8;++triangle)
        for (unsigned corner=0;corner<3;++corner)
            BOOST_CHECK_EQUAL(geometry.Indices()[triangle*3+corner],triangle+corner);
    geometry.Set_Color({0.25f,0.5f,0.75f,0.125f});
    BOOST_CHECK_EQUAL(geometry.Vertices()[9].color[3],0.125f);
    BOOST_REQUIRE(geometry.Generate(8));
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[3],1);
    BOOST_CHECK_EQUAL(geometry.Vertices().back().uv[0],3);
    BOOST_CHECK(!geometry.Generate(2));
    BOOST_CHECK(!geometry.Scale({std::numeric_limits<float>::infinity(),1},{1,1}));
    BOOST_CHECK_EQUAL(geometry.Vertices().size(),18u);
}

BOOST_AUTO_TEST_CASE(sphere_retains_poles_uvs_triangle_order_and_directional_opacity)
{
    SphereGeometry geometry;
    BOOST_REQUIRE(geometry.Generate(2,4,1));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),7u);
    const std::array<std::array<float,3>,7> positions{{{0,0,2},{0,-2,0},{2,0,0},
        {0,2,0},{-2,0,0},{0,-2,0},{0,0,-2}}};
    for (unsigned vertex=0;vertex<positions.size();++vertex)
        for (unsigned axis=0;axis<3;++axis)
            BOOST_CHECK_SMALL(geometry.Vertices()[vertex].position[axis]-positions[vertex][axis],1e-5f);
    const std::array<std::uint32_t,24> indices{0,2,1,0,3,2,0,4,3,0,5,4,
        6,4,5,6,3,4,6,2,3,6,1,2};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(),geometry.Indices().end(),indices.begin(),indices.end());
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].uv[0],0);
    BOOST_CHECK_EQUAL(geometry.Vertices()[5].uv[0],1);
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].uv[1],0.5f);
    BOOST_REQUIRE(geometry.Set_Directional_Opacity({1,0,0},1,false,false));
    BOOST_CHECK_SMALL(geometry.Vertices()[2].color[3],1e-6f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[3],1);
    BOOST_REQUIRE(geometry.Set_Directional_Opacity({1,0,0},1,true,true));
    BOOST_CHECK_EQUAL(geometry.Vertices()[2].color[0],1);
    BOOST_CHECK_EQUAL(geometry.Vertices()[2].color[3],0);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[0],0);
    BOOST_REQUIRE(geometry.Generate(1,4,2));
    const std::array<std::uint32_t,6> first{6,1,7,1,2,7};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(),geometry.Indices().begin()+6,first.begin(),first.end());
    BOOST_REQUIRE(geometry.Set_Directional_Opacity({1,0,0},1,true,true));
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[0],0);
}

BOOST_AUTO_TEST_CASE(sphere_uses_full_width_indices_and_rejects_invalid_dimensions)
{
    SphereGeometry geometry;
    BOOST_REQUIRE(geometry.Generate(1,257,256));
    BOOST_CHECK_GT(geometry.Vertices().size(),65536u);
    BOOST_CHECK_EQUAL(*std::max_element(geometry.Indices().begin(),geometry.Indices().end()),geometry.Vertices().size()-1);
    const auto count=geometry.Vertices().size();
    BOOST_CHECK(!geometry.Generate(0,4,4));
    BOOST_CHECK(!geometry.Generate(1,4,0));
    BOOST_CHECK(!geometry.Generate(1,(std::numeric_limits<std::uint32_t>::max)(),2));
    BOOST_CHECK_EQUAL(geometry.Vertices().size(),count);
    BOOST_CHECK(!geometry.Set_Directional_Opacity({1,0,0},-1,false,false));
}

BOOST_AUTO_TEST_CASE(primitives_draw_uvs_holes_and_animated_opacity_after_recreation)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,8> texels{255,0,0,255,0,255,0,255};
    const auto texture=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    AnnulusGeometry annulus;
    BOOST_REQUIRE(annulus.Generate(64));
    BOOST_REQUIRE(annulus.Scale({0.25f,0.5f},{0.9f,0.75f}));
    BOOST_REQUIRE(annulus.Set_Tiling(1));
    const auto ring=renderer.Create_Mesh(annulus.Vertices(),annulus.Indices());
    SphereGeometry sphere;
    BOOST_REQUIRE(sphere.Generate(0.9f,32,32));
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,0.25f,0.5f,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.cull=RHICullMode::None;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    auto& commands=device.Immediate_Command_List();
    std::array<std::byte,32*32*4> pixels{};
    const auto pixel=[&](unsigned x,unsigned channel) { return std::to_integer<int>(pixels[(16*32+x)*4+channel]); };
    for (unsigned pass=0;pass<2;++pass) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        parameters.textured=1;
        BOOST_REQUIRE(renderer.Draw(commands,ring,style,parameters,std::span(&texture,1)));
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        BOOST_CHECK_EQUAL(pixel(4,0),255); BOOST_CHECK_EQUAL(pixel(4,1),0);
        BOOST_CHECK_EQUAL(pixel(27,0),0); BOOST_CHECK_EQUAL(pixel(27,1),255);
        BOOST_CHECK_EQUAL(pixel(16,3),0);
        BOOST_REQUIRE(sphere.Set_Directional_Opacity({0,0,1},1,pass!=0,false));
        const auto mesh=renderer.Create_Mesh(sphere.Vertices(),sphere.Indices());
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        parameters.textured=0;
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        if (pass==0) BOOST_CHECK_LT(pixel(16,3),16);
        else BOOST_CHECK_GT(pixel(16,3),240);
        BOOST_CHECK_GT(pixel(28,3),40); BOOST_CHECK_LT(pixel(28,3),215);
        renderer.Destroy_Mesh(mesh);
        if (pass==0) { renderer.Shutdown(); BOOST_REQUIRE(renderer.Initialize(device,shaders)); }
    }
    renderer.Destroy_Mesh(ring); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
