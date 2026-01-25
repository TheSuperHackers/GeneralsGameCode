set(PATCHGET_SRC
    "CHATAPI.cpp"
    "CHATAPI.h"
    "COMINIT.cpp"
    "COMINIT.h"
    "debug.cpp"
    "debug.h"
    "DownloadManager.cpp"
    "DownloadManager.h"
    "PROCESS.cpp"
    "PROCESS.h"
    "registry.cpp"
    "Registry.h"
    "RESOURCE.h"
    "WINBLOWS.cpp"
    "WINBLOWS.h"
    "WSTYPES.h"
)

add_library(corei_patchgrabber INTERFACE)

target_sources(corei_patchgrabber INTERFACE ${PATCHGET_SRC})

target_link_libraries(corei_patchgrabber INTERFACE
    comctl32
    core_debug
    core_profile
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
