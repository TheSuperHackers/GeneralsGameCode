module;
#define BOOST_TEST_MODULE FramePreviewTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>
export module Graphics.Capture.FramePreview.Tests;
import Graphics.Capture.FramePreview;
import Graphics.FrameTargets;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(rotating_targets_share_preview_interval_and_replacement_invalidates_it)
{
    GraphicsTestDevice device({true});
    FramePreview preview;
    BOOST_REQUIRE(preview.Initialize(device, Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    constexpr auto format = RHITextureFormat::RGBA8_UNorm;
    constexpr std::uint32_t extent = 8;
    const auto depth = device.Create_Texture({extent, extent, 1, RHITextureFormat::D24_UNorm_S8,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(depth.Is_Valid());
    std::array<RHITextureHandle, 3> colors{};
    for (std::size_t channel = 0; channel < colors.size(); ++channel) {
        std::array<std::byte, extent * extent * 4> pixels{};
        for (std::size_t pixel = 0; pixel < extent * extent; ++pixel) {
            pixels[pixel * 4 + channel] = std::byte{255};
            pixels[pixel * 4 + 3] = std::byte{128};
        }
        colors[channel] = device.Create_Texture_Initialized({extent, extent, 1, format,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)}, {pixels, extent * 4});
        BOOST_REQUIRE(colors[channel].Is_Valid());
    }
    const auto check = [&](unsigned buffer, std::uint64_t identity, std::uint32_t time, unsigned expected) {
        const auto frame = preview.Read({{colors[buffer], extent, extent}, {depth, extent, extent}, identity},
            format, extent, time, 500);
        BOOST_REQUIRE(frame.Is_Valid());
        for (std::uint32_t y = 0; y < frame.height; ++y)
            for (std::uint32_t x = 0; x < frame.width; ++x)
                for (unsigned channel = 0; channel < 4; ++channel)
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(frame.pixels[y * frame.row_pitch + x * 4 + channel]),
                        channel == 3 ? 128u : channel == expected ? 255u : 0u);
    };
    check(0, 1, 100, 0);
    check(1, 1, 200, 0);
    check(2, 1, 599, 0);
    check(2, 1, 600, 2);
    check(1, 2, 601, 1); // Replaced target set refreshes immediately at the same dimensions.
    check(0, 0, 602, 0); // Anonymous sources retain individual texture identity.
    check(1, 0, 603, 1);
    preview.Shutdown();
    for (const auto color : colors) device.Destroy_Texture(color);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(preview_preserves_channels_orientation_throttle_and_resize)
{
    GraphicsTestDevice device({true});
    FramePreview preview;
    BOOST_REQUIRE(preview.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    auto& commands = device.Immediate_Command_List();
    std::uint32_t time = 100;
    for (const auto format : {RHITextureFormat::BGRA8_UNorm,RHITextureFormat::RGBA8_UNorm}) {
        for (const auto height : {8u,11u,32u}) {
            constexpr std::uint32_t width = 16;
            std::vector<std::byte> pixels(width*height*4);
            for (unsigned y=0;y<height;++y) for (unsigned x=0;x<width;++x) {
                const unsigned red = format == RHITextureFormat::BGRA8_UNorm ? 2 : 0;
                const unsigned blue = 2-red;
                const unsigned offset = (y*width+x)*4;
                pixels[offset + (y<height/2 ? red : blue)] = std::byte{255};
                pixels[offset + 3] = std::byte{x<width/2 ? std::uint8_t{64} : std::uint8_t{192}};
            }
            const auto color = device.Create_Texture_Initialized({width,height,1,format,
                static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)}, {pixels,width*4});
            const auto depth = device.Create_Texture({width,height,1,RHITextureFormat::D24_UNorm_S8,
                static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(color.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
            const FrameTargets targets{{color,width,height},{depth,width,height}};
            auto frame = preview.Read(targets,format,10,time,500);
            BOOST_REQUIRE(frame.Is_Valid());
            BOOST_CHECK_EQUAL(frame.width,8u);
            BOOST_CHECK_EQUAL(frame.height,height==32 ? 8u : 4u);
            const auto corner = [&](unsigned x,unsigned y,unsigned channel) {
                return std::to_integer<unsigned>(frame.pixels[y*frame.row_pitch+x*4+channel]);
            };
            BOOST_CHECK_EQUAL(corner(0,0,0),255u);
            BOOST_CHECK_EQUAL(corner(0,0,2),0u);
            BOOST_CHECK_EQUAL(corner(7,frame.height-1,2),255u);
            BOOST_CHECK_EQUAL(corner(7,frame.height-1,0),0u);
            BOOST_CHECK_SMALL(static_cast<int>(corner(0,0,3))-64,2);
            BOOST_CHECK_SMALL(static_cast<int>(corner(7,0,3))-192,2);
            const std::vector<std::byte> first(frame.pixels.begin(),frame.pixels.end());
            // Read restores the frame attachments. A following clear must target
            // the source, leaving the returned preview untouched until refresh.
            BOOST_REQUIRE(commands.Clear({0,1,0,1},1));
            frame = preview.Read(targets,format,8,time+499,500);
            BOOST_REQUIRE(frame.Is_Valid());
            BOOST_CHECK(std::equal(frame.pixels.begin(),frame.pixels.end(),first.begin(),first.end()));
            frame = preview.Read(targets,format,8,time+500,500);
            BOOST_REQUIRE(frame.Is_Valid());
            BOOST_CHECK_EQUAL(corner(0,0,0),0u);
            BOOST_CHECK_EQUAL(corner(0,0,1),255u);
            BOOST_CHECK_EQUAL(corner(0,0,3),255u);
            device.Destroy_Texture(color); device.Destroy_Texture(depth);
            time += 501;
        }
    }
    preview.Shutdown();
    BOOST_CHECK(!preview.Read({},RHITextureFormat::RGBA8_UNorm,8,time,500).Is_Valid());
}
