#!/bin/bash
set -euo pipefail
cd /build/cnc
wineboot
# Optional version override forwarded from `make installer`. Empty values
# fall through to the cmake-side defaults in
# GeneralsMD/Code/Main/CMakeLists.txt so plain `cmake ..` builds still
# work outside the Make pipeline.
ZULU_VERSION_FLAGS=()
if [ -n "${ZULU_VERSION_MAJOR:-}" ]; then
    ZULU_VERSION_FLAGS+=("-DZULU_VERSION_MAJOR=${ZULU_VERSION_MAJOR}")
fi
if [ -n "${ZULU_VERSION_MINOR:-}" ]; then
    ZULU_VERSION_FLAGS+=("-DZULU_VERSION_MINOR=${ZULU_VERSION_MINOR}")
fi
if [ -n "${ZULU_VERSION_BUILDNUM:-}" ]; then
    ZULU_VERSION_FLAGS+=("-DZULU_VERSION_BUILDNUM=${ZULU_VERSION_BUILDNUM}")
fi

if [ "${FORCE_CMAKE:-}" = "true" ] || [ ! -f  build/docker/build.ninja  ]; then
   wine /build/tools/cmake/bin/cmake.exe \
         --preset ${PRESET} \
        -DCMAKE_SYSTEM="Windows" \
        -DCMAKE_SYSTEM_NAME="Windows" \
        -DCMAKE_SIZEOF_VOID_P=4 \
        -DCMAKE_MAKE_PROGRAM="Z:/build/tools/ninja.exe" \
        -DCMAKE_C_COMPILER="Z:/build/tools/vs6/vc98/bin/cl.exe" \
        -DCMAKE_CXX_COMPILER="Z:/build/tools/vs6/vc98/bin/cl.exe" \
        -DGIT_EXECUTABLE="Z:/build/tools/git/git.exe" \
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
        -DCMAKE_DISABLE_PRECOMPILE_HEADERS=1 \
        -DCMAKE_C_COMPILER_WORKS=1 \
        -DCMAKE_CXX_COMPILER_WORKS=1 \
        -DZULU_CLIENT_KEY="${ZULU_CLIENT_KEY:-}" \
        "${ZULU_VERSION_FLAGS[@]}" \
        -B /build/cnc/build/docker
fi

cd /build/cnc/build/docker 
wine cmd /c "set TMP=Z:\build\tmp& set TEMP=Z:\build\tmp& Z:\build\tools\ninja.exe $MAKE_TARGET"


