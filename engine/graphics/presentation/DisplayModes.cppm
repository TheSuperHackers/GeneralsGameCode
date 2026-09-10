module;
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

export module Graphics.Presentation.DisplayModes;

namespace Graphics
{
export struct DisplayResolution final
{
    int width = 0;
    int height = 0;
    bool operator==(const DisplayResolution&) const = default;
};

// Call on the platform thread. Enumeration borrows the application's SDL
// window, preserves video-subsystem ownership, and never changes display mode.
export std::vector<DisplayResolution> Enumerate_Display_Resolutions(void* window = nullptr)
{
    std::vector<DisplayResolution> resolutions;
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return resolutions;
    struct VideoReference final
    {
        ~VideoReference() { SDL_QuitSubSystem(SDL_INIT_VIDEO); }
    } video_reference;
    auto* sdl_window = static_cast<SDL_Window*>(window);
    const auto display = sdl_window ? SDL_GetDisplayForWindow(sdl_window) : SDL_GetPrimaryDisplay();
    const auto append = [&](int width, int height) {
        if (width > 0 && height > 0) resolutions.push_back({width,height});
    };
    int count = 0;
    std::unique_ptr<SDL_DisplayMode*, decltype(&SDL_free)> modes(
        SDL_GetFullscreenDisplayModes(display,&count), &SDL_free);
    if (modes) {
        for (int index = 0; index < count; ++index)
            append(modes.get()[index]->w,modes.get()[index]->h);
    }
    if (sdl_window) {
        int width = 0, height = 0;
        if (SDL_GetWindowSizeInPixels(sdl_window,&width,&height)) append(width,height);
    }
    std::sort(resolutions.begin(),resolutions.end(),[](const auto& left,const auto& right) {
        return left.width < right.width || (left.width == right.width && left.height < right.height);
    });
    resolutions.erase(std::unique(resolutions.begin(),resolutions.end()),resolutions.end());
    return resolutions;
}
}
