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

add_executable(z_imagepacker WIN32)
set_target_properties(z_imagepacker PROPERTIES OUTPUT_NAME imagepacker)

target_sources(z_imagepacker PRIVATE ${IMAGEPACKER_SRC})

target_include_directories(z_imagepacker PRIVATE
    Include
    Resource
)

target_link_libraries(z_imagepacker PRIVATE
    benchmark
    comctl32
    dbghelplib
    imm32
    vfw32
    winmm
    z_debug
    z_gameengine
    z_gameenginedevice
    z_profile
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(z_imagepacker PRIVATE Resource/ImagePacker.rc)
endif()
