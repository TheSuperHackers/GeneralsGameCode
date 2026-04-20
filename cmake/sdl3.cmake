include(FetchContent)

if(NOT SAGE_USE_SDL3 OR IS_VS6_BUILD)
    return()
endif()

# GeneralsX @build felipebraz 17/04/2026 SDL3 Dependency
# Download and build SDL3 from source as a static library.
# This avoids manual installation and keeps the repository clean.

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

# Minimal build for Generals/Zero Hour engine integration.

# Disable Subsystems
set(SDL_RENDER OFF CACHE BOOL "" FORCE) # Disables all hardware renderers (D3D11, D3D12, Vulkan, GL)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_POWER OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)

# Disable External Platform Support
set(SDL_X11 OFF CACHE BOOL "" FORCE)
set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_METAL OFF CACHE BOOL "" FORCE)

# Disable Misc Features
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
set(SDL_LOCALE OFF CACHE BOOL "" FORCE)
set(SDL_MISC OFF CACHE BOOL "" FORCE)
set(SDL_OFFSCREEN OFF CACHE BOOL "" FORCE)
set(SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)

# SDL3 - Core library (v3.4.2)
FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.2/SDL3-3.4.2.tar.gz
    URL_HASH SHA256=ef39a2e3f9a8a78296c40da701967dd1b0d0d6e267e483863ce70f8a03b4050c
)

# SDL3_image - Image loading support (v3.4.0)
# --- SDL3_IMAGE CATEGORIES ---

# Disable Metadata/Packaging
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BACKEND_WIC OFF CACHE BOOL "" FORCE) # Avoid LNK2005

# Disable Codecs (minimal set)
set(SDLIMAGE_BACKEND_STB OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JPG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_PNG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_APNG OFF CACHE BOOL "" FORCE) # Fixes 'APNG_ENABLED not defined' warning
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)    # Ensure static for SDL3_image

FetchContent_Declare(
    SDL3_image
    URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.0/SDL3_image-3.4.0.tar.gz
    URL_HASH SHA256=2ceb75eab4235c2c7e93dafc3ef3268ad368ca5de40892bf8cffdd510f29d9d8
)

FetchContent_MakeAvailable(SDL3)

# Trick SDL3_image into thinking SDL3 is already found to avoid the broken find_package() in the build tree.
# SDL3_image specifically checks for SDL3::Headers and SDL3::SDL3 (for static builds).
if(NOT TARGET SDL3::Headers)
    if(TARGET SDL3_Headers)
        add_library(SDL3::Headers ALIAS SDL3_Headers)
    else()
        add_library(SDL3::Headers INTERFACE IMPORTED GLOBAL)
        target_include_directories(SDL3::Headers INTERFACE "${sdl3_SOURCE_DIR}/include")
    endif()
endif()

if(NOT TARGET SDL3::SDL3)
    if(TARGET SDL3-static)
        add_library(SDL3::SDL3 ALIAS SDL3-static)
    elseif(TARGET SDL3-shared)
        add_library(SDL3::SDL3 ALIAS SDL3-shared)
    endif()
endif()

set(SDL3_FOUND TRUE CACHE BOOL "" FORCE)
set(SDL3_VERSION "3.4.2" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(SDL3_image)
