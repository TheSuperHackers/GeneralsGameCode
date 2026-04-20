# Standardized vcpkg integration: Try find_package first, fallback to source build if not found.
find_package(SDL3 CONFIG QUIET)
find_package(SDL3_image CONFIG QUIET)

if(NOT SDL3_FOUND OR NOT SDL3_image_FOUND)
    message(STATUS "SDL3 not found via vcpkg/find_package, falling back to source build (FetchContent)...")
    include(FetchContent)

    FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.4/SDL3-3.4.4.tar.gz
        URL_HASH SHA256=EE712DBE6A89BB140BBFC2CE72358FB5EE5CC2240ABEABD54855012DB30B3864
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL3_image
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.2/SDL3_image-3.4.2.tar.gz
        URL_HASH SHA256=82fdb88cf1a9cbdc1c77797aaa3292e6d22ce12586be718c8ea43530df1536b4
    )

    # Official SDL configuration for a unified build tree
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_ZLIB ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_PNG ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_APNG ON CACHE BOOL "" FORCE)

    # Making them available in order ensures SDL3_image can see the SDL3 targets
    FetchContent_MakeAvailable(SDL3 SDL3_image)
endif()

# Uniform aliases to ensure linking works across both discovery methods
if(TARGET SDL3::SDL3-shared AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 ALIAS SDL3::SDL3-shared)
endif()
if(TARGET SDL3::SDL3-static AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 ALIAS SDL3::SDL3-static)
endif()
