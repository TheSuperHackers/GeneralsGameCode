module;

#include "../../profiling/Tracy.h"

#define BOOST_TEST_MODULE SegmentedLineTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

export module Graphics.Scene.Beams.SegmentedLine.Tests;

import Graphics.Scene.Beams.SegmentedLine;
import Graphics.Scene.Beams.RibbonPipeline;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Props.Geometry;
import Graphics.Materials.State;
#if defined(_WIN32)
import Graphics.RHI;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Shadows.DirectionalRenderer;
#endif

using namespace Graphics;

namespace {

RibbonPoint Make_Point(float x, float y, float z)
{
    return RibbonPoint{{x, y, z}, {1, 1, 1, 1}, 0};
}

}

BOOST_AUTO_TEST_CASE(state_limits_scaling_and_uv_reset_are_retained)
{
    SegmentedLineDescription description;
    description.texture_tile_factor = 100;
    description.uv_offset_rate = {0.5f, -0.25f};
    description.width = 4;
    description.noise_amplitude = 2;
    description.subdivision_level = RibbonPipelineMaximumSubdivisionLevel + 1;

    SegmentedLineRenderer renderer(description, 1000);
    BOOST_CHECK_EQUAL(renderer.Get_Texture_Tile_Factor(), 50.0f);
    BOOST_CHECK(renderer.Get_UV_Offset_Rate() == description.uv_offset_rate);
    BOOST_CHECK_EQUAL(renderer.Get_Current_Subdivision_Level(),
        RibbonPipelineMaximumSubdivisionLevel);

    renderer.Scale(0.5f);
    BOOST_CHECK_EQUAL(renderer.Get_Width(), 2.0f);
    BOOST_CHECK_EQUAL(renderer.Get_Noise_Amplitude(), 1.0f);

    description.subdivision_level = RibbonPipelineMaximumSubdivisionLevel + 2;
    renderer.Set_Description(description);
    BOOST_CHECK_EQUAL(renderer.Get_Current_Subdivision_Level(),
        RibbonPipelineMaximumSubdivisionLevel);

    SegmentedLineBuildInput input;
    input.time_milliseconds = 2000;
    input.use_point_colors = false;
    std::vector<PropVertex> before_reset;
    BOOST_REQUIRE(renderer.Build(input, 2,
        [](std::size_t index) { return Make_Point(static_cast<float>(index), 0, -5); },
        [] { return std::array<float, 3>{}; },
        [](std::size_t) {},
        [&](const RibbonPipelineChunk &chunk) {
            before_reset.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));
    BOOST_REQUIRE(!before_reset.empty());
    BOOST_CHECK_CLOSE(before_reset.front().uv[1], 0.75f, 0.001f);

    renderer.Reset_Line(5000);
    std::vector<PropVertex> vertices;
    input.time_milliseconds = 5000;
    renderer.Set_Current_Subdivision_Level(0);
    BOOST_REQUIRE(renderer.Build(input, 2,
        [](std::size_t index) { return Make_Point(static_cast<float>(index), 0, -5); },
        [] { return std::array<float, 3>{}; },
        [](std::size_t) {},
        [&](const RibbonPipelineChunk &chunk) {
            vertices.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));
    BOOST_REQUIRE_EQUAL(vertices.size(), 4u);
    BOOST_CHECK_SMALL(vertices[0].uv[1], 0.00001f);
}

BOOST_AUTO_TEST_CASE(build_owns_transform_uniform_color_uv_and_random_reset_contract)
{
    SegmentedLineDescription description;
    description.texture_mapping = RibbonTextureMapping::Tiled;
    description.texture_tile_factor = 0.5f;
    description.width = 2;
    description.color = {0.2f, 0.4f, 0.6f};
    description.opacity = 0.75f;
    description.merge_intersections = false;
    description.subdivision_level = 1;
    description.noise_amplitude = 0;
    description.uv_offset_rate = {0, 0.25f};
    SegmentedLineRenderer renderer(description, 1000);

    SegmentedLineBuildInput input;
    input.time_milliseconds = 2000;
    input.view[0] = 2;
    input.view[3] = 0.25f;
    input.world[0] = 0.5f;
    input.world[3] = 0.1f;

    const std::array<RibbonPoint, 2> source{
        Make_Point(-1, 0, -5), Make_Point(1, 0, -5)};
    std::vector<std::size_t> resets;
    std::vector<std::array<float, 3>> random_offsets;
    std::vector<PropVertex> vertices;
    BOOST_REQUIRE(renderer.Build(input, source.size(),
        [&](std::size_t index) { return source[index]; },
        [&] {
            random_offsets.push_back({0.1f, 0.2f, 0.3f});
            return random_offsets.back();
        },
        [&](std::size_t first_source_point) { resets.push_back(first_source_point); },
        [&](const RibbonPipelineChunk &chunk) {
            vertices.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));

    BOOST_REQUIRE_EQUAL(resets.size(), 1u);
    BOOST_CHECK_EQUAL(resets[0], 0u);
    BOOST_REQUIRE_EQUAL(random_offsets.size(), 1u);
    BOOST_REQUIRE_EQUAL(vertices.size(), 6u);
    // x' = 2 * (0.5 * x + 0.1) + 0.25 = x + 0.45.
    BOOST_CHECK_CLOSE(vertices[0].position[0], -0.55f, 0.001f);
    BOOST_CHECK_CLOSE(vertices.back().position[0], 1.45f, 0.001f);
    BOOST_CHECK_CLOSE(vertices[0].uv[1], 0.25f, 0.001f);
    BOOST_CHECK_CLOSE(vertices.back().uv[1], 0.75f, 0.001f);

    const std::array<float, 4> expected_color{
        51.0f / 255.0f, 102.0f / 255.0f, 153.0f / 255.0f, 191.0f / 255.0f};
    BOOST_CHECK(vertices.front().color == expected_color);
    BOOST_CHECK(vertices.back().color == expected_color);
}

BOOST_AUTO_TEST_CASE(bounds_include_width_and_recursive_noise_envelope)
{
    SegmentedLineDescription description;
    description.width = 2;
    description.noise_amplitude = 1;
    SegmentedLineRenderer renderer(description);
    const std::array<std::array<float, 3>, 2> points{{{0, 0, 0}, {10, 0, 0}}};

    const auto bounds = renderer.Get_Bounds(points.size(), 1,
        [&](std::size_t index) { return points[index]; });
    BOOST_REQUIRE(bounds.valid);
    BOOST_CHECK(bounds.minimum == (std::array<float, 3>{-1, -3, -3}));
    BOOST_CHECK(bounds.maximum == (std::array<float, 3>{11, 3, 3}));

    const auto invalid = renderer.Get_Bounds(1, 1,
        [&](std::size_t) { return std::array<float, 3>{}; });
    BOOST_CHECK(!invalid.valid);
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(queued_lines_retain_fog_alpha_equal_depth_order_and_resources)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders = Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    submission.Initialize(device, renderer, shadows);

    const auto target = device.Create_Texture({16, 16, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16, 16, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 16, 16, 0, 1}));

    struct TextureSource { RHITextureHandle handle; };
    const auto resolve = [](const TextureSource *source, bool)
        -> std::optional<PropMaterialTexture> {
        return PropMaterialTexture{source->handle, {}};
    };
    const auto check_pixel = [&](unsigned x, unsigned y, std::array<int, 4> expected) {
        std::array<std::byte, 16 * 16 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 16 * 4));
        for (unsigned channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y * 16 + x) * 4 + channel])
                - expected[channel], 2);
    };

    SegmentedLineDrawInput input;
    input.projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -0.1f, 0, 0, 0, 0, 1};
    input.sorting_depth = {0, 0, 1, 0};
    const std::array<RibbonPoint, 2> points{
        Make_Point(-0.75f, 0, -5), Make_Point(0.75f, 0, -5)};

    for (unsigned pass = 0; pass < 2; ++pass) {
        const std::array<std::uint8_t, 4> texel = pass == 0
            ? std::array<std::uint8_t, 4>{64, 128, 192, 128}
            : std::array<std::uint8_t, 4>{255, 255, 255, 128};
        const auto texture = device.Create_Texture_Initialized({1, 1},
            {std::as_bytes(std::span(texel)), 4});
        BOOST_REQUIRE(texture.Is_Valid());
        BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
        {
            // Both the adapter's source wrapper and ribbon scratch disappear
            // before the queue draws. Submission must own every needed value.
            TextureSource source{texture};
            SegmentedLineDescription description;
            description.width = 0.5f;
            description.merge_intersections = false;
            description.shader = MaterialState::AlphaSprite();
            description.shader.Set_Depth_Compare(MaterialState::PASS_ALWAYS);
            if (pass == 0) {
                description.opacity = 0.5f;
                description.shader.Set_Fog_Func(MaterialState::FOG_ENABLE);
                input.scene.fog.enabled = true;
                input.scene.fog.start = 0;
                input.scene.fog.end = 10;
                input.scene.fog.color = {0, 1, 0, 1};
            } else {
                description.color = {1, 0, 0};
                input.scene.fog.enabled = false;
            }
            SegmentedLineRenderer line(description);
            const auto submit = [&] {
                return line.Submit(device, renderer, submission, input, points.size(),
                    &source, resolve, [&](std::size_t index) { return points[index]; },
                    [] { return std::array<float, 3>{}; }, [](std::size_t) {});
            };
            BOOST_REQUIRE(submit());
            if (pass == 1) {
                line.Set_Color({0, 0, 1});
                BOOST_REQUIRE(submit());
            }
            BOOST_REQUIRE(device.Destroy_Texture(texture));
        }
        // The first capture checks submission itself. The second additionally
        // rebuilds GPU renderer state with queued geometry still retained.
        if (pass == 1) {
            renderer.Shutdown();
            BOOST_REQUIRE(commands.Reset_State());
            BOOST_REQUIRE(renderer.Initialize(device, shaders));
            BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
            BOOST_REQUIRE(commands.Set_Viewport({0, 0, 16, 16, 0, 1}));
        }
        BOOST_REQUIRE(submission.Flush_Transparent());
        // Half-distance green fog changes RGB, not fragment alpha. Equal-depth
        // red then blue draws retain submission order under alpha blending.
        const auto expected = pass == 0 ? std::array{8, 48, 24, 16}
                                       : std::array{64, 0, 128, 96};
        check_pixel(6, 7, expected);
        check_pixel(9, 8, expected);
        check_pixel(0, 0, {0, 0, 0, 0});
        BOOST_CHECK(!device.Retain_Texture(texture));
    }
    submission.Shutdown();
    renderer.Shutdown();
    BOOST_REQUIRE(device.Destroy_Texture(target));
    BOOST_REQUIRE(device.Destroy_Texture(depth));
}
#endif
