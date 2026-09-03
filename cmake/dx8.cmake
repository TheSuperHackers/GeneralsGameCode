FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
)

# Populate only: the fetched repo's CMakeLists.txt has no architecture condition,
# so d3d8lib is defined here, where it can diverge by CMAKE_SIZEOF_VOID_P.
FetchContent_GetProperties(dx8)
if(NOT dx8_POPULATED)
    FetchContent_Populate(dx8)
endif()

add_library(d3d8lib INTERFACE)

# MinGW-w64 x86_64 ships no libd3d8.a and no libd3dx8, so on x64 this target
# carries headers and defines only and cannot link. See issue #473.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(d3d8lib INTERFACE d3d8 dinput8 dxguid)
else()
    message(STATUS "DX8: x64 build — headers only, no D3D8 link libraries available")
endif()

# MSVC-specific configuration
if(MSVC)
    # Use bundled MSVC-compiled .lib files
    target_link_libraries(d3d8lib INTERFACE d3dx8)
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
    # i686 ships libd3d8.a and libd3dx8d.a; x86_64 ships neither.
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(d3d8lib INTERFACE d3dx8d)
    endif()
endif()

target_compile_definitions(d3d8lib INTERFACE -DBUILD_WITH_D3D8)
target_include_directories(d3d8lib INTERFACE ${dx8_SOURCE_DIR})
