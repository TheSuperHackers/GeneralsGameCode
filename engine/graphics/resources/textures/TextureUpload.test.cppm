module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureUploadTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>
export module Graphics.Resources.Textures.Upload.Tests;
import Graphics.Resources.Textures.Upload;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Graphics.Resources.Textures.Storage;
import Assets.Adapters.DDS;
import Assets.Adapters.TGA.Image;
import Assets.Images.Preparation;
using namespace Graphics;

namespace
{
void Fill(RHITextureMapping mapping, unsigned width, unsigned height, unsigned depth,
    std::array<unsigned char,4> color)
{
    for (unsigned z=0;z<depth;++z) for (unsigned y=0;y<height;++y) for (unsigned x=0;x<width;++x)
        for (unsigned channel=0;channel<4;++channel)
            mapping.bytes[z*mapping.slice_pitch+y*mapping.row_pitch+x*4+channel]=std::byte(color[channel]);
}
}

BOOST_AUTO_TEST_CASE(all_fifteen_mips_upload_and_sample_rgb_and_alpha)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) { vertex.color={1,1,1,1}; vertex.uv={0.5f,0.5f}; }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,1,1}));
    TextureUpload upload;
    for (unsigned generation=0;generation<2;++generation) {
        const auto texture=device.Create_Texture({16384,1,15});
        BOOST_REQUIRE(texture.Is_Valid());
        BOOST_REQUIRE(upload.Begin(device,texture,15));
        std::vector<std::jthread> workers;
        for (unsigned mip=0;mip<15;++mip) {
            const auto mapping=upload.Mapping(mip);
            BOOST_REQUIRE(!mapping.bytes.empty());
            workers.emplace_back([mapping,mip,generation] {
                Fill(mapping,16384>>mip,1,1,{static_cast<unsigned char>(20+mip*10),
                    static_cast<unsigned char>(80+generation*40),30,static_cast<unsigned char>(60+mip*10)});
            });
        }
        workers.clear(); // Join preparation before device submission.
        BOOST_REQUIRE(upload.Finish());
        BOOST_CHECK(!upload.Active());
        BOOST_CHECK(upload.Mapping(0).bytes.empty());
        for (unsigned mip : {0u,7u,12u,14u}) {
            style.samplers[0].min_lod=style.samplers[0].max_lod=static_cast<float>(mip);
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
            std::array<std::byte,4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,4));
            const std::array expected{20+mip*10,80+generation*40,30u,60+mip*10};
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[channel])-static_cast<int>(expected[channel]),1);
        }
        BOOST_REQUIRE(device.Destroy_Texture(texture));
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(cube_faces_and_volume_slices_keep_independent_rows_and_mips)
{
    GraphicsTestDevice device({true});
    TextureUpload upload;
    for (bool cube : {true,false}) {
        RHITexture description{8,8,4};
        description.dimension=cube ? RHITextureDimension::Cube : RHITextureDimension::Volume;
        description.array_size=cube ? 6 : 1;
        description.depth=cube ? 1 : 4;
        const auto texture=device.Create_Texture(description);
        BOOST_REQUIRE(texture.Is_Valid());
        BOOST_REQUIRE(upload.Begin(device,texture,4,description.array_size));
        for (unsigned layer=0;layer<description.array_size;++layer) for (unsigned mip=0;mip<4;++mip) {
            const unsigned width=8>>mip, depth=std::max(1u,description.depth>>mip);
            const auto mapping=upload.Mapping(mip,layer);
            BOOST_REQUIRE_GE(mapping.row_pitch,width*4);
            for (unsigned z=0;z<depth;++z) {
                auto slice=mapping;
                slice.bytes=slice.bytes.subspan(z*mapping.slice_pitch);
                Fill(slice,width,width,1,{static_cast<unsigned char>(layer*30),
                    static_cast<unsigned char>(mip*50),static_cast<unsigned char>(z*60),100});
            }
        }
        BOOST_REQUIRE(upload.Finish());
        for (unsigned layer=0;layer<description.array_size;++layer) for (unsigned mip=0;mip<4;++mip) {
            const unsigned width=8>>mip, depth=std::max(1u,description.depth>>mip);
            std::vector<std::byte> pixels(width*width*depth*4);
            BOOST_REQUIRE(device.Readback_Texture_Subresource(texture,{pixels,width*4,width*width*4,mip,layer}));
            for (unsigned z=0;z<depth;++z) {
                const unsigned offset=((z*width+width-1)*width+width-1)*4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),layer*30);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),mip*50);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+2]),z*60);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+3]),100);
            }
        }
        BOOST_REQUIRE(device.Destroy_Texture(texture));
    }
}

BOOST_AUTO_TEST_CASE(partial_mapping_failure_unwinds_owned_mappings_and_keeps_foreign_mapping)
{
    GraphicsTestDevice device({true});
    const auto texture=device.Create_Texture({4,4,3});
    BOOST_REQUIRE(texture.Is_Valid());
    RHITextureMapping foreign;
    BOOST_REQUIRE(device.Map_Texture(texture,1,0,true,foreign));
    TextureUpload upload;
    BOOST_CHECK(!upload.Begin(device,texture,3));
    BOOST_CHECK(!upload.Active());
    BOOST_CHECK(upload.Mapping(0).bytes.empty());
    RHITextureMapping check;
    BOOST_REQUIRE(device.Map_Texture(texture,0,0,false,check));
    BOOST_REQUIRE(device.Unmap_Texture(texture,0,0));
    BOOST_CHECK(!device.Map_Texture(texture,1,0,false,check));
    BOOST_REQUIRE(device.Unmap_Texture(texture,1,0));
    BOOST_REQUIRE(upload.Begin(device,texture,3));
    BOOST_CHECK(!upload.Begin(device,texture,3));
    BOOST_CHECK(!upload.Mapping(2).bytes.empty());
    BOOST_REQUIRE(upload.Finish());
    BOOST_REQUIRE(device.Destroy_Texture(texture));
}

BOOST_AUTO_TEST_CASE(upload_retains_generation_and_finishes_on_destruction)
{
    GraphicsTestDevice device({true});
    auto texture=device.Create_Texture({2,2,1});
    BOOST_REQUIRE(texture.Is_Valid());
    {
        TextureUpload upload;
        BOOST_CHECK(!upload.Begin(device,{},1));
        BOOST_CHECK(!upload.Begin(device,texture,0));
        BOOST_CHECK(!upload.Begin(device,texture,1,0));
        BOOST_REQUIRE(upload.Begin(device,texture,1));
        BOOST_REQUIRE(device.Destroy_Texture(texture));
        Fill(upload.Mapping(0),2,2,1,{20,40,60,80});
    }
    BOOST_CHECK(!device.Retain_Texture(texture));
    texture=device.Create_Texture({2,2,1});
    BOOST_REQUIRE(texture.Is_Valid());
    {
        TextureUpload upload;
        BOOST_REQUIRE(upload.Begin(device,texture,1));
        Fill(upload.Mapping(0),2,2,1,{20,40,60,80});
    }
    std::array<std::byte,16> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(texture,pixels,8));
    for (unsigned index=0;index<pixels.size();++index)
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[index]),20*(index%4+1));
    BOOST_REQUIRE(device.Destroy_Texture(texture));
}

BOOST_AUTO_TEST_CASE(dds_upload_samples_compressed_and_expanded_colors_cutouts_and_alpha)
{
    for (const bool warp : {true, false}) {
    GraphicsTestDevice device({warp});
    if (!warp && !device.Is_Valid()) {
        BOOST_TEST_MESSAGE("Hardware adapter unavailable; WARP drawing was verified.");
        continue;
    }
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({4,4,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({4,4,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    vertices[0].uv={0,1}; vertices[1].uv={1,1}; vertices[2].uv={1,0}; vertices[3].uv={0,0};
    for (auto& vertex : vertices) vertex.color={1,1,1,1};
    const auto mesh = renderer.Create_Mesh(vertices, std::array<std::uint32_t,6>{0,1,2,0,2,3});
    BOOST_REQUIRE(mesh.Is_Valid());
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,4,4}));
    for (const unsigned format : {0x31545844u,0x32545844u,0x33545844u,0x34545844u,0x35545844u}) {
        const bool bc1 = format == 0x31545844;
        const bool bc2 = format == 0x32545844 || format == 0x33545844;
        std::vector<std::byte> bytes(bc1 ? 136 : 144);
        const auto write = [&](unsigned offset, unsigned value) {
            for (unsigned i=0;i<4;++i) bytes[offset+i]=std::byte(value>>(i*8));
        };
        write(0,0x20534444); write(4,124); write(12,4); write(16,4); write(28,1);
        write(76,32); write(80,4); write(84,format);
        if (!bc1) { write(128,bc2 ? 0x88888888 : 0x00008080); write(132,bc2 ? 0x88888888 : 0); }
        write(bc1 ? 128 : 136,0xf800f800); write(bc1 ? 132 : 140,0xf0f0f0f0);
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes,bytes.size(),layout));
        for (const auto encoding : {Assets::DDS_Pixel_Encoding(layout),Assets::PixelEncoding::RGBA8,
            Assets::PixelEncoding::BGRA8,Assets::PixelEncoding::BGRA4444}) {
            const auto texture = device.Create_Texture({4,4,1,Texture_Storage_Format(encoding)});
            BOOST_REQUIRE(texture.Is_Valid());
            TextureUpload upload;
            BOOST_REQUIRE(upload.Begin(device,texture,1));
            const auto mapping = upload.Mapping(0);
            BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes,layout,0,0,encoding,4,4,1,
                mapping.bytes,mapping.row_pitch,mapping.slice_pitch,{120,0,0}));
            BOOST_REQUIRE(upload.Finish());
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
            std::array<std::byte,64> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,16));
            const bool packed = encoding == Assets::PixelEncoding::BGRA4444;
            for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
                const bool transparent = bc1 && x>=2;
                const int alpha = transparent ? 0 : bc1 ? 255 : bc2 || packed ? 136 : 128;
                const int green = transparent ? 0 : packed ? 255 : 250;
                const unsigned offset=y*16+x*4;
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset]),1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+1])-green,1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+2]),1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+3])-alpha,1);
            }
            BOOST_REQUIRE(device.Destroy_Texture(texture));
        }
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(tga_preparation_upload_preserves_image_origins_and_sampled_alpha)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target=device.Create_Texture({4,4,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth=device.Create_Texture({4,4,1,RHITextureFormat::D32_Float,
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
        PropStyle style;
        style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,4,4}));
        const std::array<unsigned,4> colors{0x00ff0000,0x4000ff00,0x800000ff,0xffffffff};
        for (const bool rle : {false,true}) {
            std::vector<std::byte> bytes(18);
            bytes[2]=std::byte(rle ? 10 : 2); bytes[12]=bytes[14]=std::byte{4};
            bytes[16]=std::byte{32}; bytes[17]=std::byte{16}; // Bottom-right source origin.
            if (rle) bytes.push_back(std::byte{15});
            for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
                const auto color=colors[((3-y)/2)*2+(3-x)/2];
                for (unsigned channel=0;channel<4;++channel) bytes.push_back(std::byte(color>>(channel*8)));
            }
            Assets::TGAImage image;
            BOOST_REQUIRE(Assets::Decode_TGA_Image(bytes,image));
            for (const auto encoding : {Assets::PixelEncoding::RGBA8,Assets::PixelEncoding::BGRA8}) {
                std::vector<Assets::PreparedImage> levels;
                BOOST_REQUIRE(Assets::Prepare_Image_Levels(image.View(),encoding,4,4,1,{},levels));
                const auto texture=device.Create_Texture({4,4,1,Texture_Storage_Format(encoding)});
                BOOST_REQUIRE(texture.Is_Valid());
                TextureUpload upload;
                BOOST_REQUIRE(upload.Begin(device,texture,1));
                const auto mapping=upload.Mapping(0);
                BOOST_REQUIRE(Assets::Copy_Prepared_Image(levels.front(),mapping.bytes,mapping.row_pitch));
                BOOST_REQUIRE(upload.Finish());
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
                std::array<std::byte,64> pixels{};
                BOOST_REQUIRE(device.Readback_Texture(target,pixels,16));
                for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
                    const auto color=colors[(y/2)*2+x/2];
                    const std::array<unsigned,4> expected{(color>>16)&255,(color>>8)&255,color&255,color>>24};
                    for (unsigned channel=0;channel<4;++channel)
                        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*4+x)*4+channel])-int(expected[channel]),1);
                }
                BOOST_REQUIRE(device.Destroy_Texture(texture));
            }
        }
        renderer.Destroy_Mesh(mesh); renderer.Shutdown();
        device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
