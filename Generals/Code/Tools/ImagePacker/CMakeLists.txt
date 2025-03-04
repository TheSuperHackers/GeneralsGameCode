set(IMAGEPACKER_SRC
    "Include/ImageDirectory.h"
    "Include/ImageInfo.h"
    "Include/ImagePacker.h"
    "Include/TexturePage.h"
    "Include/WindowProc.h"
    "Include/WinMain.h"
    "Source/ImageInfo.cpp"
    "Source/ImagePacker.cpp"
    "Source/TexturePage.cpp"
    "Source/WinMain.cpp"
    "Source/Window Procedures/DirectorySelect.cpp"
    "Source/Window Procedures/ImageErrorProc.cpp"
    "Source/Window Procedures/ImagePackerProc.cpp"
    "Source/Window Procedures/PageErrorProc.cpp"
    "Source/Window Procedures/PreviewProc.cpp"
)

add_executable(g_imagepacker WIN32)
set_target_properties(g_imagepacker PROPERTIES OUTPUT_NAME imagepacker)

target_sources(g_imagepacker PRIVATE ${IMAGEPACKER_SRC})

target_include_directories(g_imagepacker PRIVATE
    Include
    Resource
)

target_link_libraries(g_imagepacker PRIVATE
    benchmark
    comctl32
    dbghelplib
    g_gameengine
    g_gameenginedevice
    gi_libraries_include
    imm32
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_imagepacker PRIVATE Resource/ImagePacker.rc)
endif()
