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

# Define VCPKG feature before the project block
# since they need to be initialized before the toolchain is
include(cmake/config-vcpkg.cmake)

# Top level project, doesn't really affect anything.
project(genzh LANGUAGES C CXX)

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
    include(cmake/dx8.cmake)
endif()

# Define a dummy stlport target when not on VC6.
if (IS_VS6_BUILD)
    include(cmake/stlport.cmake)
else()
    add_library(stlport INTERFACE)
endif()

include(cmake/config.cmake)
include(cmake/gamespy.cmake)
include(cmake/lzhl.cmake)
include(cmake/stb.cmake)

if (IS_VS6_BUILD)
    # The original max sdk does not compile against a modern compiler.
    # If there is a desire to make this work, then a fixed max sdk needs to be created.
    add_subdirectory(Dependencies/MaxSDK)
endif()

# This target is not meant to be linked anywhere.
# It just makes the files visible in a Visual Studio solution.
add_subdirectory(Dependencies/Precompiled)

# Makes the C++98 compatibility macros available in every C++ translation unit.
# Force included rather than precompiled, because a precompiled header entry
# inherited from an interface library always sorts after the consuming target's
# own entries, and these macros must be defined before any other header is read.
# Targets that do have a precompiled header must still list precompiled.h as their
# first entry, because the Visual Studio generator prepends the precompiled header
# to the forced include list, so this global one would otherwise be read too late.
include_directories(Dependencies/Precompiled)
if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/FIPrecompiled/precompiled.h>)
else()
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include Precompiled/precompiled.h>")
endif()

add_subdirectory(Dependencies/Utility)
add_subdirectory(Dependencies/Bink)
add_subdirectory(Dependencies/Miles)
if (WIN32)
    add_subdirectory(Dependencies/DbgHelp)
else()
    add_library(dbghelploader INTERFACE)
endif()

add_subdirectory(resources)

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
