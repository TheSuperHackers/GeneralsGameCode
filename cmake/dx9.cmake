include(FetchContent)

FetchContent_Declare(
    dx9_d3dx
    URL https://www.nuget.org/api/v2/package/Microsoft.DXSDK.D3DX/9.29.952.8
    DOWNLOAD_NAME dxsdk-d3dx.nupkg
)

FetchContent_MakeAvailable(dx9_d3dx)

find_library(D3DX9_LIBRARY NAMES d3dx9 d3dx9d
    PATHS
        ${dx9_d3dx_SOURCE_DIR}/build/native/release/lib/x86
        ${dx9_d3dx_SOURCE_DIR}/build/native/debug/lib/x86
    NO_DEFAULT_PATH REQUIRED)
find_path(DIRECTXSDK_INCLUDE_DIR d3dx9.h
    PATHS ${dx9_d3dx_SOURCE_DIR}/build/native/include
    NO_DEFAULT_PATH REQUIRED)

add_library(d3d9lib INTERFACE)
target_link_libraries(d3d9lib INTERFACE
    d3d9
    ${D3DX9_LIBRARY}
    dinput8
    dxguid
)
target_include_directories(d3d9lib INTERFACE
    ${DIRECTXSDK_INCLUDE_DIR}
)
