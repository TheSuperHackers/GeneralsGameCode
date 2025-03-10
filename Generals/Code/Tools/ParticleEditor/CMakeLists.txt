# Set source files
set(PARTICLEED_SRC
    CButtonShowColor.cpp
    CColorAlphaDialog.cpp
    CSwitchesDialog.cpp
    EmissionTypePanels.cpp
    MoreParmsDialog.cpp
    ParticleEditor.cpp
    ParticleEditorDialog.cpp
    ParticleTypePanels.cpp
    ShaderTypePanels.cpp
    StdAfx.cpp
    VelocityTypePanels.cpp
    CButtonShowColor.h
    CColorAlphaDialog.h
    CSwitchesDialog.h
    EmissionTypePanels.h
    ISwapablePanel.h
    MoreParmsDialog.h
    ParticleEditor.h
    ParticleEditorDialog.h
    ParticleEditorExport.h
    ParticleTypePanels.h
    Resource.h
    ShaderTypePanels.h
    StdAfx.h
    VelocityTypePanels.h
)

# Targets to build.
add_library(particleeditorg SHARED)

set_target_properties(particleeditorg PROPERTIES PREFIX "")

target_include_directories(particleeditorg PRIVATE
    include
    res
    ../../Libraries/Include
    ../../gameengine/Include
    ../../Libraries/Source/WWVegas
    ../../Libraries/Source/WWVegas/WW3D2
    ../../Libraries/Source/WWVegas/WWDebug
    ../../Libraries/Source/WWVegas/WWLib
    ../../Libraries/Source/WWVegas/WWMath
    ../../Libraries/Source/WWVegas/WWSaveLoad
)

target_compile_definitions(particleeditorg PRIVATE _AFXDLL)

target_link_libraries(particleeditorg PRIVATE
    stlport
    winmm
    imm32
    vfw32
    d3d8lib
    profile
    dbghelplib
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(particleeditorg PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)

    target_sources(particleeditorg PRIVATE ParticleEditor.rc)
    set_target_properties(particleeditorg PROPERTIES OUTPUT_NAME ParticleEditor)
else()
    set_target_properties(particleeditorg PROPERTIES OUTPUT_NAME particleeditor)
endif()

target_sources(particleeditorg PRIVATE ${PARTICLEED_SRC})
