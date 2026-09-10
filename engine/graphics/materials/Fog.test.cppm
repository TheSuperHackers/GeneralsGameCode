module;
#define BOOST_TEST_MODULE MaterialFogTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
export module Graphics.Materials.Fog.Tests;
import Graphics.Materials.Fog;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(per_draw_fog_preserves_scene_color_and_fragment_alpha)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) vertex.color={0.8f,0.4f,0.2f,0.6f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured=0;
    SceneFog scene{true,0,1,{0.2f,0.6f,0.8f,0.1f}};
    struct Case { MaterialFogMode mode; bool enabled; std::array<float,3> expected; };
    const Case cases[]{
        {MaterialFogMode::Scene,true,{0.5f,0.5f,0.5f}},
        {MaterialFogMode::Black,true,{0.4f,0.2f,0.1f}},
        {MaterialFogMode::Disabled,true,{0.8f,0.4f,0.2f}},
        {MaterialFogMode::White,true,{0.9f,0.7f,0.6f}},
        {MaterialFogMode::Scene,true,{0.5f,0.5f,0.5f}},
        {MaterialFogMode::Scene,false,{0.8f,0.4f,0.2f}},
        {MaterialFogMode::Black,false,{0.8f,0.4f,0.2f}},
        {MaterialFogMode::White,false,{0.8f,0.4f,0.2f}},
        {MaterialFogMode::Scene,true,{0.5f,0.5f,0.5f}}};
    for (const auto& value : cases) {
        scene.enabled=value.enabled;
        const auto fog=Resolve_Material_Fog(scene,value.mode);
        parameters.fog_state=fog.state; parameters.fog_color=fog.color;
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32));
        for (unsigned channel=0;channel<3;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-value.expected[channel]*255,1.1f);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+3])-153,2);
        BOOST_CHECK((scene.color==std::array{0.2f,0.6f,0.8f,0.1f}));
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
