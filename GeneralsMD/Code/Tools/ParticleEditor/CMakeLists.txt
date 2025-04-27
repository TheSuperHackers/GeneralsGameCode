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

add_library(z_particleeditor SHARED)
set_target_properties(z_particleeditor PROPERTIES OUTPUT_NAME particleeditor PREFIX "")

target_sources(z_particleeditor PRIVATE ${PARTICLEED_SRC})

target_include_directories(z_particleeditor PRIVATE
    include
    res
)

target_link_libraries(z_particleeditor PRIVATE
    corei_libraries_source_wwvegas
    corei_libraries_source_wwvegas_wwlib
    d3d8lib
    dbghelplib
    imm32
    core_config
    stlport
    vfw32
    winmm
    z_debug
    z_profile
    zi_gameengine_include
    zi_always
    zi_libraries_source_wwvegas
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_particleeditor PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)
    target_compile_definitions(z_particleeditor PRIVATE _AFXDLL)
    target_sources(z_particleeditor PRIVATE ParticleEditor.rc)
    set_target_properties(z_particleeditor PROPERTIES OUTPUT_NAME ParticleEditor)
else()
    set_target_properties(z_particleeditor PROPERTIES OUTPUT_NAME particleeditor)
endif()
