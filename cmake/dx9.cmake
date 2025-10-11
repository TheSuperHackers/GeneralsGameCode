# Find DirectX 9 SDK
find_path(DIRECTX_INCLUDE_DIR d3d9.h
    HINTS
        $ENV{DXSDK_DIR}/Include
        "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
        "C:/Program Files/Microsoft DirectX SDK (June 2010)/Include"
    DOC "Path to DirectX SDK Include directory"
)

find_library(DIRECTX_D3D9_LIBRARY d3d9.lib
    HINTS
        $ENV{DXSDK_DIR}/Lib/x86
        "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"
        "C:/Program Files/Microsoft DirectX SDK (June 2010)/Lib/x86"
    DOC "Path to d3d9.lib"
)

find_library(DIRECTX_D3DX9_LIBRARY d3dx9.lib
    HINTS
        $ENV{DXSDK_DIR}/Lib/x86
        "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"
        "C:/Program Files/Microsoft DirectX SDK (June 2010)/Lib/x86"
    DOC "Path to d3dx9.lib"
)

if(DIRECTX_INCLUDE_DIR AND DIRECTX_D3D9_LIBRARY AND DIRECTX_D3DX9_LIBRARY)
    set(DIRECTX9_FOUND TRUE)
    message(STATUS "Found DirectX 9 SDK:")
    message(STATUS "  Include: ${DIRECTX_INCLUDE_DIR}")
    message(STATUS "  d3d9.lib: ${DIRECTX_D3D9_LIBRARY}")
    message(STATUS "  d3dx9.lib: ${DIRECTX_D3DX9_LIBRARY}")
else()
    set(DIRECTX9_FOUND FALSE)
    message(WARNING "DirectX 9 SDK not found. Please set DXSDK_DIR environment variable or install to default path.")
endif()

if(DIRECTX9_FOUND)
    include_directories(${DIRECTX_INCLUDE_DIR})
    get_filename_component(DIRECTX_LIBRARY_DIR ${DIRECTX_D3D9_LIBRARY} DIRECTORY)
    link_directories(${DIRECTX_LIBRARY_DIR})

    set(DIRECTX9_LIBRARIES ${DIRECTX_D3D9_LIBRARY} ${DIRECTX_D3DX9_LIBRARY})
    add_compile_definitions(RTS_BUILD_DIRECTX9)
endif()
