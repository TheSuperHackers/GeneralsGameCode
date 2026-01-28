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

add_library(core_particleeditor SHARED)
set_target_properties(core_particleeditor PROPERTIES OUTPUT_NAME particleeditor PREFIX "")

target_sources(core_particleeditor PRIVATE ${PARTICLEED_SRC})

target_include_directories(core_particleeditor PRIVATE
    include
    res
)

target_link_libraries(core_particleeditor PRIVATE
    core_debug
    core_profile
    corei_always_no_pch
    corei_gameengine_include
    corei_libraries_source_wwvegas
    corei_libraries_source_wwvegas_wwlib
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_particleeditor PRIVATE /NODEFAULTLIB:libci.lib /NODEFAULTLIB:libc.lib)
    target_compile_definitions(core_particleeditor PRIVATE _AFXDLL)
    target_sources(core_particleeditor PRIVATE ParticleEditor.rc)
    set_target_properties(core_particleeditor PROPERTIES OUTPUT_NAME ParticleEditor)
else()
    set_target_properties(core_particleeditor PROPERTIES OUTPUT_NAME particleeditor)
endif()
