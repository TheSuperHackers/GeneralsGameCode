# c stands for core, i stands for Interface
add_library(corei_libraries_include INTERFACE)

target_include_directories(corei_libraries_include INTERFACE "Libraries/Include")

# Do we want to build extra SDK stuff or just the game binary?
option(RTS_BUILD_CORE_TOOLS "Build core tools" ON)
add_feature_info(CoreTools GENZH_BUILD_ZEROHOUR_TOOLS "Build Core Mod Tools")
option(RTS_BUILD_CORE_EXTRAS "Build core extra tools/tests" OFF)
add_feature_info(CoreExtras GENZH_BUILD_ZEROHOUR_EXTRAS "Build Core Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
# add_subdirectory(GameEngine)

# Platform specific GameEngine code
# add_subdirectory(GameEngineDevice)

if (RTS_BUILD_CORE_TOOLS OR RTS_BUILD_CORE_EXTRAS)
    add_subdirectory(Tools)
endif()