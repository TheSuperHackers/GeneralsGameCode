set(GAMEENGINEDEVICE_SRC
    Source/W3DDevice/GameClient/W3DSegmentedLineRenderObject.cpp
    Include/W3DDevice/GameClient/W3DSegmentedLineRenderObject.h
    Include/W3DDevice/GameClient/W3DCastQuery.h
    Include/W3DDevice/GameClient/W3DIntersectionQuery.h
    Include/W3DDevice/GameClient/W3DQueryBounds.h
    Include/W3DDevice/GameClient/W3DSceneQueryMask.h
    Include/XAudio2AudioDevice/XAudio2AudioManager.h
    Include/XAudio2AudioDevice/XAudio2AudioSystem.h
    Include/XAudio2AudioDevice/XAudio2Decoder.h
    Include/XAudio2AudioDevice/XAudio2Mastering.h
    Include/XAudio2AudioDevice/XAudio2PcmStream.h
    Include/XAudio2AudioDevice/XAudio2Voice.h
    Include/XAudio2AudioDevice/NullAudioSystem.h
    Include/XAudio2AudioDevice/core/AudioStream.h
    Include/W3DDevice/Common/W3DConvert.h
    Include/W3DDevice/Common/W3DFunctionLexicon.h
    Include/W3DDevice/Common/W3DModuleFactory.h
    Include/W3DDevice/Common/W3DRadar.h
    Include/W3DDevice/Common/W3DThingFactory.h
    Include/W3DDevice/GameClient/BaseHeightMap.h
#    Include/W3DDevice/GameClient/camerashakesystem.h
    Include/W3DDevice/GameClient/Module/W3DDebrisDraw.h
    Include/W3DDevice/GameClient/Module/W3DDefaultDraw.h
    Include/W3DDevice/GameClient/Module/W3DDependencyModelDraw.h
    Include/W3DDevice/GameClient/Module/W3DLaserDraw.h
    Include/W3DDevice/GameClient/Module/W3DModelDraw.h
    Include/W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h
    Include/W3DDevice/GameClient/Module/W3DOverlordTankDraw.h
    Include/W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h
    Include/W3DDevice/GameClient/Module/W3DPoliceCarDraw.h
    Include/W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h
    Include/W3DDevice/GameClient/Module/W3DPropDraw.h
    Include/W3DDevice/GameClient/Module/W3DRopeDraw.h
    Include/W3DDevice/GameClient/Module/W3DScienceModelDraw.h
    Include/W3DDevice/GameClient/Module/W3DSupplyDraw.h
    Include/W3DDevice/GameClient/Module/W3DTankDraw.h
    Include/W3DDevice/GameClient/Module/W3DTankTruckDraw.h
    Include/W3DDevice/GameClient/Module/W3DTracerDraw.h
    Include/W3DDevice/GameClient/Module/W3DTreeDraw.h
    Include/W3DDevice/GameClient/Module/W3DTruckDraw.h
    Include/W3DDevice/GameClient/TerrainTex.h
    Include/W3DDevice/GameClient/TileData.h
    Include/W3DDevice/GameClient/WaterMaterial.h
#    Include/W3DDevice/GameClient/W3DAssetManager.h
#    Include/W3DDevice/GameClient/W3DAssetManagerExposed.h
#    Include/W3DDevice/GameClient/W3DBibBuffer.h
#    Include/W3DDevice/GameClient/W3DBridgeBuffer.h
#    Include/W3DDevice/GameClient/W3DBufferManager.h
#    Include/W3DDevice/GameClient/W3DCustomScene.h
#    Include/W3DDevice/GameClient/W3DDebugDisplay.h
#    Include/W3DDevice/GameClient/W3DDebugIcons.h
#    Include/W3DDevice/GameClient/W3DDisplay.h
#    Include/W3DDevice/GameClient/W3DDisplayString.h
#    Include/W3DDevice/GameClient/W3DDisplayStringManager.h
#    Include/W3DDevice/GameClient/W3DDynamicLight.h
#    Include/W3DDevice/GameClient/W3DFileSystem.h
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include/W3DDevice/GameClient/W3DGadget.h
#    Include/W3DDevice/GameClient/W3DGameClient.h
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include/W3DDevice/GameClient/W3DGameFont.h
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include/W3DDevice/GameClient/W3DGameWindow.h
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include/W3DDevice/GameClient/W3DGameWindowManager.h
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include/W3DDevice/GameClient/W3DGUICallbacks.h
#    Include/W3DDevice/GameClient/W3DInGameUI.h
    Include/W3DDevice/GameClient/W3DMouse.h
    Include/W3DDevice/GameClient/W3DParticleSys.h
#    Include/W3DDevice/GameClient/W3DPoly.h
#    Include/W3DDevice/GameClient/W3DProjectedShadow.h
    Include/W3DDevice/GameClient/W3DPropBuffer.h
    Include/W3DDevice/GameClient/W3DScorch.h
#    Include/W3DDevice/GameClient/W3DRoadBuffer.h
#    Include/W3DDevice/GameClient/W3DScene.h
    Include/W3DDevice/GameClient/W3DShaderManager.h
#    Include/W3DDevice/GameClient/W3DShadow.h
#    Include/W3DDevice/GameClient/W3DShroud.h
    Include/W3DDevice/GameClient/W3DSmudge.h
    Include/W3DDevice/GameClient/W3DSnow.h
#    Include/W3DDevice/GameClient/W3DStatusCircle.h
    Include/W3DDevice/GameClient/W3DTerrainTracks.h
    Include/W3DDevice/GameClient/W3DTerrainVisual.h
    Include/W3DDevice/GameClient/W3DTreeBuffer.h
    Include/W3DDevice/GameClient/W3DView.h
    Include/W3DDevice/GameClient/W3DScreenshot.h
#    Include/W3DDevice/GameClient/W3DVolumetricShadow.h
    Include/W3DDevice/GameClient/W3DWater.h
    Include/W3DDevice/GameClient/WaterResources.h
    Include/W3DDevice/GameClient/WaterReflectionRenderer.h
    Include/W3DDevice/GameClient/WaterSkyboxSystem.h
    Include/W3DDevice/GameClient/W3DWaterTracks.h
#    Include/W3DDevice/GameClient/W3DWaypointBuffer.h
    Include/W3DDevice/GameClient/WorldHeightMap.h
#    Include/W3DDevice/GameLogic/W3DGameLogic.h
#    Include/W3DDevice/GameLogic/W3DGhostObject.h
#    Include/W3DDevice/GameLogic/W3DTerrainLogic.h
    Include/SDL3Device/GameClient/SDL3Mouse.h
    Include/SDL3Device/GameClient/SDL3Keyboard.h
    Include/SDL3Device/Common/SDL3GameEngine.h
    Include/SDL3Device/Common/SDL3LocalFile.h
    Include/SDL3Device/Common/SDL3LocalFileSystem.h
    Include/SDL3Device/Common/SDL3BIGFile.h
    Include/SDL3Device/Common/SDL3BIGFileSystem.h
    Source/XAudio2AudioDevice/XAudio2AudioManager.cpp
    Source/XAudio2AudioDevice/XAudio2AudioSystem.cpp
    Source/XAudio2AudioDevice/XAudio2Decoder.cpp
    Source/XAudio2AudioDevice/XAudio2Mastering.cpp
    Source/XAudio2AudioDevice/XAudio2Voice.cpp
    Source/XAudio2AudioDevice/XAudio2PcmStream.cpp
    Source/XAudio2AudioDevice/AudioEngine.cpp
    Source/XAudio2AudioDevice/AudioFactory.cpp
    Source/XAudio2AudioDevice/NullAudioSystem.cpp
    Source/W3DDevice/Common/System/W3DFunctionLexicon.cpp
    Source/W3DDevice/Common/System/W3DRadar.cpp
    Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp
    Source/W3DDevice/Common/Thing/W3DThingFactory.cpp
    Source/W3DDevice/Common/W3DConvert.cpp
    Source/W3DDevice/GameClient/BaseHeightMap.cpp
    Source/W3DDevice/GameClient/CameraShakeSystem.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDebrisDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDefaultDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDependencyModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DLaserDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordAircraftDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordTankDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordTruckDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DPoliceCarDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DProjectileStreamDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DPropDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DRopeDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DScienceModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DSupplyDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTankDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTankTruckDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTracerDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTreeDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTruckDraw.cpp
#    Source/W3DDevice/GameClient/Shadow/W3DBufferManager.cpp
#    Source/W3DDevice/GameClient/Shadow/W3DProjectedShadow.cpp
#    Source/W3DDevice/GameClient/Shadow/W3DShadow.cpp
#    Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp
    Source/W3DDevice/GameClient/TerrainTex.cpp
    Source/W3DDevice/GameClient/TileData.cpp
#    Source/W3DDevice/GameClient/W3DAssetManager.cpp
#    Source/W3DDevice/GameClient/W3DAssetManagerExposed.cpp
#    Source/W3DDevice/GameClient/W3DBibBuffer.cpp
#    Source/W3DDevice/GameClient/W3DBridgeBuffer.cpp
#    Source/W3DDevice/GameClient/W3DDebugDisplay.cpp
#    Source/W3DDevice/GameClient/W3DDebugIcons.cpp
#    Source/W3DDevice/GameClient/W3DDisplay.cpp
#    Source/W3DDevice/GameClient/W3DDisplayString.cpp
#    Source/W3DDevice/GameClient/W3DDisplayStringManager.cpp
#    Source/W3DDevice/GameClient/W3DDynamicLight.cpp
#    Source/W3DDevice/GameClient/W3DFileSystem.cpp
#    Source/W3DDevice/GameClient/W3DGameClient.cpp
#    Source/W3DDevice/GameClient/W3DInGameUI.cpp
    Source/W3DDevice/GameClient/W3DMouse.cpp
    Source/W3DDevice/GameClient/W3DParticleSys.cpp
#    Source/W3DDevice/GameClient/W3DPoly.cpp
    Source/W3DDevice/GameClient/W3DPropBuffer.cpp
    Source/W3DDevice/GameClient/W3DScorch.cpp
#    Source/W3DDevice/GameClient/W3DRoadBuffer.cpp
#    Source/W3DDevice/GameClient/W3DScene.cpp
    Source/W3DDevice/GameClient/W3DShaderManager.cpp
#    Source/W3DDevice/GameClient/W3DShroud.cpp
    Source/W3DDevice/GameClient/W3DSmudge.cpp
    Source/W3DDevice/GameClient/W3DSnow.cpp
#    Source/W3DDevice/GameClient/W3DStatusCircle.cpp
    Source/W3DDevice/GameClient/W3DTerrainTracks.cpp
    Source/W3DDevice/GameClient/W3DTerrainVisual.cpp
    Source/W3DDevice/GameClient/W3DTreeBuffer.cpp
    Source/W3DDevice/GameClient/W3DView.cpp
    Source/W3DDevice/GameClient/W3DScreenshot.cpp
#    Source/W3DDevice/GameClient/W3dWaypointBuffer.cpp
    Source/W3DDevice/GameClient/Water/W3DWater.cpp
    Source/W3DDevice/GameClient/Water/WaterInteractionInputs.cpp
    Source/W3DDevice/GameClient/WaterResources.cpp
    Source/W3DDevice/GameClient/WaterSkyboxSystem.cpp
    Source/W3DDevice/GameClient/WaterGridSimulation.cpp
    Source/W3DDevice/GameClient/WaterMaterial.cpp
    Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp
    Source/W3DDevice/GameClient/WorldHeightMap.cpp
#    Source/W3DDevice/GameLogic/W3DGameLogic.cpp
#    Source/W3DDevice/GameLogic/W3DGhostObject.cpp
#    Source/W3DDevice/GameLogic/W3DTerrainLogic.cpp
    Source/SDL3Device/GameClient/SDL3Mouse.cpp
    Source/SDL3Device/GameClient/SDL3Keyboard.cpp
    Source/SDL3Device/Common/SDL3GameEngine.cpp
    Source/SDL3Device/Common/SDL3OSDisplay.cpp
    Source/SDL3Device/Common/SDL3LocalFile.cpp
    Source/SDL3Device/Common/SDL3LocalFileSystem.cpp
    Source/SDL3Device/Common/SDL3BIGFile.cpp
    Source/SDL3Device/Common/SDL3BIGFileSystem.cpp
)

add_library(corei_gameenginedevice_private INTERFACE)
add_library(corei_gameenginedevice_public INTERFACE)

target_sources(corei_gameenginedevice_private INTERFACE ${GAMEENGINEDEVICE_SRC})

target_include_directories(corei_gameenginedevice_public INTERFACE
    Include
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include
)

target_include_directories(corei_gameenginedevice_private INTERFACE
    Include/XAudio2AudioDevice
    Source/XAudio2AudioDevice
    ${PROJECT_SOURCE_DIR}/engine/gui/wnd/runtime/renderer/include
)

target_link_libraries(corei_gameenginedevice_private INTERFACE
    corei_always
    corei_main
    SDL3::SDL3
    stb
)

target_link_libraries(corei_gameenginedevice_public INTERFACE
    corei_gameengine_public
    d3d9lib
    xaudio2
)

target_include_directories(corei_gameenginedevice_private INTERFACE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(corei_gameenginedevice_private INTERFACE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(corei_gameenginedevice_private INTERFACE ${FFMPEG_LIBRARIES})
target_compile_definitions(corei_gameenginedevice_public INTERFACE RTS_HAS_FFMPEG)
