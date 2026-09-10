module;
#define BOOST_TEST_MODULE MovieCaptureTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
export module Graphics.Capture.MovieCapture.Tests;
import Graphics.Capture.MovieCapture;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(recorded_gpu_colors_pause_single_frame_and_resize_segments)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    const auto directory = std::filesystem::temp_directory_path() / "generals_graphics_capture_test";
    std::filesystem::create_directories(directory);
    const auto base = directory / "frame";
    for (int i = 0; i != 3; ++i)
        std::filesystem::remove(directory / ("frame" + std::to_string(i) + ".AVI"));
    MovieCapture capture;
    BOOST_REQUIRE(capture.Start(base, 0));
    auto& commands = device.Immediate_Command_List();
    const std::array formats{RHITextureFormat::RGBA8_UNorm, RHITextureFormat::BGRA8_UNorm};
    for (unsigned segment = 0; segment != formats.size(); ++segment) {
        const unsigned width = segment + 1;
        const auto texture = device.Create_Texture({width, 1, 1, formats[segment],
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        BOOST_REQUIRE(texture.Is_Valid());
        const auto depth = device.Create_Texture({width, 1, 1, RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(depth.Is_Valid());
        BOOST_REQUIRE(commands.Set_Render_Targets(texture, depth));
        BOOST_REQUIRE(commands.Clear({1, 0.25f, 0, 1}, 1));
        // Paused calls neither read back nor create a file.
        BOOST_REQUIRE(capture.Capture(device, {texture, width, 1}, formats[segment]));
        BOOST_CHECK(!std::filesystem::exists(directory / ("frame" + std::to_string(segment) + ".AVI")));
        capture.Request_Frame();
        BOOST_REQUIRE(capture.Capture(device, {texture, width, 1}, formats[segment]));
        BOOST_REQUIRE(capture.Capture(device, {texture, width, 1}, formats[segment]));
        BOOST_REQUIRE(device.Destroy_Texture(texture));
        BOOST_REQUIRE(device.Destroy_Texture(depth));
    }
    BOOST_REQUIRE(capture.Stop());
    for (unsigned segment = 0; segment != formats.size(); ++segment) {
        const auto path = directory / ("frame" + std::to_string(segment) + ".AVI");
        std::ifstream input(path, std::ios::binary);
        const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input), {}};
        BOOST_REQUIRE(bytes.size() >= 260);
        BOOST_CHECK_EQUAL(bytes[48], 1); // One requested frame per segment.
        BOOST_CHECK_EQUAL(bytes[64], segment + 1);
        BOOST_CHECK_EQUAL(bytes[232], 0); // AVI BGR, independent of source texture format.
        BOOST_CHECK_SMALL(int(bytes[233]) - 64, 1);
        BOOST_CHECK_EQUAL(bytes[234], 255);
        input.close();
        std::filesystem::remove(path);
    }
    BOOST_REQUIRE(capture.Start(base, 30));
    BOOST_CHECK(!capture.Capture(device, {{}, 1, 1}, RHITextureFormat::RGBA8_UNorm));
    BOOST_CHECK(!capture.Is_Active());
    std::filesystem::remove(directory);
}
