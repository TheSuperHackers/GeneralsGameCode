set(DEBUGWINDOW_SRC
    "DebugWindow.cpp"
    "DebugWindow.h"
    "DebugWindowDialog.cpp"
    "DebugWindowDialog.h"
    "DebugWindowExport.h"
    "StdAfx.cpp"
    "StdAfx.h"
)

add_library(core_debugwindow SHARED)

target_sources(core_debugwindow PRIVATE ${DEBUGWINDOW_SRC})

target_link_libraries(core_debugwindow PRIVATE
    core_config
    corei_always_no_pch
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(core_debugwindow PRIVATE _AFXDLL)
    target_sources(core_debugwindow PRIVATE
        DebugWindow.rc
        DebugWindow.def
    )
    set_target_properties(core_debugwindow PROPERTIES OUTPUT_NAME DebugWindow)
else()
    set_target_properties(core_debugwindow PROPERTIES OUTPUT_NAME debugwindow)
endif()
