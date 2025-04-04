set(W3DVIEW_SRC
    "AddToLineupDialog.cpp"
    "AdvancedAnimSheet.cpp"
    "AggregateNameDialog.cpp"
    "AmbientLightDialog.cpp"
    "AnimatedSoundOptionsDialog.cpp"
    "AnimationPropPage.cpp"
    "AnimationSpeed.cpp"
    "AnimMixingPage.cpp"
    "AnimReportPage.cpp"
    "AssetInfo.cpp"
    "AssetPropertySheet.cpp"
    "BackgroundBMPDialog.cpp"
    "BackgroundColorDialog.cpp"
    "BackgroundObjectDialog.cpp"
    "BoneMgrDialog.cpp"
    "CameraDistanceDialog.cpp"
    "CameraSettingsDialog.cpp"
    "ColorBar.cpp"
    "ColorPicker.cpp"
    "ColorPickerDialogClass.cpp"
    "ColorSelectionDialog.cpp"
    "ColorUtils.cpp"
    "DataTreeView.cpp"
    "DeviceSelectionDialog.cpp"
    "DialogToolbar.cpp"
    "DirectoryDialog.cpp"
    "EditLODDialog.cpp"
    "EmitterColorPropPage.cpp"
    "EmitterFramePropPage.cpp"
    "EmitterGeneralPropPage.cpp"
    "EmitterInstanceList.cpp"
    "EmitterLineGroupPropPage.cpp"
    "EmitterLinePropPage.cpp"
    "EmitterParticlePropPage.cpp"
    "EmitterPhysicsPropPage.cpp"
    "EmitterPropertySheet.cpp"
    "EmitterRotationPropPage.cpp"
    "EmitterSizePropPage.cpp"
    "EmitterUserPropPage.cpp"
    "GammaDialog.cpp"
    "Globals.cpp"
    "GraphicView.cpp"
    "HierarchyPropPage.cpp"
    "MainFrm.cpp"
    "MeshPropPage.cpp"
    "OpacitySettingsDialog.cpp"
    "OpacityVectorDialog.cpp"
    "ParticleBlurTimeKeyDialog.cpp"
    "ParticleFrameKeyDialog.cpp"
    "ParticleRotationKeyDialog.cpp"
    "ParticleSizeDialog.cpp"
    "PlaySoundDialog.cpp"
    "ResolutionDialog.cpp"
    "RestrictedFileDialog.cpp"
    "RingColorPropPage.cpp"
    "RingGeneralPropPage.cpp"
    "RingPropertySheet.cpp"
    "RingSizePropPage.cpp"
    "SaveSettingsDialog.cpp"
    "ScaleDialog.cpp"
    "SceneLightDialog.cpp"
    "ScreenCursor.cpp"
    "SoundEditDialog.cpp"
    "SphereColorPropPage.cpp"
    "SphereGeneralPropPage.cpp"
    "SpherePropertySheet.cpp"
    "SphereSizePropPage.cpp"
    "StdAfx.cpp"
    "TexturePathDialog.cpp"
    "Toolbar.cpp"
    "Utils.cpp"
    "Vector3RndCombo.cpp"
    "ViewerAssetMgr.cpp"
    "ViewerScene.cpp"
    "VolumeRandomDialog.cpp"
    "W3DView.cpp"
    "W3DViewDoc.cpp"
    "W3DViewView.cpp"
)

add_executable(z_w3dview WIN32)
set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME W3DView)

target_sources(z_w3dview PRIVATE ${W3DVIEW_SRC})

target_link_libraries(z_w3dview PRIVATE
    dbghelplib
    imm32
    Version
    vfw32
    winmm
    z_wwaudio
    z_wwcommon
    z_gameengine
    z_gameenginedevice
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(IS_VS6_BUILD)
        target_compile_definitions(z_w3dview PRIVATE HTASK_OR_DWORD=HTASK)
    else()
        target_compile_definitions(z_w3dview PRIVATE HTASK_OR_DWORD=DWORD)
    endif()
    target_compile_definitions(z_w3dview PRIVATE _AFXDLL)
    target_sources(z_w3dview PRIVATE W3DView.rc)
endif()
