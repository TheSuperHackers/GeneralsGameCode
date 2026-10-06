# Standardized vcpkg integration: Try find_package first, fallback to source build if not found.
# Require SDL3_image >= 3.4.6 for the ANI loader RIFF word-alignment fix and legacy parsing relaxation.
# NOTE: vcpkg's sdl3-image port is still stuck at 3.4.4 (no upstream port update exists yet), which
# lacks this fix, so find_package below will always fail its version check and fall through to the
# FetchContent build of the real 3.4.6 release. Until vcpkg publishes a 3.4.6+ port, animated cursor
# (.ani) loading depends entirely on this FetchContent path, not the vcpkg-installed sdl3-image.
find_package(SDL3 CONFIG QUIET)
find_package(SDL3_image 3.4.6 CONFIG QUIET)

include(FetchContent)

if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found via vcpkg/find_package, falling back to source build (FetchContent)...")

    FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
        URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
        OVERRIDE_FIND_PACKAGE
    )

    # Official SDL configuration for a unified build tree
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3)
endif()

if(NOT SDL3_image_FOUND)
    message(STATUS "SDL3_image >= 3.4.6 not found via vcpkg/find_package, falling back to source build (FetchContent)...")

    FetchContent_Declare(
        SDL3_image
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.6/SDL3_image-3.4.6.tar.gz
        URL_HASH SHA256=d2e4637ae700f72e5196b8fbd749850ed2e5e1e09c5a5be8d06ff55aaccf3b01
    )

    set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_SHARED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_STATIC ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_ZLIB OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_PNG OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_APNG OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3_image)
endif()

# Uniform aliases to ensure linking works across both discovery methods
if(TARGET SDL3::SDL3-shared AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 ALIAS SDL3::SDL3-shared)
endif()
if(TARGET SDL3::SDL3-static AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 ALIAS SDL3::SDL3-static)
endif()

# Centralized dependency restoration for SDL3 static builds.
# We apply these directly to the SDL3-static target so it correctly handles its own needs.
if(TARGET SDL3-static)
    target_link_libraries(SDL3-static INTERFACE 
        ws2_32.lib 
        winmm.lib
        imm32.lib
        version.lib
        setupapi.lib
    )
endif()
