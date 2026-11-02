set(MAX2W3D_SRC
    "aabtreebuilder.cpp"
    "aabtreebuilder.h"
    "AlphaModifier.cpp"
    "AlphaModifier.h"
    "animationcompressionsettings.cpp"
    "animationcompressionsettings.h"
    "AppData.cpp"
    "bchannel.cpp"
    "bchannel.h"
    "boneicon.cpp"
    "boneicon.h"
    "bpick.cpp"
    "bpick.h"
    "colboxsave.cpp"
    "colboxsave.h"
    "dazzlesave.cpp"
    "dazzlesave.h"
    "dllmain.cpp"
    "dllmain.h"
    "ExportAll.cpp"
    "ExportAllDlg.cpp"
    "ExportAllDlg.h"
    "exportlog.cpp"
    "exportlog.h"
    "floaterdialog.cpp"
    "floaterdialog.h"
    "FormClass.cpp"
    "FormClass.h"
    "gamemaps.cpp"
    "gamemaps.h"
    "GameMtl.cpp"
    "gamemtl.h"
    "GameMtlDlg.cpp"
    "GameMtlDlg.h"
    "GameMtlForm.cpp"
    "GameMtlForm.h"
    "GameMtlPassDlg.cpp"
    "GameMtlPassDlg.h"
    "GameMtlShaderDlg.cpp"
    "GameMtlShaderDlg.h"
    "GameMtlTextureDlg.cpp"
    "GameMtlTextureDlg.h"
    "GameMtlVertexMaterialDlg.cpp"
    "GameMtlVertexMaterialDlg.h"
    "genlodextensiondialog.cpp"
    "genlodextensiondialog.h"
    "genmtlnamesdialog.cpp"
    "genmtlnamesdialog.h"
    "gennamesdialog.cpp"
    "gennamesdialog.h"
    "geometryexportcontext.h"
    "geometryexporttask.cpp"
    "geometryexporttask.h"
    "hiersave.cpp"
    "hiersave.h"
    "hlodsave.cpp"
    "hlodsave.h"
    "InputDlg.cpp"
    "InputDlg.h"
    "logdlg.cpp"
    "logdlg.h"
    "max2w3d.def"
    "maxworldinfo.cpp"
    "maxworldinfo.h"
    "meshbuild.cpp"
    "meshbuild.h"
    "meshcon.cpp"
    "meshcon.h"
    "meshsave.cpp"
    "meshsave.h"
    "motion.cpp"
    "motion.h"
    "namedsel.cpp"
    "namedsel.h"
    "presetexportoptionsdialog.cpp"
    "presetexportoptionsdialog.h"
    "resource.h"
    "SceneSetup.cpp"
    "SceneSetupDlg.cpp"
    "SceneSetupDlg.h"
    "skin.cpp"
    "skin.h"
    "SkinCopy.cpp"
    "skindata.cpp"
    "skindata.h"
    "TARGA.cpp"
    "TARGA.h"
    "util.cpp"
    "util.h"
    "Utility.cpp"
    "vchannel.cpp"
    "vchannel.h"
    "w3d_file.h"
    "w3d_obsolete.h"
    "w3dappdata.cpp"
    "w3dappdata.h"
    "w3ddesc.cpp"
    "w3ddesc.h"
    "w3dexp.cpp"
    "w3dexp.h"
    "w3dmtl.cpp"
    "w3dmtl.h"
    "w3dutil.cpp"
    "w3dutil.h"
)

add_library(core_max2w3d SHARED)
set_target_properties(core_max2w3d PROPERTIES OUTPUT_NAME max2w3d SUFFIX ".dle")

target_include_directories(core_max2w3d PRIVATE
    "../pluglib"
)

target_sources(core_max2w3d PRIVATE ${MAX2W3D_SRC})

target_link_libraries(core_max2w3d PRIVATE
    Comctl32
    core_pluglib
    # d3d8lib is only linked to have its basetsd.h used instead of the vs6's basetsd.h
    # The headers typedef INT_PTR to different types, but deps_maxsdk expects it
    # typedef'ed to int, otherwise throws linker errors.
    d3d8lib
    winmm
)

set_target_properties(core_max2w3d PROPERTIES LINK_FLAGS "/DEF:max2w3d.def")

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(core_max2w3d PRIVATE max2w3d.rc)
endif()
