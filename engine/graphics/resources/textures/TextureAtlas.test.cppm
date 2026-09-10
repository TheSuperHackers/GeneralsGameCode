module;
#define BOOST_TEST_MODULE TextureAtlasTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>
export module Graphics.Resources.Textures.Atlas.Tests;
import Graphics.Resources.Textures.Atlas;
import Graphics.Resources.Textures.Resource;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

namespace
{
void Check_Drawing(TextureResource& texture,unsigned width,unsigned height,std::span<const unsigned> colors,unsigned mip=0)
{
    auto& device=texture.Owner();
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({width,height,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({width,height,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    vertices[0].uv={0,1}; vertices[1].uv={1,1}; vertices[2].uv={1,0}; vertices[3].uv={0,0};
    for (auto& vertex : vertices) vertex.color={1,1,1,1};
    const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
    BOOST_REQUIRE(mesh.Is_Valid());
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    style.samplers[0].Set_Filter(RHISamplerFilter::Point);
    style.samplers[0].min_lod=style.samplers[0].max_lod=static_cast<float>(mip);
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,width,height}));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture.Handle()}));
    std::vector<std::byte> pixels(std::size_t(width)*height*4);
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
    BOOST_REQUIRE_EQUAL(colors.size(),std::size_t(width)*height);
    for (unsigned i=0;i<colors.size();++i) for (unsigned c=0;c<4;++c) {
        const unsigned shift=c==3 ? 24 : (2-c)*8;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+c])-int((colors[i]>>shift)&255),1);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
AtlasTile Tile(std::span<const unsigned> pixels,unsigned width,unsigned height,unsigned x=0,unsigned y=0)
{
    return {{std::as_bytes(pixels),width,height,std::size_t(width)*4,Assets::PixelEncoding::BGRA8},x,y,true};
}
}

BOOST_AUTO_TEST_CASE(periodic_gutters_and_vertical_orientation_preserve_color_and_expanded_storage_alpha)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        for (const auto encoding : {Assets::PixelEncoding::BGRA8,Assets::PixelEncoding::BGRA5551}) {
            std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,{8,8,1},encoding));
            BOOST_REQUIRE(texture);
            BOOST_CHECK(texture->Encoding()==Assets::PixelEncoding::BGRA8);
            std::array<unsigned,16> source{};
            const std::array<unsigned,4> palette{0x00ff0000,0x4000ff00,0x800000ff,0xffffffff};
            for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) source[y*4+x]=palette[(y/2)*2+x/2];
            const std::array tiles{Tile(source,4,4,2,2)};
            BOOST_REQUIRE(Upload_Texture_Atlas(*texture,tiles,std::array{AtlasRepeatBorder{2,2,4,4,2}},
                AtlasAlpha::Source,AtlasBackground::Opaque,false));
            std::array<unsigned,64> expected{};
            for (unsigned y=0;y<8;++y) for (unsigned x=0;x<8;++x) {
                auto color=source[(3-(y+2)%4)*4+(x+2)%4];
                expected[y*8+x]=color;
            }
            Check_Drawing(*texture,8,8,expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(edge_masks_and_transparent_unused_tree_pixels_have_defined_alpha)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,{4,1,1},Assets::PixelEncoding::BGRA8));
        BOOST_REQUIRE(texture);
        const std::array source{0xff000000u,0xffffffffu,0x00ff0000u};
        const std::array tiles{Tile(source,3,1)};
        BOOST_REQUIRE(Upload_Texture_Atlas(*texture,tiles,{},AtlasAlpha::EdgeMask,AtlasBackground::EdgeGradient,false));
        Check_Drawing(*texture,4,1,std::array{0x80000000u,0x00ffffffu,0xffff0000u,0x80ff0001u});
        BOOST_REQUIRE(Upload_Texture_Atlas(*texture,tiles,{},AtlasAlpha::Source,AtlasBackground::Transparent,false));
        Check_Drawing(*texture,4,1,std::array{source[0],source[1],source[2],0u});
    }
}

BOOST_AUTO_TEST_CASE(invalid_tile_or_gutter_rejects_upload_without_changing_existing_pixels)
{
    GraphicsTestDevice device({true});
    std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,{2,2,1},Assets::PixelEncoding::BGRA8));
    BOOST_REQUIRE(texture);
    const std::array source{0x40ff0000u,0x8000ff00u,0xff0000ffu,0xffffffffu};
    auto tile=Tile(source,2,2);
    BOOST_REQUIRE(Upload_Texture_Atlas(*texture,std::array{tile},{},AtlasAlpha::Source,AtlasBackground::Transparent,false));
    tile.x=1;
    BOOST_CHECK(!Upload_Texture_Atlas(*texture,std::array{tile},{},AtlasAlpha::Source,AtlasBackground::Transparent,false));
    tile.x=0; tile.image.bytes=tile.image.bytes.first(12);
    BOOST_CHECK(!Upload_Texture_Atlas(*texture,std::array{tile},{},AtlasAlpha::Source,AtlasBackground::Transparent,false));
    BOOST_CHECK(!Upload_Texture_Atlas(*texture,{},std::array{AtlasRepeatBorder{0,0,2,2,1}},
        AtlasAlpha::Source,AtlasBackground::Transparent,false));
    Check_Drawing(*texture,2,2,std::array{source[2],source[3],source[0],source[1]});
}

BOOST_AUTO_TEST_CASE(flat_tile_positions_and_generated_mips_preserve_rgb_and_alpha)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,{4,4,3},Assets::PixelEncoding::BGRA8));
        BOOST_REQUIRE(texture);
        const std::array red{0x80ff0000u,0x80ff0000u,0x80ff0000u,0x80ff0000u};
        const std::array green{0x4000ff00u,0x4000ff00u,0x4000ff00u,0x4000ff00u};
        const std::array tiles{Tile(red,2,2,0,2),Tile(green,2,2,2,0)};
        BOOST_REQUIRE(Upload_Texture_Atlas(*texture,tiles,{},AtlasAlpha::Source,AtlasBackground::Opaque,true));
        Check_Drawing(*texture,2,2,std::array{0xff000000u,green[0],red[0],0xff000000u},1);
        Check_Drawing(*texture,1,1,std::array{0xb0404000u},2);
    }
}
