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

add_executable(guiedit WIN32)

target_include_directories(guiedit PRIVATE
    Include
    Resource
    ../../Libraries/Include
)

target_link_libraries(guiedit PRIVATE
    comctl32
    stlport
    winmm
    imm32
    vfw32
    d3d8lib
    profile
    dbghelplib
    benchmark
    wwvegas
    gameengine
    gameenginedevice
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(guiedit PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)

    target_sources(guiedit PRIVATE Resource/GUIEdit.rc)
endif()

target_sources(guiedit PRIVATE ${GUIEDIT_SRC})
