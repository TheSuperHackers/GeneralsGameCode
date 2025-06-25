# z stands for Zero Hour, i stands for Interface
add_library(zi_gameengine_include INTERFACE)
add_library(zi_libraries_include INTERFACE)
add_library(zi_libraries_source_wwvegas INTERFACE)
add_library(zi_main INTERFACE)
add_library(zi_always INTERFACE)

target_include_directories(zi_gameengine_include INTERFACE "GameEngine/Include")
target_link_libraries(zi_gameengine_include INTERFACE corei_gameengine_include)
target_include_directories(zi_libraries_include INTERFACE "Libraries/Include")
target_include_directories(zi_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(zi_main INTERFACE "Main")

target_compile_definitions(zi_always INTERFACE
    RTS_ZEROHOUR=1
)
target_link_libraries(zi_always INTERFACE
    core_utility
    zi_libraries_include
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

if (RTS_BUILD_ZEROHOUR_TOOLS OR RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(Tools)
endif()
