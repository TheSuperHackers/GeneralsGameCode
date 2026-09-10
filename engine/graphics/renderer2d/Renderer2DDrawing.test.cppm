module;
#define BOOST_TEST_MODULE Renderer2DDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Renderer2D.Drawing.Tests;
import Graphics.Renderer2D;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(editor_overlay_widths_outlines_and_letterbox_follow_frame_dimensions)
{
    GraphicsTestDevice device({true});
    Renderer2D renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    for (const unsigned size : {64u, 96u, 64u}) {
        const auto target = device.Create_Texture({size, size, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({size, size, 1, RHITextureFormat::D24_UNorm_S8,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 0));
        renderer.Begin(size, size);
        // Editor coordinates retain their half-pixel placement. Overlays must
        // remain visible over scene depth and keep authored pixel widths.
        BOOST_REQUIRE(renderer.Add_Line({7.5f, 7.5f}, {31.5f, 7.5f}, 4, Color2D::From_ARGB(0xff00ff00)));
        BOOST_REQUIRE(renderer.Add_Line({53.5f, 7.5f}, {53.5f, 31.5f}, 2, Color2D::From_ARGB(0xffff0000)));
        BOOST_REQUIRE(renderer.Add_Line({31.5f, 31.5f}, {47.5f, 47.5f}, 4, Color2D::From_ARGB(0xffffffff)));
        BOOST_REQUIRE(renderer.Add_Outline({3.5f, 15.5f, 19.5f, 31.5f}, 2, Color2D::From_ARGB(0xffffff00)));
        BOOST_REQUIRE(renderer.Add_Rect({-0.5f, size - 8.5f, size - 0.5f, size - 0.5f}, {0, 0, 0, 1}));
        BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, size, size}));
        std::vector<std::byte> pixels(size * size * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, size * 4));
        const auto check = [&](unsigned x, unsigned y, std::array<int, 3> rgb) {
            for (unsigned c = 0; c < 3; ++c)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y * size + x) * 4 + c]) - rgb[c], 2);
        };
        check(16, 7, {0, 255, 0}); check(16, 4, {0, 0, 255}); check(16, 10, {0, 0, 255});
        check(53, 20, {255, 0, 0}); check(51, 20, {0, 0, 255}); check(55, 20, {0, 0, 255});
        check(40, 40, {255, 255, 255}); check(35, 42, {0, 0, 255});
        check(4, 24, {255, 255, 0}); check(12, 16, {255, 255, 0});
        check(19, 24, {255, 255, 0}); check(12, 31, {255, 255, 0}); check(12, 24, {0, 0, 255});
        check(size / 2, size - 4, {0, 0, 0}); check(size - 4, size - 4, {0, 0, 0});
        check(size - 4, size - 12, {0, 0, 255});
        device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
    renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(rotated_atlas_image_retains_alpha_orientation_and_scissor_order)
{
    GraphicsTestDevice device({true});
    Renderer2D renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({64, 64, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64, 64, 1, RHITextureFormat::D24_UNorm_S8,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<std::byte, 4 * 4 * 4> texels{};
    const std::array<std::array<unsigned char, 4>, 4> colors = {{{255, 0, 0, 128},
        {0, 255, 0, 128}, {0, 0, 255, 128}, {255, 255, 255, 128}}};
    for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x)
        for (unsigned c = 0; c < 4; ++c)
            texels[(y * 4 + x) * 4 + c] = std::byte(colors[(y / 2) * 2 + x / 2][c]);
    const auto image = renderer.Register_Texture({TextureHandle(1, 1), 4, 4, 16, 1, texels});
    BOOST_REQUIRE(image.index.Is_Valid());
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Clear({0, 0, 0, 1}, 0));
    renderer.Begin(64, 64);
    BOOST_REQUIRE(renderer.Add_Rect({0, 0, 64, 64}, Color2D::From_ARGB(0xff102030)));
    renderer.Set_Clip(true, {20, 20, 44, 44});
    BOOST_REQUIRE(renderer.Add_Quad(
        {{{15.5f, 15.5f}, {15.5f, 47.5f}, {47.5f, 15.5f}, {47.5f, 47.5f}}},
        {{{1, 0}, {0, 0}, {1, 1}, {0, 1}}}, image, Color2D{}));
    renderer.Set_Clip(false, {});
    BOOST_REQUIRE(renderer.Add_Rect({2, 2, 6, 6}, Color2D::From_ARGB(0xff804020)));
    BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, 64, 64}));
    std::array<std::byte, 64 * 64 * 4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target, pixels, 64 * 4));
    const auto check = [&](unsigned x, unsigned y, std::array<int, 3> rgb) {
        for (unsigned c = 0; c < 3; ++c)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y * 64 + x) * 4 + c]) - rgb[c], 2);
    };
    check(24, 24, {8, 144, 24}); check(24, 40, {136, 16, 24});
    check(40, 24, {136, 144, 152}); check(40, 40, {8, 16, 152});
    check(18, 24, {16, 32, 48}); check(46, 24, {16, 32, 48});
    check(24, 18, {16, 32, 48}); check(24, 46, {16, 32, 48});
    check(4, 4, {128, 64, 32});
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(texture_residency_exceeds_binding_slots_without_losing_draw_order)
{
    GraphicsTestDevice device({true});
    Renderer2D renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({160,4,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({160,4,1,RHITextureFormat::D24_UNorm_S8,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::vector<Renderer2DTexture> textures;
    for (unsigned i=0;i<2300;++i) {
        std::array<std::byte,4> pixel{};
        pixel[i%3] = pixel[3] = std::byte{255};
        const auto texture = renderer.Register_Texture({TextureHandle(i+1,1),1,1,4,1,pixel});
        BOOST_REQUIRE(texture.index.Is_Valid());
        textures.push_back(texture);
    }
    auto& commands = device.Immediate_Command_List();
    for (unsigned frame=0;frame<2;++frame) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
        renderer.Begin(160,4);
        for (unsigned i=0;i<140;++i) {
            const unsigned source = frame ? 2299-i : 2160+i;
            BOOST_REQUIRE(renderer.Add_Quad(Rect2D{float(i),0,float(i+1),4},{0,0,1,1},
                textures[source],Color2D{},Renderer2DBlendMode::Solid));
        }
        // Preserve blending and scissor changes across texture pages, including
        // a late revisit to the first covered pixel.
        renderer.Set_Clip(true,{0,0,1,4});
        BOOST_REQUIRE(renderer.Add_Rect({0,0,2,4},{1,1,1,0.5f}));
        renderer.Set_Clip(false,{});
        BOOST_REQUIRE(renderer.Add_Quad(Rect2D{145,0,146,4},{0,0,1,1},textures[0],
            Color2D{},Renderer2DBlendMode::Solid,true));
        // Asset revisions may replace storage after recording but before draw.
        // Preserve the registry's latest-resource behavior at submission.
        std::vector<std::byte> replacement((frame+2)*4);
        for (unsigned x=0;x<frame+2;++x) replacement[x*4] = replacement[x*4+3] = std::byte{255};
        const auto updated = renderer.Register_Texture({TextureHandle(1,1),frame+2,1,
            (frame+2)*4,frame+2,replacement});
        BOOST_REQUIRE(updated.index.Is_Valid());
        BOOST_REQUIRE(renderer.Execute(device,commands,target,depth,{0,0,160,4}));
        std::array<std::byte,160*4*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,160*4));
        for (unsigned x=0;x<140;++x) for (unsigned c=0;c<3;++c) {
            const unsigned source = frame ? 2299-x : 2160+x;
            const int expected = c==source%3 ? 255 : x==0 ? 128 : 0;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[x*4+c])-expected,2);
        }
        for (unsigned c=0;c<3;++c)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[145*4+c])-76,2);
    }
    renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(compressed_texture_mapping_preserves_gpu_alpha_across_device_recreation)
{
    for (unsigned cycle = 0; cycle < 2; ++cycle) {
        GraphicsTestDevice device({true});
        Renderer2D renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const unsigned size = cycle == 0 ? 16 : 24;
        const auto target = device.Create_Texture({size, size, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({size, size, 1, RHITextureFormat::D24_UNorm_S8,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        unsigned identity = 1;
        for (auto format : {RHITextureFormat::BC1_UNorm, RHITextureFormat::BC2_UNorm, RHITextureFormat::BC3_UNorm}) {
            const auto texture = device.Create_Texture({4, 4, 3, format});
            BOOST_REQUIRE(texture.Is_Valid());
            RHITextureMapping mapping;
            BOOST_REQUIRE(device.Map_Texture(texture, 0, 0, false, mapping));
            std::fill(mapping.bytes.begin(), mapping.bytes.end(), std::byte{});
            const unsigned color_offset = format == RHITextureFormat::BC1_UNorm ? 0 : 8;
            if (format == RHITextureFormat::BC2_UNorm)
                std::fill_n(mapping.bytes.begin(), 8, std::byte{0x88});
            if (format == RHITextureFormat::BC3_UNorm) mapping.bytes[0] = mapping.bytes[1] = std::byte{128};
            // Opaque red for BC1; half-alpha red for BC2/BC3.
            mapping.bytes[color_offset + 1] = std::byte{0xf8};
            BOOST_REQUIRE(device.Unmap_Texture(texture, 0, 0));
            const auto image = renderer.Register_Texture(TextureHandle(identity++, 1), texture);
            BOOST_REQUIRE(image.index.Is_Valid());
            auto& commands = device.Immediate_Command_List();
            BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
            BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
            renderer.Begin(size, size);
            BOOST_REQUIRE(renderer.Add_Quad(Rect2D{0, 0, float(size), float(size)}, {0, 0, 1, 1},
                image, Color2D{}, Renderer2DBlendMode::Alpha));
            BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, size, size}));
            std::vector<std::byte> pixels(size * size * 4);
            BOOST_REQUIRE(device.Readback_Texture(target, pixels, size * 4));
            const unsigned center = (size * (size / 2) + size / 2) * 4;
            const int alpha = format == RHITextureFormat::BC1_UNorm ? 255 : format == RHITextureFormat::BC2_UNorm ? 136 : 128;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center]) - alpha, 2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 1]), 2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 2]) - (255 - alpha), 2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 3]) - 255, 2);
            BOOST_REQUIRE(device.Destroy_Texture(texture));
        }
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
