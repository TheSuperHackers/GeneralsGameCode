module;
#define BOOST_TEST_MODULE TrackDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Tracks.Drawing.Tests;
import Graphics.Scene.Tracks.Geometry;
import Graphics.Scene.Surfaces.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(strip_seams_distance_fade_and_zero_alpha_anchors)
{
    std::array<TrackEdge,4> edges{};
    for (unsigned i=0;i<4;++i) {
        edges[i].positions = {{{-1,float(i),0.5f},{1,float(i),0.5f}}};
        edges[i].uv = {{{0,float(i)},{1,float(i)}}};
    }
    edges[3].alpha = 0;
    TrackGeometry geometry;
    geometry.Build(edges,4,2,{1,1,1});
    BOOST_REQUIRE_EQUAL(geometry.vertices.size(),8);
    BOOST_REQUIRE_EQUAL(geometry.indices.size(),18);
    BOOST_CHECK_EQUAL(geometry.vertices[0].color[3],0);
    BOOST_CHECK_CLOSE(geometry.vertices[2].color[3],127.0f/255,0.001f);
    BOOST_CHECK_EQUAL(geometry.vertices[4].color[3],1);
    BOOST_CHECK_EQUAL(geometry.vertices[6].color[3],0);
    const std::array<std::uint32_t,12> expected{0,1,3,0,3,2,2,3,5,2,5,4};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.indices.begin(),geometry.indices.begin()+12,expected.begin(),expected.end());
    geometry.Build(std::span(edges).first(1),4,2,{1,1,1});
    BOOST_CHECK(geometry.vertices.empty() && geometry.indices.empty());
}
BOOST_AUTO_TEST_CASE(track_texture_and_vertex_fade_blend_without_writing_depth)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<TrackEdge,2> edges{};
    edges[0].positions = {{{-1,-1,0.5f},{1,-1,0.5f}}};
    edges[1].positions = {{{-1,1,0.5f},{1,1,0.5f}}};
    edges[0].alpha = edges[1].alpha = 0.5f;
    TrackGeometry geometry;
    geometry.Build(edges,8,4,{1,1,1});
    const auto mesh = renderer.Create_Mesh(geometry.vertices,geometry.indices);
    const std::array<std::uint8_t,4> texel{0,0,0,128};
    const auto texture = device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(texel)),4});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({1,1,1,0.75f},1));
    SurfaceParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    SurfaceStyle style;
    style.cull = RHICullMode::Back;
    style.front_counter_clockwise = true;
    const std::array<RHITextureHandle,4> textures{texture,{},{},{}};
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    for (int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-191,2);
    for (auto& edge : edges) for (auto& position : edge.positions) position[2] = 0.75f;
    edges[0].alpha = edges[1].alpha = 1;
    geometry.Build(edges,8,4,{1,0,0});
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,geometry.vertices,geometry.indices));
    parameters.textured = 0;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4]),255);
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4+1]),0);
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (auto handle : {texture,target,depth}) device.Destroy_Texture(handle);
}
