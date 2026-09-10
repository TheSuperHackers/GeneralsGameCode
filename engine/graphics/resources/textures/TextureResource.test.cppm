module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureResourceTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
export module Graphics.Resources.Textures.Resource.Tests;
import Graphics.Resources.Textures.Resource;
import Graphics.Resources.Textures.Upload;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Assets.Adapters.DDS;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(target_references_and_deferred_generation_have_separate_lifetimes)
{
    GraphicsTestDevice device({true});
    RHITexture description{8,8,0};
    description.usage |= static_cast<unsigned>(RHITextureUsage::RenderTarget);
    auto* texture=TextureResource::Create(&device,description,Assets::PixelEncoding::BGRA4444,
        RHITextureFormat::D24_UNorm_S8);
    BOOST_REQUIRE(texture);
    BOOST_CHECK(texture->Encoding()==Assets::PixelEncoding::BGRA8);
    BOOST_CHECK_EQUAL(texture->Description().mip_count,4u);
    const auto color=texture->Handle(),depth=texture->Depth_Attachment();
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(color,depth));
    BOOST_REQUIRE(commands.Clear_Color_Target(color,{0.25f,0.5f,0.75f,0.5f}));
    BOOST_REQUIRE(device.Retain_Texture(color));
    BOOST_CHECK(Retain_Texture_Resource(texture)==texture);
    texture->Release();
    BOOST_CHECK_EQUAL(texture->Reference_Count(),1u);
    texture->Release();
    std::array<std::byte,256> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(color,pixels,32));
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[0])-191,1);
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-128,1);
    BOOST_CHECK(!device.Retain_Texture(depth));
    BOOST_REQUIRE(device.Destroy_Texture(color));
    BOOST_CHECK(!device.Retain_Texture(color));
    Release_Texture_Resource(nullptr);
    BOOST_CHECK(Retain_Texture_Resource(nullptr)==nullptr);
}

BOOST_AUTO_TEST_CASE(allocation_rejects_invalid_shapes_and_preserves_cube_and_depth_metadata)
{
    GraphicsTestDevice device({true});
    BOOST_CHECK(!TextureResource::Create(nullptr,{4,4,1},Assets::PixelEncoding::RGBA8));
    BOOST_CHECK(!TextureResource::Create(&device,{0,4,1},Assets::PixelEncoding::RGBA8));
    RHITexture cube{8,4,0}; cube.dimension=RHITextureDimension::Cube; cube.array_size=6;
    BOOST_CHECK(!TextureResource::Create(&device,cube,Assets::PixelEncoding::BGRA8));
    cube.height=8;
    std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,cube,Assets::PixelEncoding::BC3));
    BOOST_REQUIRE(texture);
    BOOST_CHECK_EQUAL(texture->Description().mip_count,4u);
    BOOST_CHECK_EQUAL(texture->Description().array_size,6u);
    BOOST_CHECK(texture->Encoding()==Assets::PixelEncoding::BC3);
    TextureUpload upload;
    BOOST_REQUIRE(upload.Begin(device,texture->Handle(),4,6));
    BOOST_REQUIRE(upload.Finish());
    cube.usage |= static_cast<unsigned>(RHITextureUsage::RenderTarget);
    BOOST_CHECK(!TextureResource::Create(&device,cube,Assets::PixelEncoding::BC3));
    for (const auto format : {RHITextureFormat::D16_UNorm,RHITextureFormat::D24_UNorm_S8,RHITextureFormat::D32_Float}) {
        std::unique_ptr<TextureResource> depth(TextureResource::Create(&device,
            {4,4,0,format,static_cast<unsigned>(RHITextureUsage::DepthStencil)},Assets::PixelEncoding::Unknown));
        BOOST_REQUIRE(depth);
        BOOST_CHECK_EQUAL(depth->Description().mip_count,1u);
        BOOST_CHECK(depth->Description().format==format);
    }
}

BOOST_AUTO_TEST_CASE(compressed_volume_decodes_to_the_allocated_encoding)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        std::vector<std::byte> bytes(128+32);
        const auto write=[&](unsigned offset,unsigned value) {
            for (unsigned i=0;i<4;++i) bytes[offset+i]=std::byte(value>>(i*8));
        };
        write(0,0x20534444); write(4,124); write(12,4); write(16,4); write(24,2); write(28,1);
        write(76,32); write(80,4); write(84,0x33545844); write(112,0x200000);
        for (unsigned z=0;z<2;++z) {
            write(128+z*16,0x88888888); write(132+z*16,0x88888888);
            write(136+z*16,z ? 0x07e007e0 : 0xf800f800);
        }
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes,bytes.size(),layout));
        RHITexture description{4,4,1}; description.depth=2; description.dimension=RHITextureDimension::Volume;
        std::unique_ptr<TextureResource> texture(TextureResource::Create(&device,description,Assets::DDS_Pixel_Encoding(layout)));
        BOOST_REQUIRE(texture);
        BOOST_CHECK(texture->Encoding()==Assets::PixelEncoding::BGRA8);
        TextureUpload upload;
        BOOST_REQUIRE(upload.Begin(device,texture->Handle(),1));
        const auto mapping=upload.Mapping(0);
        BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes,layout,0,0,texture->Encoding(),4,4,2,
            mapping.bytes,mapping.row_pitch,mapping.slice_pitch));
        BOOST_REQUIRE(upload.Finish());
        std::array<std::byte,128> pixels{};
        BOOST_REQUIRE(device.Readback_Texture_Subresource(texture->Handle(),{pixels,16,64}));
        for (unsigned z=0;z<2;++z) for (unsigned i=0;i<16;++i) {
            const unsigned offset=z*64+i*4;
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),z ? 255u : 0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+2]),z ? 0u : 255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+3]),136u);
        }
    }
}

BOOST_AUTO_TEST_CASE(upload_survives_owner_release_and_draws_rgb_and_transparency)
{
    for (const bool warp : {true,false}) {
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
        vertices[0].uv={0,1}; vertices[1].uv={1,1}; vertices[2].uv={1,0}; vertices[3].uv={0,0};
        for (auto& vertex : vertices) vertex.color={1,1,1,1};
        const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
        BOOST_REQUIRE(mesh.Is_Valid());
        PropParameters parameters;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        PropStyle style; style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,2,2}));
        auto* texture=TextureResource::Create(&device,{2,2,1},Assets::PixelEncoding::BGRA4444);
        BOOST_REQUIRE(texture);
        const auto handle=texture->Handle();
        TextureUpload upload;
        BOOST_REQUIRE(upload.Begin(device,handle,1));
        const auto mapping=upload.Mapping(0);
        const std::array<unsigned,4> colors{0x00ff0000,0x4000ff00,0x800000ff,0xffffffff};
        for (unsigned y=0;y<2;++y) for (unsigned x=0;x<2;++x) for (unsigned c=0;c<4;++c)
            mapping.bytes[y*mapping.row_pitch+x*4+c]=std::byte(colors[y*2+x]>>(c*8));
        BOOST_REQUIRE(device.Retain_Texture(handle)); // A deferred draw owns this generation.
        texture->Release();
        BOOST_REQUIRE(upload.Finish());
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{handle}));
        std::array<std::byte,16> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8));
        for (unsigned i=0;i<4;++i) for (unsigned c=0;c<4;++c) {
            const unsigned shift=c==3 ? 24 : (2-c)*8;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+c])-int((colors[i]>>shift)&255),1);
        }
        BOOST_REQUIRE(device.Destroy_Texture(handle));
        BOOST_CHECK(!device.Retain_Texture(handle));
        std::unique_ptr<TextureResource> placeholder(TextureResource::Create_Placeholder(&device));
        BOOST_REQUIRE(placeholder);
        BOOST_CHECK(placeholder->Is_Placeholder());
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{placeholder->Handle()}));
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8));
        for (unsigned i=0;i<4;++i) {
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+3]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4]),i==0 || i==3 ? 255u : 0u);
        }
        renderer.Destroy_Mesh(mesh); renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
