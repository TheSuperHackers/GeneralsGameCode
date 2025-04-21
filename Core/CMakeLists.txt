# c stands for core, i stands for Interface
add_library(corei_libraries_include INTERFACE)
add_library(corei_libraries_source_wwvegas INTERFACE)
add_library(corei_libraries_source_wwvegas_wwdebug INTERFACE)
add_library(corei_libraries_source_wwvegas_wwlib INTERFACE)
add_library(corei_always INTERFACE)

target_include_directories(corei_libraries_include INTERFACE "Libraries/Include")
target_include_directories(corei_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(corei_libraries_source_wwvegas_wwdebug INTERFACE "Libraries/Source/WWVegas/WWDebug")
target_include_directories(corei_libraries_source_wwvegas_wwlib INTERFACE "Libraries/Source/WWVegas/WWLib")
target_link_libraries(corei_always INTERFACE
    core_utility
    corei_libraries_include
)

# Do we want to build extra SDK stuff or just the game binary?
option(RTS_BUILD_CORE_TOOLS "Build core tools" ON)
add_feature_info(CoreTools RTS_BUILD_CORE_TOOLS "Build Core Mod Tools")
option(RTS_BUILD_CORE_EXTRAS "Build core extra tools/tests" OFF)
add_feature_info(CoreExtras RTS_BUILD_CORE_EXTRAS "Build Core Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
# add_subdirectory(GameEngine)

# Platform specific GameEngine code
# add_subdirectory(GameEngineDevice)

add_subdirectory(Tools)
