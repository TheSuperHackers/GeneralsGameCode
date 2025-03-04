set(DEBUGWINDOW_SRC
    "DebugWindow.cpp"
    "DebugWindow.h"
    "DebugWindowDialog.cpp"
    "DebugWindowDialog.h"
    "DebugWindowExport.h"
    "StdAfx.cpp"
    "StdAfx.h"
)

add_library(z_debugwindow SHARED)
set_target_properties(z_debugwindow PROPERTIES OUTPUT_NAME debugwindow)

target_sources(z_debugwindow PRIVATE ${DEBUGWINDOW_SRC})

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_debugwindow PRIVATE _AFXDLL)
    target_sources(z_debugwindow PRIVATE
        DebugWindow.rc
        DebugWindow.def
    )
    set_target_properties(z_debugwindow PROPERTIES OUTPUT_NAME DebugWindow)
else()
    set_target_properties(z_debugwindow PROPERTIES OUTPUT_NAME debugwindow)
endif()
