module;
#define BOOST_TEST_MODULE SpriteGeometryDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Scene.Particles.SpriteGeometry.Drawing.Tests;
import Graphics.Scene.Particles.SpriteGeometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(authored_sprite_shapes_and_atlas_alpha_draw_after_source_release_and_resize)
{
    const std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp}); if(!warp&&!device.Is_Valid())continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        std::array<std::uint8_t,8*4*4> pixels{};
        for(unsigned y=0;y<4;++y)for(unsigned x=0;x<8;++x) {
            const auto offset=(y*8+x)*4;
            pixels[offset]=x<4?255:0; pixels[offset+1]=x<4?0:255; pixels[offset+3]=x<4?255:128;
        }
        const auto texture=device.Create_Texture_Initialized({8,4},{std::as_bytes(std::span(pixels)),32});
        BOOST_REQUIRE(texture.Is_Valid());
        for(auto shape:{SpriteShape::Triangle,SpriteShape::Quad})for(unsigned frame:{0u,1u}) {
            SpriteGeometry geometry;
            SpritePoint point;point.position={0,0,.5f};point.size=shape==SpriteShape::Quad?1.0f:.2f;
            BOOST_REQUIRE(Get_Sprite_Atlas_Region(frame,2,1,point.texture_region));
            BOOST_REQUIRE(geometry.Build(1,shape,identity,[&](auto) { return point; }));
            const auto mesh=renderer.Create_Mesh(geometry.Vertices(),geometry.Indices());
            BOOST_REQUIRE(mesh.Is_Valid());
            geometry={};point={};
            PropStyle style;style.depth_test=false;style.depth_write=false;style.cull=RHICullMode::None;style.blend=RHIBlendMode::Alpha;
            style.source_blend=RHIBlendFactor::SourceAlpha;
            style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
            PropParameters parameters;parameters.textured=1;parameters.view_projection=identity;
            auto& commands=device.Immediate_Command_List();
            for(unsigned width:{32u,64u,32u}) {
                const auto target=device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
                const auto depth=device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
                BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));
                BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
                std::vector<std::byte> result(width*32*4);BOOST_REQUIRE(device.Readback_Texture(target,result,width*4));
                const auto center=(16*width+width/2)*4;
                // Material blending applies the authored factors to alpha too:
                // a*a + destination_alpha*(1-a), with a = 128/255.
                const std::array<int,4> expected=frame?std::array<int,4>{0,128,127,191}:std::array<int,4>{255,0,0,255};
                for(unsigned c=0;c<4;++c)BOOST_CHECK_SMALL(std::to_integer<int>(result[center+c])-expected[c],2);
                const auto corner=(10*width+width*13/20)*4;
                BOOST_CHECK_SMALL(std::to_integer<int>(result[corner+2])-(shape==SpriteShape::Triangle?255:expected[2]),2);
                device.Destroy_Texture(target);device.Destroy_Texture(depth);
            }
            renderer.Destroy_Mesh(mesh);
        }
        device.Destroy_Texture(texture);renderer.Shutdown();
    }
}
