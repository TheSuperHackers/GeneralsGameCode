module;
#define BOOST_TEST_MODULE GlyphAtlasDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Text.GlyphAtlas.Drawing.Tests;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
import Graphics.Text.GlyphAtlas;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(glyph_pages_preserve_coverage_and_do_not_overwrite_existing_glyphs)
{
    GlyphAtlas atlas(4);
    const std::array<std::uint8_t,4> low{64,64,64,64}, high{192,192,192,192};
    const auto first=atlas.Add(2,2,low);
    const auto second=atlas.Add(2,2,high);
    BOOST_REQUIRE(first); BOOST_REQUIRE(second);
    BOOST_CHECK(first->page==0); BOOST_CHECK(second->page==1);
    BOOST_CHECK_EQUAL(atlas.Page_Count(),2);
    BOOST_CHECK(!atlas.Add(3,2,high));
    BOOST_CHECK(!atlas.Add(2,2,std::span(high).first(3)));
    BOOST_CHECK_EQUAL(atlas.Pixels(0)[0],0);
    BOOST_CHECK_EQUAL(atlas.Pixels(0)[(1*4+1)*4+3],64);
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f};vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f};vertices[3].position={-1,1,0.5f};
    for(auto& vertex : vertices) vertex.uv={0.5f,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.depth_write=false;
    style.source_blend=RHIBlendFactor::SourceAlpha;
    style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
    for(unsigned page=0;page<2;++page) {
        const auto texture=device.Create_Texture_Initialized({4,4},{std::as_bytes(atlas.Pixels(page)),16});
        BOOST_REQUIRE(texture.Is_Valid());
        BOOST_REQUIRE(commands.Clear({0,0,1,0},1));
        const std::array textures{texture};
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        const int coverage=page==0 ? 64 : 192;
        const std::array expected{coverage,coverage,255,coverage*coverage/255};
        for(int channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected[channel],2);
        device.Destroy_Texture(texture);
    }
    renderer.Destroy_Mesh(mesh);renderer.Shutdown();
    device.Destroy_Texture(target);device.Destroy_Texture(depth);
    atlas.Clear();
    BOOST_CHECK_EQUAL(atlas.Page_Count(),0);
    BOOST_CHECK(atlas.Add(2,2,high)->page==0);
}
