# c stands for common, i stands for Interface
add_library(ci_libraries_include INTERFACE)
add_library(ci_libraries_source INTERFACE)

target_include_directories(ci_libraries_include INTERFACE "Libraries/Include")
target_include_directories(ci_libraries_source INTERFACE "Libraries/Source")

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_COMMON_TOOLS "Build tools for Generals/Zero Hour" ON)
option(GENZH_BUILD_COMMON_EXTRAS "Build extra tools/tests for Generals/Zero Hour" OFF)

# Contains internal libraries
add_subdirectory(Libraries)

if (GENZH_BUILD_COMMON_TOOLS OR GENZH_BUILD_COMMON_EXTRAS)
    add_subdirectory(Tools)
endif()
