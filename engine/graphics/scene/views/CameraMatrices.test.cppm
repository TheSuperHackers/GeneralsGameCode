module;
#define BOOST_TEST_MODULE CameraMatricesTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
export module Graphics.Scene.Views.CameraMatrices.Tests;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.MaterialPassQueue;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(instance_transforms_and_perspective_camera_remain_independent_when_queued)
{
    for (bool software : {true,false}) {
        GraphicsTestDevice device({software});
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target = device.Create_Texture({64,64,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({64,64,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,64,64}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        std::array<PropVertex,4> vertices{};
        vertices[0].position = {-0.2f,-0.2f,0.5f}; vertices[1].position = {0.2f,-0.2f,0.5f};
        vertices[2].position = {0.2f,0.2f,0.5f}; vertices[3].position = {-0.2f,0.2f,0.5f};
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        for (auto& vertex : vertices) vertex.color = {1,0,0,0.5f};
        const auto red = renderer.Create_Mesh(vertices,indices);
        for (auto& vertex : vertices) vertex.color = {0,1,0,0.75f};
        const auto green = renderer.Create_Mesh(vertices,indices);
        auto& camera = Get_Camera_Matrices();
        const auto saved = camera;
        camera.view.values = {0,-1,0,0.25f,1,0,0,0,0,0,1,0,0,0,0,1};
        camera.projection.values = {2,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1};
        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view = camera.view.values;
        parameters.view_projection = Compose_Matrices(camera.projection,camera.view).values;
        MaterialPassQueue queue;
        parameters.world[3] = -0.5f;
        BOOST_REQUIRE(queue.Submit(renderer,red,style,parameters,{}));
        parameters.world[3] = 0.5f;
        BOOST_REQUIRE(queue.Submit(renderer,green,style,parameters,{}));
        // A later view and object cannot change either recorded draw.
        camera = {};
        parameters.world[3] = 10;
        parameters.view_projection = {};
        BOOST_REQUIRE(queue.Flush(commands));
        std::array<std::byte,64*64*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
        const auto check = [&](unsigned x,unsigned y,std::array<int,4> expected) {
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*64+x)*4+channel])-expected[channel],2);
        };
        // Rotation, translation, and perspective divide put the centers here.
        check(42,42,{255,0,0,128});
        check(42,21,{0,255,0,191});
        check(21,42,{0,0,0,0});
        check(32,32,{0,0,0,0});
        camera = saved;
        renderer.Destroy_Mesh(red); renderer.Destroy_Mesh(green);
        renderer.Shutdown();
        device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(camera_space_effects_do_not_change_main_or_reflected_object_placement)
{
    for (bool software : {true,false}) {
        GraphicsTestDevice device({software});
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        std::array<PropVertex,4> vertices{};
        vertices[0].position = {-0.15f,-0.15f,0.5f}; vertices[1].position = {0.15f,-0.15f,0.5f};
        vertices[2].position = {0.15f,0.15f,0.5f}; vertices[3].position = {-0.15f,0.15f,0.5f};
        for (auto& vertex : vertices) vertex.color = {1,1,1,0.5f};
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        const auto mesh = renderer.Create_Mesh(vertices,indices);
        CameraMatrices main;
        main.view.values[3] = -0.5f;
        CameraMatrices reflected = main;
        reflected.view.values[0] = -1;
        reflected.view.values[3] = 0.5f;
        MaterialPassQueue queue;
        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        style.cull = RHICullMode::Back;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view_projection = Compose_Matrices(main.projection,main.view).values;
        style.front_counter_clockwise = true;
        BOOST_REQUIRE(queue.Submit(renderer,mesh,style,parameters,{}));
        // An effect supplies camera-space geometry and projection directly.
        parameters.view_projection = main.projection.values;
        parameters.world[7] = 0.5f;
        BOOST_REQUIRE(queue.Submit(renderer,mesh,style,parameters,{}));
        parameters.world = Matrix4x4::Identity().values;
        parameters.view_projection = Compose_Matrices(reflected.projection,reflected.view).values;
        style.front_counter_clockwise = false;
        BOOST_REQUIRE(queue.Submit(renderer,mesh,style,parameters,{}));
        BOOST_REQUIRE(queue.Flush(commands));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        for (auto point : {std::array{8,16},std::array{16,8},std::array{24,16}}) {
            const auto offset = (point[1]*32+point[0])*4;
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset]),255);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+3])-128,2);
        }
        BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*32+16)*4]),0);
        renderer.Destroy_Mesh(mesh); renderer.Shutdown();
        device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
