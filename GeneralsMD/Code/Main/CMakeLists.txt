# Product version definitions
set(BUILD_USER "")
set(BUILD_LOCATION "")
set(PRODUCT_VERSION_MAJOR 1)
set(PRODUCT_VERSION_MINOR 4)
set(PRODUCT_VERSION_PATCH 0)
set(PRODUCT_VERSION_TWEAK 0)
set(PRODUCT_VERSION_STRING "${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH}.${PRODUCT_VERSION_TWEAK}")
set(PRODUCT_NAME "Command & Conquer Generals Zero Hour")
set(FILE_VERSION_MAJOR ${PRODUCT_VERSION_MAJOR}) # TODO come up with a file version?
set(FILE_VERSION_MINOR ${PRODUCT_VERSION_MINOR})
set(FILE_VERSION_PATCH ${PRODUCT_VERSION_PATCH})
set(FILE_VERSION_TWEAK ${PRODUCT_VERSION_TWEAK})
set(FILE_VERSION_STRING "${FILE_VERSION_MAJOR}.${FILE_VERSION_MINOR}.${FILE_VERSION_PATCH}.${FILE_VERSION_TWEAK}")
set(FILE_DESCRIPTION "Community Version")
set(FILE_NAME "GeneralsZH")

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



add_executable(z_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME ${FILE_NAME})
else()
    string(TOLOWER ${FILE_NAME} FILE_NAME_LOWER)
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME ${FILE_NAME_LOWER})
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
