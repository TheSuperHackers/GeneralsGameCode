# Clang/LLVM toolchain for the supported GeneralsMD target.
#
# LLVM_ROOT may point at an LLVM installation. If it is not set, the tools are
# resolved from PATH, which keeps this toolchain usable on developer machines
# and CI runners with different installation locations.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(_llvm_hints)
if(DEFINED LLVM_ROOT AND NOT LLVM_ROOT STREQUAL "")
    list(APPEND _llvm_hints "${LLVM_ROOT}" "${LLVM_ROOT}/bin")
endif()
if(DEFINED ENV{LLVM_ROOT} AND NOT "$ENV{LLVM_ROOT}" STREQUAL "")
    list(APPEND _llvm_hints "$ENV{LLVM_ROOT}" "$ENV{LLVM_ROOT}/bin")
endif()

find_program(CLANG_C_EXECUTABLE
    NAMES clang clang.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(CLANG_CXX_EXECUTABLE
    NAMES clang++ clang++.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(CLANG_CL_EXECUTABLE
    NAMES clang-cl clang-cl.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLD_LINK_EXECUTABLE
    NAMES lld-link lld-link.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLVM_RC_EXECUTABLE
    NAMES llvm-rc llvm-rc.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLVM_MT_EXECUTABLE
    NAMES llvm-mt llvm-mt.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLVM_LIB_EXECUTABLE
    NAMES llvm-lib llvm-lib.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLVM_AR_EXECUTABLE
    NAMES llvm-ar llvm-ar.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(LLVM_RANLIB_EXECUTABLE
    NAMES llvm-ranlib llvm-ranlib.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)
find_program(CLANG_SCAN_DEPS_EXECUTABLE
    NAMES clang-scan-deps clang-scan-deps.exe
    HINTS ${_llvm_hints}
    PATH_SUFFIXES bin
)

foreach(_tool IN ITEMS
    CLANG_C_EXECUTABLE
    CLANG_CXX_EXECUTABLE
    CLANG_CL_EXECUTABLE
    LLD_LINK_EXECUTABLE
    LLVM_RC_EXECUTABLE
    LLVM_MT_EXECUTABLE
    LLVM_LIB_EXECUTABLE
    LLVM_AR_EXECUTABLE
    LLVM_RANLIB_EXECUTABLE
    CLANG_SCAN_DEPS_EXECUTABLE
)
    if(NOT DEFINED ${_tool} OR "${${_tool}}" STREQUAL "")
        message(FATAL_ERROR
            "Clang/LLVM tool '${_tool}' was not found. Install LLVM and add its bin directory to PATH, "
            "or set LLVM_ROOT to the LLVM installation directory.")
    endif()
endforeach()

set(CMAKE_C_COMPILER "${CLANG_C_EXECUTABLE}" CACHE FILEPATH "Clang C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${CLANG_CXX_EXECUTABLE}" CACHE FILEPATH "Clang C++ compiler" FORCE)
get_filename_component(LLVM_BIN_DIRECTORY "${CLANG_C_EXECUTABLE}" DIRECTORY)
set(LLVM_BIN_DIRECTORY "${LLVM_BIN_DIRECTORY}" CACHE PATH "LLVM executable directory" FORCE)
set(CMAKE_C_COMPILER_TARGET "x86_64-pc-windows-msvc" CACHE STRING "Clang C target" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "x86_64-pc-windows-msvc" CACHE STRING "Clang C++ target" FORCE)
set(CMAKE_LINKER "${LLD_LINK_EXECUTABLE}" CACHE FILEPATH "LLVM linker" FORCE)
set(CMAKE_RC_COMPILER "${LLVM_RC_EXECUTABLE}" CACHE FILEPATH "LLVM resource compiler" FORCE)
set(CMAKE_MT "${LLVM_MT_EXECUTABLE}" CACHE FILEPATH "LLVM manifest tool" FORCE)
set(CMAKE_AR "${LLVM_AR_EXECUTABLE}" CACHE FILEPATH "LLVM archiver" FORCE)
set(CMAKE_RANLIB "${LLVM_RANLIB_EXECUTABLE}" CACHE FILEPATH "LLVM ranlib" FORCE)
set(CMAKE_C_COMPILER_AR "${LLVM_AR_EXECUTABLE}" CACHE FILEPATH "LLVM C archiver" FORCE)
set(CMAKE_CXX_COMPILER_AR "${LLVM_AR_EXECUTABLE}" CACHE FILEPATH "LLVM C++ archiver" FORCE)
set(CMAKE_C_COMPILER_CLANG_SCAN_DEPS "${CLANG_SCAN_DEPS_EXECUTABLE}" CACHE FILEPATH "Clang dependency scanner" FORCE)
set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${CLANG_SCAN_DEPS_EXECUTABLE}" CACHE FILEPATH "Clang C++ dependency scanner" FORCE)

set(_vcpkg_root "")
if(DEFINED VCPKG_ROOT AND NOT VCPKG_ROOT STREQUAL "")
    set(_vcpkg_root "${VCPKG_ROOT}")
elseif(DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
    set(_vcpkg_root "$ENV{VCPKG_ROOT}")
endif()

if(_vcpkg_root STREQUAL "")
    message(FATAL_ERROR
        "VCPKG_ROOT is not set. Install vcpkg and set VCPKG_ROOT to its root directory.")
endif()

file(TO_CMAKE_PATH "${_vcpkg_root}" _vcpkg_root)
if(NOT EXISTS "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
    message(FATAL_ERROR
        "VCPKG_ROOT does not contain scripts/buildsystems/vcpkg.cmake: ${_vcpkg_root}")
endif()

set(VCPKG_ROOT "${_vcpkg_root}" CACHE PATH "vcpkg installation root" FORCE)
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet" FORCE)
include("${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")

unset(_llvm_hints)
unset(_vcpkg_root)
unset(_tool)
