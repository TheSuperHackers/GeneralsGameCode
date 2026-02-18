cmake_minimum_required(VERSION 3.25)

# Use packagename_ROOT for FindPackage.
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# Disable default MSVC setting CRT type so we can set it ourselves.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Disable default MSVC warning level so we can set it ourselves.
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

# Allow specifying MSVC debug configurations.
#if(POLICY CMP0141)
#    cmake_policy(SET CMP0141 NEW)
#endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# Top level project, doesn't really affect anything.
if(APPLE)
    project(genzh LANGUAGES C CXX OBJC OBJCXX)
else()
    project(genzh LANGUAGES C CXX)
endif()

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

# Debug symbol stripping for Release builds (MinGW)
include(cmake/debug_strip.cmake)

include(FetchContent)

# MinGW-w64 specific configuration
if(MINGW)
    include(cmake/mingw.cmake)
    include(cmake/reactos-atl.cmake)
    include(cmake/widl.cmake)
endif()

# Find/Add build dependencies and stubs shared by all projects
if((WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows") AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/miles.cmake)
    include(cmake/bink.cmake)
    include(cmake/dx8.cmake)
endif()

# Define a dummy stlport target when not on VC6.
if (IS_VS6_BUILD)
    include(cmake/stlport.cmake)
else()
    add_library(stlport INTERFACE)
endif()

include(cmake/config.cmake)
if(APPLE)
    # ── GameSpy SDK ──
    # Doesn't compile on macOS — stubs are in Platform/MacOS/Source/Stubs/
    FetchContent_Declare(
        gamespy
        GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
        GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
    )
    FetchContent_Populate(gamespy)
    # Stub targets providing include paths only
    add_library(gamespy INTERFACE)
    add_library(gamespy::gamespy ALIAS gamespy)
    target_include_directories(gamespy INTERFACE
        ${gamespy_SOURCE_DIR}/include
        ${gamespy_SOURCE_DIR}/include/gamespy
    )

    # ── DirectX 8 SDK ──
    # On macOS: use our d3d8_stub.h (clean C++ interfaces, no COM) — MetalDevice8 implements these
    # Do NOT use the real DX8 SDK headers — they require GUID, IStream, and real COM vtable layout
    add_library(d3d8lib INTERFACE)
    target_include_directories(d3d8lib INTERFACE
        ${CMAKE_SOURCE_DIR}/Platform/MacOS/Include  # d3d8_stub.h + shim d3d8.h/d3d8types.h/d3d8caps.h
    )
    target_compile_definitions(d3d8lib INTERFACE BUILD_WITH_D3D8)
    # Individual DX8 targets referenced elsewhere
    add_library(d3d8 INTERFACE)
    add_library(d3dx8 INTERFACE)
    target_link_libraries(d3dx8 INTERFACE d3d8lib)  # propagate include paths
    add_library(dinput8 INTERFACE)
    add_library(dxguid INTERFACE)

    # ── Miles/Bink/other Windows libs ──
    add_library(binkstub INTERFACE)
    target_include_directories(binkstub INTERFACE
        ${CMAKE_SOURCE_DIR}/Dependencies/bink  # <bink.h>
    )
    add_library(milesstub INTERFACE)
    target_include_directories(milesstub INTERFACE
        ${CMAKE_SOURCE_DIR}/Dependencies/miles      # <mss/mss.h>
        ${CMAKE_SOURCE_DIR}/Dependencies/miles/mss  # <mss.h>
    )
    add_library(comctl32 INTERFACE)
    add_library(vfw32 INTERFACE)
    add_library(winmm INTERFACE)
    add_library(imm32 INTERFACE)
else()
    include(cmake/gamespy.cmake)
endif()
include(cmake/lzhl.cmake)

if (IS_VS6_BUILD)
    # The original max sdk does not compile against a modern compiler.
    # If there is a desire to make this work, then a fixed max sdk needs to be created.
    add_subdirectory(Dependencies/MaxSDK)
endif()
add_subdirectory(Dependencies/Utility)

# To be removed when abandoning VC6
if (IS_VS6_BUILD)
    include_directories(Dependencies/Utility)
    add_compile_options(/FIUtility/CppMacros.h)
endif()

add_subdirectory(resources)

# macOS platform layer (shim headers + Metal/Cocoa implementations)
if(APPLE)
    add_subdirectory(Platform/MacOS)
endif()

add_subdirectory(Core)

# Add main build targets
if(RTS_BUILD_ZEROHOUR)
    add_subdirectory(GeneralsMD)
endif()

if(RTS_BUILD_GENERALS)
    add_subdirectory(Generals)
endif()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
