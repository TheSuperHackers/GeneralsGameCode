# Acquire separately with this directory's manifest. Disabled builds never run
# vcpkg or look for Crashpad. A copied, pinned installed prefix works without a
# vcpkg toolchain in the game build.
set(RTS_CRASHPAD_PACKAGE_ROOT "" CACHE PATH "Pinned x86-windows-static-md Crashpad installed prefix")
set(RTS_CRASHPAD_REVISION "7e0af1d4d45b526f01677e74a56f4a951b70517d")
set(RTS_CRASHPAD_MINI_CHROMIUM_REVISION "dce72d97d1c2e9beb5e206c6a05a702269794ca3")
set(package "${RTS_CRASHPAD_PACKAGE_ROOT}")
if(NOT EXISTS "${package}/share/crashpad/vcpkg.spdx.json")
    message(FATAL_ERROR "Set RTS_CRASHPAD_PACKAGE_ROOT to the x86-windows-static-md prefix built with Dependencies/Crashpad/vcpkg.json. See docs/crashpad.md.")
endif()
file(READ "${package}/share/crashpad/vcpkg.spdx.json" provenance)
foreach(required "crashpad:x86-windows-static-md@2024-04-11#7"
        "${RTS_CRASHPAD_REVISION}" "${RTS_CRASHPAD_MINI_CHROMIUM_REVISION}")
    string(FIND "${provenance}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Crashpad package does not match the pinned x86 package: missing ${required}")
    endif()
endforeach()

# The baseline port's config uses these two variables even outside a vcpkg
# toolchain. Keep the workaround scoped to this dependency directory.
get_filename_component(_VCPKG_INSTALLED_DIR "${package}" DIRECTORY)
get_filename_component(VCPKG_TARGET_TRIPLET "${package}" NAME)
list(PREPEND CMAKE_PREFIX_PATH "${package}")
find_package(crashpad CONFIG REQUIRED PATHS "${package}/share/crashpad" NO_DEFAULT_PATH)
set(handler "${package}/tools/crashpad/crashpad_handler.exe")
if(NOT EXISTS "${handler}")
    message(FATAL_ERROR "The pinned package is missing its matching crashpad_handler.exe")
endif()
set(RTS_CRASHPAD_HANDLER "${handler}" PARENT_SCOPE)

# Deploy the redistributable x86 VC runtime from the selected Visual Studio.
# The Windows 10+ Universal CRT is supplied by the OS. Debug builds still need
# a developer installation of the non-redistributable debug CRT.
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)
if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    message(FATAL_ERROR "The Visual Studio x86 redistributable runtime was not found")
endif()
set(RTS_CRASHPAD_RUNTIME_FILES "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" PARENT_SCOPE)

add_library(rts_crashpad SHARED CrashpadBridge.cpp CrashpadBridge.h CrashpadBridge.def)
target_compile_features(rts_crashpad PRIVATE cxx_std_20)
target_link_libraries(rts_crashpad PRIVATE crashpad::crashpad)
set_target_properties(rts_crashpad PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    DISABLE_PRECOMPILE_HEADERS ON)
target_compile_options(rts_crashpad PRIVATE /Zi)
target_link_options(rts_crashpad PRIVATE /DEBUG)
target_compile_definitions(rts_crashpad PRIVATE
    "RTS_CRASHPAD_COMPILER=\"${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\""
    "RTS_CRASHPAD_CONFIGURATION=\"$<CONFIG>\""
    "RTS_CRASHPAD_REVISION=\"${RTS_CRASHPAD_REVISION}\"")

set(notices "${CMAKE_CURRENT_BINARY_DIR}/notices")
file(MAKE_DIRECTORY "${notices}")
configure_file("${package}/share/crashpad/copyright" "${notices}/crashpad-LICENSE" COPYONLY)
configure_file("${package}/share/zlib/copyright" "${notices}/zlib-LICENSE" COPYONLY)
configure_file("${package}/share/crashpad/vcpkg.spdx.json" "${notices}/crashpad-package.spdx.json" COPYONLY)
configure_file("${package}/share/zlib/vcpkg.spdx.json" "${notices}/zlib-package.spdx.json" COPYONLY)
configure_file("${package}/share/crashpad/vcpkg_abi_info.txt" "${notices}/crashpad-package-abi.txt" COPYONLY)
configure_file(getopt-LICENSE "${notices}/getopt-LICENSE" COPYONLY)
configure_file(mini-chromium-LICENSE "${notices}/mini-chromium-LICENSE" COPYONLY)
set(RTS_CRASHPAD_NOTICES "${notices}" PARENT_SCOPE)
