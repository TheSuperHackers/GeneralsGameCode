set(AUTORUN_SRC
    "ARGS.CPP"
    "ARGS.H"
    "autorun.cpp"
    "autorun.h"
    "CallbackHook.h"
    "CDCNTRL.CPP"
    "CDCNTRL.H"
    "DrawButton.cpp"
    "DrawButton.h"
    "EZGIMEX.cpp"
    "GameText.cpp"
    "GameText.h"
    "GETCD.CPP"
    "GetCD.h"
    "gimex.h"
    "IGR.cpp"
    "IGR.h"
    "Jsupport.cpp"
    "JSUPPORT.H"
    "leanAndMeanAutorun.h"
    "Locale_API.cpp"
    "Locale_API.h"
    "locale.cpp"
    "locale.h"
    "POINT.h"
    "RECT.h"
    "resource.h"
    "TTFont.cpp"
    "TTFont.h"
    "Utils.cpp"
    "Utils.h"
    "ViewHTML.cpp"
    "ViewHTML.h"
    "WinFix.CPP"
    "WinFix.H"
    "Wnd_file.cpp"
    "Wnd_File.h"
    "WSYS_File.cpp"
    "WSYS_file.h"
    "WSYS_FileSystem.cpp"
    "WSYS_FileSystem.h"
    "WSYS_RAMFile.cpp"
    "WSYS_RAMFile.h"
    "WSYS_StdFile.cpp"
    "WSYS_StdFile.h"
    "WSYS_StdFileSystem.cpp"
    "WSYS_StdFileSystem.h"
)

macro(setup_autorun appname)
    add_executable(${appname} WIN32)

    target_sources(${appname} PRIVATE ${AUTORUN_SRC})

    target_link_libraries(${appname} PRIVATE
        gi_always
        core_config
        winmm
    )

    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        target_sources(${appname} PRIVATE AUTORUN.RC)
    endif()
endmacro()

setup_autorun(g_autorun_en)
setup_autorun(g_autorun_de)
setup_autorun(g_autorun_fr)
setup_autorun(g_autorun_es)
set_target_properties(g_autorun_en PROPERTIES OUTPUT_NAME autorun_en)
set_target_properties(g_autorun_de PROPERTIES OUTPUT_NAME autorun_de)
set_target_properties(g_autorun_fr PROPERTIES OUTPUT_NAME autorun_fr)
set_target_properties(g_autorun_es PROPERTIES OUTPUT_NAME autorun_es)

# TODO: Configure different resources for languages. See Autorun.dsp
