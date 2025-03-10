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

    target_include_directories(${appname} PRIVATE
        ../../Libraries/Include
    )
    target_sources(${appname} PRIVATE ${PATCHGET_SRC})

    target_link_libraries(${appname} PRIVATE
        gamespy::gamespy
        comctl32
        winmm
        wwvegas
        GameEngine
    )

    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        target_sources(${appname} PRIVATE SCRIPT1.RC)
    endif()
endmacro()

setup_patchgrabber(patchgrabber_en)
setup_patchgrabber(patchgrabber_de)
setup_patchgrabber(patchgrabber_fr)
setup_patchgrabber(patchgrabber_es)
target_compile_definitions(patchgrabber_en PRIVATE AFX_RESOURCE_DLL)
target_compile_definitions(patchgrabber_de PRIVATE AFX_TARG_DEU AFX_RESOURCE_DLL)
target_compile_definitions(patchgrabber_fr PRIVATE AFX_TARG_FRA AFX_RESOURCE_DLL)
target_compile_definitions(patchgrabber_es PRIVATE AFX_TARG_ESN AFX_RESOURCE_DLL)

# TODO: Add language codes with MSVC
