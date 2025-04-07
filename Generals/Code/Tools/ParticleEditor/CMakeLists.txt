# Set source files
set(PARTICLEED_SRC
    "CButtonShowColor.cpp"
    "CColorAlphaDialog.cpp"
    "CSwitchesDialog.cpp"
    "EmissionTypePanels.cpp"
    "MoreParmsDialog.cpp"
    "ParticleEditor.cpp"
    "ParticleEditorDialog.cpp"
    "ParticleTypePanels.cpp"
    "ShaderTypePanels.cpp"
    "StdAfx.cpp"
    "VelocityTypePanels.cpp"
    "CButtonShowColor.h"
    "CColorAlphaDialog.h"
    "CSwitchesDialog.h"
    "EmissionTypePanels.h"
    "ISwapablePanel.h"
    "MoreParmsDialog.h"
    "ParticleEditor.h"
    "ParticleEditorDialog.h"
    "ParticleEditorExport.h"
    "ParticleTypePanels.h"
    "Resource.h"
    "ShaderTypePanels.h"
    "StdAfx.h"
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
    gz_config
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
