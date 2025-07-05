add_executable(z_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)
else()
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)
endif()

target_link_libraries(z_generals PRIVATE
    binkstub
    comctl32
    core_debug
    core_profile
    d3d8
    d3dx8
    dbghelplib
    dinput8
    dxguid
    imm32
    milesstub
    vfw32
    winmm
    z_gameengine
    z_gameenginedevice
    zi_always
)

if(EXISTS "${CMAKE_SOURCE_DIR}/next_tag.txt")
    file(READ "${CMAKE_SOURCE_DIR}/next_tag.txt" GAME_VERSION)
    string(STRIP "${GAME_VERSION}" GAME_VERSION)
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${GAME_VERSION}")
    set(VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(VERSION_MINOR "${CMAKE_MATCH_2}")
    set(VERSION_BUILDNUM "${CMAKE_MATCH_3}")
else()
    set(GAME_VERSION "")
    set(VERSION_MAJOR "1")
    set(VERSION_MINOR "4")
    set(VERSION_BUILDNUM "601")
endif()

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_BUILDUSER \"TheSuperHackers\"
#define VERSION_BUILDLOC \"GitHub\"
#define SUPERHACKERS_VERSION_WIDE L\"${GAME_VERSION}\"
"
)

# Based on original binary values for these variables.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR ${VERSION_MAJOR}
#define VERSION_MINOR ${VERSION_MINOR}
#define VERSION_BUILDNUM ${VERSION_BUILDNUM}
#define VERSION_LOCALBUILDNUM 9001 // it's over 9000!
"
)

target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(z_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(z_generals PRIVATE
        RTS.RC
    )
endif()
