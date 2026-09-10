module;
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Cursors.Cursor;
export import Assets.Images.Preparation;

namespace Graphics
{
export struct CursorFrame final
{
    Assets::ImageView image;
    std::uint32_t duration_ms = 0;
};

// Native cursor resources are independent of the graphics device. All lifecycle
// and presentation calls run on SDL's main thread; SDL owns animation timing.
export class Cursor final
{
public:
    Cursor() = default;
    Cursor(const Cursor&) = delete;
    Cursor& operator=(const Cursor&) = delete;
    ~Cursor() { Reset(); }

    bool Initialize(std::span<const CursorFrame> frames, int hotspot_x, int hotspot_y)
    {
        if (frames.empty() || frames.size() > std::numeric_limits<int>::max()) return false;
        const auto width = frames.front().image.width, height = frames.front().image.height;
        if (!width || !height || width > std::numeric_limits<int>::max() / 4
            || height > std::numeric_limits<int>::max()) return false;
        using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
        std::vector<Surface> surfaces;
        std::vector<SDL_CursorFrameInfo> native_frames;
        surfaces.reserve(frames.size());
        native_frames.reserve(frames.size());
        for (const auto& frame : frames) {
            if (frame.image.width != width || frame.image.height != height) return false;
            std::vector<Assets::PreparedImage> pixels;
            if (!Assets::Prepare_Image_Levels(frame.image, Assets::PixelEncoding::RGBA8,
                width, height, 1, {}, pixels)) return false;
            Surface surface(SDL_CreateSurface(static_cast<int>(width), static_cast<int>(height),
                SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
            if (!surface || !SDL_ConvertPixels(static_cast<int>(width), static_cast<int>(height),
                SDL_PIXELFORMAT_RGBA32, pixels.front().bytes.data(), static_cast<int>(pixels.front().row_pitch),
                SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch)) return false;
            native_frames.push_back({surface.get(), frame.duration_ms});
            surfaces.push_back(std::move(surface));
        }
        SDL_Cursor* replacement = SDL_CreateAnimatedCursor(native_frames.data(),
            static_cast<int>(native_frames.size()),
            std::clamp(hotspot_x, 0, static_cast<int>(width) - 1),
            std::clamp(hotspot_y, 0, static_cast<int>(height) - 1));
        if (!replacement) return false;
        Reset();
        m_cursor = replacement;
        return true;
    }

    bool Show() const noexcept
    {
        return m_cursor && (SDL_GetCursor() == m_cursor || SDL_SetCursor(m_cursor)) && SDL_ShowCursor();
    }

    void Reset() noexcept
    {
        if (!m_cursor) return;
        if (SDL_GetCursor() == m_cursor) SDL_SetCursor(SDL_GetDefaultCursor());
        SDL_DestroyCursor(std::exchange(m_cursor, nullptr));
    }

private:
    SDL_Cursor* m_cursor = nullptr;
};
}
