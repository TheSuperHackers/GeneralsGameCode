module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureEditTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
export module Graphics.Resources.Textures.Edit.Tests;
import Graphics.Resources.Textures.Edit;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Assets.Images.Color;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(readback_edits_retain_replaced_generation_and_commit_only_writable_maps)
{
    GraphicsTestDevice device({true});
    auto* texture=TextureResource::Create(&device,{4,4,2},Assets::PixelEncoding::BGRA8);
    BOOST_REQUIRE(texture);
    const auto handle=texture->Handle();
    const std::array<unsigned,4> initial{0x80402010,0x80402010,0x80402010,0x80402010};
    BOOST_REQUIRE(device.Update_Texture(handle,{std::as_bytes(std::span(initial)),8,0,1}));
    std::unique_ptr<TextureEdit> edit(TextureEdit::Readback(*texture,1));
    BOOST_REQUIRE(edit);
    BOOST_REQUIRE(device.Retain_Texture(handle));
    texture->Release();
    const Assets::ImageRegion region{1,1,2,2};
    auto mapping=edit->Map(&region);
    BOOST_REQUIRE_EQUAL(mapping.bytes.size(),4u);
    BOOST_CHECK(edit->Map().bytes.empty());
    mapping.bytes[0]=std::byte{0xee}; mapping.bytes[3]=std::byte{0x22};
    BOOST_REQUIRE(edit->Unmap());
    std::array<std::byte,16> pixels{};
    BOOST_REQUIRE(device.Readback_Texture_Subresource(handle,{pixels,8,0,1}));
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),0x10u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[12]),0xeeu);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[15]),0x22u);
    const auto saved=pixels;
    mapping=edit->Map(nullptr,true);
    BOOST_REQUIRE(!mapping.bytes.empty());
    mapping.bytes[0]=std::byte{0xff};
    BOOST_REQUIRE(edit->Unmap());
    edit.reset();
    BOOST_REQUIRE(device.Readback_Texture_Subresource(handle,{pixels,8,0,1}));
    BOOST_CHECK(pixels==saved);
    BOOST_REQUIRE(device.Destroy_Texture(handle));
    BOOST_CHECK(!device.Retain_Texture(handle));
}

BOOST_AUTO_TEST_CASE(final_release_commits_pending_edit_and_compressed_tail_roundtrips)
{
    GraphicsTestDevice device({true});
    std::unique_ptr<TextureEdit> image(TextureEdit::Create(4,4,Assets::PixelEncoding::BC1));
    BOOST_REQUIRE(image);
    image->Image().Bytes()[0]=std::byte{0}; image->Image().Bytes()[1]=std::byte{0xf8};
    std::unique_ptr<TextureResource> texture(image->Create_Texture(&device,3));
    BOOST_REQUIRE(texture);
    BOOST_REQUIRE(device.Update_Texture(texture->Handle(),{image->Image().Bytes(),8,0,2}));
    std::unique_ptr<TextureEdit> copy(TextureEdit::Readback(*texture,2));
    BOOST_REQUIRE(copy);
    BOOST_CHECK_EQUAL(copy->Image().Width(),1u);
    BOOST_CHECK_EQUAL(copy->Image().Height(),1u);
    BOOST_CHECK_EQUAL(copy->Image().Bytes().size(),8u);
    BOOST_CHECK(copy->Image().Bytes()[1]==std::byte{0xf8});
    BOOST_CHECK(copy->Map().bytes.empty());
    std::unique_ptr<TextureResource> color(TextureResource::Create(&device,{2,2,1},Assets::PixelEncoding::BGRA8));
    BOOST_REQUIRE(color);
    {
        std::unique_ptr<TextureEdit> pending(TextureEdit::Readback(*color,0));
        BOOST_REQUIRE(pending);
        auto mapping=pending->Map();
        BOOST_REQUIRE(!mapping.bytes.empty());
        mapping.bytes[0]=std::byte{0x91};
    }
    std::array<std::byte,16> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(color->Handle(),pixels,8));
    BOOST_CHECK(pixels[0]==std::byte{0x91});
}

BOOST_AUTO_TEST_CASE(packed_image_conversion_and_gpu_edit_preserve_sampled_rgb_and_alpha)
{
    for (bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target=device.Create_Texture({2,2,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth=device.Create_Texture({2,2,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        std::array<PropVertex,4> vertices{};
        vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
        vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
        for (auto& vertex : vertices) { vertex.color={1,1,1,1}; vertex.uv={0.5f,0.5f}; }
        const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
        BOOST_REQUIRE(mesh.Is_Valid());
        PropParameters parameters;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        PropStyle style; style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,2,2}));
        std::unique_ptr<TextureEdit> image(TextureEdit::Create(2,2,Assets::PixelEncoding::BGRA4444));
        BOOST_REQUIRE(image);
        for (unsigned i=0;i<4;++i) { image->Image().Bytes()[i*2]=std::byte{0x00}; image->Image().Bytes()[i*2+1]=std::byte{0x8f}; }
        for (unsigned i=0;i<4;++i)
            BOOST_REQUIRE(Assets::Replace_Image_RGB(image->Image().Bytes().subspan(i*2,2),
                image->Image().Encoding(),{1,0,0}));
        std::unique_ptr<TextureResource> texture(image->Create_Texture(&device,1));
        BOOST_REQUIRE(texture);
        BOOST_CHECK(texture->Encoding()==Assets::PixelEncoding::BGRA8);
        for (unsigned generation=0;generation<2;++generation) {
            if (generation) {
                std::unique_ptr<TextureEdit> edit(TextureEdit::Readback(*texture,0));
                std::unique_ptr<TextureEdit> replacement(TextureEdit::Create(1,1,Assets::PixelEncoding::BGRA8));
                BOOST_REQUIRE(edit); BOOST_REQUIRE(replacement);
                BOOST_REQUIRE(Assets::Fill_Packed_Image_Region(replacement->Image(),{0,0,1,1},0));
                BOOST_REQUIRE(Assets::Write_Packed_Image_Pixel(replacement->Image(),0,0,0x4000ff00));
                BOOST_REQUIRE(edit->Copy_From(*replacement,{0,0,1,1},{0,0,2,2}));
            }
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture->Handle()}));
            std::array<std::byte,16> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,8));
            for (unsigned i=0;i<4;++i) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4])-(generation ? 0 : 240),1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+1])-(generation ? 255 : 0),1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+3])-(generation ? 64 : 128),1);
            }
        }
        renderer.Destroy_Mesh(mesh); renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
