module;
#define BOOST_TEST_MODULE TransparentDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Props.TransparentDrawing.Tests;
import Graphics.Scene.Props.TransparentGeometry;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(sorts_triangles_across_batches_and_releases_the_queue)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    TransparentGeometry queue;
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth_target=device.Create_Texture({16,16,1,RHITextureFormat::D24_UNorm_S8,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth_target));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    PropParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,0,0.5f,0,0,0,1};
    PropStyle style;
    style.depth_test=false; style.depth_write=false;
    style.source_blend=RHIBlendFactor::SourceAlpha;
    style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
    const std::array<float,4> depth{0,0,1,0};
    std::array<PropVertex,6> vertices{};
    for (int triangle=0; triangle<2; ++triangle) {
        vertices[triangle*3].position={-1,-1,triangle ? 11.0f : 19.0f};
        vertices[triangle*3+1].position={3,-1,vertices[triangle*3].position[2]};
        vertices[triangle*3+2].position={-1,3,vertices[triangle*3].position[2]};
        for (int corner=0;corner<3;++corner)
            vertices[triangle*3+corner].color=triangle
                ? std::array<float,4>{1,0,0,0.5f} : std::array<float,4>{0,0,1,0.5f};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,3,4,5};
    parameters.world[11]=-20;
    const auto retained = renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(queue.Submit(renderer,retained,style,parameters,{},depth));
    BOOST_REQUIRE(renderer.Destroy_Mesh(retained));
    parameters.world[11]=0;
    for (int i=0;i<3;++i) { vertices[i].position[2]=-5; vertices[i].color={0,1,0,0.5f}; }
    BOOST_REQUIRE(queue.Submit(renderer,std::span(vertices).first(3),std::span(indices).first(3),
        style,parameters,{},depth));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(queue.Flush(renderer,commands));
    BOOST_CHECK(queue.Empty());
    BOOST_CHECK(renderer.Mesh_Geometry(retained) == nullptr);
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64));
    // Red far, green middle, blue near: sorting whole batches cannot produce this.
    const std::array expected{32,64,128,112};
    for (int channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
    BOOST_REQUIRE(queue.Flush(renderer,commands));
    BOOST_REQUIRE(queue.Submit(renderer,std::span(vertices).first(3),std::span(indices).first(3),
        style,parameters,{},depth));
    queue.Clear(renderer);
    BOOST_CHECK(queue.Empty());
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    style.stencil.enabled=true;
    style.stencil.write_mask=0;
    style.stencil.front.comparison=RHIComparison::Equal;
    style.stencil.back=style.stencil.front;
    style.stencil.reference=1;
    BOOST_REQUIRE(queue.Submit(renderer,std::span(vertices).first(3),std::span(indices).first(3),
        style,parameters,{},depth));
    style.stencil.reference=0;
    for (int i=0;i<3;++i) vertices[i].color={1,0,0,0.5f};
    BOOST_REQUIRE(queue.Submit(renderer,std::span(vertices).first(3),std::span(indices).first(3),
        style,parameters,{},depth));
    // The queued green material must retain its rejecting stencil reference.
    BOOST_REQUIRE(queue.Flush(renderer,commands));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64));
    const std::array stencil_expected{128,0,0,64};
    for (int channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-stencil_expected[channel],2);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth_target);
}
