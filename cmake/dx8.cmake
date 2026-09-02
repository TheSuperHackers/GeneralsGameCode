FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
)

# Populate the source only (do not add_subdirectory it): the fetched
# min-dx8-sdk repo's own CMakeLists.txt has no architecture condition at all,
# so the d3d8lib target is defined here instead, where it can diverge by
# CMAKE_SIZEOF_VOID_P without forking that upstream repo.
FetchContent_GetProperties(dx8)
if(NOT dx8_POPULATED)
    FetchContent_Populate(dx8)
endif()

add_library(d3d8lib INTERFACE)

# Common libraries for all compilers.
# 64-bit: MinGW-w64 x86_64 provides libdinput8.a and libdxguid.a but no
# libd3d8.a (only libd3d8thk.a) and no libd3dx8 at all. The headers are
# architecture-independent, so on x64 this target carries includes and defines
# only; it cannot link, which is expected — a playable x64 build is out of
# scope, see issue #473. This is a precondition for W3D to compile on x64, not
# a guarantee: W3D also depends on several Core libraries (WWLib, debug,
# Compression, WWSaveLoad, WWAudio) that fail for unrelated reasons and still
# block it as of this change.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(d3d8lib INTERFACE d3d8 dinput8 dxguid)
else()
    message(STATUS "DX8: x64 build — headers only, no D3D8 link libraries available")
endif()

# MSVC-specific configuration
if(MSVC)
    # Use bundled MSVC-compiled .lib files. d3dx8.lib is pe-i386 (32-bit)
    # only -- there is no 64-bit build of it in the fetched min-dx8-sdk repo
    # -- so linking it into a 64-bit target would fail, the same reason the
    # top-level d3d8/dinput8/dxguid link above is gated on
    # CMAKE_SIZEOF_VOID_P EQUAL 4. No x64 MSVC preset exercises this today
    # (issue #473), but the condition should match its sibling regardless.
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(d3d8lib INTERFACE d3dx8)
    endif()
    target_link_directories(d3d8lib BEFORE INTERFACE ${dx8_SOURCE_DIR})
    target_link_options(d3d8lib INTERFACE /NODEFAULTLIB:libci.lib)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "12.0.8804")
        # Modern MSVC (VS 2013+) has complete DirectX headers in Windows SDK
        target_link_libraries(d3d8lib INTERFACE legacy_stdio_definitions)
        target_link_options(d3d8lib INTERFACE /SAFESEH:NO)
    else()
        # VC6 and older MSVC need extra headers - their DirectX SDK is missing newer definitions
        target_include_directories(d3d8lib INTERFACE ${dx8_SOURCE_DIR}/extra)
    endif()
endif()

# MinGW-specific configuration
if(MINGW)
    # MinGW-w64 DirectX 8 support varies by architecture:
    # i686 (32-bit): libd3d8.a + libd3dx8d.a (debug only, no release version)
    # x86_64 (64-bit): libd3d8thk.a only (no libd3dx8 libraries at all)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(d3d8lib INTERFACE d3dx8d)
    endif()
endif()

target_compile_definitions(d3d8lib INTERFACE -DBUILD_WITH_D3D8)
target_include_directories(d3d8lib INTERFACE ${dx8_SOURCE_DIR})
