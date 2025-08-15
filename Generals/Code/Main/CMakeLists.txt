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
    gi_always
    imm32
    milesstub
    vfw32
    winmm
)

# Set build information variables with default values that can be overridden
if(NOT DEFINED VERSION_BUILDUSER)
    set(VERSION_BUILDUSER "\"The Super Hackers\"")
endif()

if(NOT DEFINED VERSION_BUILDLOC)
    set(VERSION_BUILDLOC "\"https://github.com/TheSuperHackers/GeneralsGameCode\"")
endif()

# Normalize to ensure they are C string literals even when provided via -D on the CLI
set(_VERSION_BUILDUSER ${VERSION_BUILDUSER})
if(NOT _VERSION_BUILDUSER MATCHES "^\".*\"$")
    set(_VERSION_BUILDUSER "\"${VERSION_BUILDUSER}\"")
endif()

set(_VERSION_BUILDLOC ${VERSION_BUILDLOC})
if(NOT _VERSION_BUILDLOC MATCHES "^\".*\"$")
    set(_VERSION_BUILDLOC "\"${VERSION_BUILDLOC}\"")
endif()

# Use git revision count as build number if available, otherwise use default or override
if(NOT DEFINED VERSION_BUILDNUM)
    if(DEFINED GIT_REV_LIST_COUNT)
        set(VERSION_BUILDNUM ${GIT_REV_LIST_COUNT})
    else()
        set(VERSION_BUILDNUM 601)
    endif()
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#ifndef VERSION_BUILDNUM
#define VERSION_BUILDNUM ${VERSION_BUILDNUM}
#endif

#ifndef VERSION_LOCALBUILDNUM
#define VERSION_LOCALBUILDNUM 0
#endif

#ifndef VERSION_BUILDUSER
#define VERSION_BUILDUSER ${_VERSION_BUILDUSER}
#endif

#ifndef VERSION_BUILDLOC
#define VERSION_BUILDLOC ${_VERSION_BUILDLOC}
#endif
"
)

# Based on original binary values for these variables.
if (IS_VS6_BUILD)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 7
"
)
else()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
"
)
endif()

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
