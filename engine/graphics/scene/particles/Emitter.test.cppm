module;

#include "../../profiling/Tracy.h"
#define BOOST_TEST_MODULE EmitterTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

export module Graphics.Scene.Particles.Emitter.Tests;

import Assets.Math;
import Assets.Particles;
import Graphics.Materials.State;
import Graphics.Scene.Particles.EmitterKinematics;
import Graphics.Scene.Particles.EmitterVisualState;
import Graphics.Scene.Particles.EmitterEmission;
import Graphics.Scene.Particles.EmitterDetail;
import Graphics.Scene.Particles.EmitterRenderer;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Beams.SegmentedLine;
import Graphics.Scene.Props.Geometry;
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
Assets::EmitterAssetDesc Description()
{
    Assets::EmitterAssetDesc description;
    description.lifetime = 1;
    description.emission_rate = 100;
    description.color.start = {1, 1, 1, 1};
    description.opacity.start = 1;
    description.size.start = 0.25f;
    return description;
}
const auto no_scalar = [](std::array<float, 1>) { return std::array{0.0f}; };
const auto no_color = [](std::array<float, 3>) { return std::array<float, 3>{}; };
const auto no_vector = [] { return std::array<float, 3>{}; };
}

BOOST_AUTO_TEST_CASE(birth_queue_wrap_retains_slot_identity_order_and_expiry_boundary)
{
    EmitterKinematics particles(3, 100, {}, false, 0);
    for (unsigned birth = 1; birth <= 4; ++birth)
        particles.Append_Birth() = {{static_cast<float>(birth), 0, 0}, {}, birth,
            static_cast<std::uint8_t>(birth)};
    BOOST_CHECK_EQUAL(particles.Pending_Count(), 3);
    BOOST_REQUIRE(particles.Advance(10, 0));
    BOOST_REQUIRE_EQUAL(particles.Count(), 3);
    BOOST_CHECK_EQUAL(particles.Positions(0)[0][0], 2);
    BOOST_CHECK_EQUAL(particles.Positions(0)[2][0], 4);
    particles.Append_Birth() = {{11, 0, 0}, {}, 11, 9};
    BOOST_REQUIRE(particles.Advance(20, 0));
    const auto chronological = particles.Active_Ranges();
    BOOST_CHECK_EQUAL(chronological[0].begin, 1);
    BOOST_CHECK_EQUAL(chronological[0].end, 3);
    BOOST_CHECK_EQUAL(chronological[1].end, 1);
    const auto storage = particles.Active_Ranges(true);
    BOOST_CHECK_EQUAL(storage[0].begin, 0);
    BOOST_CHECK_EQUAL(storage[0].end, 1);
    BOOST_CHECK_EQUAL(particles.Positions(0)[0][0], 11);
    BOOST_CHECK_EQUAL(particles.Groups()[0], 9);
    BOOST_REQUIRE(particles.Advance(104, 0));
    BOOST_REQUIRE_EQUAL(particles.Count(), 1);
    BOOST_CHECK_EQUAL(particles.Positions(0)[0][0], 11);
    BOOST_REQUIRE(particles.Advance(111, 0));
    BOOST_CHECK_EQUAL(particles.Count(), 0);
}

BOOST_AUTO_TEST_CASE(accelerated_births_duplicate_time_and_empty_clones_keep_the_clock_contract)
{
    EmitterKinematics particles(4, 100, {0.5f, 0, 0}, false, 10);
    particles.Append_Birth() = {{1, 0, 0}, {2, 0, 0}, 10, 3};
    BOOST_CHECK(!particles.Advance(10, 0));
    BOOST_CHECK_EQUAL(particles.Count(), 0);
    BOOST_CHECK_EQUAL(particles.Pending_Count(), 1);
    BOOST_REQUIRE(particles.Advance(20, 0));
    BOOST_CHECK_EQUAL(particles.Positions(0)[0][0], 46);
    BOOST_CHECK_EQUAL(particles.Velocities()[0][0], 7);
    BOOST_REQUIRE(particles.Advance(30, 0));
    BOOST_CHECK_EQUAL(particles.Positions(0)[0][0], 141);
    BOOST_CHECK_EQUAL(particles.Velocities()[0][0], 12);
    EmitterKinematics clone(particles, 50);
    BOOST_CHECK_EQUAL(clone.Count(), 0);
    BOOST_CHECK_EQUAL(clone.Pending_Count(), 0);
    BOOST_CHECK_EQUAL(clone.Last_Update(), 50);
    BOOST_CHECK_EQUAL(clone.Acceleration()[0], .5f);
    const auto bounds = particles.Bounds(2, 0);
    BOOST_CHECK_EQUAL(bounds.minimum.x, 139);
    BOOST_CHECK_EQUAL(bounds.maximum.x, 143);
}

BOOST_AUTO_TEST_CASE(emission_retains_strict_interval_finite_bursts_restart_and_random_call_counts)
{
    auto description = Description();
    description.burst_size = 2;
    description.max_emissions = 3;
    EmitterEmission emission(description);
    EmitterKinematics particles(8, 1000, {}, false, 0);
    unsigned positions = 0, velocities = 0;
    const auto position = [&] { ++positions; return no_vector(); };
    const auto velocity = [&] { ++velocities; return no_vector(); };
    emission.Start();
    BOOST_REQUIRE(emission.Emit(particles, 10, 10, {}, position, velocity));
    BOOST_CHECK_EQUAL(particles.Pending_Count(), 0);
    BOOST_REQUIRE(emission.Emit(particles, 1, 11, {}, position, velocity));
    BOOST_CHECK_EQUAL(particles.Pending_Count(), 2);
    BOOST_CHECK_EQUAL(emission.Remaining(), 1);
    BOOST_REQUIRE(emission.Emit(particles, 10, 21, {}, position, velocity));
    BOOST_CHECK(emission.Complete());
    BOOST_CHECK_EQUAL(particles.Pending_Count(), 3);
    BOOST_CHECK_EQUAL(positions, 3);
    BOOST_CHECK_EQUAL(velocities, 3);
    particles.Advance(21, 0);
    BOOST_CHECK_EQUAL(particles.Timestamps()[0], 10);
    BOOST_CHECK_EQUAL(particles.Timestamps()[2], 20);
    emission.Start();
    BOOST_CHECK(!emission.Complete());
    BOOST_CHECK_EQUAL(emission.Remaining(), 3);
    BOOST_CHECK_EQUAL(emission.Remainder(), 1);
    BOOST_CHECK_EQUAL(emission.Group(), 2);
    emission.Reset();
    BOOST_CHECK_EQUAL(emission.Group(), 2);
    BOOST_CHECK_EQUAL(emission.Remainder(), 0);
}

BOOST_AUTO_TEST_CASE(visual_tracks_keep_random_stream_order_tail_motion_and_source_independence)
{
    auto description = Description();
    description.geometry_mode = Assets::EmitterGeometryMode::LineGroupPrism;
    description.color.random = {.1f, .2f, .3f, 0};
    description.opacity.random = .1f;
    description.size.random = .2f;
    description.rotation.random = 1;
    description.initial_orientation_random = 1;
    description.frame.random = 1;
    description.blur_time.start = .5f;
    description.blur_time.random = .1f;
    std::vector<unsigned> samples;
    const auto scalar = [&](std::array<float, 1>) {
        samples.push_back(1);
        return std::array{0.0f};
    };
    const auto color = [&](std::array<float, 3>) {
        samples.push_back(3);
        return std::array<float, 3>{};
    };
    EmitterVisualState visuals(description, 40, 1000, scalar, color);
    BOOST_REQUIRE_EQUAL(samples.size(), 7 * 32);
    for (unsigned sample = 0; sample < samples.size(); ++sample)
        BOOST_CHECK_EQUAL(samples[sample], sample < 32 ? 3 : 1);
    description = {};
    EmitterKinematics particles(40, 1000, {}, false, 0);
    particles.Append_Birth() = {{0, 0, 0}, {0.01f, 0, 0}, 0, 0};
    particles.Advance(100, 0);
    visuals.Evaluate(particles, 100, 0);
    BOOST_CHECK_SMALL(visuals.Tail(0)[0] + 4, 0.00001f);
    BOOST_CHECK_EQUAL(visuals.Color(0)[0], 1);
    BOOST_CHECK_EQUAL(visuals.Color(0)[3], 1);
    EmitterVisualState clone(visuals);
    BOOST_CHECK_EQUAL(samples.size(), 7 * 32);
    visuals.Scale(2);
    BOOST_CHECK_EQUAL(clone.Start_Size(), .25f);
    BOOST_CHECK_EQUAL(visuals.Start_Size(), .5f);
}

BOOST_AUTO_TEST_CASE(varying_alpha_uses_white_rgb_and_lifetime_crossing_keys_still_interpolate)
{
    auto description = Description();
    description.color.start = {.2f, .4f, .6f, 1};
    description.opacity.keys = {{2, 0}};
    description.size.start = 1;
    description.size.keys = {{2, 5}};
    EmitterVisualState visuals(description, 2, 1000, no_scalar, no_color);
    EmitterKinematics particles(2, 1000, {}, false, 0);
    particles.Append_Birth() = {{}, {}, 0, 0};
    particles.Advance(500, 0);
    visuals.Evaluate(particles, 500, 0);
    BOOST_CHECK_EQUAL(visuals.Color(0)[0], 1);
    BOOST_CHECK_EQUAL(visuals.Color(0)[1], 1);
    BOOST_CHECK_EQUAL(visuals.Color(0)[3], .75f);
    BOOST_CHECK_EQUAL(visuals.Size(0), 2);
    BOOST_CHECK_EQUAL(visuals.Max_Size(), 3);
}

BOOST_AUTO_TEST_CASE(decimation_uses_physical_slots_and_clone_level_limits)
{
    unsigned visible = 0;
    for (unsigned slot = 0; slot < 32; ++slot) {
        visible += Emitter_Slot_Is_Visible(slot, 8);
        BOOST_CHECK(!Emitter_Slot_Is_Visible(slot, 16));
    }
    BOOST_CHECK_EQUAL(visible, 16);
    BOOST_CHECK(Emitter_Slot_Is_Visible(0, 11));
    BOOST_CHECK(!Emitter_Slot_Is_Visible(1, 11));
    EmitterDetail detail(2, Assets::EmitterGeometryMode::SpriteQuads);
    BOOST_CHECK_EQUAL(detail.Count(), 17);
    BOOST_CHECK_EQUAL(detail.Level(), 16);
    detail.Set_Level(0);
    BOOST_CHECK_EQUAL(detail.Decimation(), 16);
    EmitterDetail clone(detail);
    BOOST_CHECK_EQUAL(clone.Count(), 2);
    BOOST_CHECK_EQUAL(clone.Decimation(), 1);
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(all_emitter_modes_draw_with_fog_alpha_and_retained_queued_resources)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders = Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    submission.Initialize(device, renderer, shadows);
    const auto target = device.Create_Texture({32, 32, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32, 32, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 32, 32, 0, 1}));
    struct TextureSource { RHITextureHandle handle; };
    const auto resolve = [](const TextureSource *source, bool) -> std::optional<PropMaterialTexture> {
        return PropMaterialTexture{source->handle, {}};
    };
    for (const auto mode : {Assets::EmitterGeometryMode::SpriteTriangles,
            Assets::EmitterGeometryMode::SpriteQuads, Assets::EmitterGeometryMode::Line,
            Assets::EmitterGeometryMode::LineGroupTetra, Assets::EmitterGeometryMode::LineGroupPrism}) {
        BOOST_TEST_CONTEXT("mode " << static_cast<unsigned>(mode)) {
            const std::array<std::uint8_t, 4> texel{64, 128, 192, 128};
            const auto texture = device.Create_Texture_Initialized({1, 1},
                {std::as_bytes(std::span(texel)), 4});
            BOOST_REQUIRE(texture.Is_Valid());
            BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
            {
                TextureSource source{texture};
                auto description = Description();
                description.geometry_mode = mode;
                description.size.start = .5f;
                description.opacity.start = .5f;
                description.blur_time.start = .5f;
                description.line_properties.merge_intersections = false;
                EmitterKinematics particles(4, 1000, {}, false, 0);
                particles.Append_Birth() = {{-.5f, 0, -5}, {0, 0, 0.001f}, 0, 0};
                if (mode == Assets::EmitterGeometryMode::Line)
                    particles.Append_Birth() = {{.5f, 0, -5}, {}, 0, 0};
                particles.Advance(1, 0);
                EmitterVisualState visuals(description, 4, 1000, no_scalar, no_color);
                visuals.Evaluate(particles, 1, 0);
                auto shader = MaterialState::AlphaSprite();
                shader.Set_Depth_Compare(MaterialState::PASS_ALWAYS);
                EmitterRenderer emitter(description, shader, 4, 0);
                EmitterDrawInput input;
                input.projection.values = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -.1f, 0, 0, 0, 0, 1};
                input.scene.fog.enabled = true;
                input.scene.fog.start = 0;
                input.scene.fog.end = 10;
                input.scene.fog.color = {0, 1, 0, 1};
                BOOST_REQUIRE(emitter.Submit(device, renderer, submission, particles, visuals, 0,
                    input, &source, resolve, [](bool) { return no_vector(); }, [](std::size_t) {}));
                BOOST_REQUIRE(device.Destroy_Texture(texture));
            }
            renderer.Shutdown();
            BOOST_REQUIRE(commands.Reset_State());
            BOOST_REQUIRE(renderer.Initialize(device, shaders));
            BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
            BOOST_REQUIRE(commands.Set_Viewport({0, 0, 32, 32, 0, 1}));
            BOOST_REQUIRE(submission.Flush_Transparent());
            std::array<std::byte, 32 * 32 * 4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
            unsigned matched = 0;
            for (unsigned pixel = 0; pixel < 32 * 32; ++pixel) {
                const int red = std::to_integer<int>(pixels[pixel * 4]);
                const int green = std::to_integer<int>(pixels[pixel * 4 + 1]);
                const int blue = std::to_integer<int>(pixels[pixel * 4 + 2]);
                const int alpha = std::to_integer<int>(pixels[pixel * 4 + 3]);
                if (std::abs(red - 8) <= 2 && std::abs(green - 48) <= 2
                    && std::abs(blue - 24) <= 2 && std::abs(alpha - 16) <= 2)
                    ++matched;
            }
            BOOST_CHECK_GT(matched, 10);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[0]), 0);
            BOOST_CHECK(!device.Retain_Texture(texture));
        }
    }
    submission.Shutdown();
    renderer.Shutdown();
    BOOST_REQUIRE(device.Destroy_Texture(target));
    BOOST_REQUIRE(device.Destroy_Texture(depth));
}

BOOST_AUTO_TEST_CASE(line_emission_groups_keep_gaps_and_only_the_last_run_connects_to_the_source)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    submission.Initialize(device, renderer, shadows);
    const auto target = device.Create_Texture({64, 16, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64, 16, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 64, 16, 0, 1}));
    auto description = Description();
    description.geometry_mode = Assets::EmitterGeometryMode::Line;
    description.size.start = .3f;
    description.line_properties.merge_intersections = false;
    EmitterKinematics particles(8, 1000, {}, false, 0);
    const std::array<float, 6> positions{-.85f, -.55f, -.2f, .2f, .55f, .7f};
    for (unsigned index = 0; index < positions.size(); ++index)
        particles.Append_Birth() = {{positions[index], 0, -5}, {}, 0,
            static_cast<std::uint8_t>(index == 2 || index == 3 ? 8 : 7)};
    particles.Advance(1, 0);
    EmitterVisualState visuals(description, 8, 1000, no_scalar, no_color);
    EmitterRenderer emitter(description, MaterialState::AlphaSprite(), 8, 0);
    EmitterDrawInput input;
    input.projection.values = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -.1f, 0, 0, 0, 0, 1};
    input.source_active = true;
    input.source_group = 7;
    input.source_position = {.9f, 0, -5};
    const auto resolve = [](const unsigned *, bool) -> std::optional<PropMaterialTexture> {
        BOOST_FAIL("Untextured emitters must not resolve a texture");
        return std::nullopt;
    };
    for (const unsigned threshold : {0u, 16u}) {
        BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
        BOOST_REQUIRE(emitter.Submit(device, renderer, submission, particles, visuals, threshold, input,
            static_cast<const unsigned *>(nullptr), resolve,
            [](bool) { return no_vector(); }, [](std::size_t) {}));
        BOOST_REQUIRE(submission.Flush_Transparent());
        std::array<std::byte, 64 * 16 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 64 * 4));
        const auto red = [&](unsigned x) { return std::to_integer<int>(pixels[(8 * 64 + x) * 4]); };
        const int visible = threshold == 0 ? 255 : 0;
        BOOST_CHECK_EQUAL(red(9), visible);
        BOOST_CHECK_EQUAL(red(32), visible);
        BOOST_CHECK_EQUAL(red(52), visible);
        BOOST_CHECK_EQUAL(red(58), visible);
        BOOST_CHECK_EQUAL(red(21), 0);
        BOOST_CHECK_EQUAL(red(44), 0);
    }
    submission.Shutdown();
    renderer.Shutdown();
    BOOST_REQUIRE(device.Destroy_Texture(target));
    BOOST_REQUIRE(device.Destroy_Texture(depth));
}

BOOST_AUTO_TEST_CASE(sprite_emitter_atlas_tracks_select_the_authored_frame_after_byte_wrap)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    submission.Initialize(device, renderer, shadows);
    const auto target = device.Create_Texture({32, 32, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32, 32, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 32, 32, 0, 1}));
    std::array<std::uint8_t, 8 * 8 * 4> texels{};
    const std::array<std::array<std::uint8_t, 4>, 4> colors{{
        {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 255}}};
    for (unsigned y = 0; y < 8; ++y) for (unsigned x = 0; x < 8; ++x)
        for (unsigned channel = 0; channel < 4; ++channel)
            texels[(y * 8 + x) * 4 + channel] = colors[(y / 4) * 2 + x / 4][channel];
    const auto texture = device.Create_Texture_Initialized({8, 8}, {std::as_bytes(std::span(texels)), 8 * 4});
    BOOST_REQUIRE(texture.Is_Valid());
    const auto resolve = [](const RHITextureHandle *source, bool) -> std::optional<PropMaterialTexture> {
        return PropMaterialTexture{*source, {}};
    };
    for (unsigned frame = 0; frame < 4; ++frame) {
        BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
        auto description = Description();
        description.geometry_mode = Assets::EmitterGeometryMode::SpriteQuads;
        description.size.start = 1;
        description.atlas.columns = description.atlas.rows = 2;
        description.frame.start = 256;
        description.frame.keys = {{1, 256.0f + frame * 2}};
        EmitterKinematics particles(2, 1000, {}, false, 0);
        particles.Append_Birth() = {{0, 0, -5}, {}, 0, 0};
        particles.Advance(500, 0);
        EmitterVisualState visuals(description, 2, 1000, no_scalar, no_color);
        visuals.Evaluate(particles, 500, 0);
        EmitterRenderer emitter(description, MaterialState::AlphaSprite(), 2, 0);
        EmitterDrawInput input;
        input.projection.values = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -.1f, 0, 0, 0, 0, 1};
        BOOST_REQUIRE(emitter.Submit(device, renderer, submission, particles, visuals, 0, input,
            &texture, resolve, [](bool) { return no_vector(); }, [](std::size_t) {}));
        BOOST_REQUIRE(submission.Flush_Transparent());
        std::array<std::byte, 32 * 32 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
        for (unsigned channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16 * 32 + 16) * 4 + channel])
                - colors[frame][channel], 2);
    }
    submission.Shutdown();
    renderer.Shutdown();
    BOOST_REQUIRE(device.Destroy_Texture(texture));
    BOOST_REQUIRE(device.Destroy_Texture(target));
    BOOST_REQUIRE(device.Destroy_Texture(depth));
}
#endif
