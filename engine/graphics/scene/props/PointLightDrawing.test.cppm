module;
#define BOOST_TEST_MODULE PointLightDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Props.PointLightDrawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(local_light_range_attenuation_and_ambient)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal={0,0,1};
        vertex.material_ambient={1,1,1,1};
        vertex.material_diffuse={1,1,1,0.4f};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured=0;
    parameters.camera_position={0,0,5,1};
    parameters.light_direction[0]={0,0,-1,1};
    parameters.light_position[0]={0,0,2.5f,1};
    parameters.light_diffuse[0]={1,0.5f,0,0};
    parameters.light_ambient[0]={0,0.25f,0.5f,0};
    parameters.light_attenuation[0]={1,0,0,3};
    PropStyle style;
    style.depth_test=false; style.depth_write=false;
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,1,1}));
    const auto check=[&](std::array<int,4> expected) {
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,4> pixel{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixel,4));
        for (unsigned c=0;c<4;++c)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixel[c])-expected[c],2);
    };
    check({255,191,128,102});
    // At distance two, 1/(1 + 0.5*d + 0.5*d*d) is one quarter.
    parameters.light_attenuation[0]={1,0.5f,0.5f,3};
    check({64,48,32,102});
    parameters.light_attenuation[0][3]=1.9f;
    check({0,0,0,102});
    parameters.light_attenuation[0]={1,0,0,3};
    parameters.light_position[0][3]=2;
    parameters.light_spot[0]={1,0.4f,0.8f,0};
    check({255,191,128,102});
    // A spot facing away contributes neither diffuse nor local ambient.
    parameters.light_direction[0]={0,0,1,1};
    check({0,0,0,102});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}
