set(PATCHGET_SRC
    "CHATAPI.CPP"
    "CHATAPI.H"
    "COMINIT.CPP"
    "COMINIT.H"
    "debug.cpp"
    "debug.h"
    "DownloadManager.cpp"
    "DownloadManager.h"
    "PROCESS.CPP"
    "PROCESS.H"
    "registry.cpp"
    "Registry.h"
    "RESOURCE.H"
    "WINBLOWS.CPP"
    "WINBLOWS.H"
    "WSTYPES.H"
)

add_library(corei_patchgrabber INTERFACE)

target_sources(corei_patchgrabber INTERFACE ${PATCHGET_SRC})

target_link_libraries(corei_patchgrabber INTERFACE
    comctl32
    core_debug
    core_profile
    dbghelplib
    gamespy::gamespy
    imm32
    vfw32
    winmm
)

target_compile_definitions(corei_patchgrabber INTERFACE
    $<$<CONFIG:Debug>:DEBUG_CRASHING>
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_patchgrabber INTERFACE SCRIPT1.RC)
endif()
