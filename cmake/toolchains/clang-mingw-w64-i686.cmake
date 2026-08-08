# Copyright 2026 TheSuperHackers
# Clang MinGW-w64 32-bit (i686) Toolchain File
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-mingw-w64-i686.cmake
# Requires: clang, lld, and mingw-w64 headers/libraries installed

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Find Clang compilers (prefer newer versions)
find_program(CLANG_C_COMPILER NAMES clang-22 clang-21 clang-20 clang-19 clang-18 clang-17 clang)
find_program(CLANG_CXX_COMPILER NAMES clang++-22 clang++-21 clang++-20 clang++-19 clang++-18 clang++-17 clang++)
find_program(LLD_LINKER NAMES lld-22 lld-21 lld-20 lld-19 lld-18 lld-17 lld ld.lld)
find_program(LLVM_AR NAMES llvm-ar-22 llvm-ar-21 llvm-ar-20 llvm-ar-19 llvm-ar-18 llvm-ar-17 llvm-ar)
find_program(LLVM_RANLIB NAMES llvm-ranlib-22 llvm-ranlib-21 llvm-ranlib-20 llvm-ranlib-19 llvm-ranlib-18 llvm-ranlib-17 llvm-ranlib)

if(NOT CLANG_C_COMPILER OR NOT CLANG_CXX_COMPILER)
    message(FATAL_ERROR "Clang not found. Install with: apt-get install clang-19 lld-19")
endif()

# Specify the cross compiler with MinGW target
set(CMAKE_C_COMPILER ${CLANG_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
set(CMAKE_C_COMPILER_TARGET i686-w64-mingw32)
set(CMAKE_CXX_COMPILER_TARGET i686-w64-mingw32)

# Use MinGW tools for resource compilation
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
set(CMAKE_DLLTOOL i686-w64-mingw32-dlltool)

# Use LLVM archiver and ranlib if available, otherwise fall back to MinGW
if(LLVM_AR)
    set(CMAKE_AR ${LLVM_AR})
endif()
if(LLVM_RANLIB)
    set(CMAKE_RANLIB ${LLVM_RANLIB})
endif()

# Use LLD linker for faster linking
if(LLD_LINKER)
    set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
    set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
endif()

# Target environment
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Force 32-bit pointer size
set(CMAKE_SIZEOF_VOID_P 4)

# Disable MFC-dependent tools (not compatible with MinGW-w64)
set(RTS_BUILD_CORE_TOOLS OFF CACHE BOOL "Disable MFC-dependent core tools for MinGW" FORCE)
set(RTS_BUILD_GENERALS_TOOLS OFF CACHE BOOL "Disable MFC-dependent Generals tools for MinGW" FORCE)
set(RTS_BUILD_ZEROHOUR_TOOLS OFF CACHE BOOL "Disable MFC-dependent Zero Hour tools for MinGW" FORCE)
