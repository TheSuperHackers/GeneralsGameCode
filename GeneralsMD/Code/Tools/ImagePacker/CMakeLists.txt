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

add_executable(imagepacker WIN32)

target_include_directories(imagepacker PRIVATE
    Include
    Resource
    ../../Libraries/Include
)

target_link_libraries(imagepacker PRIVATE
    comctl32
    winmm
    imm32
    vfw32
    benchmark
    dbghelplib
    profile
    gameengine
    gameenginedevice
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(imagepacker PRIVATE Resource/ImagePacker.rc)
endif()

target_sources(imagepacker PRIVATE ${IMAGEPACKER_SRC})
