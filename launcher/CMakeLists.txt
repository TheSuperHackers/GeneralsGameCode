cmake_minimum_required(VERSION 3.25)

# Standalone WIN32 binary; intentionally does NOT link any of the engine
# libraries, so it stays tiny and starts before the game does its heavy
# startup work (registry probes, BIG mounting, DirectX init, ...).
add_executable(z_launcher WIN32)

set_target_properties(z_launcher PROPERTIES OUTPUT_NAME "ZuluLauncher")

# Match the game's ZULU_VERSION_* defaults so a plain `cmake ..` build
# outside the Make pipeline still produces a consistent pair of binaries.
# The top-level Makefile passes /D overrides parsed from APPVERSION in
# installer/Zulu.nsi.
if(NOT DEFINED ZULU_VERSION_MAJOR)
    set(ZULU_VERSION_MAJOR 1)
endif()
if(NOT DEFINED ZULU_VERSION_MINOR)
    set(ZULU_VERSION_MINOR 4)
endif()
if(NOT DEFINED ZULU_VERSION_BUILDNUM)
    set(ZULU_VERSION_BUILDNUM 601)
endif()

# configure_file uses forward slashes; RC files happily accept those even
# on Windows. We point at the game's icon in the source tree so the launcher
# shortcut looks identical to a direct-to-game shortcut.
set(ZULU_LAUNCHER_ICON     "${CMAKE_SOURCE_DIR}/GeneralsMD/Code/Main/Generals.ico")
set(ZULU_LAUNCHER_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/ZuluLauncher.manifest")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ZuluLauncher.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ZuluLauncher.rc
    @ONLY
)

target_sources(z_launcher PRIVATE
    ZuluLauncher.cpp
)

if(MSVC)
    # VS2005+ injects a default manifest; we ship our own, so turn it off
    # to avoid duplicate-manifest link warnings (same pattern as the game).
    if(NOT IS_VS6_BUILD)
        target_link_options(z_launcher PRIVATE "/MANIFEST:NO")
    endif()

    target_sources(z_launcher PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/ZuluLauncher.rc
    )
endif()

# wininet: HTTPS GET for latest.json and the installer download.
# version: GetFileVersionInfo / VerQueryValue for the installed-version read.
# shell32: ShellExecuteEx for handing off to the elevation-requesting installer.
target_link_libraries(z_launcher PRIVATE
    wininet
    version
    shell32
    advapi32
    user32
    kernel32
)

target_include_directories(z_launcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(z_launcher)
endif()
