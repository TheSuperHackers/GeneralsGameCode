module;
#define BOOST_TEST_MODULE ProjectedShadowDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Shadows.Projected.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Shadows.Projected;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(gpu_shadow_texture_preserves_cutouts_and_background_after_recreation)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const std::array<std::uint8_t,8> cutout_pixels{0,0,0,0,0,0,0,255};
    const auto cutout=device.Create_Texture_Initialized({2,1},
        {std::as_bytes(std::span(cutout_pixels)),8});
    BOOST_REQUIRE(cutout.Is_Valid());
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    vertices[0].uv={0,1}; vertices[1].uv={1,1};
    vertices[2].uv={1,0}; vertices[3].uv={0,0};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D24_UNorm_S8,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(target.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
    auto& commands=device.Immediate_Command_List();
    for (unsigned pass=0;pass<2;++pass) {
        const unsigned extent=pass==0 ? 8 : 16;
        const auto mask=device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)|static_cast<unsigned>(RHITextureUsage::ShaderResource)});
        const auto mask_depth=device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(mask.Is_Valid());
        BOOST_REQUIRE(mask_depth.Is_Valid());
        BOOST_REQUIRE(commands.Set_Render_Targets(mask,mask_depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,extent,extent}));
        BOOST_REQUIRE(commands.Clear({1,1,1,1},1));
        PropParameters caster;
        caster.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        caster.world[0]=0.5f;
        caster.alpha_cutoff=0.5f;
        PropStyle style;
        style.depth_test=false; style.depth_write=false;
        style.blend=RHIBlendMode::Disabled;
        style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,caster,std::span(&cutout,1)));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
        BOOST_REQUIRE(commands.Clear({0.8f,0.6f,0.4f,1},1));
        PropParameters receiver;
        receiver.view_projection=caster.view_projection;
        BOOST_REQUIRE(Draw_Projected_Shadow(renderer,commands,mesh,receiver,std::span(&mask,1)));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        // Outside the caster and inside its transparent half, the terrain
        // remains unchanged. The opaque half projects a black silhouette.
        for (unsigned x : {0u,2u,5u,7u}) {
            const std::array expected=x==5 ? std::array{0,0,0,255} : std::array{204,153,102,255};
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+x)*4+channel])-expected[channel],2);
        }
        device.Destroy_Texture(mask);
        device.Destroy_Texture(mask_depth);
        if (pass==0) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,shaders));
        }
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(cutout); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(decal_blends_and_projected_receiver_cutout)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,4> texel{128,64,32,128};
    const auto texture=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(texel)),4});
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D24_UNorm_S8,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto check=[&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    };
    BOOST_REQUIRE(commands.Clear({0.5f,0.5f,0.5f,1},1));
    BOOST_REQUIRE(Draw_Decal(renderer,commands,mesh,parameters,texture,DecalBlend::Multiply));
    check({64,32,16,128});
    BOOST_REQUIRE(commands.Clear({0.5f,0.5f,0.5f,1},1));
    BOOST_REQUIRE(Draw_Decal(renderer,commands,mesh,parameters,texture,DecalBlend::Alpha));
    check({128,96,80,191});
    BOOST_REQUIRE(commands.Clear({0.25f,0.25f,0.25f,0},1));
    BOOST_REQUIRE(Draw_Decal(renderer,commands,mesh,parameters,texture,DecalBlend::Additive));
    check({192,128,96,128});
    // Receiver geometry behind the depth buffer must not mark or darken it.
    BOOST_REQUIRE(commands.Clear({1,1,1,1},0.25f));
    BOOST_REQUIRE(Draw_Projected_Shadow(renderer,commands,mesh,parameters,std::span(&texture,1)));
    check({255,255,255,255});
    BOOST_REQUIRE(commands.Clear({1,1,1,1},1));
    parameters.alpha_cutoff=0.75f;
    BOOST_REQUIRE(Draw_Projected_Shadow(renderer,commands,mesh,parameters,std::span(&texture,1)));
    check({255,255,255,255});
    parameters.alpha_cutoff=96.0f/255.0f;
    BOOST_REQUIRE(Draw_Projected_Shadow(renderer,commands,mesh,parameters,std::span(&texture,1)));
    check({128,64,32,128});
    // Perspective projection uses homogeneous division before sampling.
    const std::array<std::uint8_t,8> pattern{255,0,0,255,0,255,0,255};
    const auto projected=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(pattern)),8});
    parameters.uv_sources={1,1,0,0};
    parameters.uv_transform[0]={0,0,0,0.75f,0,0,0,0.5f,0,0,0,0,0,0,0,2};
    BOOST_REQUIRE(commands.Clear({1,1,1,1},1));
    BOOST_REQUIRE(Draw_Projected_Shadow(renderer,commands,mesh,parameters,std::span(&projected,1)));
    check({191,64,0,255});
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(projected); device.Destroy_Texture(texture);
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

