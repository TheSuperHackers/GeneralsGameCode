add_executable(g_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME generalsv)
else()
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME generalsv)
endif()

target_link_libraries(g_generals PRIVATE
    binkstub
    comctl32
    dbghelplib
    dinput8
    dxguid
    # Conditionally link DirectX 9 libraries
    if(RTS_BUILD_DIRECTX9)
        d3d9
        d3dx9
    else()
        d3d8
        d3dx8
    endif()
    g_gameengine
    g_gameenginedevice
    gi_always
    imm32
    milesstub
    vfw32
    winmm
)

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"\"
#define VERSION_BUILDLOC \"\"
"
)

# Based on original binary values for these variables.
if (IS_VS6_BUILD)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 7
#define VERSION_BUILDNUM 601
"
)
else()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
#define VERSION_BUILDNUM 601
"
)
endif()

target_link_options(g_generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/../Libraries/Source/WWVegas/WW3D2
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/GameEngineDevice/Include/MilesAudioDevice
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Libraries/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/GameEngine/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/GameEngine/Include/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/GameEngineDevice/Include/Win32Device/Common
)

if(RTS_BUILD_DIRECTX9)
    target_include_directories(g_generals PRIVATE ${DIRECTX_INCLUDE_DIR})
endif()

target_sources(g_generals PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(g_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(g_generals PRIVATE
        RTS.RC
    )
endif()
