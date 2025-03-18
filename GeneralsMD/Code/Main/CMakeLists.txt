add_executable(z_generals WIN32)
set_target_properties(z_generals PROPERTIES OUTPUT_NAME generals)

target_link_libraries(z_generals PRIVATE
    binkstub
    d3d8
    d3dx8
    dbghelplib
    dinput8
    dxguid
    imm32
    milesstub
    safedisc
    vfw32
    winmm
    z_debug
    z_gameengine
    z_gameenginedevice
    z_profile
    zi_libraries_include
    zi_libraries_source
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

target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(z_generals PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(z_generals PRIVATE
        RTS.RC
    )
endif()
