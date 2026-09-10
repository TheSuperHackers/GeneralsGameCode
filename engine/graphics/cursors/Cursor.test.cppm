module;
#define BOOST_TEST_MODULE CursorTests
#define NOMINMAX
#include <boost/test/included/unit_test.hpp>
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
export module Graphics.Cursors.Cursor.Tests;
import Graphics.Cursors.Cursor;
using namespace Graphics;

namespace
{
struct Video final
{
    Video() { BOOST_REQUIRE(SDL_Init(SDL_INIT_VIDEO)); }
    ~Video() { SDL_Quit(); }
};
struct Pixels final
{
    std::vector<std::byte> bytes = std::vector<std::byte>(32 * 32 * 4);
    Pixels(unsigned red = 255)
    {
        for (unsigned y = 0; y < 32; ++y) for (unsigned x = 0; x < 32; ++x) {
            auto* pixel = bytes.data() + (y * 32 + x) * 4;
            pixel[0] = std::byte(red);
            pixel[3] = std::byte(x < 8 ? 0 : x < 16 ? 128 : 255);
        }
    }
    Assets::ImageView View() const { return {bytes,32,32,128,Assets::PixelEncoding::RGBA8}; }
};
}

BOOST_AUTO_TEST_CASE(selection_is_explicit_failed_replacement_preserves_cursor_and_reset_respects_other_owners)
{
    Video video;
    Pixels pixels;
    const std::array frames{CursorFrame{pixels.View(), 100}};
    Cursor first, second;
    SDL_Cursor* initial = SDL_GetCursor();
    BOOST_REQUIRE(first.Initialize(frames, 4, 7));
    BOOST_CHECK(SDL_GetCursor() == initial);
    BOOST_REQUIRE(first.Show());
    SDL_Cursor* selected = SDL_GetCursor();
    BOOST_CHECK(selected != initial);
    BOOST_REQUIRE(first.Show());
    BOOST_CHECK(SDL_GetCursor() == selected);
    BOOST_CHECK(!first.Initialize({}, 0, 0));
    const std::array invalid{CursorFrame{{},100}};
    BOOST_CHECK(!first.Initialize(invalid, 0, 0));
    auto mismatch = pixels.View();
    mismatch.height = 31;
    const std::array mixed{frames[0],CursorFrame{mismatch,100}};
    BOOST_CHECK(!first.Initialize(mixed, 0, 0));
    BOOST_CHECK(SDL_GetCursor() == selected);
    BOOST_REQUIRE(second.Initialize(frames, 0, 0));
    BOOST_REQUIRE(second.Show());
    selected = SDL_GetCursor();
    first.Reset();
    BOOST_CHECK(SDL_GetCursor() == selected);
    BOOST_CHECK(!first.Show());
    second.Reset();
    BOOST_CHECK(SDL_GetCursor() == SDL_GetDefaultCursor());
    BOOST_CHECK(!second.Show());
}

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(native_animation_draws_all_frames_with_alpha_and_hotspots_after_resize)
{
    Video video;
    POINT previous_position{};
    GetCursorPos(&previous_position);
    SDL_Window* window = SDL_CreateWindow("Cursor drawing test", 160, 120, SDL_WINDOW_ALWAYS_ON_TOP);
    BOOST_REQUIRE(window);
    struct Cleanup final {
        SDL_Window* window;
        POINT position;
        ~Cleanup() { SDL_DestroyWindow(window); SetCursorPos(position.x, position.y); }
    } cleanup{window, previous_position};
    SDL_RaiseWindow(window);
    SDL_WarpMouseInWindow(window, 80, 60);
    for (unsigned i = 0; i < 10 && SDL_GetMouseFocus() != window; ++i) {
        SDL_Delay(10);
        SDL_PumpEvents();
    }
    BOOST_REQUIRE(SDL_GetMouseFocus() == window);
    std::vector<Pixels> images;
    std::vector<CursorFrame> frames;
    for (unsigned i = 0; i < 80; ++i) images.emplace_back(80 + i * 2);
    for (const auto& image : images) frames.push_back({image.View(), 100});
    Cursor cursor;
    BOOST_REQUIRE(cursor.Initialize(frames, 5, 9));
    BOOST_REQUIRE(cursor.Show());
    SDL_Cursor* selected = SDL_GetCursor();
    for (int size : {160,200,160}) {
        BOOST_REQUIRE(SDL_SetWindowSize(window, size, 120));
        SDL_PumpEvents();
        BOOST_REQUIRE(cursor.Show());
        BOOST_CHECK(SDL_GetCursor() == selected);
        const HCURSOR native = GetCursor();
        BOOST_REQUIRE(native != nullptr);
        ICONINFO info{};
        BOOST_REQUIRE(GetIconInfo(native, &info));
        BOOST_CHECK_EQUAL(info.xHotspot, 5);
        BOOST_CHECK_EQUAL(info.yHotspot, 9);
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);
        BITMAPINFO bitmap{};
        bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmap.bmiHeader.biWidth = 32;
        bitmap.bmiHeader.biHeight = -32;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = 32;
        void* memory = nullptr;
        HBITMAP target = CreateDIBSection(nullptr, &bitmap, DIB_RGB_COLORS, &memory, nullptr, 0);
        BOOST_REQUIRE(target != nullptr);
        HDC dc = CreateCompatibleDC(nullptr);
        BOOST_REQUIRE(dc != nullptr);
        HGDIOBJ previous = SelectObject(dc, target);
        auto* output = static_cast<std::uint32_t*>(memory);
        for (unsigned frame : {0u,21u,63u,79u}) {
            std::fill_n(output, 32 * 32, 0x00204060u);
            BOOST_REQUIRE(DrawIconEx(dc, 0, 0, native, 32, 32, frame, nullptr, DI_NORMAL));
            GdiFlush();
            const unsigned red = 80 + frame * 2;
            BOOST_CHECK_EQUAL(output[3] & 0xffffff, 0x204060u);
            BOOST_CHECK_EQUAL((output[12] >> 16) & 255, (red * 128 + 127) / 255 + 16);
            BOOST_CHECK_EQUAL((output[12] >> 8) & 255, 32);
            BOOST_CHECK_EQUAL(output[12] & 255, 48);
            BOOST_CHECK_EQUAL(output[24] & 0xffffff, red << 16);
        }
        SelectObject(dc, previous);
        DeleteDC(dc);
        DeleteObject(target);
    }
}
#endif
