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
    "Source/WindowProcedures/DirectorySelect.cpp"
    "Source/WindowProcedures/ImageErrorProc.cpp"
    "Source/WindowProcedures/ImagePackerProc.cpp"
    "Source/WindowProcedures/PageErrorProc.cpp"
    "Source/WindowProcedures/PreviewProc.cpp"
)

add_library(corei_imagepacker INTERFACE)

target_sources(corei_imagepacker INTERFACE ${IMAGEPACKER_SRC})

target_include_directories(corei_imagepacker INTERFACE
    Include
    Resource
)

target_link_libraries(corei_imagepacker INTERFACE
    comctl32
    core_debug
    core_profile
    core_wwcommon
    imm32
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_imagepacker INTERFACE Resource/ImagePacker.rc)
endif()
