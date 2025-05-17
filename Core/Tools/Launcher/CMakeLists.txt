add_subdirectory(DatGen)

set(LAUNCHER_SRC
    "BFISH.CPP"
    "BFISH.H"
    "configfile.cpp"
    "configfile.h"
    "dialog.cpp"
    "dialog.h"
    "dictionary.h"
    "filed.h"
    "findpatch.cpp"
    "findpatch.h"
    "loadbmp.cpp"
    "loadbmp.h"
    "main.cpp"
    "monod.cpp"
    "monod.h"
    "odevice.h"
    "patch.cpp"
    "patch.h"
    "process.cpp"
    "process.h"
    "Protect.cpp"
    "Protect.h"
    "resource.h"
    "streamer.cpp"
    "streamer.h"
    "wdebug.cpp"
    "wdebug.h"
    "winblows.cpp"
    "winblows.h"
    "wstring.cpp"
    "wstring.h"
    "wstypes.h"
    "Toolkit/Debug/DebugPrint.cpp"
    "Toolkit/Debug/DebugPrint.h"
    "Toolkit/Storage/File.cpp"
    "Toolkit/Storage/File.h"
    "Toolkit/Storage/Rights.h"
    "Toolkit/Storage/Stream.h"
    "Toolkit/Support/RefCounted.h"
    "Toolkit/Support/RefPtr.h"
    "Toolkit/Support/StringConvert.cpp"
    "Toolkit/Support/StringConvert.h"
    "Toolkit/Support/UString.cpp"
    "Toolkit/Support/UString.h"
    "Toolkit/Support/UTypes.h"
    "Toolkit/Support/Visualc.h"
)

add_library(corei_launcher INTERFACE)

target_sources(corei_launcher INTERFACE ${LAUNCHER_SRC})

target_include_directories(corei_launcher INTERFACE
    Toolkit
)

target_compile_definitions(corei_launcher INTERFACE
    USE_NOTEPAD
    $<$<CONFIG:Debug>:USE_GAMEDIR_FROM_LCF>
)

target_link_libraries(corei_launcher INTERFACE
    comctl32
    core_config
    core_utility
    safedisc
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_launcher INTERFACE launcher1.rc)
endif()
