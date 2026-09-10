module;
#define BOOST_TEST_MODULE LocalLightingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
export module Graphics.Scene.Lighting.Local.Tests;
import Graphics.Scene.Lighting;
import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Props.LightingParameters;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(selection_keeps_strongest_lights_and_ambient_from_every_source)
{
    LocalLighting lighting;
    lighting.Reset({},{0.1f,0.2f,0.3f});
    for (int i=0;i<8;++i) {
        MaterialLightSource source;
        source.direction={static_cast<float>(i),0,1};
        source.diffuse={0.1f*(i+1),0,0};
        source.ambient={0.02f,0.03f,0.04f};
        source.intensity=0; // Authored directional colors already carry intensity.
        lighting.Add(source);
    }
    lighting.Finalize();
    BOOST_REQUIRE_EQUAL(lighting.count,4u);
    for (unsigned i=0;i<4;++i) BOOST_CHECK_EQUAL(lighting.lights[i].direction[0],7-i);
    BOOST_CHECK_SMALL(lighting.ambient[0]-0.26f,0.00001f);
    BOOST_CHECK_SMALL(lighting.ambient[1]-0.44f,0.00001f);
    BOOST_CHECK_SMALL(lighting.ambient[2]-0.62f,0.00001f);
}

BOOST_AUTO_TEST_CASE(equal_contributions_keep_submission_order_and_near_black_is_rejected)
{
    LocalLighting lighting;
    for (int i=0;i<6;++i) {
        MaterialLightSource source;
        source.direction={static_cast<float>(i),0,1}; source.diffuse={0.2f,0.2f,0.2f};
        lighting.Add(source);
    }
    MaterialLightSource black;
    black.diffuse={0.049f,0.049f,0.049f}; black.ambient={1,1,1}; black.intensity=100;
    lighting.Add(black);
    BOOST_REQUIRE_EQUAL(lighting.count,4u);
    for (unsigned i=0;i<4;++i) BOOST_CHECK_EQUAL(lighting.lights[i].direction[0],i);
    BOOST_CHECK((lighting.ambient==std::array<float,3>{}));
}

BOOST_AUTO_TEST_CASE(point_samples_retain_raw_colors_and_range_for_per_pixel_lighting)
{
    LocalLighting lighting;
    lighting.Reset({10,20,30},{});
    MaterialLightSource point;
    point.type=RenderLightType::Point; point.position={10,20,36};
    point.diffuse={0.8f,0.4f,0.2f}; point.ambient={0.2f,0.1f,0.05f}; point.intensity=0.5f;
    point.attenuate=true; point.attenuation_start=2; point.attenuation_end=10;
    lighting.Add(point);
    BOOST_REQUIRE_EQUAL(lighting.count,1u);
    const auto& sample=lighting.lights[0];
    BOOST_CHECK(sample.point);
    BOOST_CHECK((sample.position==point.position));
    BOOST_CHECK((sample.direction==std::array{0.0f,0.0f,1.0f}));
    BOOST_CHECK_SMALL(sample.diffuse[0]-0.2f,0.00001f);
    BOOST_CHECK_SMALL(sample.source_diffuse[0]-0.4f,0.00001f);
    BOOST_CHECK_SMALL(sample.source_ambient[0]-0.1f,0.00001f);
    BOOST_CHECK_SMALL(lighting.ambient[0]-0.05f,0.00001f);
    BOOST_CHECK_EQUAL(sample.inner_radius,2);
    BOOST_CHECK_EQUAL(sample.outer_radius,10);
}

BOOST_AUTO_TEST_CASE(step_attenuation_and_coincident_points_remain_finite)
{
    for (float distance : {0.0f,2.0f,2.01f}) {
        LocalLighting lighting;
        MaterialLightSource point;
        point.type=RenderLightType::Point; point.position={0,0,distance};
        point.diffuse={0.4f,0.2f,0.1f}; point.attenuate=true;
        point.attenuation_start=point.attenuation_end=2;
        lighting.Add(point);
        BOOST_REQUIRE_EQUAL(lighting.count,1u);
        BOOST_CHECK_SMALL(lighting.lights[0].diffuse[0]-(distance>2 ? 0 : 0.4f),0.00001f);
        BOOST_CHECK_EQUAL(lighting.lights[0].direction[2],distance>0 ? 1 : 0);
    }
}

BOOST_AUTO_TEST_CASE(spot_cones_sample_in_world_space_and_finalize_clamps_ambient)
{
    for (const auto direction : {std::array{0.0f,0.0f,-1.0f},std::array{0.0f,0.0f,1.0f}}) {
        LocalLighting lighting;
        lighting.Reset({},{-0.1f,0.9f,2});
        MaterialLightSource spot;
        spot.type=RenderLightType::Spot; spot.position={0,0,2};
        spot.direction=direction; spot.cone_cosine=0.5f;
        spot.diffuse={0.4f,0.2f,0.1f}; spot.ambient={0,0.4f,0}; spot.intensity=0.5f;
        lighting.Add(spot); lighting.Finalize();
        BOOST_REQUIRE_EQUAL(lighting.count,1u);
        BOOST_CHECK(!lighting.lights[0].point);
        BOOST_CHECK_SMALL(lighting.lights[0].diffuse[0]-(direction[2]<0 ? 0.2f : 0),0.00001f);
        BOOST_CHECK_EQUAL(lighting.ambient[0],0);
        BOOST_CHECK_EQUAL(lighting.ambient[2],1);
        BOOST_CHECK_SMALL(lighting.ambient[1]-(direction[2]<0 ? 1 : 0.9f),0.00001f);
    }
}

BOOST_AUTO_TEST_CASE(sampled_lights_draw_rgb_and_alpha_and_clear_previous_draw_state)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,1,1}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal={0,0,1};
        vertex.material_ambient={1,1,1,1};
        vertex.material_diffuse={1,1,1,0.6f};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured=0; parameters.camera_position={0,0,10,1};
    LocalLighting lighting;
    auto draw=[&](std::array<float,3> expected) {
        lighting.Finalize();
        Set_Prop_Lighting(parameters,&lighting);
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,4));
        for (unsigned channel=0;channel<3;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[channel])-expected[channel]*255,1.1f);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-153,1);
    };
    lighting.Reset({0,0,0.5f},{0.05f,0.025f,0.01f});
    MaterialLightSource source;
    source.direction={0,0,1}; source.diffuse={0.4f,0.2f,0.1f};
    lighting.Add(source);
    draw({0.45f,0.225f,0.11f});
    lighting.Reset({0,0,0.5f},{0.05f,0.025f,0.01f});
    source.type=RenderLightType::Point; source.position={0,0,2.5f};
    source.ambient={0.1f,0.04f,0.02f}; source.intensity=0.5f;
    source.attenuate=true; source.attenuation_start=2; source.attenuation_end=10;
    lighting.Add(source);
    // At distance two, ambient sampling is unattenuated and pixel attenuation
    // is 1/(1+0.05*2+0.08*4). Raw point colors must not be attenuated twice.
    draw({0.1f+0.25f/1.42f,0.045f+0.12f/1.42f,0.02f+0.06f/1.42f});
    lighting.Reset({},{0.12f,0.08f,0.04f});
    draw({0.12f,0.08f,0.04f});
    lighting.Reset({0,0,0.5f},{0.05f,0.025f,0.01f});
    source.type=RenderLightType::Spot; source.direction={0,0,-1}; source.cone_cosine=0.5f;
    lighting.Add(source);
    draw({0.3f,0.145f,0.07f});
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
