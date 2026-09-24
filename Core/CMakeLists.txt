# Contains base types, functions, macros and pragmas for all game build targets
add_subdirectory(Libraries)

# i stands for Interface
add_library(corei_gameengine_include INTERFACE)
add_library(corei_libraries_source_wwvegas INTERFACE)
add_library(corei_main INTERFACE)
add_library(corei_always INTERFACE)
add_library(corei_always_no_pch INTERFACE) # Use this for Shared Libs with MFC AFX

target_include_directories(corei_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(corei_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(corei_main INTERFACE "Main")

target_link_libraries(corei_always INTERFACE
    core_libraries # Gives force included headers. Link early.
    core_config
    core_profile_tracy
    core_utility
    resources
)
target_link_libraries(corei_always_no_pch INTERFACE
    core_libraries # Gives force included headers. Link early.
    core_config
    core_utility
    resources
)

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

add_subdirectory(Tools)
