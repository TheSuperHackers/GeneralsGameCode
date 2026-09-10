module;
#define BOOST_TEST_MODULE CursorLoadTests
#include <boost/test/included/unit_test.hpp>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <vector>
export module Graphics.Cursors.Load.Tests;
import Graphics.Cursors.Load;

namespace
{
struct Video final
{
    Video() { BOOST_REQUIRE(SDL_Init(SDL_INIT_VIDEO)); }
    ~Video() { SDL_Quit(); }
};
std::vector<std::byte> Tga()
{
    std::vector<std::byte> bytes(18 + 32 * 32 * 4, std::byte{255});
    std::fill_n(bytes.begin(), 18, std::byte{0});
    bytes[2] = std::byte{2};
    bytes[12] = bytes[14] = std::byte{32};
    bytes[16] = std::byte{32};
    bytes[17] = std::byte{40};
    return bytes;
}
std::vector<std::byte> Dds()
{
    std::vector<std::byte> bytes(128 + 64 * 8);
    const auto u32 = [&](unsigned offset, unsigned value) {
        for (unsigned b = 0; b < 4; ++b) bytes[offset + b] = std::byte(value >> (b * 8));
    };
    u32(0, 0x20534444); u32(4, 124); u32(8, 0x81007);
    u32(12, 32); u32(16, 32); u32(20, 512); u32(28, 1);
    u32(76, 32); u32(80, 4); u32(84, 0x31545844); u32(108, 0x1000);
    return bytes;
}
}

BOOST_AUTO_TEST_CASE(authored_sequence_names_are_unbounded_and_failed_loading_preserves_selection)
{
    Video video;
    Graphics::Cursor cursor;
    std::vector<std::string> requests;
    const auto reader = [&](std::string_view path, std::vector<std::byte>& bytes) {
        requests.emplace_back(path);
        if (!path.ends_with(".tga")) return false;
        bytes = Tga();
        return true;
    };
    BOOST_REQUIRE(Graphics::Load_Cursor(cursor, {"attack", 80, 20, 5, 9}, reader));
    BOOST_REQUIRE(cursor.Show());
    SDL_Cursor* selected = SDL_GetCursor();
    BOOST_REQUIRE_EQUAL(requests.size(), 160);
    BOOST_CHECK_EQUAL(requests.front(), "attack0000.dds");
    BOOST_CHECK_EQUAL(requests[1], "attack0000.tga");
    BOOST_CHECK_EQUAL(requests.back(), "attack0079.tga");
    const auto missing = [](std::string_view, std::vector<std::byte>&) { return false; };
    BOOST_CHECK(!Graphics::Load_Cursor(cursor, {"missing", 3, 30}, missing));
    BOOST_CHECK(SDL_GetCursor() == selected);
    BOOST_CHECK(!Graphics::Load_Cursor(cursor, {"bad", -1, 30}, reader));
    BOOST_CHECK(!Graphics::Load_Cursor(cursor, {"bad", 1, std::numeric_limits<double>::infinity()}, reader));
    BOOST_CHECK(!Graphics::Load_Cursor(cursor, {"bad", 1, -1}, reader));
    requests.clear();
    BOOST_REQUIRE(Graphics::Load_Cursor(cursor, {"arrow", 1, 0}, reader));
    BOOST_CHECK_EQUAL(requests[1], "arrow.tga");
}

BOOST_AUTO_TEST_CASE(dds_only_sources_load_without_requesting_absent_tga_images)
{
    Video video;
    Graphics::Cursor cursor;
    unsigned reads = 0;
    const auto reader = [&](std::string_view path, std::vector<std::byte>& bytes) {
        ++reads;
        BOOST_CHECK_EQUAL(path, "arrow.dds");
        bytes = Dds();
        return true;
    };
    BOOST_REQUIRE(Graphics::Load_Cursor(cursor, {"arrow", 1, 0}, reader));
    BOOST_CHECK_EQUAL(reads, 1);
    BOOST_REQUIRE(cursor.Show());
}
