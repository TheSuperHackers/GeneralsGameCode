module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureLoadTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <thread>
#include <vector>
export module Graphics.Resources.Textures.Load.Tests;
import Graphics.Resources.Textures.Load;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Graphics.Resources.Textures.Quality;
using namespace Graphics;

namespace
{
TextureImageReader Memory_Source(std::vector<std::byte> source)
{
    return [source=std::move(source)](std::size_t prefix,std::vector<std::byte>& bytes,std::size_t& size) {
        if (prefix > source.size()) return false;
        size=source.size();
        bytes.assign(source.begin(),source.begin()+(prefix ? prefix : size));
        return true;
    };
}

std::vector<std::byte> TGA(std::array<unsigned,4> colors)
{
    std::vector<std::byte> bytes(18+16);
    bytes[2]=std::byte{2}; bytes[12]=bytes[14]=std::byte{2};
    bytes[16]=std::byte{32}; bytes[17]=std::byte{0x28};
    for (unsigned i=0;i<4;++i) for (unsigned c=0;c<4;++c)
        bytes[18+i*4+c]=std::byte(colors[i]>>(c*8));
    return bytes;
}

std::vector<std::byte> DDS(RHITextureDimension dimension,unsigned extent=4,unsigned mips=1)
{
    const unsigned layers=dimension==RHITextureDimension::Cube ? 6 : 1;
    const unsigned depth=dimension==RHITextureDimension::Volume ? 2 : 1;
    std::vector<std::byte> bytes(128);
    const auto write=[&](unsigned offset,unsigned value) {
        for (unsigned c=0;c<4;++c) bytes[offset+c]=std::byte(value>>(c*8));
    };
    write(0,0x20534444); write(4,124); write(12,extent); write(16,extent); write(28,mips);
    write(76,32); write(80,4); write(84,0x33545844);
    if (dimension==RHITextureDimension::Cube) write(112,0xfe00);
    if (dimension==RHITextureDimension::Volume) { write(24,depth); write(112,0x200000); }
    for (unsigned layer=0;layer<layers;++layer) for (unsigned mip=0;mip<mips;++mip) {
        const unsigned blocks=(std::max(1u,extent>>mip)+3)/4;
        for (unsigned z=0;z<std::max(1u,depth>>mip);++z) for (unsigned i=0;i<blocks*blocks;++i) {
            const unsigned offset=static_cast<unsigned>(bytes.size());
            bytes.resize(offset+16);
            const unsigned alpha=(8+layer+mip)*0x11111111u;
            write(offset,alpha); write(offset+4,alpha);
            write(offset+8,((layer+z)%2) ? 0x07e007e0 : 0xf800f800);
        }
    }
    return bytes;
}

void Check_Drawing(Device& device,RHITextureHandle texture,const std::array<unsigned,4>& colors,unsigned mip=0)
{
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
    style.samplers[0].min_lod=style.samplers[0].max_lod=static_cast<float>(mip);
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,2,2}));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
    std::array<std::byte,16> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,8));
    for (unsigned i=0;i<4;++i) for (unsigned c=0;c<4;++c) {
        const unsigned shift=c==3 ? 24 : (2-c)*8;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+c])-int((colors[i]>>shift)&255),1);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
}

BOOST_AUTO_TEST_CASE(queued_image_decodes_on_worker_and_publishes_complete_rgb_and_alpha_on_owner)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        ResourceLoadQueue queue;
        BOOST_REQUIRE(queue.Start());
        const auto owner=std::this_thread::get_id();
        std::thread::id header_thread,decode_thread,publish_thread;
        const std::array<unsigned,4> colors{0x00ff0000,0x4000ff00,0x800000ff,0xffffffff};
        TextureLoadRequest request;
        request.device=&device; request.mips.requested_count=1;
        request.read_tga=[reader=Memory_Source(TGA(colors)),&header_thread,&decode_thread]
            (std::size_t prefix,std::vector<std::byte>& bytes,std::size_t& size) {
            (prefix ? header_thread : decode_thread)=std::this_thread::get_id();
            return reader(prefix,bytes,size);
        };
        unsigned publications=0;
        std::unique_ptr<TextureResource> published;
        auto source=std::make_shared<const ResourceLoadSource>([&]() {
            return std::make_unique<TextureLoadJob>(request,[&](TextureResource* resource) {
                ++publications; publish_thread=std::this_thread::get_id(); published.reset(resource);
            });
        });
        BOOST_REQUIRE(queue.Request(source,ResourceLoadPriority::Background));
        BOOST_CHECK(!published);
        source.reset(); // An already created job owns its immutable request.
        BOOST_REQUIRE(queue.Drain());
        BOOST_REQUIRE(published);
        BOOST_CHECK_EQUAL(publications,1u);
        BOOST_CHECK(header_thread==owner); BOOST_CHECK(decode_thread!=owner); BOOST_CHECK(publish_thread==owner);
        BOOST_CHECK(!published->Is_Placeholder());
        Check_Drawing(device,published->Handle(),colors);
        BOOST_REQUIRE(queue.Shutdown());
    }
}

BOOST_AUTO_TEST_CASE(dds_mip_selection_and_publication_preserve_retained_draw_generation)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        std::unique_ptr<TextureResource> published;
        RHITextureHandle retained{};
        for (unsigned generation=0;generation<2;++generation) {
            Get_Texture_Quality_Settings()={static_cast<int>(generation),1,false};
            const auto quality=Get_Texture_Quality_Settings();
            TextureLoadRequest request;
            request.device=&device;
            request.mips.reduction=static_cast<unsigned>(quality.mip_reduction);
            request.mips.minimum_dimension=static_cast<unsigned>(quality.minimum_dimension);
            request.prefer_16_bits=quality.prefer_16_bits;
            request.read_dds=Memory_Source(DDS(RHITextureDimension::Texture2D,16,5));
            TextureLoadJob job(request,[&](TextureResource* resource) { published.reset(resource); });
            BOOST_REQUIRE(job.Prepare());
            // A quality change affects the next load, not this prepared upload
            // or a retained texture generation that is still being drawn.
            Get_Texture_Quality_Settings()={4,32,true};
            bool decoded=false;
            { std::jthread worker([&] { decoded=job.Decode(); }); }
            BOOST_REQUIRE(decoded);
            job.Complete(decoded);
            BOOST_REQUIRE(published);
            BOOST_CHECK(!published->Is_Placeholder());
            BOOST_CHECK_EQUAL(published->Description().width,16u>>generation);
            BOOST_CHECK_EQUAL(published->Description().mip_count,3u-generation);
            if (!generation) { retained=published->Handle(); BOOST_REQUIRE(device.Retain_Texture(retained)); }
            for (unsigned mip=0;mip<published->Description().mip_count;++mip) {
                const unsigned color=((8+generation+mip)*17u<<24)|0xff0000u;
                Check_Drawing(device,published->Handle(),{color,color,color,color},mip);
            }
        }
        published.reset();
        Check_Drawing(device,retained,{0x88ff0000,0x88ff0000,0x88ff0000,0x88ff0000});
        BOOST_REQUIRE(device.Destroy_Texture(retained));
        BOOST_CHECK(!device.Retain_Texture(retained));
        Get_Texture_Quality_Settings()={};
    }
}

BOOST_AUTO_TEST_CASE(cube_faces_and_compressed_volume_slices_reach_their_allocated_subresources)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        for (const auto dimension : {RHITextureDimension::Cube,RHITextureDimension::Volume}) {
            TextureLoadRequest request;
            request.device=&device; request.dimension=dimension; request.mips.requested_count=1;
            request.read_dds=Memory_Source(DDS(dimension));
            std::unique_ptr<TextureResource> published;
            TextureLoadJob job(request,[&](TextureResource* resource) { published.reset(resource); });
            BOOST_REQUIRE(job.Prepare());
            BOOST_REQUIRE(job.Decode()); job.Complete(true);
            BOOST_REQUIRE(published);
            BOOST_CHECK(!published->Is_Placeholder());
            if (dimension==RHITextureDimension::Cube) {
                BOOST_CHECK_EQUAL(published->Description().array_size,6u);
                for (unsigned face=0;face<6;++face) {
                    std::array<std::byte,16> bytes{};
                    BOOST_REQUIRE(device.Readback_Texture_Subresource(published->Handle(),{bytes,16,0,0,face}));
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[0]),(8+face)*17u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[8]),face%2 ? 0xe0u : 0u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[9]),face%2 ? 7u : 0xf8u);
                }
            } else {
                BOOST_CHECK(published->Encoding()==Assets::PixelEncoding::BGRA8);
                BOOST_CHECK_EQUAL(published->Description().depth,2u);
                std::array<std::byte,128> bytes{};
                BOOST_REQUIRE(device.Readback_Texture_Subresource(published->Handle(),{bytes,16,64}));
                for (unsigned z=0;z<2;++z) for (unsigned i=0;i<16;++i) {
                    const unsigned offset=z*64+i*4;
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[offset]),0u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[offset+1]),z ? 255u : 0u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[offset+2]),z ? 0u : 255u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[offset+3]),136u);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(failed_source_never_publishes_partial_pixels_and_completion_is_single_use)
{
    GraphicsTestDevice device({true});
    for (const bool fail_header : {true,false}) {
        TextureLoadRequest request;
        request.device=&device; request.mips.requested_count=1;
        const auto reader=Memory_Source(TGA({0xff0000ff,0xff0000ff,0xff0000ff,0xff0000ff}));
        request.read_tga=[reader,fail_header](std::size_t prefix,std::vector<std::byte>& bytes,std::size_t& size) {
            if (fail_header || !prefix) return false;
            return reader(prefix,bytes,size);
        };
        std::unique_ptr<TextureResource> published;
        unsigned publications=0;
        TextureLoadJob job(request,[&](TextureResource* resource) { ++publications; published.reset(resource); });
        BOOST_CHECK_EQUAL(job.Prepare(),!fail_header);
        BOOST_CHECK(!job.Decode());
        BOOST_CHECK(!published);
        job.Complete(false); job.Complete(false);
        BOOST_REQUIRE(published);
        BOOST_CHECK(published->Is_Placeholder());
        BOOST_CHECK_EQUAL(publications,1u);
        Check_Drawing(device,published->Handle(),{0xffff00ff,0xff000000,0xff000000,0xffff00ff});
    }
}

BOOST_AUTO_TEST_CASE(single_tga_initializes_every_requested_cube_face)
{
    GraphicsTestDevice device({true});
    TextureLoadRequest request;
    request.device=&device; request.dimension=RHITextureDimension::Cube; request.mips.requested_count=1;
    const std::array<unsigned,4> colors{0x00ff0000,0x4000ff00,0x800000ff,0xffffffff};
    request.read_tga=Memory_Source(TGA(colors));
    std::unique_ptr<TextureResource> published;
    TextureLoadJob job(request,[&](TextureResource* resource) { published.reset(resource); });
    BOOST_REQUIRE(job.Prepare()); BOOST_REQUIRE(job.Decode()); job.Complete(true);
    BOOST_REQUIRE(published);
    BOOST_CHECK(!published->Is_Placeholder());
    for (unsigned face=0;face<6;++face) {
        std::array<std::byte,16> bytes{};
        BOOST_REQUIRE(device.Readback_Texture_Subresource(published->Handle(),{bytes,8,0,0,face}));
        for (unsigned i=0;i<4;++i) for (unsigned c=0;c<4;++c)
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[i*4+c]),(colors[i]>>(c*8))&255u);
    }
}

BOOST_AUTO_TEST_CASE(authored_color_shift_and_precision_decode_into_the_actual_image_encoding)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        TextureLoadRequest request;
        request.device=&device; request.mips.requested_count=1;
        request.prefer_16_bits=true; request.hsv_shift={120,0,0};
        request.read_tga=Memory_Source(TGA({0x00ff0000,0x40ff0000,0x80ff0000,0xffff0000}));
        std::unique_ptr<TextureResource> published;
        TextureLoadJob job(request,[&](TextureResource* resource) { published.reset(resource); });
        BOOST_REQUIRE(job.Prepare()); BOOST_REQUIRE(job.Decode()); job.Complete(true);
        BOOST_REQUIRE(published);
        BOOST_CHECK(published->Encoding()==Assets::PixelEncoding::BGRA8);
        Check_Drawing(device,published->Handle(),{0x0000ff00,0x4000ff00,0x8000ff00,0xff00ff00});
    }
}
