# Set source files
set(PARTICLEED_SRC
    "CButtonShowColor.cpp"
    "CButtonShowColor.h"
    "CColorAlphaDialog.cpp"
    "CColorAlphaDialog.h"
    "CSwitchesDialog.cpp"
    "CSwitchesDialog.h"
    "EmissionTypePanels.cpp"
    "EmissionTypePanels.h"
    "ISwapablePanel.h"
    "MoreParmsDialog.cpp"
    "MoreParmsDialog.h"
    "ParticleEditor.cpp"
    "ParticleEditor.h"
    "ParticleEditorDialog.cpp"
    "ParticleEditorDialog.h"
    "ParticleEditorExport.h"
    "ParticleTypePanels.cpp"
    "ParticleTypePanels.h"
    "Resource.h"
    "ShaderTypePanels.cpp"
    "ShaderTypePanels.h"
    "StdAfx.cpp"
    "StdAfx.h"
    "VelocityTypePanels.cpp"
    "VelocityTypePanels.h"
)

# Targets to build.
add_library(g_particleeditor SHARED)
set_target_properties(g_particleeditor PROPERTIES OUTPUT_NAME particleeditor PREFIX "")

target_sources(g_particleeditor PRIVATE ${PARTICLEED_SRC})

target_include_directories(g_particleeditor PRIVATE
    include
    res
)

target_compile_definitions(g_particleeditor PRIVATE _AFXDLL)

target_link_libraries(g_particleeditor PRIVATE
    d3d8lib
    dbghelplib
    gi_gameengine_include
    gi_always
    gi_libraries_source_wwvegas
    gi_libraries_source_wwvegas_wwlib
    core_config
    imm32
    stlport
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_particleeditor PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)

    target_sources(g_particleeditor PRIVATE ParticleEditor.rc)
    set_target_properties(g_particleeditor PROPERTIES OUTPUT_NAME ParticleEditor)
else()
    set_target_properties(g_particleeditor PROPERTIES OUTPUT_NAME particleeditor)
endif()
