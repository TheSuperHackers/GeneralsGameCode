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

add_executable(z_guiedit WIN32)
set_target_properties(z_guiedit PROPERTIES OUTPUT_NAME guiedit)

target_sources(z_guiedit PRIVATE ${GUIEDIT_SRC})

target_include_directories(z_guiedit PRIVATE
    Include
    Resource
)

target_link_libraries(z_guiedit PRIVATE
    benchmark
    comctl32
    core_debug
    core_profile
    d3d8lib
    dbghelplib
    imm32
    stlport
    vfw32
    winmm
    z_gameengine
    z_gameenginedevice
    z_wwvegas
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_guiedit PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)

    target_sources(z_guiedit PRIVATE Resource/GUIEdit.rc)
endif()
