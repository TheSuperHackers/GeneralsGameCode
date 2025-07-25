# g stands for Generals, i stands for Interface
add_library(gi_gameengine_include INTERFACE)
add_library(gi_libraries_include INTERFACE)
add_library(gi_libraries_source_wwvegas INTERFACE)
add_library(gi_main INTERFACE)
add_library(gi_always INTERFACE)

target_include_directories(gi_gameengine_include INTERFACE "GameEngine/Include")
target_link_libraries(gi_gameengine_include INTERFACE corei_gameengine_include)
target_include_directories(gi_libraries_include INTERFACE "Libraries/Include")
target_include_directories(gi_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(gi_main INTERFACE "Main")

target_compile_definitions(gi_always INTERFACE
    RTS_GENERALS=1
)
target_link_libraries(gi_always INTERFACE
    core_utility
    gi_libraries_include
    # Must stay below so headers from game are included first
    corei_libraries_include
)

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

# Contains the main game exe target.
add_subdirectory(Main)

if (RTS_BUILD_GENERALS_TOOLS OR RTS_BUILD_GENERALS_EXTRAS)
    add_subdirectory(Tools)
endif()
