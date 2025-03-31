# g stands for Generals, i stands for Interface
add_library(gi_gameengine INTERFACE)
add_library(gi_gameengine_include INTERFACE)
add_library(gi_gameenginedevice_include INTERFACE)
add_library(gi_libraries_include INTERFACE)
add_library(gi_libraries_source INTERFACE)
add_library(gi_main INTERFACE)

target_include_directories(gi_gameengine INTERFACE "GameEngine")
target_include_directories(gi_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(gi_gameenginedevice_include INTERFACE "GameEngineDevice/Source")
target_include_directories(gi_libraries_include INTERFACE "Libraries/Include")
target_include_directories(gi_libraries_source INTERFACE "Libraries/Source")
target_include_directories(gi_main INTERFACE "Main")

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

# Contains the main game exe target.
add_subdirectory(Main)

if (GENZH_BUILD_GENERALS_TOOLS OR GENZH_BUILD_GENERALS_EXTRAS)
    add_subdirectory(Tools)
endif()
