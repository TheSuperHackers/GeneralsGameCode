module;
#define BOOST_TEST_MODULE ModelPlaybackTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
export module Graphics.Scene.Models.Playback.Tests;
import Graphics.Scene.Models.Playback;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.AffineTransform;
import Assets.Cache.Animations;
import Assets.ModelRig;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;

using namespace Graphics;
namespace {
Assets::AnimationAssetHandle Clip(Assets::AnimationCache& cache, unsigned frames = 5) {
    Assets::ModelAnimationDesc description;
    description.name = "MOVE";
    description.skeleton_name = "RIG";
    description.frame_count = frames;
    description.frame_rate = 1;
    std::string error;
    const auto clip = cache.Publish(description, 2, Assets::AnimationSampling::Consecutive, error);
    BOOST_REQUIRE_MESSAGE(clip, error);
    return clip;
}
}

BOOST_AUTO_TEST_CASE(endpoint_wrapping_and_large_deltas_preserve_frame_authored_rules) {
    Assets::AnimationCache cache;
    const auto clip = Clip(cache);
    ModelPlayback playback(cache);
    struct Expected { ModelPlaybackMode mode; float start; unsigned elapsed; float frame; float direction; };
    const std::array cases{
        Expected{ModelPlaybackMode::Manual, 2, 100000, 2, 1},
        Expected{ModelPlaybackMode::Once, 0, 5000, 4, 1},
        Expected{ModelPlaybackMode::Loop, 0, 4000, 0, 1},
        Expected{ModelPlaybackMode::Loop, 0, 7000, 3, 1},
        Expected{ModelPlaybackMode::Loop, 0, 9000, 0, 1},
        Expected{ModelPlaybackMode::OnceBackwards, 4, 5000, 0, -1},
        Expected{ModelPlaybackMode::LoopBackwards, 4, 4000, 0, -1},
        Expected{ModelPlaybackMode::LoopBackwards, 4, 5000, 3, -1},
        Expected{ModelPlaybackMode::LoopBackwards, 4, 9000, 4, -1},
        Expected{ModelPlaybackMode::PingPong, 0, 4000, 4, -1},
        Expected{ModelPlaybackMode::PingPong, 0, 6000, 2, -1},
        Expected{ModelPlaybackMode::PingPong, 0, 10000, -2, -1}
    };
    for (const auto& expected : cases) {
        playback.Set(clip, expected.start, expected.mode, 100);
        float direction = 0;
        // Millisecond-to-frame multiplication retains float rounding before
        // wrapping. Keep endpoint expectations independent of that arithmetic.
        BOOST_CHECK_SMALL(playback.Current_Frame(100 + expected.elapsed, &direction) - expected.frame, .000001f);
        BOOST_CHECK_EQUAL(direction, expected.direction);
        BOOST_CHECK_EQUAL(playback.Frame(), expected.start);
    }
    playback.Set(clip, 0, ModelPlaybackMode::PingPong, 0);
    playback.Advance(4000);
    playback.Advance(9000);
    BOOST_CHECK_EQUAL(playback.Frame(), 1);
    playback.Advance(10000);
    BOOST_CHECK_EQUAL(playback.Frame(), 2);
    playback.Set(clip, 0, ModelPlaybackMode::PingPong, 0);
    playback.Advance(4000);
    playback.Advance(13000);
    BOOST_CHECK_EQUAL(playback.Frame(), 0);
}

BOOST_AUTO_TEST_CASE(published_completion_multiplier_and_unsigned_clock_are_explicit) {
    Assets::AnimationCache cache;
    const auto clip = Clip(cache);
    ModelPlayback playback(cache);
    playback.Set(clip, 0, ModelPlaybackMode::Once, 0);
    BOOST_CHECK_EQUAL(playback.Current_Frame(5000), 4);
    BOOST_CHECK(!playback.Is_Complete());
    playback.Advance(5000);
    BOOST_CHECK(playback.Is_Complete());
    playback.Set(clip, 0, ModelPlaybackMode::Once, 0xfffffe0cu);
    playback.Set_Multiplier(2);
    playback.Advance(500);
    BOOST_CHECK_EQUAL(playback.Frame(), 2);
    playback.Advance(500);
    BOOST_CHECK_EQUAL(playback.Frame(), 2);
    playback.Set_Multiplier(-1);
    playback.Advance(1500);
    BOOST_CHECK_EQUAL(playback.Frame(), 1);
    playback.Set(clip, 0, ModelPlaybackMode::Once, 0);
    BOOST_CHECK_EQUAL(playback.Multiplier(), 1);
}

BOOST_AUTO_TEST_CASE(replacement_blending_and_reset_balance_aliased_clip_ownership) {
    Assets::AnimationCache cache;
    const auto clip = Clip(cache);
    {
        ModelPlayback playback(cache);
        playback.Set(clip, 2, ModelPlaybackMode::Manual, 0);
        cache.Clear();
        BOOST_CHECK_EQUAL(cache.Reference_Count(clip), 1);
        playback.Set(clip, 3, ModelPlaybackMode::Loop, 0);
        BOOST_CHECK_EQUAL(cache.Reference_Count(clip), 1);
        playback.Blend(clip, 0, clip, 4, .5f);
        BOOST_CHECK_EQUAL(cache.Reference_Count(clip), 2);
        BOOST_CHECK(!playback.Clip());
        BOOST_CHECK(!playback.Is_Advancing());
        playback.Set(clip, 0, ModelPlaybackMode::Once, 0);
        BOOST_CHECK_EQUAL(cache.Reference_Count(clip), 1);
        playback.Reset();
        BOOST_CHECK(!cache.Resolve(clip));
    }
}

BOOST_AUTO_TEST_CASE(single_frame_clips_keep_zero_endpoint_playback) {
    Assets::AnimationCache cache;
    const auto clip = Clip(cache, 1);
    ModelPlayback playback(cache);
    for (const auto mode : {ModelPlaybackMode::Once, ModelPlaybackMode::Loop,
        ModelPlaybackMode::OnceBackwards, ModelPlaybackMode::LoopBackwards}) {
        playback.Set(clip, 0, mode, 0);
        playback.Advance(10000);
        BOOST_CHECK_EQUAL(playback.Frame(), 0);
    }
}

BOOST_AUTO_TEST_CASE(blended_pose_keeps_translation_rotation_visibility_and_source_lifetime) {
    Assets::AnimationCache cache;
    Assets::ModelAnimationDesc first;
    first.name = "FIRST";
    first.skeleton_name = "RIG";
    first.frame_count = 1;
    first.frame_rate = 1;
    first.channels = {
        {1, Assets::ModelChannelComponent::TranslationX, 0, true, {{2,0,0,0}}},
        {1, Assets::ModelChannelComponent::Visibility, 0, true, {{0,0,0,0}}}
    };
    auto second = first;
    second.name = "SECOND";
    second.channels = {
        {1, Assets::ModelChannelComponent::TranslationX, 0, true, {{6,0,0,0}}},
        {1, Assets::ModelChannelComponent::Rotation, 0, true, {{0,0,1,0}}},
        {1, Assets::ModelChannelComponent::Visibility, 0, true, {{1,0,0,0}}}
    };
    std::string error;
    const auto a = cache.Publish(first, 2, Assets::AnimationSampling::Consecutive, error);
    BOOST_REQUIRE_MESSAGE(a, error);
    const auto b = cache.Publish(second, 2, Assets::AnimationSampling::Consecutive, error);
    BOOST_REQUIRE_MESSAGE(b, error);
    ModelPlayback playback(cache);
    playback.Blend(a, 0, b, 0, .5f);
    cache.Clear();
    first.channels.clear();
    second.channels.clear();
    Assets::ModelRigDesc rig;
    rig.skeleton_name = "RIG";
    rig.bones = {{"ROOT"}, {"JOINT", 0, {1,0,0}}, {"TIP", 1, {1,0,0}}};
    ModelHierarchy hierarchy(rig);
    auto root = Affine_Identity();
    root.matrix[3] = 10;
    playback.Evaluate(hierarchy, root, 50000);
    BOOST_CHECK_SMALL(hierarchy.World_Transform(1).matrix[3] - 15.f, .00001f);
    BOOST_CHECK_SMALL(hierarchy.World_Transform(2).matrix[3] - 15.f, .00001f);
    BOOST_CHECK_SMALL(hierarchy.World_Transform(2).matrix[7] - 1.f, .00001f);
    BOOST_CHECK(hierarchy.Visible(1));
    RenderTransform query;
    BOOST_CHECK(!playback.Evaluate_Bone(hierarchy, 2, 0, root, query));
    BOOST_CHECK(query.matrix == root.matrix);
    playback.Blend(b, 0, a, 0, .5f);
    playback.Evaluate(hierarchy, root, 50000);
    BOOST_CHECK(hierarchy.Visible(1));
    BOOST_CHECK_SMALL(hierarchy.World_Transform(2).matrix[3] - 15.f, .00001f);
    playback.Reset();
    BOOST_CHECK(!cache.Resolve(a));
    BOOST_CHECK(!cache.Resolve(b));
    playback.Evaluate(hierarchy, root, 50000);
    BOOST_CHECK_EQUAL(hierarchy.World_Transform(2).matrix[3], 12.f);
}

BOOST_AUTO_TEST_CASE(playback_pose_drives_visible_geometry_after_cache_release_and_target_recreation) {
    Assets::AnimationCache cache;
    Assets::ModelAnimationDesc clip;
    clip.name = "MOVE";
    clip.skeleton_name = "RIG";
    clip.frame_count = 3;
    clip.frame_rate = 1;
    clip.channels = {
        {1, Assets::ModelChannelComponent::TranslationX, 0, true,
            {{-.5f,0,0,0},{0,0,0,0},{.5f,0,0,0}}},
        {1, Assets::ModelChannelComponent::Visibility, 0, true,
            {{1,0,0,0},{0,0,0,0},{1,0,0,0}}}
    };
    std::string error;
    const auto handle = cache.Publish(clip, 2, Assets::AnimationSampling::Consecutive, error);
    BOOST_REQUIRE_MESSAGE(handle, error);
    ModelPlayback playback(cache);
    playback.Set(handle, 0, ModelPlaybackMode::Once, 0);
    cache.Clear();
    Assets::ModelRigDesc rig;
    rig.skeleton_name = "RIG";
    rig.bones = {{"ROOT"}, {"MOVING", 0}};
    ModelHierarchy hierarchy(rig);
    for (bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands = device.Immediate_Command_List();
        PropStyle style;
        style.depth_test = style.depth_write = false;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for (unsigned width : {32u, 64u, 32u}) {
            const auto target = device.Create_Texture({width,16,1,RHITextureFormat::RGBA8_UNorm,
                static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth = device.Create_Texture({width,16,1,RHITextureFormat::D32_Float,
                static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,width,16}));
            for (bool reverse : {false, true}) {
                playback.Set(handle, reverse ? 2.f : 0.f,
                    reverse ? ModelPlaybackMode::OnceBackwards : ModelPlaybackMode::Once, 0);
                for (unsigned time : {0u, 1000u, 2000u}) {
                    playback.Evaluate(hierarchy, Affine_Identity(), time);
                    BOOST_REQUIRE(commands.Clear({0,0,0,0}, 1));
                    if (hierarchy.Visible(1)) {
                        std::array<PropVertex,3> vertices{};
                        vertices[0].position = {-.14f,-.7f,.5f};
                        vertices[1].position = {.14f,-.7f,.5f};
                        vertices[2].position = {0,.7f,.5f};
                        for (auto& vertex : vertices) {
                            vertex.position[0] += hierarchy.World_Transform(1).matrix[3];
                            vertex.color = {1,0,0,1};
                        }
                        const auto mesh = renderer.Create_Mesh(vertices, std::array<std::uint32_t,3>{0,1,2});
                        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));
                        renderer.Destroy_Mesh(mesh);
                    }
                    std::vector<std::byte> pixels(width * 16 * 4);
                    BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
                    const unsigned frame = reverse ? 2 - time / 1000 : time / 1000;
                    for (unsigned column : {width/4, width/2, width*3/4}) {
                        const bool covered = frame != 1 && column == (frame == 0 ? width/4 : width*3/4);
                        BOOST_TEST_CONTEXT("warp=" << warp << " width=" << width << " reverse=" << reverse
                            << " time=" << time << " column=" << column) {
                            const auto offset = (8 * width + column) * 4;
                            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]), covered ? 255u : 0u);
                            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]), 0u);
                            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+3]), covered ? 255u : 0u);
                        }
                    }
                }
            }
            device.Destroy_Texture(target);
            device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
