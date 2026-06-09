# Standardized vcpkg integration: Try find_package first, fallback to source build if not found.
find_package(SDL3 CONFIG QUIET)
find_package(SDL3_image CONFIG QUIET)

if(NOT SDL3_FOUND OR NOT SDL3_image_FOUND)
    message(STATUS "SDL3 not found via vcpkg/find_package, falling back to source build (FetchContent)...")
    include(FetchContent)

    FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.10/SDL3-3.4.10.tar.gz
        URL_HASH SHA256=12b34280415ec8418c864408b93d008a20a6530687ee613d60bfbd20411f2785
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL3_image
        # Pin to commit with ANI loader RIFF word-alignment chunk size parsing fix
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG 0e2eaa923ddea285dfa35c4bf0c0092d3799e2ee
    )

    # Official SDL configuration for a unified build tree
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_SHARED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_STATIC ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_ZLIB OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_PNG OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_APNG OFF CACHE BOOL "" FORCE)

    # Populate SDL3 and SDL3_image
    FetchContent_MakeAvailable(SDL3)
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
