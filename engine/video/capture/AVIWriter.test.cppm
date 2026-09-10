module;
#define BOOST_TEST_MODULE AVIWriterTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <span>
#include <vector>
export module Video.Capture.AVIWriter.Tests;
import Video.Capture.AVIWriter;
using namespace Engine::Video;

BOOST_AUTO_TEST_CASE(capture_preserves_colors_row_orientation_pitch_and_index)
{
    const auto path=std::filesystem::path("capture-colors.avi");
    struct RemoveFile { std::filesystem::path path; ~RemoveFile() { std::error_code error; std::filesystem::remove(path,error); } } cleanup{path};
    AVIWriter writer;
    BOOST_REQUIRE(writer.Open(path,1,2,29.97f));
    // Each source row has four bytes of padding; AVI rows have one byte.
    const std::array<std::uint8_t,16> pixels{255,0,0,128,91,92,93,94,0,0,255,255,95,96,97,98};
    DecodedVideoFrame frame;
    frame.width=1; frame.height=2; frame.row_pitch=8;
    frame.format=PixelFormat::RGBA8; frame.pixels=std::as_bytes(std::span(pixels));
    BOOST_REQUIRE(writer.Append(frame));
    frame.width=2;
    BOOST_CHECK(!writer.Append(frame));
    BOOST_CHECK_EQUAL(writer.Frame_Count(),1);
    frame.width=1; frame.format=PixelFormat::BGRA8;
    BOOST_REQUIRE(writer.Append(frame));
    BOOST_REQUIRE(writer.Close());
    BOOST_REQUIRE(writer.Close());
    BOOST_CHECK(!writer.Is_Open());
    std::ifstream input(path,std::ios::binary);
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input),{}};
    const auto u32=[&](std::size_t offset) {
        BOOST_REQUIRE_LE(offset+4,bytes.size());
        return std::uint32_t(bytes[offset]) | std::uint32_t(bytes[offset+1])<<8
            | std::uint32_t(bytes[offset+2])<<16 | std::uint32_t(bytes[offset+3])<<24;
    };
    BOOST_REQUIRE_EQUAL(bytes.size(),296u);
    BOOST_CHECK_EQUAL(u32(4),bytes.size()-8);
    BOOST_CHECK_EQUAL(u32(48),2); BOOST_CHECK_EQUAL(u32(140),2);
    BOOST_CHECK_EQUAL(u32(128),1000); BOOST_CHECK_EQUAL(u32(132),29970);
    BOOST_CHECK_EQUAL(u32(216),36);
    const std::array<unsigned char,8> first{255,0,0,0,0,0,255,0};
    const std::array<unsigned char,8> second{0,0,255,0,255,0,0,0};
    BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(),first.end(),bytes.begin()+232,bytes.begin()+240);
    BOOST_CHECK_EQUAL_COLLECTIONS(second.begin(),second.end(),bytes.begin()+248,bytes.begin()+256);
    BOOST_CHECK_EQUAL(u32(260),32); // Two index entries.
    BOOST_CHECK_EQUAL(u32(272),4); BOOST_CHECK_EQUAL(u32(288),20);
    BOOST_CHECK_EQUAL(u32(276),8); BOOST_CHECK_EQUAL(u32(292),8);
}

BOOST_AUTO_TEST_CASE(capture_rejects_invalid_input_and_finalizes_on_destruction)
{
    const auto path=std::filesystem::path("capture-lifetime.avi");
    struct RemoveFile { std::filesystem::path path; ~RemoveFile() { std::error_code error; std::filesystem::remove(path,error); } } cleanup{path};
    {
        AVIWriter writer;
        BOOST_CHECK(!writer.Open(path,0,1,30));
        BOOST_CHECK(!writer.Open(path,1,1,0));
        BOOST_CHECK(!writer.Open(path,1,1,std::numeric_limits<float>::infinity()));
        BOOST_REQUIRE(writer.Open(path,1,1,30));
        BOOST_CHECK(!writer.Open(path,1,1,30));
        std::array<std::byte,3> rgb{std::byte{1},std::byte{2},std::byte{3}};
        DecodedVideoFrame frame;
        frame.width=frame.height=1; frame.row_pitch=3;
        frame.format=PixelFormat::RGB8; frame.pixels=rgb;
        BOOST_REQUIRE(writer.Append(frame));
    }
    std::ifstream input(path,std::ios::binary);
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input),{}};
    BOOST_REQUIRE_EQUAL(bytes.size(),260u);
    BOOST_CHECK_EQUAL(bytes[48],1); BOOST_CHECK_EQUAL(bytes[140],1);
    BOOST_CHECK_EQUAL(bytes[232],3); BOOST_CHECK_EQUAL(bytes[234],1);
}
