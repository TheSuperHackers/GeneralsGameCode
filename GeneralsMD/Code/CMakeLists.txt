# z stands for Zero Hour, i stands for Interface
cmake_minimum_required(VERSION 4.4.3)

add_library(zi_gameengine_include INTERFACE)
add_library(zi_libraries_source_wwvegas INTERFACE)
add_library(zi_main INTERFACE)
add_library(zi_always INTERFACE)
add_library(zi_always_no_pch INTERFACE) # Use this for Shared Libs with MFC AFX

target_include_directories(zi_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(zi_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(zi_main INTERFACE "Main")

target_compile_definitions(zi_always INTERFACE
    RTS_ZEROHOUR=1
)
target_compile_definitions(zi_always_no_pch INTERFACE
    RTS_ZEROHOUR=1
)
target_link_libraries(zi_gameengine_include INTERFACE
    engine_navigation
    corei_gameengine_include
)
target_link_libraries(zi_always INTERFACE
    corei_always # Must stay below so headers from game are included first
)
target_link_libraries(zi_always_no_pch INTERFACE
    corei_always_no_pch # Must stay below so headers from game are included first
)

# The supported GeneralsMD toolchain uses CMake 4 and supports C++ modules.
set(ENGINE_JOBS_BUILD_TESTS ${RTS_BUILD_ZEROHOUR_EXTRAS})
add_subdirectory("${CMAKE_SOURCE_DIR}/engine/jobs" "${CMAKE_CURRENT_BINARY_DIR}/engine_jobs")
set(ENGINE_ECS_BUILD_TESTS ${RTS_BUILD_ZEROHOUR_EXTRAS})
add_subdirectory("${CMAKE_SOURCE_DIR}/engine/ecs" "${CMAKE_CURRENT_BINARY_DIR}/engine_ecs")

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

# Contains the main game exe target.
add_subdirectory(Main)

engine_navigation_add_world_tests()

if (RTS_BUILD_ZEROHOUR_TOOLS OR RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(Tools)
endif()
