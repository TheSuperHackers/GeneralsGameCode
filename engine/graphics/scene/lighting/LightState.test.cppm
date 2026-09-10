module;

#define BOOST_TEST_MODULE LightStateTests

#include "../../profiling/Tracy.h"
#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

export module Graphics.Scene.Lighting.State.Tests;

import Graphics.Scene.Lighting;
import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Lighting.State;
import Graphics.Scene.RenderScene;
import Graphics.Scene.Props.LightingParameters;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
import Assets.Lights;
import Assets.Math;

using namespace Graphics;

namespace
{

constexpr float kFloatTolerance = 0.00001f;

void Check_Vector(const std::array<float, 3>& actual,
    const std::array<float, 3>& expected)
{
    for (std::size_t axis = 0; axis < actual.size(); ++axis)
        BOOST_CHECK_SMALL(actual[axis] - expected[axis], kFloatTolerance);
}

void Check_Color(const std::array<float, 3>& actual,
    const std::array<float, 3>& expected)
{
    Check_Vector(actual, expected);
}

void Check_Vector(const Assets::Vector3f& actual,
    const std::array<float, 3>& expected)
{
    BOOST_CHECK_SMALL(actual.x - expected[0], kFloatTolerance);
    BOOST_CHECK_SMALL(actual.y - expected[1], kFloatTolerance);
    BOOST_CHECK_SMALL(actual.z - expected[2], kFloatTolerance);
}

void Check_Light_Source(const MaterialLightSource& source,
    RenderLightType type,
    const std::array<float, 3>& position,
    const std::array<float, 3>& direction,
    const std::array<float, 3>& ambient,
    const std::array<float, 3>& diffuse,
    float intensity,
    double attenuation_start,
    double attenuation_end,
    bool attenuate,
    float cone_cosine)
{
    BOOST_CHECK(source.type == type);
    Check_Vector(source.position, position);
    Check_Vector(source.direction, direction);
    Check_Color(source.ambient, ambient);
    Check_Color(source.diffuse, diffuse);
    BOOST_CHECK_SMALL(source.intensity - intensity, kFloatTolerance);
    BOOST_CHECK_SMALL(static_cast<float>(source.attenuation_start - attenuation_start), kFloatTolerance);
    BOOST_CHECK_SMALL(static_cast<float>(source.attenuation_end - attenuation_end), kFloatTolerance);
    BOOST_CHECK_EQUAL(source.attenuate, attenuate);
    BOOST_CHECK_SMALL(source.cone_cosine - cone_cosine, kFloatTolerance);
}

}

BOOST_AUTO_TEST_CASE(light_state_defaults_match_authored_ww3d2_values)
{
    const LightState state;

    BOOST_CHECK(state.authored.type == Assets::LightType::Point);
    BOOST_CHECK(!state.authored.cast_shadows);
    BOOST_CHECK_SMALL(state.authored.intensity - 1.0f, kFloatTolerance);
    Check_Vector(state.authored.ambient, {1.0f, 1.0f, 1.0f});
    Check_Vector(state.authored.diffuse, {1.0f, 1.0f, 1.0f});
    Check_Vector(state.authored.specular, {1.0f, 1.0f, 1.0f});
    BOOST_CHECK(!state.authored.near_attenuation_enabled);
    BOOST_CHECK_SMALL(state.authored.near_attenuation_start, kFloatTolerance);
    BOOST_CHECK_SMALL(state.authored.near_attenuation_end, kFloatTolerance);
    BOOST_CHECK(!state.authored.far_attenuation_enabled);
    BOOST_CHECK_SMALL(state.authored.far_attenuation_start - 50.0f, kFloatTolerance);
    BOOST_CHECK_SMALL(state.authored.far_attenuation_end - 100.0f, kFloatTolerance);
    BOOST_CHECK_SMALL(state.authored.spot_angle - 0.7853981633974483f, kFloatTolerance);
    BOOST_CHECK_SMALL(state.spot_angle_cosine - 0.707f, kFloatTolerance);
    BOOST_CHECK_SMALL(state.authored.spot_exponent - 1.0f, kFloatTolerance);
    Check_Vector(state.authored.spot_direction, {0.0f, 0.0f, 1.0f});
}

BOOST_AUTO_TEST_CASE(light_conversion_preserves_affine_world_directions_and_authored_values)
{
    // Matrix3D and RenderTransform use three affine rows followed by a
    // translation column. This rotated, unequally scaled transform keeps the
    // same convention at the graphics boundary as the game adapter.
    const RenderTransform transform{{
        2.0f, -1.0f, 0.5f, 13.0f,
        1.0f, 3.0f, -0.75f, -7.0f,
        0.5f, 0.25f, 4.0f, 5.0f,
        0.0f, 0.0f, 0.0f, 1.0f}};
    const std::array<float, 3> position{13.0f, -7.0f, 5.0f};
    // Directional and point lights use the negative world-Z basis exactly as
    // the retired renderer did. It intentionally remains non-normalized.
    const std::array<float, 3> negative_world_z{-0.5f, 0.75f, -4.0f};

    LightState state;
    state.authored.ambient = {0.11f, 0.22f, 0.33f};
    state.authored.diffuse = {0.41f, 0.52f, 0.63f};
    state.authored.specular = {0.71f, 0.82f, 0.93f};
    state.authored.intensity = 2.5f;
    state.authored.far_attenuation_enabled = true;
    state.authored.far_attenuation_start = 12.0f;
    state.authored.far_attenuation_end = 80.0f;
    state.authored.spot_angle = 0.61f;
    state.spot_angle_cosine = 0.819f;
    state.authored.spot_exponent = 3.5f;

    state.authored.type = Assets::LightType::Directional;
    const auto directional = Make_Material_Light(state, transform);
    Check_Light_Source(directional, RenderLightType::Directional, position,
        negative_world_z, {0.11f, 0.22f, 0.33f}, {0.41f, 0.52f, 0.63f}, state.authored.intensity,
        12.0, 80.0, true, state.spot_angle_cosine);
    BOOST_CHECK_SMALL(directional.direction[0] * directional.direction[0]
        + directional.direction[1] * directional.direction[1]
        + directional.direction[2] * directional.direction[2] - 16.8125f,
        kFloatTolerance);

    state.authored.type = Assets::LightType::Point;
    const auto point = Make_Material_Light(state, transform);
    Check_Light_Source(point, RenderLightType::Point, position,
        negative_world_z, {0.11f, 0.22f, 0.33f}, {0.41f, 0.52f, 0.63f}, state.authored.intensity,
        12.0, 80.0, true, state.spot_angle_cosine);
    state.authored.far_attenuation_enabled = false;
    const auto point_without_far_attenuation = Make_Material_Light(state, transform);
    BOOST_CHECK(!point_without_far_attenuation.attenuate);
    state.authored.far_attenuation_enabled = true;

    // The authored spot cone starts in local space. Applying the affine basis
    // gives this world vector; its length must remain intact for the retained
    // cone calculation in LocalLighting.
    state.authored.type = Assets::LightType::Spot;
    state.authored.spot_direction = {1.5f, -2.0f, 0.5f};
    const std::array<float, 3> spot_world_direction{
        5.25f, -4.875f, 2.25f};
    const auto spot = Make_Material_Light(state, transform);
    Check_Light_Source(spot, RenderLightType::Spot, position,
        spot_world_direction, {0.11f, 0.22f, 0.33f}, {0.41f, 0.52f, 0.63f}, state.authored.intensity,
        12.0, 80.0, true, state.spot_angle_cosine);
    BOOST_CHECK_SMALL(spot.direction[0] * spot.direction[0]
        + spot.direction[1] * spot.direction[1]
        + spot.direction[2] * spot.direction[2] - 56.390625f,
        kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(light_state_copy_and_assignment_do_not_share_authored_state)
{
    LightState source;
    source.authored.type = Assets::LightType::Spot;
    source.authored.far_attenuation_enabled = true;
    source.authored.cast_shadows = true;
    source.authored.intensity = 2.0f;
    source.authored.ambient = {0.1f, 0.2f, 0.3f};
    source.authored.diffuse = {0.4f, 0.5f, 0.6f};
    source.authored.specular = {0.7f, 0.8f, 0.9f};
    source.authored.near_attenuation_start = 1.0f;
    source.authored.near_attenuation_end = 2.0f;
    source.authored.far_attenuation_start = 12.0f;
    source.authored.far_attenuation_end = 34.0f;
    source.authored.spot_angle = 0.4f;
    source.spot_angle_cosine = 0.92f;
    source.authored.spot_exponent = 4.0f;
    source.authored.spot_direction = {2.0f, 3.0f, 4.0f};

    LightState copied = source;
    copied.authored.ambient.x = 9.0f;
    copied.authored.diffuse.y = 8.0f;
    copied.authored.spot_direction.z = 7.0f;
    copied.authored.far_attenuation_enabled = false;
    copied.authored.cast_shadows = false;
    copied.spot_angle_cosine = 0.1f;
    BOOST_CHECK_SMALL(source.authored.ambient.x - 0.1f, kFloatTolerance);
    BOOST_CHECK_SMALL(source.authored.diffuse.y - 0.5f, kFloatTolerance);
    BOOST_CHECK_SMALL(source.authored.spot_direction.z - 4.0f, kFloatTolerance);
    BOOST_CHECK(source.authored.far_attenuation_enabled);
    BOOST_CHECK(source.authored.cast_shadows);
    BOOST_CHECK_SMALL(source.spot_angle_cosine - 0.92f, kFloatTolerance);

    LightState assigned;
    assigned = source;
    assigned.authored.specular.z = 6.0f;
    assigned.authored.far_attenuation_end = 99.0f;
    assigned.authored.spot_direction.x = -3.0f;
    assigned.spot_angle_cosine = 0.2f;
    BOOST_CHECK_SMALL(source.authored.specular.z - 0.9f, kFloatTolerance);
    BOOST_CHECK_SMALL(source.authored.far_attenuation_end - 34.0f, kFloatTolerance);
    BOOST_CHECK_SMALL(source.authored.spot_direction.x - 2.0f, kFloatTolerance);
    BOOST_CHECK_SMALL(source.spot_angle_cosine - 0.92f, kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(converted_lights_reach_prop_pixels_and_spot_cones_keep_directional_sampling)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,
        Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));

    std::array<PropVertex, 4> vertices{};
    vertices[0].position = {-1.0f, -1.0f, 0.5f};
    vertices[1].position = {1.0f, -1.0f, 0.5f};
    vertices[2].position = {1.0f, 1.0f, 0.5f};
    vertices[3].position = {-1.0f, 1.0f, 0.5f};
    for (auto& vertex : vertices) {
        vertex.normal = {0.0f, 0.0f, 1.0f};
        vertex.material_ambient = {1.0f, 1.0f, 1.0f, 1.0f};
        vertex.material_diffuse = {1.0f, 1.0f, 1.0f, 0.6f};
    }
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(mesh.Is_Valid());

    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.depth_test = false;
    style.depth_write = false;

    const RenderTransform rotated_transform{{
        2.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 3.0f, 0.0f, 0.0f,
        0.5f, 0.25f, -1.0f, 2.5f,
        0.0f, 0.0f, 0.0f, 1.0f}};
    const RenderTransform identity_transform{{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 2.5f,
        0.0f, 0.0f, 0.0f, 1.0f}};

    PropParameters parameters;
    parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1};
    parameters.textured = 0;
    parameters.camera_position = {0, 0, 10, 1};

    auto make_target = [&]() {
        return device.Create_Texture({1, 1, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    };
    auto make_depth = [&]() {
        return device.Create_Texture({1, 1, 1, RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    };

    auto target = make_target();
    auto depth = make_depth();
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());

    auto draw_case = [&](const LightState& state,
        const RenderTransform& transform,
        std::array<float, 3> center,
        std::array<float, 3> scene_ambient,
        std::array<float, 3> expected,
        bool expected_point) {
        LocalLighting lighting;
        lighting.Reset(center, scene_ambient);
        {
            // The source is deliberately destroyed before material parameters
            // are uploaded, proving LocalLighting owns the converted sample.
            const auto source = Make_Material_Light(state, transform);
            lighting.Add(source);
        }
        BOOST_REQUIRE_EQUAL(lighting.count, 1u);
        BOOST_CHECK_EQUAL(lighting.lights[0].point, expected_point);
        lighting.Finalize();
        Set_Prop_Lighting(parameters, &lighting);

        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0, 0, 1, 1}));
        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));
        std::array<std::byte, 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 4));
        for (unsigned channel = 0; channel < 3; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[channel])
                - expected[channel] * 255.0f, 1.1f);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3]) - 153, 1);
    };

    LightState directional;
    directional.authored.type = Assets::LightType::Directional;
    directional.authored.ambient = {0.08f, 0.05f, 0.02f};
    directional.authored.diffuse = {0.31f, 0.17f, 0.09f};
    // Directional lighting intentionally retains the old path's unscaled
    // authored diffuse value even when intensity is greater than one.
    directional.authored.intensity = 2.0f;
    draw_case(directional, rotated_transform, {0.0f, 0.0f, 0.5f},
        {0.02f, 0.03f, 0.04f}, {0.41f, 0.25f, 0.15f}, false);

    LightState point;
    point.authored.type = Assets::LightType::Point;
    point.authored.ambient = {0.1f, 0.2f, 0.3f};
    point.authored.diffuse = {0.6f, 0.4f, 0.2f};
    point.authored.intensity = 0.5f;
    point.authored.far_attenuation_enabled = true;
    point.authored.far_attenuation_start = 2.0f;
    point.authored.far_attenuation_end = 10.0f;
    // LocalLighting contributes sampled ambient to SceneAmbient, while the
    // prop shader attenuates that same point ambient together with diffuse.
    // At distance two the shader denominator is 1 + .05*2 + .08*4 = 1.42.
    draw_case(point, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.02f, 0.03f, 0.04f},
        {0.02f + 0.05f + (0.3f + 0.05f) / 1.42f,
            0.03f + 0.1f + (0.2f + 0.1f) / 1.42f,
            0.04f + 0.15f + (0.1f + 0.15f) / 1.42f}, true);

    LightState spot_inside;
    spot_inside.authored.type = Assets::LightType::Spot;
    spot_inside.authored.ambient = {0.1f, 0.04f, 0.02f};
    spot_inside.authored.diffuse = {0.4f, 0.2f, 0.1f};
    spot_inside.authored.intensity = 0.5f;
    spot_inside.authored.far_attenuation_enabled = true;
    spot_inside.authored.far_attenuation_start = 2.0f;
    spot_inside.authored.far_attenuation_end = 10.0f;
    spot_inside.authored.spot_direction = {0.0f, 0.0f, -1.0f};
    spot_inside.spot_angle_cosine = 0.5f;
    draw_case(spot_inside, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.05f, 0.025f, 0.01f}, {0.3f, 0.145f, 0.07f}, false);

    LightState spot_outside = spot_inside;
    spot_outside.authored.spot_direction = {0.0f, 0.0f, 1.0f};
    // The outside-cone source remains a directional sample but contributes no
    // light, so only the scene ambient reaches the pixel.
    draw_case(spot_outside, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.05f, 0.025f, 0.01f}, {0.05f, 0.025f, 0.01f}, false);

    // Exercise the same converted cases after both render targets and the
    // renderer's GPU resources have been recreated.
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
    BOOST_REQUIRE(renderer.Initialize(device,
        Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    target = make_target();
    depth = make_depth();
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());
    draw_case(directional, rotated_transform, {0.0f, 0.0f, 0.5f},
        {0.02f, 0.03f, 0.04f}, {0.41f, 0.25f, 0.15f}, false);
    draw_case(point, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.02f, 0.03f, 0.04f},
        {0.02f + 0.05f + (0.3f + 0.05f) / 1.42f,
            0.03f + 0.1f + (0.2f + 0.1f) / 1.42f,
            0.04f + 0.15f + (0.1f + 0.15f) / 1.42f}, true);
    draw_case(spot_inside, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.05f, 0.025f, 0.01f}, {0.3f, 0.145f, 0.07f}, false);
    draw_case(spot_outside, identity_transform, {0.0f, 0.0f, 0.5f},
        {0.05f, 0.025f, 0.01f}, {0.05f, 0.025f, 0.01f}, false);

    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}
