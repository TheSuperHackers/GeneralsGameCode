module;
#define BOOST_TEST_MODULE FrameImageWriterTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <vector>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
export module Video.Capture.ImageWriter.Tests;
import Video.Capture.ImageWriter;
using namespace Engine::Video;

BOOST_AUTO_TEST_CASE(lossless_images_preserve_row_orientation_padding_and_channels)
{
    const auto path = std::filesystem::path("frame-image-test.data");
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove(path, error); } } cleanup{path};
    const std::array<unsigned char, 24> source{
        0,0,255,0, 0,255,0,64, 71,72,73,74,
        255,0,0,128, 0,255,255,255, 75,76,77,78};
    DecodedVideoFrame frame{2, 2, 12, PixelFormat::BGRA8, 0, 0, std::as_bytes(std::span(source))};
    const std::array<unsigned char, 12> expected{255,0,0, 0,255,0, 0,0,255, 255,255,0};
    for (const auto format : {FrameImageFormat::PNG, FrameImageFormat::BMP, FrameImageFormat::TGA}) {
        BOOST_REQUIRE(Write_Frame_Image(path, frame, {format}));
        std::ifstream input(path, std::ios::binary);
        const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input), {}};
        int width=0, height=0, channels=0;
        unsigned char* decoded = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &width, &height, &channels, 3);
        BOOST_REQUIRE(decoded != nullptr);
        BOOST_CHECK_EQUAL(width, 2); BOOST_CHECK_EQUAL(height, 2);
        BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), decoded, decoded+12);
        stbi_image_free(decoded);
    }
    BOOST_CHECK_EQUAL(source[8], 71); // Caller storage and padding remain untouched.
}

BOOST_AUTO_TEST_CASE(jpeg_and_gamma_output_and_invalid_input)
{
    const auto path = std::filesystem::path("frame-image-gamma.data");
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove(path, error); } } cleanup{path};
    std::array<std::byte, 8*8*4> source{};
    for (unsigned pixel=0; pixel!=64; ++pixel) {
        source[pixel*4]=std::byte{64};
        source[pixel*4+1]=std::byte{64};
        source[pixel*4+2]=std::byte{64};
    }
    DecodedVideoFrame frame{8,8,32,PixelFormat::RGBA8,0,0,source};
    BOOST_REQUIRE(Write_Frame_Image(path, frame, {FrameImageFormat::JPEG, 100, 2.0f}));
    std::ifstream input(path, std::ios::binary);
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input), {}};
    int width=0, height=0, channels=0;
    unsigned char* decoded=stbi_load_from_memory(bytes.data(),static_cast<int>(bytes.size()),&width,&height,&channels,3);
    BOOST_REQUIRE(decoded != nullptr);
    BOOST_CHECK_EQUAL(width,8); BOOST_CHECK_EQUAL(height,8);
    for (unsigned channel=0; channel!=8*8*3; ++channel)
        BOOST_CHECK_SMALL(int(decoded[channel])-128,2);
    stbi_image_free(decoded);
    frame.row_pitch=1;
    BOOST_CHECK(!Write_Frame_Image(path,frame));
    frame.row_pitch=32;
    frame.format=PixelFormat::Unknown;
    BOOST_CHECK(!Write_Frame_Image(path,frame));
}
