module;
#define BOOST_TEST_MODULE DisplayModesTests
#include <boost/test/included/unit_test.hpp>
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
export module Graphics.Presentation.DisplayModes.Tests;
import Graphics.Presentation.DisplayModes;

BOOST_AUTO_TEST_CASE(enumeration_retains_window_pixels_and_video_lifetime)
{
    BOOST_REQUIRE(SDL_InitSubSystem(SDL_INIT_VIDEO));
    struct VideoReference final { ~VideoReference() { SDL_QuitSubSystem(SDL_INIT_VIDEO); } } video_reference;
    std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Display mode test",317,193,SDL_WINDOW_HIDDEN),&SDL_DestroyWindow);
    BOOST_REQUIRE(window);
    int width = 0, height = 0;
    BOOST_REQUIRE(SDL_GetWindowSizeInPixels(window.get(),&width,&height));
    const auto flags = SDL_GetWindowFlags(window.get());
    const auto modes = Graphics::Enumerate_Display_Resolutions(window.get());
    BOOST_REQUIRE(!modes.empty());
    BOOST_CHECK(std::find(modes.begin(),modes.end(),Graphics::DisplayResolution{width,height}) != modes.end());
    BOOST_CHECK(SDL_WasInit(SDL_INIT_VIDEO) != 0);
    BOOST_CHECK_EQUAL(SDL_GetWindowFlags(window.get()),flags);
    int after_width = 0, after_height = 0;
    BOOST_REQUIRE(SDL_GetWindowSizeInPixels(window.get(),&after_width,&after_height));
    BOOST_CHECK_EQUAL(after_width,width);
    BOOST_CHECK_EQUAL(after_height,height);
    for (std::size_t index = 0; index < modes.size(); ++index) {
        BOOST_CHECK_GT(modes[index].width,0);
        BOOST_CHECK_GT(modes[index].height,0);
        if (index) BOOST_CHECK(modes[index-1].width < modes[index].width ||
            (modes[index-1].width == modes[index].width && modes[index-1].height < modes[index].height));
    }
    BOOST_CHECK(Graphics::Enumerate_Display_Resolutions(window.get()) == modes);
}

BOOST_AUTO_TEST_CASE(enumeration_without_an_application_balances_video_initialization)
{
    BOOST_REQUIRE_EQUAL(SDL_WasInit(SDL_INIT_VIDEO),0u);
    const auto modes = Graphics::Enumerate_Display_Resolutions();
    BOOST_CHECK(!modes.empty());
    BOOST_CHECK_EQUAL(SDL_WasInit(SDL_INIT_VIDEO),0u);
}
