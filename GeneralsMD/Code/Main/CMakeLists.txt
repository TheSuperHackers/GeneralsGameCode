add_executable(z_generals)
set_property(TARGET z_generals PROPERTY CXX_SCAN_FOR_MODULES ON)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME "generalszh${RTS_BUILD_OUTPUT_SUFFIX}")
else()
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME "generalszh${RTS_BUILD_OUTPUT_SUFFIX}")
endif()

target_link_libraries(z_generals PRIVATE
    comctl32
    imm32
    SDL3::SDL3
    vfw32
    winmm
    z_gameengine
    z_gameenginedevice
    zi_always
    generals_graphics
)

get_target_property(GRAPHICS_SHADER_ASSET_DIRECTORY generals_graphics GRAPHICS_SHADER_ASSET_DIRECTORY)

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

# Copy runtime game data next to the built executable.
# The game sets working directory to the executable directory on startup,
# so required data must exist directly under that directory (for example Data/...).
set(RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY "${FFMPEG_DEBUG_RUNTIME_DIRECTORY}")
set(RTS_FFMPEG_RUNTIME_DIRECTORY "${FFMPEG_RUNTIME_DIRECTORY}")
if(NOT EXISTS "${RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY}" OR NOT EXISTS "${RTS_FFMPEG_RUNTIME_DIRECTORY}")
    foreach(runtime_prefix IN LISTS CMAKE_PREFIX_PATH)
        if(NOT EXISTS "${RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY}" AND EXISTS "${runtime_prefix}/debug/bin")
            set(RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY "${runtime_prefix}/debug/bin")
        endif()
        if(NOT EXISTS "${RTS_FFMPEG_RUNTIME_DIRECTORY}" AND EXISTS "${runtime_prefix}/bin")
            set(RTS_FFMPEG_RUNTIME_DIRECTORY "${runtime_prefix}/bin")
        endif()
    endforeach()
endif()

add_custom_command(TARGET z_generals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:z_generals>/Run"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/GeneralsMD/Run"
        "$<TARGET_FILE_DIR:z_generals>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>"
        "$<TARGET_FILE_DIR:z_generals>"
)

if(EXISTS "${RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY}" OR EXISTS "${RTS_FFMPEG_RUNTIME_DIRECTORY}")
    add_custom_command(TARGET z_generals POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${RTS_FFMPEG_DEBUG_RUNTIME_DIRECTORY},${RTS_FFMPEG_RUNTIME_DIRECTORY}>"
            "$<TARGET_FILE_DIR:z_generals>")
endif()

if(GRAPHICS_SHADER_ASSET_DIRECTORY)
    add_custom_command(TARGET z_generals POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:z_generals>/GraphicsShaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${GRAPHICS_SHADER_ASSET_DIRECTORY}"
            "$<TARGET_FILE_DIR:z_generals>/GraphicsShaders"
        VERBATIM
    )
endif()



target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    SDL3Main.cpp
    SDL3Main.h
    Platform/SDLPlatformWindow.h
)

# The legacy engine interface publishes a PCH containing Windows/intrinsic
# macros. Clang must parse SDL headers before those macros are active.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(z_generals PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
endif()

if(WIN32)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # lld-link embeds the default manifest correctly, but rejects the
        # MSVC-only /MANIFESTINPUT spelling used by the original build.
        target_link_options(z_generals PRIVATE "LINKER:/MANIFEST:EMBED")
    else()
        target_link_options(z_generals PRIVATE
            "LINKER:/MANIFEST:EMBED"
            "LINKER:/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/app.manifest")
    endif()
    target_sources(z_generals PRIVATE RTS.RC)
endif()
