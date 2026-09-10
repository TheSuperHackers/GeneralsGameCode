module;
#define BOOST_TEST_MODULE RibbonTextureCoordinatesTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <limits>
#include <cstddef>
#include <span>
#include <vector>

export module Graphics.Scene.Beams.RibbonTextureCoordinates.Tests;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonGeometry;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonIntersections;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

namespace {

RibbonPoint Make_Point(float x, float y, float z, float v, std::array<float, 4> color = {1, 1, 1, 1})
{
    RibbonPoint point;
    point.position = {x, y, z};
    point.color = color;
    point.v = v;
    return point;
}

RibbonIntersection Make_Joint(std::size_t point_count, const RibbonPoint &point,
    std::array<float, 3> direction)
{
    RibbonIntersection joint;
    joint.point_count = point_count;
    joint.direction = direction;
    joint.point = point;
    return joint;
}

}

BOOST_AUTO_TEST_CASE(scrolling_wraps_both_directions_and_repeated_views_do_not_advance_time)
{
    RibbonTextureCoordinates state(1000);
    state.SetOffset({0.75f, 0.25f});
    state.SetRate({0.5f, -0.5f});
    const auto offset = state.Advance(2000);
    BOOST_CHECK_CLOSE(offset[0], 0.25f, 0.001f);
    BOOST_CHECK_CLOSE(offset[1], 0.75f, 0.001f);
    BOOST_CHECK(state.Advance(2000) == offset);
    BOOST_CHECK(state.Rate() == (std::array<float, 2>{0.5f, -0.5f}));
    state.SetOffset({-2.25f, 3.5f});
    const auto wrapped = state.Advance(2000);
    BOOST_CHECK_EQUAL(wrapped[0], 0.75f);
    BOOST_CHECK_EQUAL(wrapped[1], 0.5f);
}

BOOST_AUTO_TEST_CASE(clock_rollover_reset_and_copies_preserve_independent_animation)
{
    RibbonTextureCoordinates state((std::numeric_limits<std::uint32_t>::max)() - 99);
    state.SetRate({1, -1});
    const auto wrapped = state.Advance(100);
    BOOST_CHECK_CLOSE(wrapped[0], 0.2f, 0.001f);
    BOOST_CHECK_CLOSE(wrapped[1], 0.8f, 0.001f);
    auto copy = state;
    state.Reset(1000);
    BOOST_CHECK(state.Advance(1000) == (std::array<float, 2>{0, 0}));
    BOOST_CHECK_CLOSE(state.Advance(1250)[0], 0.25f, 0.001f);
    BOOST_CHECK(copy.Advance(100) == wrapped);
    BOOST_CHECK_CLOSE(copy.Advance(350)[0], 0.45f, 0.001f);
}

BOOST_AUTO_TEST_CASE(authored_mapping_uses_absolute_point_indices_across_chunks)
{
    BOOST_CHECK(Ribbon_Texture_U(RibbonTextureMapping::Across) == (std::array<float, 2>{0, 1}));
    BOOST_CHECK(Ribbon_Texture_U(RibbonTextureMapping::Along) == (std::array<float, 2>{0, 0}));
    BOOST_CHECK(Ribbon_Texture_U(RibbonTextureMapping::Tiled) == (std::array<float, 2>{0, 1}));
    for (const auto mapping : {RibbonTextureMapping::Across, RibbonTextureMapping::Along, RibbonTextureMapping::Tiled}) {
        const float end = Ribbon_Texture_V(mapping, 128, 0.5f);
        BOOST_CHECK_EQUAL(end, mapping == RibbonTextureMapping::Across ? 0.0f : 64.0f);
        BOOST_CHECK_EQUAL(Ribbon_Texture_V(mapping, 128 + 1, 0.5f),
            mapping == RibbonTextureMapping::Across ? 0.0f : 64.5f);
        BOOST_CHECK_EQUAL(Ribbon_Texture_V(mapping, 129, 0), 0);
    }
}

BOOST_AUTO_TEST_CASE(scrolled_texture_pixels_and_transparency_survive_source_release_and_resize)
{
    for (bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const std::array<std::uint8_t, 16> image{
            255,0,0,255, 0,255,0,255, 0,0,255,255, 0,0,0,0};
        const auto texture = device.Create_Texture_Initialized({4,1}, {std::as_bytes(std::span(image)),16});
        BOOST_REQUIRE(texture.Is_Valid());
        RibbonTextureCoordinates coordinates;
        coordinates.SetOffset({0.125f,0.5f});
        coordinates.SetRate({1,0});
        PropStyle style;
        style.depth_test = false;
        style.depth_write = false;
        style.cull = RHICullMode::None;
        style.source_blend = RHIBlendFactor::SourceAlpha;
        style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
        style.samplers[0].Set_Filter(RHISamplerFilter::Point);
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        auto& commands = device.Immediate_Command_List();
        for (unsigned sample = 0; sample < 4; ++sample) {
            const auto uv = coordinates.Advance(sample * 250);
            std::array<RibbonPoint,2> source{
                Make_Point(-.5f,0,.5f,0), Make_Point(.5f,0,.5f,0)};
            RibbonEdges edges;
            BOOST_REQUIRE(edges.Build(source,1));
            RibbonIntersections intersections;
            BOOST_REQUIRE(intersections.Build(edges,true,1,1.5f));
            RibbonGeometry geometry;
            BOOST_REQUIRE(geometry.Build(edges.Points(), intersections.Top(), intersections.Bottom(),
                RibbonTextureMapping::Across, uv));
            BOOST_REQUIRE_EQUAL(geometry.Vertices().size(), 4u);
            BOOST_REQUIRE_EQUAL(geometry.Indices().size(), 6u);
            const auto mesh = renderer.Create_Mesh(geometry.Vertices(),geometry.Indices());
            BOOST_REQUIRE(mesh.Is_Valid());
            source = {};
            edges = {};
            intersections = {};
            geometry = {};
            for (unsigned width : {32u,64u,32u}) {
                const auto target = device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,
                    static_cast<unsigned>(RHITextureUsage::RenderTarget)});
                const auto depth = device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,
                    static_cast<unsigned>(RHITextureUsage::DepthStencil)});
                BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
                BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));
                BOOST_REQUIRE(commands.Clear({1,1,1,1},1));
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
                std::vector<std::byte> pixels(width*32*4);
                BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const auto center = (16*width+width/2)*4;
                const std::array<unsigned,4> expected = sample == 0
                    ? std::array<unsigned,4>{0,0,255,255}
                    : sample == 1 ? std::array<unsigned,4>{255,255,255,255}
                    : sample == 2 ? std::array<unsigned,4>{255,0,0,255}
                    : std::array<unsigned,4>{0,255,0,255};
                for (unsigned channel = 0; channel < 4; ++channel)
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center+channel]) - static_cast<int>(expected[channel]), 2);
                device.Destroy_Texture(target);
                device.Destroy_Texture(depth);
            }
            renderer.Destroy_Mesh(mesh);
        }
        device.Destroy_Texture(texture);
        renderer.Shutdown();
    }
}

BOOST_AUTO_TEST_CASE(merged_corner_geometry_preserves_gpu_coverage_after_source_release)
{
    for (bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));

        const std::array<float, 3> top_direction{0.70710677f, 0.70710677f, 0};
        const std::array<float, 3> bottom_direction{0.70710677f, -0.70710677f, 0};
        std::array<RibbonPoint, 4> points{
            Make_Point(-.8f,0,.5f,0,{1,0,0,1}), Make_Point(-.3f,0,.5f,1,{1,0,0,1}),
            Make_Point(.3f,0,.5f,2,{1,0,0,1}), Make_Point(.8f,0,.5f,3,{1,0,0,1})};
        std::array<RibbonIntersection, 2> top{
            Make_Joint(3, points[0], top_direction), Make_Joint(1, points[3], top_direction)};
        std::array<RibbonIntersection, 4> bottom{
            Make_Joint(1, points[0], bottom_direction), Make_Joint(1, points[1], bottom_direction),
            Make_Joint(1, points[2], bottom_direction), Make_Joint(1, points[3], bottom_direction)};
        RibbonGeometry geometry;
        BOOST_REQUIRE(geometry.Build(points,top,bottom,RibbonTextureMapping::Across,{}));
        BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),6u);
        BOOST_REQUIRE_EQUAL(geometry.Indices().size(),12u);
        const auto mesh=renderer.Create_Mesh(geometry.Vertices(),geometry.Indices());
        BOOST_REQUIRE(mesh.Is_Valid());
        points={};
        top={};
        bottom={};
        geometry={};

        PropStyle style;
        style.blend=RHIBlendMode::Disabled;
        style.depth_test=false;
        style.depth_write=false;
        style.cull=RHICullMode::None;
        PropParameters parameters;
        parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        auto& commands=device.Immediate_Command_List();
        for (unsigned pass=0; pass<2; ++pass) {
            const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
                static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
                static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
            BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
            std::array<std::byte,32*32*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
            const auto check_pixel = [&](unsigned x, unsigned y, std::array<unsigned,4> expected) {
                const auto offset = static_cast<std::size_t>((y*32+x)*4);
                for (unsigned channel=0; channel<4; ++channel)
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+channel]),expected[channel]);
            };
            // Each sample is well inside one of the three fan triangles or the
            // final strip pair; the outside sample proves the target stayed clear.
            check_pixel(10,15,{255,0,0,255});
            check_pixel(13,17,{255,0,0,255});
            check_pixel(17,16,{255,0,0,255});
            check_pixel(20,16,{255,0,0,255});
            check_pixel(0,0,{0,0,0,255});
            device.Destroy_Texture(target);
            device.Destroy_Texture(depth);
        }

        renderer.Destroy_Mesh(mesh);
        renderer.Shutdown();
    }
}
