add_executable(generals WIN32)

target_link_directories(generals PRIVATE
    ../GameEngine
)

target_link_libraries(generals PRIVATE
    gameengineg
    gameenginedeviceg
    winmm
    imm32
    dbghelplib
    vfw32
    d3d8
    d3dx8
    dinput8
    dxguid
    binkstub
    milesstub
    profile
    safedisc
)

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"Someone\"
#define VERSION_BUILDLOC \"Someplace\"
"
)

# Based on original binary values for these variables.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_BUILDNUM 601
"
)

target_link_options(generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(generals PRIVATE
    ../Libraries/Include
    ../Libraries/Source
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(generals PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(generals PRIVATE
        RTS.RC
    )
endif()
