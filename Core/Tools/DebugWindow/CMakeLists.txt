# TheSuperHackers: Build either MFC or wxWidgets version based on availability
if(BUILD_TOOLS AND wxWidgets_FOUND)
    # wxWidgets version - cross-platform
    include(${CMAKE_SOURCE_DIR}/cmake/TargetExports.cmake)
    include(${wxWidgets_USE_FILE})
    
    # Generate XRC resource file
    find_program(WXRC_EXECUTABLE wxrc)
    if(WXRC_EXECUTABLE)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp
            COMMAND ${WXRC_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc
            COMMENT "Compiling XRC resource file"
        )
        set(XRC_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui_xrc.cpp)
    else()
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
        set_target_properties(core_debugwindow PROPERTIES OUTPUT_NAME DebugWindow)
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
    
    # Optional test executable
    if(BUILD_TESTING)
        add_executable(test_debugwindow test_debugwindow.cpp)
        target_link_libraries(test_debugwindow PRIVATE core_debugwindow)
        if(NOT WIN32)
            target_link_libraries(test_debugwindow PRIVATE pthread)
        endif()
    endif()
    
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
