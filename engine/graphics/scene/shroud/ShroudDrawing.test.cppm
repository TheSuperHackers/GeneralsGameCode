module;
#define BOOST_TEST_MODULE ShroudDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Shroud.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Shroud.Image;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(packed_shroud_pixels_preserve_borders_changes_and_resource_recreation)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    constexpr unsigned texture_width=6, texture_height=5;
    const auto target=device.Create_Texture({texture_width,texture_height,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({texture_width,texture_height,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[0].uv={0,1};
    vertices[1].position={1,-1,0.5f}; vertices[1].uv={1,1};
    vertices[2].position={1,1,0.5f}; vertices[2].uv={1,0};
    vertices[3].position={-1,1,0.5f}; vertices[3].uv={0,0};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,texture_width,texture_height}));
    for (const auto format : {RHITextureFormat::BGR565_UNorm,RHITextureFormat::BGRA4444_UNorm}) {
        auto texture=device.Create_Texture({texture_width,texture_height,1,format});
        BOOST_REQUIRE(texture.Is_Valid());
        ShroudImage image;
        BOOST_CHECK(!image.Upload(device,texture));
        // Nonzero row padding must never become a visible map cell.
        std::array<std::uint16_t,10> cells{0xf800,0x07e0,0x001f,0xffff,0xffff,
            0xabcd,0x1234,0xffff,0xf800,0x07e0};
        unsigned width=3,height=2;
        std::uint16_t border=0x39e7;
        const auto draw_and_check=[&] {
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::span(&texture,1)));
            std::array<std::byte,texture_width*texture_height*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,texture_width*4));
            for (unsigned y=0;y<texture_height;++y) {
                for (unsigned x=0;x<texture_width;++x) {
                    const unsigned packed=x>=1 && x<=width && y>=1 && y<=height
                        ? cells[(y-1)*5+x-1] : border;
                    const auto expected=format==RHITextureFormat::BGR565_UNorm
                        ? std::array<float,4>{float((packed>>11)&31)*255/31,
                            float((packed>>5)&63)*255/63,float(packed&31)*255/31,255}
                        : std::array<float,4>{float((packed>>8)&15)*17,
                            float((packed>>4)&15)*17,float(packed&15)*17,float((packed>>12)&15)*17};
                    for (unsigned channel=0;channel<4;++channel)
                        BOOST_CHECK_SMALL(float(std::to_integer<unsigned>(pixels[(y*texture_width+x)*4+channel]))
                            -expected[channel],1.1f);
                }
            }
        };
        const auto publish=[&] {
            BOOST_REQUIRE(image.Set_Cells(cells,width,height,5,texture_width,texture_height,border));
            BOOST_REQUIRE(image.Upload(device,texture));
            draw_and_check();
        };
        publish();
        publish();
        cells[1]=0x6bad;
        publish();
        border=0xeca9;
        publish();
        width=2; height=1;
        publish();
        BOOST_CHECK(!image.Set_Cells(std::span(cells).first(1),3,2,5,texture_width,texture_height,0));
        BOOST_REQUIRE(image.Upload(device,texture));
        draw_and_check();
        BOOST_REQUIRE(device.Destroy_Texture(texture));
        texture=device.Create_Texture({texture_width,texture_height,1,format});
        BOOST_REQUIRE(image.Upload(device,texture));
        draw_and_check();
        std::array<std::uint16_t,texture_width*texture_height> cleared{};
        BOOST_REQUIRE(device.Update_Texture(texture,{std::as_bytes(std::span(cleared)),texture_width*2}));
        image.Invalidate_Upload();
        BOOST_REQUIRE(image.Upload(device,texture));
        draw_and_check();
        BOOST_REQUIRE(device.Destroy_Texture(texture));
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}
