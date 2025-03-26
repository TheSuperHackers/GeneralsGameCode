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
project(genzh LANGUAGES C CXX)

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

include(FetchContent)

# Find/Add build dependencies and stubs shared by all projects
if((WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows") AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/miles.cmake)
    include(cmake/bink.cmake)
    if (IS_VS6_BUILD)
        include(cmake/stlport.cmake)
    else()
        add_library(stlport INTERFACE) # Do not use stlport
    endif()
    include(cmake/dx8.cmake)
    include(cmake/dbghelp.cmake)
endif()

include(cmake/gamespy.cmake)
include(cmake/lzhl.cmake)
include(cmake/zlib.cmake)
add_subdirectory(Dependencies/Benchmark)
add_subdirectory(Dependencies/MaxSDK)
add_subdirectory(Dependencies/SafeDisc)
add_subdirectory(Dependencies/Utility)

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_ZEROHOUR "Build Zero Hour code." ON)
option(GENZH_BUILD_GENERALS "Build Generals code." ON)
option(GENZH_BUILD_INTERNAL "Build code with the \"Internal\" configuration." OFF)
option(GENZH_BUILD_PROFILE "Build code with the \"Profile\" configuration." OFF)
option(GENZH_BUILD_DEBUG "Build code with the \"Debug\" configuration." OFF)

if(NOT GENZH_BUILD_ZEROHOUR AND NOT GENZH_BUILD_GENERALS)
    set(GENZH_BUILD_ZEROHOUR TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(ZeroHourStuff GENZH_BUILD_ZEROHOUR "Build Zero Hour code")
add_feature_info(GeneralsStuff GENZH_BUILD_GENERALS "Build Generals code")
add_feature_info(InternalBuild GENZH_BUILD_INTERNAL "Building as an \"Internal\" build")
add_feature_info(ProfileBuild GENZH_BUILD_PROFILE "Building as a \"Profile\" build")
add_feature_info(DebugBuild GENZH_BUILD_DEBUG "Building as a \"Debug\" build")

add_library(gz_config INTERFACE)

if(NOT IS_VS6_BUILD)
    # Because we set CMAKE_CXX_STANDARD_REQUIRED and CMAKE_CXX_EXTENSIONS in the compilers.cmake this should be enforced.
    target_compile_features(gz_config INTERFACE cxx_std_20)
endif()

target_compile_options(gz_config INTERFACE ${GENZH_FLAGS})

# This disables a lot of warnings steering developers to use windows only functions/function names.
if(MSVC)
    target_compile_definitions(gz_config INTERFACE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS $<$<CONFIG:DEBUG>:_DEBUG_CRT>)
endif()

if(GENZH_BUILD_DEBUG)
    target_compile_definitions(gz_config INTERFACE _DEBUG WWDEBUG DEBUG)
else()
    target_compile_definitions(gz_config INTERFACE _RELEASE)

    if(GENZH_BUILD_INTERNAL)
        target_compile_definitions(gz_config INTERFACE _INTERNAL)
    endif()
    if(GENZH_BUILD_PROFILE)
        target_compile_definitions(gz_config INTERFACE _PROFILE)
    endif()
endif()

# Add main build targets
if(GENZH_BUILD_ZEROHOUR)
    add_subdirectory(GeneralsMD)
endif()

if(GENZH_BUILD_GENERALS)
    add_subdirectory(Generals)
endif()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
