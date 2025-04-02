# c stands for common, i stands for Interface
add_library(ci_libraries_include INTERFACE)
add_library(ci_libraries_source INTERFACE)
add_library(ci_libraries_source_wwvegas INTERFACE)
add_library(ci_libraries_source_wwvegas_ww3d2 INTERFACE)
add_library(ci_libraries_source_wwvegas_wwdebug INTERFACE)
add_library(ci_libraries_source_wwvegas_wwlib INTERFACE)
add_library(ci_libraries_source_wwvegas_wwmath INTERFACE)
add_library(ci_libraries_source_wwvegas_wwsaveload INTERFACE)

target_include_directories(ci_libraries_include INTERFACE "Libraries/Include")
target_include_directories(ci_libraries_source INTERFACE "Libraries/Source")
target_include_directories(ci_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(ci_libraries_source_wwvegas_ww3d2 INTERFACE "Libraries/Source/WWVegas/WW3D2")
target_include_directories(ci_libraries_source_wwvegas_wwdebug INTERFACE "Libraries/Source/WWVegas/WWDebug")
target_include_directories(ci_libraries_source_wwvegas_wwlib INTERFACE "Libraries/Source/WWVegas/WWLib")
target_include_directories(ci_libraries_source_wwvegas_wwmath INTERFACE "Libraries/Source/WWVegas/WWMath")
target_include_directories(ci_libraries_source_wwvegas_wwsaveload INTERFACE "Libraries/Source/WWVegas/WWSaveLoad")

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_COMMON_TOOLS "Build tools for Generals/Zero Hour" ON)
add_feature_info(CommonTools GENZH_BUILD_COMMON_TOOLS "Build Mod Tools")
option(GENZH_BUILD_COMMON_EXTRAS "Build extra tools/tests for Generals/Zero Hour" OFF)
add_feature_info(CommonExtras GENZH_BUILD_COMMON_EXTRAS "Build Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Contains internal libraries
add_subdirectory(Libraries)

if (GENZH_BUILD_COMMON_TOOLS OR GENZH_BUILD_COMMON_EXTRAS)
    add_subdirectory(Tools)
endif()
