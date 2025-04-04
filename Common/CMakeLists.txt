# c stands for Common, i stands for Interface
add_library(ci_libraries_include INTERFACE)
add_library(ci_libraries_source INTERFACE)

target_include_directories(ci_libraries_include INTERFACE "Libraries/Include")
target_include_directories(ci_libraries_source INTERFACE "Libraries/Source")

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_COMMON_TOOLS "Build common tools" ON)
add_feature_info(CommonTools GENZH_BUILD_COMMON_TOOLS "Build Common Mod Tools")
option(GENZH_BUILD_COMMON_EXTRAS "Build common extra tools/tests" OFF)
add_feature_info(CommonExtras GENZH_BUILD_COMMON_EXTRAS "Build Common Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Contains internal libraries
add_subdirectory(Libraries)

if (GENZH_BUILD_COMMON_TOOLS OR GENZH_BUILD_COMMON_EXTRAS)
    add_subdirectory(Tools)
endif()