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

macro(setup_patchgrabber appname)
    add_executable(${appname} WIN32)
    target_sources(${appname} PRIVATE ${PATCHGET_SRC})
    target_link_libraries(${appname} PRIVATE
        comctl32
        dbghelplib
        g_gameengine
        g_gameenginedevice
        g_wwvegas
        gamespy::gamespy
        gi_always
        imm32
        vfw32
        winmm
    )

    target_compile_definitions(${appname} PRIVATE
        $<$<CONFIG:Debug>:DEBUG_CRASHING>
    )

    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        target_sources(${appname} PRIVATE SCRIPT1.RC)
    endif()
endmacro()

setup_patchgrabber(g_patchgrabber_en)
setup_patchgrabber(g_patchgrabber_de)
setup_patchgrabber(g_patchgrabber_fr)
setup_patchgrabber(g_patchgrabber_es)
set_target_properties(g_patchgrabber_en PROPERTIES OUTPUT_NAME patchgrabber_en)
set_target_properties(g_patchgrabber_de PROPERTIES OUTPUT_NAME patchgrabber_de)
set_target_properties(g_patchgrabber_fr PROPERTIES OUTPUT_NAME patchgrabber_fr)
set_target_properties(g_patchgrabber_es PROPERTIES OUTPUT_NAME patchgrabber_es)
target_compile_definitions(g_patchgrabber_en PRIVATE AFX_RESOURCE_DLL)
target_compile_definitions(g_patchgrabber_de PRIVATE AFX_TARG_DEU AFX_RESOURCE_DLL)
target_compile_definitions(g_patchgrabber_fr PRIVATE AFX_TARG_FRA AFX_RESOURCE_DLL)
target_compile_definitions(g_patchgrabber_es PRIVATE AFX_TARG_ESN AFX_RESOURCE_DLL)

# TODO: Add language codes with MSVC
