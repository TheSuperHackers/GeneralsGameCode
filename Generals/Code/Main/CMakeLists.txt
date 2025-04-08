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
    d3d8
    d3dx8
    dbghelplib
    dinput8
    dxguid
    g_gameengine
    g_gameenginedevice
    gi_libraries_include
    imm32
    milesstub
    safedisc
    vfw32
    winmm
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
#define VERSION_MINOR 8
#define VERSION_BUILDNUM 601
"
)

target_link_options(g_generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(g_generals PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_generals PRIVATE
        RTS.RC
    )
endif()
