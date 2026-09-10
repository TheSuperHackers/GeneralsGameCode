module;
#define BOOST_TEST_MODULE LineGroupDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
export module Graphics.Scene.Particles.LineGroupDrawing.Tests;
import Graphics.Scene.Particles.LineGroupGeometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(line_group_shapes_preserve_texture_transparency_after_source_release_and_target_recreation)
{
    const std::array<float,9> rotation{1,0,0,0,1,0,0,0,1};
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        for(auto shape:{LineGroupShape::Tetrahedron,LineGroupShape::Prism})for(bool opaque:{true,false}) {
            const std::array<std::uint8_t,4> pixel{0,255,0,static_cast<std::uint8_t>(opaque?255:0)};
            const auto texture=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
            BOOST_REQUIRE(texture.Is_Valid());
            LineGroupPoint point;
            point.head={-.3f,0,.5f};point.tail={.3f,0,.5f};point.size=.2f;
            point.tail_color=point.head_color;
            LineGroupGeometry geometry;
            BOOST_REQUIRE(geometry.Build(1,shape,rotation,[&](auto){return point;}));
            const auto mesh=renderer.Create_Mesh(geometry.Vertices(),geometry.Indices());
            BOOST_REQUIRE(mesh.Is_Valid());
            point={};geometry={};
            PropStyle style;style.depth_test=false;style.depth_write=false;style.cull=RHICullMode::None;
            style.source_blend=RHIBlendFactor::SourceAlpha;style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
            PropParameters parameters;parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            auto& commands=device.Immediate_Command_List();
            for(unsigned width:{64u,96u,64u}) {
                const auto target=device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
                const auto depth=device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
                BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
                BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));
                BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
                std::vector<std::byte> pixels(width*32*4);
                BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const auto center=(16*width+width/2)*4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center]),0);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center+1]),opaque?255u:0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center+2]),opaque?0u:255u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),255u);
                device.Destroy_Texture(target);device.Destroy_Texture(depth);
            }
            renderer.Destroy_Mesh(mesh);device.Destroy_Texture(texture);
        }
        renderer.Shutdown();
    }
}
