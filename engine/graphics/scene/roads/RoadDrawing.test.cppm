module;
#define BOOST_TEST_MODULE RoadDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Scene.Roads.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.Surfaces.Geometry;
import Assets.Math;
import Graphics.Scene.Surfaces.Renderer;
import Graphics.Scene.Roads.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(road_cloud_and_masked_lightmap_preserve_transparent_edges_and_destination_alpha)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t, 4> color) {
        return device.Create_Texture_Initialized({1, 1}, {std::as_bytes(std::span(color)), 4});
    };
    const auto road = texture({255, 0, 0, 128});
    const auto transparent = texture({255, 0, 0, 0});
    const auto cloud = texture({128, 128, 128, 255});
    const auto noise = texture({128, 128, 128, 255});
    const auto target = device.Create_Texture({16, 16, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16, 16, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(road.Is_Valid() && transparent.Is_Valid() && cloud.Is_Valid() && noise.Is_Valid()
        && target.Is_Valid() && depth.Is_Valid());
    std::array<SurfaceVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0.5f};
    vertices[1].position = {1, -1, 0.5f};
    vertices[2].position = {1, 1, 0.5f};
    vertices[3].position = {-1, 1, 0.5f};
    const std::array<std::uint32_t, 6> indices{0, 2, 3, 0, 1, 2};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    SurfaceParameters parameters;
    parameters.view_projection = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    parameters.cloud = 1;
    SurfaceStyle alpha;
    SurfaceStyle multiply;
    multiply.blend = RHIBlendMode::Multiply;
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 16, 16}));
    const auto check_pixel = [&](std::array<int, 4> expected) {
        std::array<std::byte, 16 * 16 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 16 * 4));
        for (std::size_t channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8 * 16 + 8) * 4 + channel]) - expected[channel], 2);
    };
    BOOST_REQUIRE(commands.Clear({0.2f, 0.4f, 0.8f, 0.75f}, 1));
    std::array<RHITextureHandle, 3> textures{road, cloud, noise};
    BOOST_CHECK(!renderer.Draw(commands, mesh, alpha, parameters, {}));
    BOOST_REQUIRE(renderer.Draw(commands, mesh, alpha, parameters, textures));
    check_pixel({90, 51, 102, 191});
    parameters.masked_modulation = 1;
    BOOST_REQUIRE(renderer.Draw(commands, mesh, multiply, parameters, textures));
    check_pixel({67, 38, 76, 191});
    BOOST_REQUIRE(commands.Clear({0.2f, 0.4f, 0.8f, 0.75f}, 1));
    parameters.lightmap = 1;
    BOOST_REQUIRE(Draw_Road(renderer, commands, mesh, parameters, textures, true));
    check_pixel({67, 38, 76, 191});
    parameters.lightmap = 0;
    BOOST_REQUIRE(commands.Clear({0.2f, 0.4f, 0.8f, 0.75f}, 1));
    textures[0] = transparent;
    parameters.masked_modulation = 0;
    BOOST_REQUIRE(renderer.Draw(commands, mesh, alpha, parameters, textures));
    parameters.masked_modulation = 1;
    BOOST_REQUIRE(renderer.Draw(commands, mesh, multiply, parameters, textures));
    check_pixel({51, 102, 204, 191});

    // Device shutdown preserves CPU meshes while releasing GPU ownership.
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    parameters.masked_modulation = 0;
    textures[0] = road;
    BOOST_REQUIRE(renderer.Draw(commands, mesh, alpha, parameters, textures));
    check_pixel({90, 51, 102, 191});
    BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    BOOST_CHECK(!renderer.Draw(commands, mesh, alpha, parameters, textures));
    renderer.Shutdown();
    for (auto handle : {road, transparent, cloud, noise, target, depth}) BOOST_CHECK(device.Destroy_Texture(handle));
}

BOOST_AUTO_TEST_CASE(surface_geometry_rejects_invalid_edits_without_changing_segment_topology)
{
    SurfaceGeometry geometry;
    std::array<SurfaceVertex, 3> vertices{};
    vertices[0].position = {-2, 3, 4};
    vertices[1].position = {5, 6, 7};
    vertices[2].position = {8, 9, 10};
    const std::array<std::uint32_t, 3> indices{2, 0, 1};
    BOOST_REQUIRE(geometry.Assign(vertices, indices));
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].position[2], 7);
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(), geometry.Indices().end(), indices.begin(), indices.end());
    BOOST_CHECK(!geometry.Assign(vertices, std::array<std::uint32_t, 3>{0, 1, 3}));
    BOOST_CHECK(!geometry.Assign(vertices, std::array<std::uint32_t, 2>{0, 1}));
    BOOST_CHECK_EQUAL(geometry.Indices()[0], 2);
}

BOOST_AUTO_TEST_CASE(surface_vertices_preserve_packed_channels_uvs_and_topology_after_source_release)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const std::array<std::uint8_t,8> texels{255,255,255,255,128,255,64,255};
    const auto texture=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    BOOST_REQUIRE(texture.Is_Valid());
    SurfaceMeshHandle mesh;
    {
        std::vector<SurfaceVertex> vertices(8);
        const std::array<std::uint32_t,2> colors{0x80402010u,0x40ff8040u};
        for (unsigned quad=0;quad<2;++quad) {
            const float left=quad==0 ? -.875f : .125f;
            const float right=left+.75f;
            vertices[quad*4].position={left,-.75f,.5f};
            vertices[quad*4+1].position={right,-.75f,.5f};
            vertices[quad*4+2].position={right,.75f,.5f};
            vertices[quad*4+3].position={left,.75f,.5f};
            for (unsigned corner=0;corner<4;++corner) {
                auto& vertex=vertices[quad*4+corner];
                vertex.color=Assets::Color_From_ARGB(colors[quad]).To_Array();
                vertex.uv={quad==0 ? .25f : .75f,.5f};
            }
        }
        const std::array<std::uint32_t,12> indices{0,1,2,0,2,3,4,5,6,4,6,7};
        mesh=renderer.Create_Mesh(vertices,indices);
        BOOST_REQUIRE(mesh.Is_Valid());
    }
    SurfaceParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    SurfaceStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.color_write_mask=15;
    style.linear_filter=false;
    for (unsigned recreation=0;recreation<2;++recreation) {
        const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
        BOOST_REQUIRE(commands.Clear({.125f,.25f,.5f,.75f},1));
        const std::array<RHITextureHandle,3> textures{texture,{},{}};
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        const auto check=[&](unsigned x,unsigned y,std::array<int,4> expected) {
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*16+x)*4+channel])-expected[channel],2);
        };
        check(2,4,{64,32,16,128}); check(5,11,{64,32,16,128});
        check(10,4,{128,128,16,64}); check(13,11,{128,128,16,64});
        check(8,8,{32,64,128,191}); check(0,0,{32,64,128,191});
        BOOST_CHECK(device.Destroy_Texture(target));
        BOOST_CHECK(device.Destroy_Texture(depth));
        if (recreation==0) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,shaders));
        }
    }
    BOOST_CHECK(renderer.Destroy_Mesh(mesh));
    renderer.Shutdown();
    BOOST_CHECK(device.Destroy_Texture(texture));
}
