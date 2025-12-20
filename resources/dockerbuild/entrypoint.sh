#!/bin/bash
set -euo pipefail
cd /build/cnc
wineboot
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
        -B /build/cnc/build/docker
fi

cd /build/cnc/build/docker 
wine cmd /c "set TMP=Z:\build\tmp& set TEMP=Z:\build\tmp& Z:\build\tools\ninja.exe $MAKE_TARGET"


