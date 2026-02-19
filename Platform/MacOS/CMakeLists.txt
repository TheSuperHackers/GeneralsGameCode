# Platform/MacOS — Metal backend, Cocoa integration, platform stubs
# This library provides the macOS platform layer that replaces Win32Device components.

# ── Metal rendering backend (DX8 → Metal) ──
set(METAL_SRC
    Source/Metal/MetalDevice8.mm
    Source/Metal/MetalInterface8.mm
    Source/Metal/MetalSurface8.mm
    Source/Metal/MetalTexture8.mm
    Source/Metal/MetalVertexBuffer8.mm
    Source/Metal/MetalIndexBuffer8.mm
)

# ── Main application (game client, window, input, shaders) ──
set(MAIN_SRC
    Source/Main/MacOSGameClient.mm
    Source/Main/MacOSWindowManager.mm
    Source/Main/MacOSGameWindowManager.mm
    Source/Main/MacOSGadgetDraw.mm
    Source/Main/D3DXStubs.mm
    Source/Main/StdKeyboard.mm
    Source/Main/StdMouse.mm
)

# ── Client (display, text rendering) ──
set(CLIENT_SRC
    Source/Client/MacOSDisplay.mm
    Source/Client/MacOSDisplayString.mm
)

# ── Audio ──
set(AUDIO_SRC
    Source/Audio/MacOSAudioManager.mm
)

# ── File system (cross-platform .big + local) ──
set(COMMON_SRC
    Source/Common/StdBIGFile.cpp
    Source/Common/StdBIGFileSystem.cpp
    Source/Common/StdLocalFile.cpp
    Source/Common/StdLocalFileSystem.cpp
)

# ── Stubs (GameSpy, LZHL, WWDownload — Windows-only libs) ──
set(STUBS_SRC
    Source/Stubs/GameSpyStubs.cpp
    Source/Stubs/GitInfoStubs.cpp
    Source/Stubs/LZHLStubs.cpp
    Source/Stubs/MacOSW3DShaderManager.mm
    Source/Stubs/WWDownloadStubs.cpp
)

# ── Debug ──
set(DEBUG_SRC
    Source/Debug/MacOSScreenshot.mm
)

# ── Combine all sources ──
add_library(macos_platform STATIC
    ${METAL_SRC}
    ${MAIN_SRC}
    ${CLIENT_SRC}
    ${AUDIO_SRC}
    ${COMMON_SRC}
    ${STUBS_SRC}
    ${DEBUG_SRC}
)

# ── Include directories ──
# Platform/MacOS/Include is already in d3d8lib INTERFACE (set in root CMakeLists.txt),
# but we also need it for our own sources.
target_include_directories(macos_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Private includes for implementation files
target_include_directories(macos_platform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Metal
    # WWVegas include paths — needed because .mm files include game engine headers
    # whose chain reaches always.h, WWCommon.h, wwdebug.h etc.
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas
    # WW3D2 headers (dx8indexbuffer.h, dx8vertexbuffer.h, dx8wrapper.h)
    ${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/Source/WWVegas/WW3D2
    # Precompiled header path (PreRTS.h) — use GeneralsMD for Zero Hour
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include/Precompiled
    # W3DDevice headers (W3DShaderManager.h, W3DAssetManager.h, W3DInGameUI.h, etc.)
    # Use GeneralsMD (Zero Hour) versions to match zi_always/zi_gameengine_include
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include
    ${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/Source/WWVegas
    # Core WWVegas subdirectories (bare includes from WW3D2 chain)
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDebug
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWMath
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWSaveLoad
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WW3D2
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWAudio
    # Core GameEngine/GameEngineDevice headers (W3DShaderManager.h, etc.)
    ${CMAKE_SOURCE_DIR}/Core/GameEngineDevice/Include
    ${CMAKE_SOURCE_DIR}/Core/GameEngine/Include
)

# ── Force-include Carbon compatibility header ──
# This header blocks conflicting Carbon sub-headers (AIFF, Files, IntlResources,
# ToolUtils) and pre-imports Apple frameworks before any game headers.
# Resolves all Carbon↔Game type conflicts (WideChar, ChunkHeader, FileInfo, RGBColor).
target_compile_options(macos_platform PRIVATE
    -include ${CMAKE_CURRENT_SOURCE_DIR}/Include/macos_carbon_compat.h
)
# ── macOS frameworks ──
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)

target_link_libraries(macos_platform PUBLIC
    ${METAL_FRAMEWORK}
    ${METALKIT_FRAMEWORK}
    ${QUARTZCORE_FRAMEWORK}
    ${COCOA_FRAMEWORK}
    ${APPKIT_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${AUDIOTOOLBOX_FRAMEWORK}
    ${AVFOUNDATION_FRAMEWORK}
)

# ── Project dependencies ──
target_link_libraries(macos_platform PUBLIC
    d3d8lib
    core_config
    corei_always
    corei_gameengine_include
    zi_libraries_source_wwvegas
)
# Zero Hour includes and defines — PRIVATE to avoid leaking RTS_ZEROHOUR
# to the Generals target that also links macos_platform
target_link_libraries(macos_platform PRIVATE
    zi_always
    zi_gameengine_include
)

# ── Compile Metal shaders ──
# Ensure Metal Toolchain is available (auto-download if missing)
execute_process(
    COMMAND xcrun -sdk macosx metal --version
    RESULT_VARIABLE METAL_CHECK_RESULT
    OUTPUT_QUIET ERROR_QUIET
)
if(NOT METAL_CHECK_RESULT EQUAL 0)
    message(STATUS "Metal Toolchain not found — downloading automatically...")
    execute_process(
        COMMAND xcodebuild -downloadComponent MetalToolchain
        RESULT_VARIABLE METAL_DL_RESULT
    )
    if(NOT METAL_DL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to download Metal Toolchain. Please run manually:\n  xcodebuild -downloadComponent MetalToolchain")
    endif()
    message(STATUS "Metal Toolchain installed successfully")
endif()

set(METAL_SHADER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Main/MacOSShaders.metal)
set(METAL_AIR_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MacOSShaders.air)
set(METAL_LIB_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/default.metallib)

add_custom_command(
    OUTPUT ${METAL_AIR_OUTPUT}
    COMMAND xcrun -sdk macosx metal -c ${METAL_SHADER_SOURCE} -o ${METAL_AIR_OUTPUT}
    DEPENDS ${METAL_SHADER_SOURCE}
    COMMENT "Compiling Metal shaders"
)

add_custom_command(
    OUTPUT ${METAL_LIB_OUTPUT}
    COMMAND xcrun -sdk macosx metallib ${METAL_AIR_OUTPUT} -o ${METAL_LIB_OUTPUT}
    DEPENDS ${METAL_AIR_OUTPUT}
    COMMENT "Linking Metal shader library"
)

add_custom_target(metal_shaders ALL DEPENDS ${METAL_LIB_OUTPUT})
add_dependencies(macos_platform metal_shaders)

# ── ObjC++ settings ──
# Enable ARC for all ObjC++ files in this target.
target_compile_options(macos_platform PRIVATE
    $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>
)

# The inherited PCH (from core_utility INTERFACE) was compiled without ARC,
# which conflicts with our -fobjc-arc flag for OBJCXX files.
# By specifying our own PCH with the same content, CMake generates a new PCH
# binary for this target. Since this target has -fobjc-arc, the OBJCXX PCH
# variant will include the ARC flag, avoiding the mismatch.
target_precompile_headers(macos_platform PRIVATE
    [["Utility/CppMacros.h"]]
)
