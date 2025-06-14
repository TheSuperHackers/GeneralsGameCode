set(GUIEDIT_SRC
    "Include/DialogProc.h"
    "Include/EditWindow.h"
    "Include/GUIEdit.h"
    "Include/GUIEditColor.h"
    "Include/GUIEditDisplay.h"
    "Include/GUIEditWindowManager.h"
    "Include/HierarchyView.h"
    "Include/LayoutScheme.h"
    "Include/Properties.h"
    "Include/WinMain.h"
    "Source/EditWindow.cpp"
    "Source/GUIEdit.cpp"
    "Source/GUIEditDisplay.cpp"
    "Source/GUIEditWindowManager.cpp"
    "Source/HierarchyView.cpp"
    "Source/LayoutScheme.cpp"
    "Source/Properties.cpp"
    "Source/Save.cpp"
    "Source/WinMain.cpp"
    "Source/Dialog Procedures/CallbackEditor.cpp"
    "Source/Dialog Procedures/CheckBoxProperties.cpp"
    "Source/Dialog Procedures/ColorDialog.cpp"
    "Source/Dialog Procedures/ComboBoxProperties.cpp"
    "Source/Dialog Procedures/GenericProperties.cpp"
    "Source/Dialog Procedures/GridSettings.cpp"
    "Source/Dialog Procedures/ListboxProperties.cpp"
    "Source/Dialog Procedures/NewLayoutDialog.cpp"
    "Source/Dialog Procedures/ProgressBarProperties.cpp"
    "Source/Dialog Procedures/PushButtonProperties.cpp"
    "Source/Dialog Procedures/RadioButtonProperties.cpp"
    "Source/Dialog Procedures/SliderProperties.cpp"
    "Source/Dialog Procedures/StaticTextProperties.cpp"
    "Source/Dialog Procedures/TabControlProperties.cpp"
    "Source/Dialog Procedures/TextEntryProperties.cpp"
)

add_executable(g_guiedit WIN32)
set_target_properties(g_guiedit PROPERTIES OUTPUT_NAME guiedit)

target_sources(g_guiedit PRIVATE ${GUIEDIT_SRC})

target_include_directories(g_guiedit PRIVATE
    Include
    Resource
)

target_link_libraries(g_guiedit PRIVATE
    comctl32
    d3d8lib
    g_gameengine
    g_gameenginedevice
    g_wwvegas
    gi_always
    imm32
    stlport
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_guiedit PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)

    target_sources(g_guiedit PRIVATE Resource/GUIEdit.rc)
endif()
