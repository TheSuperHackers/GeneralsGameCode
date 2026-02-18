#define CPP_11(x) x
// Precompiled header must come first
#include "PreRTS.h"

// Remaining stubs
#include "Common/GameLOD.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include <stdio.h>

// Stubs for W3DShaderManager to bypass DirectX dependencies and provide
// "high-end" hardware profile for macOS.

TextureClass *W3DShaderManager::m_Textures[8] = {nullptr};
ChipsetType W3DShaderManager::m_currentChipset = DC_GEFORCE4;
GraphicsVenderID W3DShaderManager::m_currentVendor =
    (GraphicsVenderID)0x10DE; // NVIDIA
__int64 W3DShaderManager::m_driverVersion = 0;
W3DShaderManager::ShaderTypes W3DShaderManager::m_currentShader =
    W3DShaderManager::ST_INVALID;
Int W3DShaderManager::m_currentShaderPass = 0;
Bool W3DShaderManager::m_renderingToTexture = false;
IDirect3DSurface8 *W3DShaderManager::m_oldRenderSurface = nullptr;
IDirect3DTexture8 *W3DShaderManager::m_renderTexture = nullptr;
IDirect3DSurface8 *W3DShaderManager::m_newRenderSurface = nullptr;
IDirect3DSurface8 *W3DShaderManager::m_oldDepthSurface = nullptr;

W3DShaderManager::W3DShaderManager(void) {}

void W3DShaderManager::init(void) {
  printf("W3DShaderManager::init stub called\n");
}

void W3DShaderManager::shutdown(void) {
  printf("W3DShaderManager::shutdown stub called\n");
}

void W3DShaderManager::updateCloud() {}

ChipsetType W3DShaderManager::getChipset(void) { return DC_GEFORCE4; }

Int W3DShaderManager::getShaderPasses(ShaderTypes shader) { return 1; }

Int W3DShaderManager::setShader(ShaderTypes shader, Int pass) {
  m_currentShader = shader;
  m_currentShaderPass = pass;
  return TRUE;
}

Int W3DShaderManager::setShroudTex(Int stage) { return TRUE; }

void W3DShaderManager::resetShader(ShaderTypes shader) {}

HRESULT W3DShaderManager::LoadAndCreateD3DShader(const char *strFilePath,
                                                 const DWORD *pDeclaration,
                                                 DWORD Usage, Bool ShaderType,
                                                 DWORD *pHandle) {
  return S_OK;
}

Bool W3DShaderManager::testMinimumRequirements(ChipsetType *videoChipType,
                                               CpuType *cpuType, Int *cpuFreq,
                                               MemValueType *numRAM,
                                               Real *intBenchIndex,
                                               Real *floatBenchIndex,
                                               Real *memBenchIndex) {
  if (videoChipType)
    *videoChipType = DC_GEFORCE4;
  if (cpuType)
    *cpuType = P4;
  if (cpuFreq)
    *cpuFreq = 3000;
  if (numRAM)
    *numRAM = (MemValueType)2048 * 1024 * 1024; // 2 GB
  if (intBenchIndex)
    *intBenchIndex = 10.0f;
  if (floatBenchIndex)
    *floatBenchIndex = 10.0f;
  if (memBenchIndex)
    *memBenchIndex = 10.0f;
  return TRUE;
}

StaticGameLODLevel W3DShaderManager::getGPUPerformanceIndex(void) {
  return STATIC_GAME_LOD_VERY_HIGH;
}

Real W3DShaderManager::GetCPUBenchTime(void) {
  return 0.1f; // Fast
}

Bool W3DShaderManager::filterPreRender(FilterTypes filter, Bool &skipRender,
                                       CustomScenePassModes &scenePassMode) {
  skipRender = false;
  return false;
}

Bool W3DShaderManager::filterPostRender(FilterTypes filter, FilterModes mode,
                                        Coord2D &scrollDelta,
                                        Bool &doExtraRender) {
  return false;
}

Bool W3DShaderManager::filterSetup(FilterTypes filter, FilterModes mode) {
  return false;
}

void W3DShaderManager::startRenderToTexture(void) {}
IDirect3DTexture8 *W3DShaderManager::endRenderToTexture(void) {
  return nullptr;
}
IDirect3DTexture8 *W3DShaderManager::getRenderTexture(void) { return nullptr; }
void W3DShaderManager::drawViewport(Int color) {}

Bool testMinimumRequirements(ChipsetType *videoChipType, CpuType *cpuType,
                             Int *cpuFreq, MemValueType *numRAM,
                             Real *intBenchIndex, Real *floatBenchIndex,
                             Real *memBenchIndex) {
  return W3DShaderManager::testMinimumRequirements(
      videoChipType, cpuType, cpuFreq, numRAM, intBenchIndex, floatBenchIndex,
      memBenchIndex);
}

// Filter class stubs
Int ScreenBWFilter::init() { return TRUE; }
Int ScreenBWFilter::shutdown() { return TRUE; }
Bool ScreenBWFilter::preRender(Bool &skipRender,
                               CustomScenePassModes &scenePassMode) {
  return false;
}
Bool ScreenBWFilter::postRender(FilterModes mode, Coord2D &scrollDelta,
                                Bool &doExtraRender) {
  return false;
}
Int ScreenBWFilter::set(FilterModes mode) { return TRUE; }
void ScreenBWFilter::reset() {}
Int ScreenBWFilter::m_fadeFrames = 0;
Int ScreenBWFilter::m_fadeDirection = 0;
Int ScreenBWFilter::m_curFadeFrame = 0;
Real ScreenBWFilter::m_curFadeValue = 0;

Int ScreenBWFilterDOT3::init() { return TRUE; }
Int ScreenBWFilterDOT3::shutdown() { return TRUE; }
Bool ScreenBWFilterDOT3::preRender(Bool &skipRender,
                                   CustomScenePassModes &scenePassMode) {
  return false;
}
Bool ScreenBWFilterDOT3::postRender(FilterModes mode, Coord2D &scrollDelta,
                                    Bool &doExtraRender) {
  return false;
}
Int ScreenBWFilterDOT3::set(FilterModes mode) { return TRUE; }
void ScreenBWFilterDOT3::reset() {}

Int ScreenMotionBlurFilter::init() { return TRUE; }
Int ScreenMotionBlurFilter::shutdown() { return TRUE; }
Bool ScreenMotionBlurFilter::preRender(Bool &skipRender,
                                       CustomScenePassModes &scenePassMode) {
  return false;
}
Bool ScreenMotionBlurFilter::postRender(FilterModes mode, Coord2D &scrollDelta,
                                        Bool &doExtraRender) {
  return false;
}
Int ScreenMotionBlurFilter::set(FilterModes mode) { return TRUE; }
void ScreenMotionBlurFilter::reset() {}
Bool ScreenMotionBlurFilter::setup(FilterModes mode) { return false; }
ScreenMotionBlurFilter::ScreenMotionBlurFilter() {}
Coord3D ScreenMotionBlurFilter::m_zoomToPos = {0, 0, 0};
Bool ScreenMotionBlurFilter::m_zoomToValid = false;

Int ScreenCrossFadeFilter::init() { return TRUE; }
Int ScreenCrossFadeFilter::shutdown() { return TRUE; }
Bool ScreenCrossFadeFilter::preRender(Bool &skipRender,
                                      CustomScenePassModes &scenePassMode) {
  return false;
}
Bool ScreenCrossFadeFilter::postRender(FilterModes mode, Coord2D &scrollDelta,
                                       Bool &doExtraRender) {
  return false;
}
Int ScreenCrossFadeFilter::set(FilterModes mode) { return TRUE; }
void ScreenCrossFadeFilter::reset() {}
Bool ScreenCrossFadeFilter::updateFadeLevel() { return false; }
Int ScreenCrossFadeFilter::m_fadeFrames = 0;
Int ScreenCrossFadeFilter::m_fadeDirection = 0;
Int ScreenCrossFadeFilter::m_curFadeFrame = 0;
Real ScreenCrossFadeFilter::m_curFadeValue = 0;
Bool ScreenCrossFadeFilter::m_skipRender = false;
TextureClass *ScreenCrossFadeFilter::m_fadePatternTexture = nullptr;
