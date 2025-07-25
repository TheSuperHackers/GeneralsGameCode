# Product version definitions
set(BUILD_USER "")
set(BUILD_LOCATION "")
set(PRODUCT_VERSION_MAJOR 1)
if (IS_VS6_BUILD)
    set(PRODUCT_VERSION_MINOR 7)
else()
    set(PRODUCT_VERSION_MINOR 8)
endif()
set(PRODUCT_VERSION_PATCH 0)
set(PRODUCT_VERSION_TWEAK 0)
set(PRODUCT_VERSION_STRING "${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH}.${PRODUCT_VERSION_TWEAK}")
set(PRODUCT_NAME "Command & Conquer Generals")
set(FILE_VERSION_MAJOR ${PRODUCT_VERSION_MAJOR}) # TODO come up with a file version?
set(FILE_VERSION_MINOR ${PRODUCT_VERSION_MINOR})
set(FILE_VERSION_PATCH ${PRODUCT_VERSION_PATCH})
set(FILE_VERSION_TWEAK ${PRODUCT_VERSION_TWEAK})
set(FILE_VERSION_STRING "${FILE_VERSION_MAJOR}.${FILE_VERSION_MINOR}.${FILE_VERSION_PATCH}.${FILE_VERSION_TWEAK}")
set(FILE_DESCRIPTION "Community Version")
set(FILE_NAME "GeneralsV")

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"${BUILD_USER}\"
#define VERSION_BUILDLOC \"${BUILD_LOCATION}\"
"
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR ${PRODUCT_VERSION_MAJOR}
#define VERSION_MINOR ${PRODUCT_VERSION_MINOR}
#define VERSION_BUILDNUM ${PRODUCT_VERSION_PATCH}
"
)

# Configure Version rc file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.rc.in
               ${CMAKE_CURRENT_BINARY_DIR}/Version.rc)



add_executable(g_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME ${FILE_NAME})
else()
    string(TOLOWER ${FILE_NAME} FILE_NAME_LOWER)
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME ${FILE_NAME_LOWER})
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
