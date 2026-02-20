if(wxWidgets_FOUND)
    # wxWidgets version - cross-platform
    include(${CMAKE_SOURCE_DIR}/cmake/TargetExports.cmake)
    include(${wxWidgets_USE_FILE})
    
    # Generate XRC resource file
    # Try to find wxrc (works with vcpkg, system installs, etc.)
    # When cross-compiling, also check for native wxrc as a hint
    if(CMAKE_CROSSCOMPILING)
        find_program(WXRC_EXECUTABLE wxrc
            HINTS /opt/wxwidgets-3.2.9-native/bin
                  /usr/local/bin
            DOC "wxrc resource compiler")
    else()
        find_program(WXRC_EXECUTABLE wxrc
            DOC "wxrc resource compiler")
    endif()
    
    if(WXRC_EXECUTABLE)
        message(STATUS "Found wxrc: ${WXRC_EXECUTABLE}")
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp
            COMMAND ${WXRC_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc
            COMMENT "Embedding XRC resources into C++ code"
        )
        set(XRC_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp)
    else()
        message(WARNING "wxrc not found - using runtime XRC loading (requires ui/dbgwinui.xrc file at runtime)")
        message(WARNING "For production builds, install native wxWidgets to enable XRC embedding")
        # Fallback to runtime loading
        set(XRC_SOURCES wxdbgwinui.cpp)
    endif()
    
    set(DEBUGWINDOW_WX_SRC
        "dbgwinapp.cpp"
        "dbgwinapp.h"
        "dbgwinframe.cpp"
        "dbgwinframe.h"
        "debugwindow_wx.cpp"
        "debugwindow_wx.h"
        "wxdbgwinui.h"
        ${XRC_SOURCES}
    )
    
    add_library(core_debugwindow SHARED)
    target_sources(core_debugwindow PRIVATE ${DEBUGWINDOW_WX_SRC})
    
    # Platform-specific threading implementation
    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        target_sources(core_debugwindow PRIVATE debugwindow_win32.cpp)
        set_target_properties(core_debugwindow PROPERTIES 
            OUTPUT_NAME DebugWindow
            PREFIX ""  # Remove "lib" prefix added by MinGW
        )
    else()
        target_sources(core_debugwindow PRIVATE debugwindow_posix.cpp)
        target_link_libraries(core_debugwindow PRIVATE pthread)
        set_target_properties(core_debugwindow PROPERTIES OUTPUT_NAME debugwindow)
    endif()
    
    target_link_libraries(core_debugwindow PRIVATE
        core_config
        ${wxWidgets_LIBRARIES}
    )
    
    target_include_directories(core_debugwindow PRIVATE ${wxWidgets_INCLUDE_DIRS})
    target_compile_definitions(core_debugwindow PRIVATE ${wxWidgets_DEFINITIONS})
    
    # Export the C API symbols
    target_exports(core_debugwindow SYMBOLS
        CreateDebugDialog
        DestroyDebugDialog
        CanAppContinue
        ForceAppContinue
        RunAppFast
        AppendMessage
        SetFrameNumber
        AppendMessageAndPause
        AdjustVariable
        AdjustVariableAndPause
    )

elseif(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # Original MFC version - Windows only
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
    
    target_compile_definitions(core_debugwindow PRIVATE _AFXDLL)
    target_sources(core_debugwindow PRIVATE
        DebugWindow.rc
        DebugWindow.def
    )
    set_target_properties(core_debugwindow PROPERTIES OUTPUT_NAME DebugWindow)
    
else()
    # No GUI tools available on this platform
    message(STATUS "DebugWindow: No GUI framework available, skipping build")
endif()
