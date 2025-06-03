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

add_library(corei_autorun INTERFACE)

target_sources(corei_autorun INTERFACE ${AUTORUN_SRC})

target_link_libraries(corei_autorun INTERFACE
    core_config
    core_utility
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_autorun INTERFACE AUTORUN.RC)
endif()
