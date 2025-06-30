# i stands for Interface
add_library(corei_gameengine_include INTERFACE)
add_library(corei_libraries_include INTERFACE)
add_library(corei_libraries_source_wwvegas INTERFACE)
add_library(corei_libraries_source_wwvegas_wwlib INTERFACE)
add_library(corei_always INTERFACE)

target_include_directories(corei_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(corei_libraries_include INTERFACE "Libraries/Include")
target_include_directories(corei_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(corei_libraries_source_wwvegas_wwlib INTERFACE "Libraries/Source/WWVegas/WWLib")
target_link_libraries(corei_always INTERFACE
    core_utility
    corei_libraries_include
    resources
)

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
# add_subdirectory(GameEngineDevice)

add_subdirectory(Tools)
