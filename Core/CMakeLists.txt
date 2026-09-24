# i stands for Interface
add_library(corei_gameengine_include INTERFACE)
add_library(corei_libraries_include INTERFACE)
add_library(corei_libraries_source_wwvegas INTERFACE)
add_library(corei_main INTERFACE)
add_library(corei_always INTERFACE)
add_library(corei_always_no_pch INTERFACE) # Use this for Shared Libs with MFC AFX

target_include_directories(corei_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(corei_libraries_include INTERFACE "Libraries/Include")
target_include_directories(corei_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(corei_main INTERFACE "Main")

target_sources(corei_libraries_include PRIVATE
    Libraries/Include/Lib/BaseFunctions.h
    Libraries/Include/Lib/BaseType.h
    Libraries/Include/Lib/BaseTypeCore.h
    Libraries/Include/Lib/Coord2D.h
    Libraries/Include/Lib/Coord3D.h
    Libraries/Include/Lib/ICoord2D.h
    Libraries/Include/Lib/ICoord3D.h
    Libraries/Include/Lib/IRegion2D.h
    Libraries/Include/Lib/IRegion3D.h
    Libraries/Include/Lib/PathUtil.h
    Libraries/Include/Lib/RealRange.h
    Libraries/Include/Lib/Region2D.h
    Libraries/Include/Lib/Region3D.h
    Libraries/Include/Lib/RGBColor.h
    Libraries/Include/Lib/trig.h
    Libraries/Include/rts/debug.h
    Libraries/Include/rts/profile.h
)
target_link_libraries(corei_always INTERFACE
    core_config
    core_profile_tracy
    core_utility
    corei_libraries_include
    resources
)
target_link_libraries(corei_always_no_pch INTERFACE
    core_config
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
add_subdirectory(GameEngineDevice)

add_subdirectory(Tools)
