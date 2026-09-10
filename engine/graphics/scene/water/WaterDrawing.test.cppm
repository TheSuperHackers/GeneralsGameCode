module;
#define BOOST_TEST_MODULE WaterDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>
export module Graphics.Scene.Water.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Water.Renderer;
import Graphics.Scene.Terrain.Renderer;
import Graphics.Resources.Textures.Sampling;
import Assets.Adapters.DDS;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(water_surface_tessellation_preserves_polygon_edges_and_height)
{
    const std::array<std::array<float,3>,4> corners{{{0,0,7},{16,0,7},{12,16,7},{0,16,7}}};
    WaterGeometry geometry;
    BOOST_REQUIRE(geometry.Assign_Surface_Patch(corners,8));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),12u);
    BOOST_REQUIRE_EQUAL(geometry.Indices().size(),36u);
    BOOST_CHECK(geometry.Vertices().front().position == corners[0]);
    BOOST_CHECK(geometry.Vertices()[2].position == corners[1]);
    BOOST_CHECK(geometry.Vertices().back().position == corners[2]);
    BOOST_CHECK(geometry.Vertices()[9].position == corners[3]);
    float area = 0;
    const auto vertices = geometry.Vertices();
    const auto indices = geometry.Indices();
    for (std::size_t index = 0; index < indices.size(); index += 3) {
        const auto& a = vertices[indices[index]].position;
        const auto& b = vertices[indices[index+1]].position;
        const auto& c = vertices[indices[index+2]].position;
        area += std::abs((b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]))*0.5f;
    }
    BOOST_CHECK_SMALL(area-224.0f,0.001f);
    for (const auto& vertex : vertices) {
        BOOST_CHECK_EQUAL(vertex.position[2],7);
        BOOST_CHECK(vertex.color == (std::array<float,4>{1,1,1,1}));
    }
    BOOST_CHECK(!geometry.Assign_Surface_Patch(corners,0));
}

BOOST_AUTO_TEST_CASE(water_wave_fronts_split_in_thirds_conserve_amplitude_and_expire)
{
    WaterWaves waves;
    BOOST_REQUIRE(waves.Emit_Circle({50,75},6,80,3,0));
    for (const auto [time,count] : {std::pair{0.0f,6u},{0.2f,18u},{0.8f,54u}}) {
        const auto particles = waves.Prepare(time);
        BOOST_REQUIRE_EQUAL(particles.size(),count);
        float total = 0;
        for (const auto& particle : particles) {
            total += particle.amplitude;
            BOOST_CHECK(particle.position == (std::array<float,2>{50,75}));
            BOOST_CHECK_SMALL(std::hypot(particle.velocity[0],particle.velocity[1])-80.0f,0.0001f);
            BOOST_CHECK_SMALL(particle.creation_time+0.075f,0.000001f);
        }
        BOOST_CHECK_SMALL(total-6.0f,0.00001f);
    }
    BOOST_CHECK(waves.Prepare(2).empty());
    BOOST_CHECK_EQUAL(waves.Ring_Count(),0u);
    WaterBodyMotion body{1,{0,0},{1,0},45};
    waves.Update_Bodies(2,std::span(&body,1));
    body.position[0] = 3;
    waves.Update_Bodies(2.0333333f,std::span(&body,1));
    BOOST_REQUIRE_EQUAL(waves.Ring_Count(),1u);
    auto particles = waves.Prepare(2.0333333f);
    BOOST_REQUIRE(!particles.empty());
    BOOST_CHECK_EQUAL(particles.front().position[0],48);
    waves.Update_Bodies(2.0333333f,std::span(&body,1));
    BOOST_CHECK_EQUAL(waves.Ring_Count(),1u);
    waves.Update_Bodies(2.0666667f,{});
    BOOST_CHECK(waves.Prepare(10).empty());
    waves.Clear();
    BOOST_CHECK_EQUAL(waves.Ring_Count(),0u);
}

BOOST_AUTO_TEST_CASE(ocean_environment_follows_light_orientation_and_linear_light_energy)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,4> black_pixel{0,0,0,255};
    const auto black = device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(black_pixel)),4});
    const std::array<float,4> normal_pixel{0.5f,0.5f,1,1};
    const auto normal = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(normal_pixel)),16});
    const std::array<std::uint8_t,8> environment_pixels{16,0,0,128,0,0,16,128};
    const auto environment = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(environment_pixels)),8});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::array<float,3>,4> corners{{{-1,-1,0.5f},{1,-1,0.5f},{1,1,0.5f},{-1,1,0.5f}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,1);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,1000000,0.5f,1};
    WaterStyle style;
    style.pass = WaterPass::Ocean;
    style.blend = RHIBlendMode::Disabled;
    std::array<RHITextureHandle,9> textures{black,black,normal,black,black,black,environment,black,black};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto check = [&](std::array<int,3> expected) {
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (unsigned channel = 0; channel < 3; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
    };
    check({136,0,0});
    parameters.environment_frame = {{{-1,0,0,0},{0,-1,0,0},{0,0,1,0}}};
    check({0,0,136});
    parameters.sun_color = {0.5f,0.5f,0.5f,1};
    check({0,0,100});
    const auto mipmapped_environment = device.Create_Texture({512,256,10});
    BOOST_REQUIRE(mipmapped_environment.Is_Valid());
    for (unsigned mip = 0; mip < 10; ++mip) {
        const unsigned width = (std::max)(1u,512u >> mip);
        const unsigned height = (std::max)(1u,256u >> mip);
        std::vector<std::uint8_t> pixels(width*height*4);
        if (mip == 0) for (unsigned pixel = 0; pixel < width*height; ++pixel) {
            pixels[pixel*4] = 16;
            pixels[pixel*4+3] = 128;
        }
        BOOST_REQUIRE(device.Update_Texture(mipmapped_environment,
            {std::as_bytes(std::span(pixels)),width*4,0,mip}));
    }
    textures[6] = mipmapped_environment;
    parameters.sun_color = {1,1,1,1};
    parameters.camera_position = {0,0,0.5f,1};
    check({136,0,0});
    const auto frame = Water_Environment_Frame({0,0,1});
    BOOST_CHECK(frame == (std::array<std::array<float,4>,3>{{{0,1,0,0},{-1,0,0,0},{0,0,1,0}}}));
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {black,normal,environment,mipmapped_environment,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(ocean_refraction_rejects_edges_in_linear_color)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,4> black_pixel{0,0,0,255};
    const auto black = device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(black_pixel)),4});
    const std::array<float,4> normal_pixel{1,0.5f,0,1};
    const auto normal = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(normal_pixel)),16});
    std::array<std::uint8_t,16*16*4> scene_pixels{};
    const auto scene = device.Create_Texture_Initialized({16,16},{std::as_bytes(std::span(scene_pixels)),16*4});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::array<float,3>,4> corners{{{-1,-1,0.5f},{1,-1,0.5f},{1,1,0.5f},{-1,1,0.5f}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,1);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {0,0,1,0};
    WaterStyle style;
    style.pass = WaterPass::Ocean;
    style.blend = RHIBlendMode::Disabled;
    const std::array<RHITextureHandle,9> textures{black,black,normal,black,black,scene,black,black,black};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    for (const std::uint8_t brightness : {80,128}) {
        for (unsigned y = 0; y < 16; ++y)
            for (unsigned x = 8; x < 16; ++x)
                scene_pixels[(y*16+x)*4] = brightness;
        BOOST_REQUIRE(device.Update_Texture(scene,{std::as_bytes(std::span(scene_pixels)),16*4}));
        BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+7)*4])-(brightness == 80 ? 80 : 0),2);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {black,normal,scene,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(ocean_depth_color_replaces_captured_seabed_and_foreground_masks_wave_crests)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto black = solid({0,0,0,0});
    std::array<std::uint8_t,16*16*4> scene_pixels;
    scene_pixels.fill(128);
    const auto scene = device.Create_Texture_Initialized({16,16},{std::as_bytes(std::span(scene_pixels)),16*4});
    const auto lut = solid({64,128,192,0});
    const std::array<float,4> normal_pixel{0.5f,0.5f,1,1};
    const auto normal = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(normal_pixel)),16});
    const std::array<float,2> depths{0.75f,0.25f};
    const auto scene_depth = device.Create_Texture_Initialized({2,1,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(depths)),8});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::array<float,3>,4> corners{{{-1,-1,0.5f},{1,-1,0.5f},{1,1,0.5f},{-1,1,0.5f}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,1);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.inverse_view_projection = {1,0,0,0,0,1,0,0,0,0,-200,100,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {0,0,1,1};
    parameters.surface_options = {0,1,0.1f,1};
    WaterStyle style;
    style.pass = WaterPass::Ocean;
    const std::array<RHITextureHandle,11> textures{black,black,normal,black,black,scene,black,black,scene_depth,black,lut};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0.25f,0,0,1},1));
    TerrainRenderer terrain;
    BOOST_REQUIRE(terrain.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<TerrainCell,2> cells;
    for (unsigned index = 0; index < cells.size(); ++index) {
        cells[index].origin = {static_cast<float>(index)-1,-1};
        cells[index].spacing = {1,2};
        cells[index].heights.fill(index ? 0.25f : 0.75f);
    }
    BOOST_REQUIRE(terrain.Set_Cells(cells));
    TerrainDrawParameters terrain_parameters;
    terrain_parameters.view_projection = parameters.projection;
    BOOST_REQUIRE(terrain.Render(commands,TerrainSurfacePass::Surface,terrain_parameters,
        std::array<RHITextureHandle,2>{black,black}));
    terrain.Shutdown();
    BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.25f,0,0,1}));
    WaterStyle underwater_style;
    underwater_style.pass = WaterPass::Underwater;
    underwater_style.blend = RHIBlendMode::Disabled;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,underwater_style,parameters,textures));
    BOOST_REQUIRE(commands.Copy_Texture(target,scene));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    const std::array<int,4> underwater{32,64,96,255};
    const std::array<int,4> foreground{64,0,0,255};
    for (unsigned channel = 0; channel < 4; ++channel) {
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+2)*4+channel])-underwater[channel],2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+13)*4+channel])-foreground[channel],2);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {black,scene,lut,normal,scene_depth,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(underwater_instances_tint_submerged_geometry_before_surface_sampling)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> pixel) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
    };
    const auto scene = solid({128,128,128,255});
    const auto lut = solid({128,64,32,0});
    const auto black = solid({0,0,0,0});
    const float submerged_depth = 0.75f;
    const auto scene_depth = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(&submerged_depth,1)),4});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,0.5f,-1}; vertices[1].position = {1,0.5f,-1};
    vertices[2].position = {1,0.5f,1}; vertices[3].position = {-1,0.5f,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.inverse_view_projection = {1,0,0,0,0,1,0,0,0,0,-200,100,0,0,0,1};
    WaterStyle style;
    style.pass = WaterPass::Underwater;
    style.blend = RHIBlendMode::Disabled;
    const std::array<RHITextureHandle,11> textures{black,black,black,black,black,scene,black,black,scene_depth,black,lut};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const OceanPatchGrid grid{{0,0},{1,1},{},1,1};
    for (const unsigned frame : {0u,1u,2u,3u,4u}) {
        if (frame == 1) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        }
        if (frame == 2) {
            const std::array<std::uint8_t,4> caustic{128,128,128,128}, depth_color{255,255,255,128}, dark{0,0,0,255};
            BOOST_REQUIRE(device.Update_Texture(black,{std::as_bytes(std::span(caustic)),4}));
            BOOST_REQUIRE(device.Update_Texture(lut,{std::as_bytes(std::span(depth_color)),4}));
            BOOST_REQUIRE(device.Update_Texture(scene,{std::as_bytes(std::span(dark)),4}));
        }
        if (frame == 3) parameters.sun_color = {0,0,0,1};
        if (frame == 4) {
            const std::array<std::uint8_t,4> depth_color{255,255,255,0};
            BOOST_REQUIRE(device.Update_Texture(lut,{std::as_bytes(std::span(depth_color)),4}));
        }
        BOOST_REQUIRE(commands.Clear_Color_Target(target,{0,0,0,0}));
        BOOST_REQUIRE(renderer.Draw_Patches(commands,mesh,style,parameters,textures,grid));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        const int caustic = frame == 2 ? 41 : frame == 3 ? 31 : 0;
        const std::array<int,4> expected = frame < 2 ? std::array<int,4>{64,32,16,255} :
            std::array<int,4>{caustic,caustic,caustic,255};
        for (unsigned channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {scene,lut,black,scene_depth,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(underwater_depth_lut_normalizes_local_water_level)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto scene = solid({128,128,128,255});
    const auto black = solid({0,0,0,0});
    const std::array<std::uint8_t,8> ramp{255,255,255,0,0,0,0,0};
    const auto lut = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(ramp)),8});
    const float submerged_depth = 0.9925f;
    const auto scene_depth = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(&submerged_depth,1)),4});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::array<float,3>,4> corners{{{-1,-1,0.5f},{1,-1,0.5f},{1,1,0.5f},{-1,1,0.5f}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,1);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.inverse_view_projection = parameters.projection;
    WaterStyle style;
    style.pass = WaterPass::Underwater;
    style.blend = RHIBlendMode::Disabled;
    const std::array<RHITextureHandle,11> textures{black,black,black,black,black,scene,black,black,scene_depth,black,lut};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    for (const float water_level : {20.0f,200.0f}) {
        parameters.world[10] = water_level * 2;
        parameters.projection[10] = 1 / (water_level * 2);
        parameters.inverse_view_projection[10] = water_level * 2;
        const float scene_position = 0.25f + 0.5f / water_level;
        BOOST_REQUIRE(device.Update_Texture(scene_depth,{std::as_bytes(std::span(&scene_position,1)),4}));
        for (const float transparency : {0.0f,0.25f,1.0f}) {
            parameters.surface_options[1] = transparency;
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
            std::array<std::byte,16*16*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
            for (unsigned channel = 0; channel < 3; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-64,2);
        }
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {scene,black,lut,scene_depth,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(shoreline_depth_fades_underwater_tint_and_surface_reflection)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto black = solid({0,0,0,0});
    const auto reflection = solid({255,255,255,255});
    const auto lut = solid({0,0,255,0});
    const std::array<float,4> normal_pixel{0.5f,0.5f,1,1};
    const auto normal = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(normal_pixel)),16});
    constexpr unsigned width = 12, height = 4;
    std::array<std::uint8_t,width*height*4> scene_pixels;
    scene_pixels.fill(128);
    const auto capture = device.Create_Texture_Initialized({width,height},
        {std::as_bytes(std::span(scene_pixels)),width*4});
    std::array<float,width*height> depth_pixels{};
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
            depth_pixels[y*width+x] = (21.0f - x/4) / 42.0f;
    const auto scene_depth = device.Create_Texture_Initialized({width,height,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(depth_pixels)),width*4});
    const auto target = device.Create_Texture({width,height,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({width,height,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::array<float,3>,4> corners{{{-1,-1,0.5f},{1,-1,0.5f},{1,1,0.5f},{-1,1,0.5f}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,1);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection = parameters.inverse_view_projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.world[10] = parameters.inverse_view_projection[10] = 42;
    parameters.projection[10] = 1.0f/42;
    parameters.camera_position = {0,0,1000000,1};
    parameters.surface_options = {0,2,1,1};
    parameters.effects = {1,0,1,1};
    const std::array<RHITextureHandle,11> textures{black,black,normal,black,reflection,capture,black,black,scene_depth,black,lut};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,width,height}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    WaterStyle style;
    style.pass = WaterPass::Underwater;
    style.blend = RHIBlendMode::Disabled;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    const auto check = [&](const std::array<std::array<int,3>,3>& expected) {
        std::array<std::byte,width*height*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
        for (unsigned band = 0; band < 3; ++band)
            for (unsigned channel = 0; channel < 3; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(width+band*4+2)*4+channel])-expected[band][channel],2);
    };
    check({{{128,128,128},{91,91,128},{0,0,127}}});
    BOOST_REQUIRE(commands.Copy_Texture(target,capture));
    style.pass = WaterPass::Ocean;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({{{128,128,128},{162,162,183},{186,186,220}}});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {black,reflection,lut,normal,capture,scene_depth,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(ocean_shell_map_perspective_preserves_underwater_pixels)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto black = solid({0,0,0,0});
    const auto lut = solid({64,128,192,0});
    const std::array<float,4> normal_pixel{0.5f,0.5f,1,1};
    const auto normal = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(normal_pixel)),16});
    constexpr unsigned size = 32;
    WaterParameters parameters;
    parameters.world = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.view = {1,0,0,-827.888489f,0,0.608761489f,0.793353319f,-156.610382f,
        0,-0.793353319f,0.608761489f,-334.08429f,0,0,0,1};
    parameters.projection = {2.14450669f,0,0,0,0,3.81245661f,0,0,0,0,-1.00977671f,-10.0977669f,0,0,-1,0};
    parameters.inverse_view_projection = {0.466307908f,-7.39299e-7f,-81.9868927f,82.7884598f,
        0,0.159676433f,16.8064594f,-16.1774158f,0,0.208094284f,-32.4451408f,32.1535912f,
        0,0,-0.0990313366f,0.0999995247f};
    parameters.camera_position = {827.888489f,-169.76f,327.64f,1};
    parameters.displacement_domain = {0,0,2000,2000};
    parameters.effects = {0,0,1,1};
    parameters.surface_options = {0,3,1,1};
    std::array<float,size*size> depth_pixels{};
    for (unsigned y = 0; y < size; ++y) {
        const float ndc_y = 1-2*(y+0.5f)/size;
        const auto& inverse = parameters.inverse_view_projection;
        constexpr float seabed_height = 17.5f;
        const float depth = -(inverse[9]*ndc_y+inverse[11]-seabed_height*inverse[15])
            /(inverse[10]-seabed_height*inverse[14]);
        for (unsigned x = 0; x < size; ++x) depth_pixels[y*size+x] = depth;
    }
    const auto scene_depth = device.Create_Texture_Initialized({size,size,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(depth_pixels)),size*sizeof(float)});
    std::array<std::uint8_t,size*size*4> scene_pixels;
    scene_pixels.fill(128);
    const auto capture = device.Create_Texture_Initialized({size,size},
        {std::as_bytes(std::span(scene_pixels)),size*4});
    const auto target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(depth.Is_Valid());
    const std::array<std::array<float,3>,4> corners{{{1536,968,21},{1513,-696,21},{-20,-694,21},{-22,577,21}}};
    const auto mesh = renderer.Create_Surface_Patch(corners,8);
    WaterStyle style;
    style.pass = WaterPass::Ocean;
    const std::array<RHITextureHandle,11> textures{black,black,normal,black,black,capture,black,black,scene_depth,black,lut};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,size,size}));
    const auto draw = [&](const std::array<RHITextureHandle,11>& inputs) {
        BOOST_REQUIRE(device.Update_Texture(capture,{std::as_bytes(std::span(scene_pixels)),size*4}));
        WaterStyle underwater_style;
        underwater_style.pass = WaterPass::Underwater;
        underwater_style.blend = RHIBlendMode::Disabled;
        BOOST_REQUIRE(renderer.Draw(commands,mesh,underwater_style,parameters,inputs));
        BOOST_REQUIRE(commands.Copy_Texture(target,capture));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,inputs));
    };
    TerrainRenderer terrain;
    BOOST_REQUIRE(terrain.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    TerrainCell seabed;
    seabed.origin = {-1000,-1000};
    seabed.spacing = {4000,4000};
    seabed.heights.fill(17.5f);
    BOOST_REQUIRE(terrain.Set_Cells(std::span(&seabed,1)));
    TerrainDrawParameters terrain_parameters;
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned column = 0; column < 4; ++column)
            for (unsigned index = 0; index < 4; ++index)
                terrain_parameters.view_projection[row*4+column] +=
                    parameters.projection[row*4+index]*parameters.view[index*4+column];
    BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
    BOOST_REQUIRE(terrain.Render(commands,TerrainSurfacePass::Surface,terrain_parameters,
        std::array<RHITextureHandle,2>{black,black}));
    terrain.Shutdown();
    BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.25f,0,0,1}));
    draw(textures);
    std::array<std::byte,size*size*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
    for (unsigned y = 8; y < 24; ++y) {
        for (unsigned x = 8; x < 24; ++x) {
            const std::array<int,3> expected{32,64,96};
            for (unsigned channel = 0; channel < 3; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*size+x)*4+channel])-expected[channel],2);
        }
    }
    const std::array<float,4> trough_pixel{0,0,-0.025f,0};
    const auto trough = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(trough_pixel)),sizeof(trough_pixel)});
    auto trough_textures = textures;
    trough_textures[1] = trough;
    for (const float reconstruct : {1.0f,0.0f,1.0f}) {
        parameters.effects[3] = reconstruct;
        BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.25f,0,0,1}));
        draw(trough_textures);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        const std::array<int,3> expected{32,64,96};
        for (unsigned channel = 0; channel < 3; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*size+16)*4+channel])-expected[channel],2);
    }
    device.Destroy_Texture(trough);
    if (!std::filesystem::exists(std::filesystem::path(GRAPHICS_WATER_TEXTURE_DIRECTORY)/"WaterOceanOctave.dds")) {
        renderer.Destroy_Mesh(mesh);
        renderer.Shutdown();
        for (const auto texture : {black,lut,normal,scene_depth,capture,target,depth}) device.Destroy_Texture(texture);
        return;
    }
    const auto load = [&](const char* name) {
        std::ifstream file(std::filesystem::path(GRAPHICS_WATER_TEXTURE_DIRECTORY)/name,std::ios::binary|std::ios::ate);
        BOOST_REQUIRE(file.is_open());
        std::vector<std::byte> bytes(static_cast<std::size_t>(file.tellg()));
        file.seekg(0);
        BOOST_REQUIRE(file.read(reinterpret_cast<char*>(bytes.data()),bytes.size()));
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes,bytes.size(),layout));
        std::vector<std::byte> rgba;
        BOOST_REQUIRE(Assets::Decode_DDS_Surface(bytes,layout,0,0,0,rgba));
        const auto& surface = layout.surfaces.front();
        const auto texture = device.Create_Texture_Initialized({surface.width,surface.height},{rgba,surface.width*4});
        BOOST_REQUIRE(texture.Is_Valid());
        return texture;
    };
    const auto octave = load("WaterOceanOctave.dds");
    const auto foam = load("WaterOceanFoam.dds");
    const auto caustics = load("WaterCaustics.dds");
    const auto depth_lut = load("WaterDepthLut.dds");
    auto deep_textures = textures;
    deep_textures[10] = depth_lut;
    BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.25f,0,0,1}));
    draw(deep_textures);
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
    const std::array<int,3> shallow_color{27,66,87};
    for (unsigned channel = 0; channel < 3; ++channel)
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*size+16)*4+channel])-shallow_color[channel],2);
    for (const float time : {0.0f,5.0f,15.0f}) {
        BOOST_REQUIRE(renderer.Displacement().Render(commands,{target,depth,{0,0,size,size}},
            octave,parameters.displacement_domain,time,{},1));
        auto authored = textures;
        authored[1] = renderer.Displacement().Texture();
        authored[2] = octave;
        authored[3] = foam;
        authored[9] = caustics;
        authored[10] = depth_lut;
        parameters.animation[2] = time;
        BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.25f,0,0,1}));
        draw(authored);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        unsigned visible = 0;
        for (unsigned y = 8; y < 24; ++y)
            for (unsigned x = 8; x < 24; ++x)
                visible += std::to_integer<unsigned>(pixels[(y*size+x)*4+2]) > 0;
        BOOST_CHECK_GT(visible,240u);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {black,lut,normal,scene_depth,capture,target,depth,octave,foam,caustics,depth_lut}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(ocean_displacement_preserves_signed_waves_filters_both_axes_and_restores_targets)
{
    GraphicsTestDevice device({true});
    OceanDisplacement displacement;
    const auto directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(displacement.Initialize(device,directory));
    const std::array<float,4> low{0,0,0,0}, neutral{0,0,0,0.5f};
    const auto static_low = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(low)),sizeof(low)});
    const auto static_neutral = device.Create_Texture_Initialized({1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(neutral)),sizeof(neutral)});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const AttachmentSelection restore{target,depth,{0,0,16,16}};
    auto& commands = device.Immediate_Command_List();
    constexpr unsigned resolution = OceanDisplacement::Resolution;
    std::vector<std::uint16_t> pixels(resolution*resolution*4);
    const auto read = [&] {
        BOOST_REQUIRE(device.Readback_Texture(displacement.Texture(),
            std::as_writable_bytes(std::span(pixels)),resolution*8));
    };
    const auto sample = [&](unsigned x, unsigned y, unsigned channel) {
        const unsigned value = pixels[(y*resolution+x)*4+channel];
        const int exponent = (value >> 10) & 31;
        const float mantissa = static_cast<float>(value & 1023);
        const float magnitude = exponent ? std::ldexp(1+mantissa/1024,exponent-15) : std::ldexp(mantissa,-24);
        return value & 32768 ? -magnitude : magnitude;
    };
    const std::array<float,4> domain{0,0,resolution,resolution};
    BOOST_REQUIRE(displacement.Render(commands,restore,static_low,domain,0));
    read();
    for (const unsigned coordinate : {0u,512u,1023u}) {
        BOOST_CHECK_SMALL(sample(coordinate,coordinate,0),0.00001f);
        BOOST_CHECK_SMALL(sample(coordinate,coordinate,1),0.00001f);
        BOOST_CHECK_SMALL(sample(coordinate,coordinate,2) - (-0.12f*100/441),0.00004f);
    }
    OceanWaveParticle wave{{512.5f,512.5f},{0,0},1,0};
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0,std::span(&wave,1)));
    read();
    BOOST_CHECK_SMALL(sample(512,511,2)-1.0f/441,0.000005f);
    BOOST_CHECK_SMALL(sample(517,511,2)-0.5f/441,0.000005f);
    BOOST_CHECK_SMALL(sample(512,516,2)-0.5f/441,0.000005f);
    BOOST_CHECK_GT(sample(517,511,0),0.0028f);
    BOOST_CHECK_LT(sample(507,511,0),-0.0028f);
    BOOST_CHECK_GT(sample(512,516,1),0.0028f);
    BOOST_CHECK_LT(sample(512,506,1),-0.0028f);
    BOOST_CHECK_SMALL(sample(523,511,2),0.000001f);
    BOOST_CHECK_SMALL(sample(512,522,2),0.000001f);
    wave.velocity = {3,-4};
    wave.amplitude = -0.8f;
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0,std::span(&wave,1)));
    read();
    BOOST_CHECK_SMALL(sample(512,511,2),0.000001f);
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,2,std::span(&wave,1)));
    read();
    BOOST_CHECK_SMALL(sample(518,519,2)-0.8f*std::exp(-0.5f)/441,0.000005f);
    BOOST_REQUIRE(commands.Clear({1,0,0,1},1));
    std::array<std::byte,16*16*4> restored{};
    BOOST_REQUIRE(device.Readback_Texture(target,restored,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(restored[(8*16+8)*4]),255u);
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0,{},1));
    const auto before_reuse = commands.Submission_Counts();
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0,{},1));
    BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls,before_reuse.draw_calls);
    BOOST_REQUIRE(device.Update_Texture(static_neutral,{std::as_bytes(std::span(low)),sizeof(low)}));
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0,{},2));
    BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before_reuse.draw_calls,3u);
    read();
    BOOST_CHECK_SMALL(sample(512,511,2)-(-0.12f*100/441),0.00004f);
    BOOST_REQUIRE(device.Update_Texture(static_neutral,{std::as_bytes(std::span(neutral)),sizeof(neutral)}));
    displacement.Shutdown();
    BOOST_CHECK(!displacement.Texture().Is_Valid());
    BOOST_REQUIRE(displacement.Initialize(device,directory));
    BOOST_REQUIRE(displacement.Render(commands,restore,static_neutral,domain,0));
    read();
    BOOST_CHECK_SMALL(sample(518,519,2),0.000001f);
    displacement.Shutdown();
    for (const auto handle : {static_low,static_neutral,target,depth}) device.Destroy_Texture(handle);
}

BOOST_AUTO_TEST_CASE(ocean_patch_layout_covers_world_bounds_and_reuses_unchanged_rectangle)
{
    OceanPatches patches;
    OceanPatchGrid grid{{-7,-1},{12,12},{3,5,7},3,2};
    BOOST_REQUIRE(patches.Prepare(grid));
    BOOST_REQUIRE_EQUAL(patches.Worlds().size(),12u);
    const std::array<float,16> first{2,0,0,-9, 0,0,2,-1, 0,1,0,7, 0,0,0,1};
    const std::array<float,16> last{2,0,0,9, 0,0,2,11, 0,1,0,7, 0,0,0,1};
    BOOST_CHECK(patches.Worlds().front() == first);
    BOOST_CHECK(patches.Worlds().back() == last);
    const auto revision = patches.Revision();
    const auto* storage = patches.Worlds().data();
    grid.minimum = {-8,-.5f}; grid.maximum = {11,11.5f};
    BOOST_REQUIRE(patches.Prepare(grid));
    BOOST_CHECK_EQUAL(patches.Revision(),revision);
    BOOST_CHECK(patches.Worlds().data() == storage);
    grid.maximum[0] = 15.1f;
    BOOST_REQUIRE(patches.Prepare(grid));
    BOOST_CHECK_EQUAL(patches.Worlds().size(),15u);
    BOOST_CHECK_NE(patches.Revision(),revision);
    grid = {{0,0},{4097,1},{},1,1};
    BOOST_REQUIRE(patches.Prepare(grid));
    BOOST_REQUIRE_EQUAL(patches.Worlds().size(),4097u);
    BOOST_CHECK_EQUAL(patches.Worlds().back()[3],4096);
    grid.maximum = {};
    BOOST_REQUIRE(patches.Prepare(grid));
    BOOST_CHECK(patches.Worlds().empty());
}

BOOST_AUTO_TEST_CASE(ocean_instances_match_ordered_draws_across_reuse_range_change_and_recreation)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const auto solid = [&](std::array<std::uint8_t,4> pixel) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
    };
    const auto base = solid({64,128,192,128}), black = solid({0,0,0,255});
    const auto white = solid({255,255,255,255}), normal = solid({128,128,255,255});
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-.2f,.5f,-.2f}; vertices[1].position = {.2f,.5f,-.2f};
    vertices[2].position = {.2f,.5f,.2f}; vertices[3].position = {-.2f,.5f,.2f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.fog_state = {0,.1f,1,0}; parameters.fog_color = {1,0,0,1};
    parameters.tint[3] = 0.5f;
    WaterStyle style; style.pass = WaterPass::Ocean;
    const std::array<RHITextureHandle,9> textures{base,black,normal,black,black,black,black,white,white};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    for (unsigned frame=0; frame<4; ++frame) {
        if (frame == 3) { renderer.Shutdown(); BOOST_REQUIRE(renderer.Initialize(device,shaders)); }
        const unsigned columns = frame < 2 ? 2 : 1;
        const OceanPatchGrid grid{{0,0},{static_cast<float>(columns),2},{},1,1};
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        // Independent matrices matching the source y-then-x loop. Keep the
        // ordinary shader path as the comparison for blending and topology.
        for (unsigned y=0; y<2; ++y) for (unsigned x=0; x<columns; ++x) {
            parameters.world = {1,0,0,static_cast<float>(x), 0,0,1,static_cast<float>(y), 0,1,0,0, 0,0,0,1};
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        }
        std::array<std::byte,32*32*4> expected{}, actual{};
        BOOST_REQUIRE(device.Readback_Texture(target,expected,32*4));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        const auto before = commands.Submission_Counts();
        BOOST_REQUIRE(renderer.Draw_Patches(commands,mesh,style,parameters,textures,grid));
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,1u);
        BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles-before.triangles,columns*4u);
        BOOST_REQUIRE(device.Readback_Texture(target,actual,32*4));
        BOOST_CHECK(actual == expected);
        for (const unsigned pixel : {15u*32+15,16u*32+16}) {
            BOOST_CHECK_SMALL(std::to_integer<int>(actual[pixel*4])-128,2);
            BOOST_CHECK_SMALL(std::to_integer<int>(actual[pixel*4+3])-64,2);
        }
        BOOST_CHECK_EQUAL(std::to_integer<int>(actual[(3*32+3)*4+3]),0);
        BOOST_CHECK_SMALL(std::to_integer<int>(actual[31*4])-(columns == 2 ? 128 : 0),2);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    for (const auto texture : {base,black,white,normal,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(water_sampling_uses_mips_and_updates_cached_quality_settings)
{
    const auto saved = Get_Texture_Sampling_Settings();
    struct Restore final {
        TextureSamplingSettings saved;
        ~Restore() {
            Set_Texture_Sampling_Mode(static_cast<int>(saved.mode));
            Set_Texture_Anisotropy(saved.anisotropy);
        }
    } restore{saved};
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const auto texture = device.Create_Texture({128,128,3});
    BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid() && texture.Is_Valid());
    for (unsigned mip = 0; mip < 3; ++mip) {
        const unsigned extent = 128 >> mip;
        std::vector<std::uint8_t> pixels(extent * extent * 4, 0);
        for (unsigned index = 0; index < extent * extent; ++index) {
            pixels[index * 4 + mip] = 255;
            pixels[index * 4 + 3] = 255;
        }
        RHITextureUpload upload{std::as_bytes(std::span(pixels)), extent * 4};
        upload.mip_level = mip;
        BOOST_REQUIRE(device.Update_Texture(texture, upload));
    }
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,.5f}; vertices[1].position = {1,-1,.5f};
    vertices[2].position = {1,1,.5f}; vertices[3].position = {-1,1,.5f};
    for (auto &vertex : vertices) vertex.uv = {(vertex.position[0]+1)*.5f, .5f};
    const auto mesh = renderer.Create_Mesh(vertices, std::array<std::uint32_t,6>{0,1,2,0,2,3});
    BOOST_REQUIRE(mesh.Is_Valid());
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    WaterStyle style; style.pass = WaterPass::Sky; style.blend = RHIBlendMode::Disabled;
    const std::array<RHITextureHandle,9> textures{texture};
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    for (const auto mode : {TextureSamplingMode::Trilinear, TextureSamplingMode::Anisotropic,
        TextureSamplingMode::None, TextureSamplingMode::Trilinear}) {
        Set_Texture_Sampling_Mode(static_cast<int>(mode));
        Set_Texture_Anisotropy(16);
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        // The footprint spans four texels horizontally and less than one
        // vertically: anisotropy preserves that fine axis, trilinear uses mip 2.
        const unsigned channel = mode == TextureSamplingMode::Trilinear ? 2u : 0u;
        for (unsigned component = 0; component < 3; ++component)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+16)*4+component])
                - (component == channel ? 255 : 0), 2);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(reflected_camera_preserves_water_plane_projection_and_camera_side)
{
    // Tilted RTS camera away from the origin; row 3 contains homogeneous
    // values, not the eye position. This is also the game adapter's contract.
    const std::array<float,16> camera{
        1,0,0,300, 0,0.8f,-0.6f,400, 0,0.6f,0.8f,500, 0,0,0,1};
    const WaterView view(camera,7);
    BOOST_CHECK(!view.underwater);
    BOOST_CHECK_EQUAL(view.camera_position[0],300);
    BOOST_CHECK_EQUAL(view.camera_position[1],400);
    BOOST_CHECK_EQUAL(view.camera_position[2],500);
    const auto project = [](const auto& transform,std::array<float,3> point) {
        std::array<float,3> local{};
        for (unsigned column=0;column<3;++column)
            for (unsigned row=0;row<3;++row)
                local[column] += transform[row*4+column] * (point[row]-transform[row*4+3]);
        return std::array<float,2>{local[0]/local[2],local[1]/local[2]};
    };
    for (const std::array<float,3> point : {
        std::array<float,3>{0,0,7}, {100,200,7}, {500,300,7}}) {
        const auto main = project(camera,point);
        const auto reflection = project(view.reflected_camera,point);
        BOOST_CHECK_SMALL(main[0]-reflection[0],0.00001f);
        BOOST_CHECK_SMALL(main[1]-reflection[1],0.00001f);
    }
    BOOST_CHECK_GT(view.reflection_clip_plane[2]*8+view.reflection_clip_plane[3],0);
    BOOST_CHECK_LT(view.reflection_clip_plane[2]*6+view.reflection_clip_plane[3],0);
    const WaterView reflected(view.reflected_camera,7);
    BOOST_CHECK(reflected.underwater);
    BOOST_CHECK_GT(reflected.reflection_clip_plane[2]*6+reflected.reflection_clip_plane[3],0);
    BOOST_CHECK_EQUAL_COLLECTIONS(camera.begin(),camera.end(),
        reflected.reflected_camera.begin(),reflected.reflected_camera.end());
}

BOOST_AUTO_TEST_CASE(ocean_preserves_refracted_shroud_and_concentrates_foam_on_crests)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> pixel) {
        return device.Create_Texture_Initialized({1,1}, {std::as_bytes(std::span(pixel)),4});
    };
    const auto black = solid({0,0,0,255});
    const auto white = solid({255,255,255,255});
    const auto normal = solid({128,128,255,255});
    const auto shroud = solid({128,128,128,255});
    const auto scene = solid({0,0,64,255});
    const auto reflection = solid({128,0,0,255});
    const auto environment = solid({16,0,0,128});
    // Floating-point displacement retains signed horizontal movement and
    // height in world units. A 0.01 height sample raises the crest by 3 units.
    const std::array<float,4> crest_sample{0,0,0.01f,0};
    const auto crest = device.Create_Texture_Initialized(
        {1,1,1,RHITextureFormat::RGBA32_Float},
        {std::as_bytes(std::span(crest_sample)),sizeof(crest_sample)});
    BOOST_REQUIRE(crest.Is_Valid());
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.projection[10] = 0.01f;
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {1,1,1,0};
    WaterStyle style;
    style.pass = WaterPass::Ocean;
    style.blend = RHIBlendMode::Disabled;
    std::array<RHITextureHandle,9> textures{
        black,black,normal,black,reflection,scene,black,shroud,white};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto check = [&](std::array<int,3> expected) {
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (unsigned channel=0;channel<3;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
    };
    // Ocean.fx adds half the planar reflection, shrouded once. The captured
    // underwater scene is already darkened and must retain its blue value.
    check({68,0,64});
    style.pass = WaterPass::Surface;
    textures = {black,normal,black,white,reflection,scene,black,shroud,white};
    check({26,0,27});
    style.pass = WaterPass::Ocean;
    textures = {black,black,normal,black,reflection,scene,black,shroud,white};
    parameters.effects = {};
    textures[3] = white;
    check({48,48,48});
    textures[1] = crest;
    check({168,168,168});
    textures[1] = black;
    textures[3] = black;
    textures[6] = environment;
    parameters.camera_position = {1000000,0,0.5f,1};
    // The environment image stores linear RGBM radiance. Alpha scales RGB;
    // treating it as opacity or ignoring it changes reflected light energy.
    check({136,0,0});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (auto texture : {black,white,normal,shroud,scene,reflection,environment,crest,target,depth})
        device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(planar_reflection_preserves_orientation_across_target_aspect_ratios)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    // Four different quadrants expose flipped UVs and sampling a single texel.
    const std::array<std::uint8_t,16> pattern{
        255,0,0,255, 0,255,0,255,
        0,0,255,255, 255,255,0,255};
    const auto reflection = device.Create_Texture_Initialized({2,2},
        {std::as_bytes(std::span(pattern)),8});
    const auto solid = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},
            {std::as_bytes(std::span(color)),4});
    };
    const auto black = solid({0,0,0,255});
    const auto white = solid({255,255,255,255});
    const auto normal = solid({128,128,255,255});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f};
    vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f};
    vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {1,0,0,0};
    const std::array<RHITextureHandle,9> textures{
        black,normal,black,white,reflection,black,black,white,white};
    WaterStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    for (const std::uint32_t width : {32u,64u}) {
        constexpr std::uint32_t height = 24;
        const auto target = device.Create_Texture({width,height,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({width,height,1,RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,width,height}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,64*height*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
        for (std::uint32_t quadrant=0;quadrant<4;++quadrant) {
            const auto x = (quadrant%2 == 0 ? width/8 : width*7/8);
            const auto y = (quadrant/2 == 0 ? height/8 : height*7/8);
            for (std::uint32_t channel=0;channel<3;++channel) {
                const int expected = pattern[quadrant*4+channel] == 0 ? 0 : 102;
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*width+x)*4+channel])-expected,2);
            }
        }
        device.Destroy_Texture(target);
        device.Destroy_Texture(depth);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {reflection,black,white,normal}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(refraction_rejects_displacement_across_object_edges)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> pixel) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
    };
    const auto black = solid({0,0,0,255});
    const auto white = solid({255,255,255,255});
    const auto normal = solid({255,128,255,255});
    std::array<std::uint8_t,64*4> scene{};
    for (unsigned x=0;x<64;++x) {
        scene[x*4+(x<32 ? 2 : 0)] = 255;
        scene[x*4+3] = 255;
    }
    const auto refraction = device.Create_Texture_Initialized({64,1},
        {std::as_bytes(std::span(scene)),64*4});
    const auto target = device.Create_Texture({64,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {0,0,1,0};
    const std::array<RHITextureHandle,9> textures{
        black,normal,black,white,black,refraction,black,white,white};
    WaterStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,64,32}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    std::array<std::byte,64*32*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
    // The distorted coordinate crosses from underwater blue into red geometry.
    // RA3 rejects that sample rather than pulling the object's edge into water.
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*64+30)*4]),0);
    BOOST_CHECK_GT(std::to_integer<int>(pixels[(16*64+30)*4+2]),90);
    style.pass = WaterPass::Ocean;
    const std::array<RHITextureHandle,9> ocean_textures{
        black,black,normal,black,black,refraction,black,white,white};
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,ocean_textures));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*64+31)*4]),0);
    BOOST_CHECK_GT(std::to_integer<int>(pixels[(16*64+31)*4+2]),30);
    std::array<float,64> depth_samples;
    depth_samples.fill(0.8f);
    const auto scene_depth = device.Create_Texture_Initialized(
        {64,1,1,RHITextureFormat::R32_Float},
        {std::as_bytes(std::span(depth_samples)),64*sizeof(float)});
    BOOST_REQUIRE(scene_depth.Is_Valid());
    parameters.surface_options = {0,1,1,1};
    for (const auto pass : {WaterPass::Surface,WaterPass::Ocean}) {
        style.pass = pass;
        auto depth_textures = pass == WaterPass::Surface ? textures : ocean_textures;
        depth_textures[8] = scene_depth;
        for (const bool foreground : {false,true}) {
            for (unsigned x=0;x<64;++x)
                depth_samples[x] = foreground && x>=32 ? 0.2f : 0.8f;
            BOOST_REQUIRE(device.Update_Texture(scene_depth,
                {std::as_bytes(std::span(depth_samples)),64*sizeof(float)}));
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,depth_textures));
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
            const auto pixel = (16*64+31)*4;
            const bool reject = foreground || pass == WaterPass::Ocean;
            BOOST_CHECK_GT(std::to_integer<int>(pixels[pixel+(reject ? 2 : 0)]),30);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[pixel+(reject ? 0 : 2)]),0);
        }
    }
    // Exercise the actual depth attachment capture used by the water adapter,
    // including source writes after capture. It must sample the saved depth.
    for (const auto format : {RHITextureFormat::D24_UNorm_S8, RHITextureFormat::D32_Float}) {
        const auto attachment = device.Create_Texture({64,32,1,format,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(attachment.Is_Valid());
        BOOST_REQUIRE(commands.Set_Depth_Target(attachment));
        BOOST_REQUIRE(commands.Clear_Depth(0.8f));
        const auto captured = renderer.Capture_Depth(commands, {attachment,64,32}, format);
        BOOST_REQUIRE(captured.Is_Valid());
        BOOST_REQUIRE(commands.Clear_Depth(0.2f));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        for (const auto pass : {WaterPass::Surface,WaterPass::Ocean}) {
            style.pass = pass;
            auto captured_textures = pass == WaterPass::Surface ? textures : ocean_textures;
            captured_textures[8] = captured;
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,captured_textures));
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
            const unsigned retained = pass == WaterPass::Ocean ? 2 : 0;
            BOOST_CHECK_GT(std::to_integer<int>(pixels[(16*64+31)*4+retained]),30);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*64+31)*4+2-retained]),0);
        }
        device.Destroy_Texture(attachment);
    }
    device.Destroy_Texture(scene_depth);
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (auto texture : {black,white,normal,refraction,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(degenerate_strip_connectors_preserve_winding)
{
    const std::array<std::uint32_t,10> strip{2,0,3,1,1,4,4,2,5,3};
    const auto triangles = Expand_Water_Strip(strip);
    const std::array<std::uint32_t,12> expected{2,0,3,3,0,1,4,2,5,5,2,3};
    BOOST_CHECK_EQUAL_COLLECTIONS(triangles.begin(),triangles.end(),expected.begin(),expected.end());
}

BOOST_AUTO_TEST_CASE(refraction_samples_the_scene_pixel_when_the_viewport_has_an_offset)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto solid = [&](std::array<std::uint8_t,4> pixel) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
    };
    const auto black = solid({0,0,0,255});
    const auto white = solid({255,255,255,255});
    const auto normal = solid({128,128,255,255});
    std::array<std::uint8_t,64*32*4> capture{};
    for (unsigned y=0;y<32;++y) for (unsigned x=0;x<64;++x) {
        capture[(y*64+x)*4+(x<32 ? 2 : 0)] = 255;
        capture[(y*64+x)*4+3] = 255;
    }
    const auto target = device.Create_Texture({64,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {0,0,1,0};
    auto& commands = device.Immediate_Command_List();
    for (const auto format : {RHITextureFormat::RGBA8_UNorm,RHITextureFormat::BGRA8_UNorm}) {
        auto source_pixels = capture;
        if (format == RHITextureFormat::BGRA8_UNorm) {
            for (unsigned i=0;i<source_pixels.size();i+=4)
                std::swap(source_pixels[i],source_pixels[i+2]);
        }
        const auto scene = device.Create_Texture_Initialized({64,32,1,format,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)},
            {std::as_bytes(std::span(source_pixels)),64*4});
        BOOST_REQUIRE(scene.Is_Valid());
        const auto snapshot = renderer.Capture_Color(commands,{scene,64,32},format);
        BOOST_REQUIRE(snapshot.Is_Valid());
        BOOST_CHECK(renderer.Capture_Color(commands,{scene,64,32},format) == snapshot);
        // Subsequent scene writes must not change the water's saved image.
        BOOST_REQUIRE(commands.Set_Render_Targets(scene,depth));
        BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({32,0,32,32}));
        for (const auto pass : {WaterPass::Surface,WaterPass::Ocean}) {
            WaterStyle style;
            style.pass = pass;
            style.blend = RHIBlendMode::Disabled;
            const std::array<RHITextureHandle,9> textures = pass == WaterPass::Surface
                ? std::array<RHITextureHandle,9>{black,normal,black,white,black,snapshot,black,white,white}
                : std::array<RHITextureHandle,9>{black,black,normal,black,black,snapshot,black,white,white};
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
            std::array<std::byte,64*32*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
            // This pixel belongs to the red half of the captured scene. Mapping
            // the full camera image onto this viewport incorrectly samples blue.
            BOOST_CHECK_GT(std::to_integer<int>(pixels[(16*64+36)*4]),80);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*64+36)*4+2]),0);
        }
        device.Destroy_Texture(scene);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (auto texture : {black,white,normal,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(surface_normal_octaves_repeat_beyond_the_first_texture_tile)
{
    GraphicsTestDevice device({true});
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,8> normal_pixels{255,128,255,255, 0,128,255,255};
    const std::array<std::uint8_t,8> reflection_pixels{255,0,0,255, 0,0,255,255};
    const std::array<std::uint8_t,4> black_pixel{0,0,0,255};
    const auto normal = device.Create_Texture_Initialized({2,1},
        {std::as_bytes(std::span(normal_pixels)),8});
    const auto reflection = device.Create_Texture_Initialized({2,1},
        {std::as_bytes(std::span(reflection_pixels)),8});
    const auto black = device.Create_Texture_Initialized({1,1},
        {std::as_bytes(std::span(black_pixel)),4});
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    for (auto& vertex : vertices) vertex.secondary_uv = {0.25f,0.25f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection =
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,1000000,1};
    parameters.effects = {1,0,0,0};
    WaterStyle style;
    style.blend = RHIBlendMode::Disabled;
    const std::array<RHITextureHandle,9> textures{
        black,normal,black,black,reflection,black,black,black,black};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    std::array<std::byte,32*32*4> first{}, repeated{};
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    BOOST_REQUIRE(device.Readback_Texture(target,first,32*4));
    for (auto& vertex : vertices) vertex.secondary_uv[0] += 1;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    BOOST_REQUIRE(device.Readback_Texture(target,repeated,32*4));
    BOOST_CHECK_GT(std::to_integer<int>(first[(16*32+16)*4+2]),20);
    for (unsigned channel=0;channel<3;++channel)
        BOOST_CHECK_SMALL(std::to_integer<int>(first[(16*32+16)*4+channel]) -
            std::to_integer<int>(repeated[(16*32+16)*4+channel]),1);
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (auto texture : {normal,reflection,black,target,depth}) device.Destroy_Texture(texture);
}
BOOST_AUTO_TEST_CASE(water_shader_passes_preserve_sampling_blending_and_displacement)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    WaterRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto base = texture({128,64,32,255});
    const auto wave = texture({255,0,0,128});
    const auto normal = texture({128,128,255,255});
    const auto black = texture({0,0,0,0});
    const auto white = texture({255,255,255,255});
    const auto grey = texture({128,128,128,255});
    const auto displacement = texture({255,0,0,0});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<WaterVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f};
    vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f};
    vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    WaterParameters parameters;
    parameters.world = parameters.view = parameters.projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,10,1};
    parameters.displacement_domain = {0,0,1,1};
    parameters.effects = {0,1,0,0};
    std::array<RHITextureHandle,9> textures{base,normal,black,white,black,black,black,grey,white};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto check = [&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    };
    WaterStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.wireframe = true;
    BOOST_REQUIRE(commands.Clear({0,0,1,0.75f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    std::array<std::byte,16*16*4> wire_pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,wire_pixels,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(wire_pixels[(8*16+4)*4+2]),255);
    style.wireframe = false;
    BOOST_REQUIRE(commands.Clear({0,0,1,0.75f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({80,40,20,255});
    // The ninth sampled texture drives deep-water opacity.
    parameters.surface_options = {0,1,0.5f,1};
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({80,40,20,128});
    style.pass = WaterPass::Sky;
    style.blend = RHIBlendMode::Alpha;
    textures[0] = wave;
    BOOST_REQUIRE(commands.Clear({0,0,1,0.75f},0));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({128,0,127,160});
    style.pass = WaterPass::Track;
    BOOST_REQUIRE(commands.Clear({0,0,1,0.75f},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({128,0,127,160});
    style.pass = WaterPass::Ocean;
    style.blend = RHIBlendMode::Disabled;
    parameters.fog_state = {0,0.1f,1,0};
    parameters.fog_color = {1,0,0,1};
    textures = {base,black,normal,black,black,black,black,white,white};
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({255,0,0,128});
    // A displacement texture sampled by the vertex stage moves this patch
    // outside the viewport; fragment-only texture binding cannot pass this.
    textures[1] = displacement;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    check({0,0,255,255});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for(auto handle : {base,wave,normal,black,white,grey,displacement,target,depth}) device.Destroy_Texture(handle);
}
