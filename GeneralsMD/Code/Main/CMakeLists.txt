# ── Executable target ──
if(APPLE)
    add_executable(z_generals)
else()
    add_executable(z_generals WIN32)
endif()

# Use a binary name that doesn't conflict with original game.
set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)

target_link_libraries(z_generals PRIVATE
    core_debug
    core_profile
    z_gameengine
    z_gameenginedevice
    zi_always
)

if(NOT APPLE)
    target_link_libraries(z_generals PRIVATE
        binkstub
        comctl32
        d3d8
        d3dx8
        dinput8
        dxguid
        imm32
        milesstub
        vfw32
        winmm
    )
else()
    target_link_libraries(z_generals PRIVATE macos_platform)
endif()

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"\"
#define VERSION_BUILDLOC \"\"
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

if(MSVC)
    target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(APPLE)
    target_include_directories(z_generals PRIVATE
        ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include/Precompiled
    )
endif()

# ── Entry point (platform-specific) ──
if(APPLE)
    target_sources(z_generals PRIVATE
        ${CMAKE_SOURCE_DIR}/Platform/MacOS/Source/Main/MacOSMain.mm
    )
    set_source_files_properties(
        ${CMAKE_SOURCE_DIR}/Platform/MacOS/Source/Main/MacOSMain.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc -include ${CMAKE_SOURCE_DIR}/Dependencies/Utility/Utility/CppMacros.h"
        SKIP_PRECOMPILE_HEADERS TRUE
    )
else()
    target_sources(z_generals PRIVATE
        WinMain.cpp
        WinMain.h
    )
endif()

# RC files optional for MinGW builds
# Icon: LoadIcon() handles missing resource gracefully (uses default system icon)
# Manifest: DPI awareness metadata (nice to have but not essential)
if(MSVC)
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

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalszh.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(z_generals)
endif()
