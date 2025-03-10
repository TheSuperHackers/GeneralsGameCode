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

include(FetchContent)

# Find/Add build dependencies and stubs shared by all projects
if((WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows") AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/miles.cmake)
    include(cmake/bink.cmake)
    include(cmake/stlport.cmake)
    include(cmake/dx8.cmake)
    include(cmake/dbghelp.cmake)
endif()

include(cmake/gamespy.cmake)
include(cmake/lzhl.cmake)
include(cmake/zlib.cmake)
add_subdirectory(Dependencies/Benchmark)
add_subdirectory(Dependencies/SafeDisc)

# EA Compression library, shared between games (diff is comments only)
add_subdirectory(GeneralsMD/Code/Libraries/Source/Compression)

# BrowserDispatch COM object interface (No difference)
add_subdirectory(GeneralsMD/Code/Libraries/Source/EABrowserDispatch)

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_ZEROHOUR_PROJECT "Build Zero Hour code." ON)
option(GENZH_GENERALS_PROJECT "Build Generals code." ON) # TODO

if(NOT GENZH_ZEROHOUR_PROJECT AND NOT GENZH_GENERALS_PROJECT)
    set(GENZH_ZEROHOUR_PROJECT TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(ZeroHourStuff GENZH_ZEROHOUR_PROJECT "Build Zero Hour code")
add_feature_info(GeneralsStuff GENZH_GENERALS_PROJECT "Build Generals code")

# Add main build targets
if(GENZH_ZEROHOUR_PROJECT)
    add_subdirectory(GeneralsMD)
endif()

if(GENZH_GENERALS_PROJECT)
    add_subdirectory(Generals)
endif()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
