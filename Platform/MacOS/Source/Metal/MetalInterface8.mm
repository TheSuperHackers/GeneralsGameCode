/**
 * MetalInterface8.mm — IDirect3D8 implementation on Apple Metal
 */
#ifdef __APPLE__

#import "MetalInterface8.h"
#import "MetalDevice8.h"
#include <cstdio>
#include <cstring>

MetalInterface8::MetalInterface8() : m_RefCount(1) {
  fprintf(stderr, "[MetalInterface8] Created\n");
}

MetalInterface8::~MetalInterface8() {
  fprintf(stderr, "[MetalInterface8] Destroyed\n");
}

// IUnknown

STDMETHODIMP MetalInterface8::QueryInterface(REFIID riid, void **ppvObj) {
  if (ppvObj)
    *ppvObj = nullptr;
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MetalInterface8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalInterface8::Release() {
  ULONG r = --m_RefCount;
  if (r == 0) {
    delete this;
    return 0;
  }
  return r;
}

// IDirect3D8

STDMETHODIMP MetalInterface8::RegisterSoftwareDevice(void *p) {
  return E_NOTIMPL;
}

STDMETHODIMP_(UINT) MetalInterface8::GetAdapterCount() { return 1; }

STDMETHODIMP
MetalInterface8::GetAdapterIdentifier(UINT Adapter, DWORD Flags,
                                      D3DADAPTER_IDENTIFIER8 *pId) {
  if (!pId)
    return E_POINTER;
  memset(pId, 0, sizeof(*pId));
  strncpy(pId->Description, "Apple Metal GPU", sizeof(pId->Description) - 1);
  pId->VendorId = 0x106B; // Apple
  pId->DeviceId = 0x0001;
  return D3D_OK;
}

STDMETHODIMP_(UINT) MetalInterface8::GetAdapterModeCount(UINT Adapter) {
  return 1;
}

STDMETHODIMP MetalInterface8::EnumAdapterModes(UINT Adapter, UINT Mode,
                                               D3DDISPLAYMODE *pMode) {
  if (!pMode)
    return E_POINTER;
  pMode->Width = 800;
  pMode->Height = 600;
  pMode->RefreshRate = 60;
  pMode->Format = D3DFMT_A8R8G8B8;
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::GetAdapterDisplayMode(UINT Adapter,
                                                    D3DDISPLAYMODE *pMode) {
  if (!pMode)
    return E_POINTER;
  pMode->Width = 800;
  pMode->Height = 600;
  pMode->RefreshRate = 60;
  pMode->Format = D3DFMT_A8R8G8B8;
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::CheckDeviceType(UINT a, DWORD dt, D3DFORMAT df,
                                              D3DFORMAT bbf, BOOL w) {
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::CheckDeviceFormat(UINT a, DWORD dt, D3DFORMAT af,
                                                DWORD u, DWORD rt,
                                                D3DFORMAT cf) {
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::CheckDeviceMultiSampleType(UINT a, DWORD dt,
                                                         D3DFORMAT sf, BOOL w,
                                                         DWORD mst) {
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::CheckDepthStencilMatch(UINT a, DWORD dt,
                                                     D3DFORMAT af,
                                                     D3DFORMAT rtf,
                                                     D3DFORMAT dsf) {
  return D3D_OK;
}

STDMETHODIMP MetalInterface8::GetDeviceCaps(UINT Adapter, DWORD DeviceType,
                                            D3DCAPS8 *pCaps) {
  if (!pCaps)
    return E_POINTER;

  // Fill caps directly — no need to create a temporary MetalDevice8.
  // These are static capabilities and don't depend on device state.
  memset(pCaps, 0, sizeof(*pCaps));
  pCaps->DeviceType = D3DDEVTYPE_HAL;
  pCaps->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
  pCaps->MaxSimultaneousTextures = 8;
  pCaps->MaxTextureBlendStages = 8;
  pCaps->VertexShaderVersion = 0x0101;
  pCaps->PixelShaderVersion = 0x0101;
  pCaps->MaxPrimitiveCount = 0xFFFFFF;
  pCaps->MaxVertexIndex = 0xFFFFFF;
  pCaps->MaxStreams = 8;
  pCaps->MaxActiveLights = 4;
  pCaps->MaxTextureWidth = 4096;
  pCaps->MaxTextureHeight = 4096;
  pCaps->RasterCaps =
      D3DPRASTERCAPS_FOGRANGE | 0x00000100 | 0x00000200 | D3DPRASTERCAPS_ZBIAS;
  pCaps->TextureCaps = 0x00000001 | 0x00000002 | 0x00000004;
  pCaps->TextureOpCaps =
      D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
      D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_ADD |
      D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA;
  pCaps->PrimitiveMiscCaps = D3DPMISCCAPS_COLORWRITEENABLE;
  pCaps->Caps2 = D3DCAPS2_FULLSCREENGAMMA;
  pCaps->SrcBlendCaps = 0x1FFF;
  pCaps->DestBlendCaps = 0x1FFF;
  pCaps->ZCmpCaps = 0xFF;
  pCaps->AlphaCmpCaps = 0xFF;
  pCaps->StencilCaps = 0xFF;
  return D3D_OK;
}

STDMETHODIMP_(HMONITOR) MetalInterface8::GetAdapterMonitor(UINT Adapter) {
  return nullptr;
}

STDMETHODIMP MetalInterface8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,
                                           HWND hFocusWindow,
                                           DWORD BehaviorFlags,
                                           D3DPRESENT_PARAMETERS *pPP,
                                           IDirect3DDevice8 **ppDevice) {
  if (!ppDevice)
    return E_POINTER;

  MetalDevice8 *dev = new MetalDevice8();
  if (!dev->InitMetal(hFocusWindow)) {
    delete dev;
    *ppDevice = nullptr;
    return E_FAIL;
  }

  *ppDevice = dev;
  fprintf(stderr, "[MetalInterface8] CreateDevice: OK (%p)\n", dev);
  return D3D_OK;
}

// ═══════════════════════════════════════════════════════════════
//  extern "C" Factory Functions — called from D3DXStubs.cpp
// ═══════════════════════════════════════════════════════════════

extern "C" IDirect3D8 *CreateMetalInterface8() { return new MetalInterface8(); }

extern "C" IDirect3DDevice8 *CreateMetalDevice8() { return new MetalDevice8(); }

// Wrapper with void* return type — called from windows.h GetProcAddress stub
// which cannot include d3d8.h. This avoids return-type conflicts.
extern "C" void *_CreateMetalInterface8_Wrapper() {
  return static_cast<void *>(CreateMetalInterface8());
}

#endif // __APPLE__
