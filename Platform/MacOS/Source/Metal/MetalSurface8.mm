#import "MetalSurface8.h"
#import "MetalDevice8.h"
#include <cstdlib>
#include <cstring>

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL E_FAIL
#endif

MetalSurface8::MetalSurface8(MetalDevice8 *device, SurfaceKind kind, UINT w,
                             UINT h, D3DFORMAT fmt)
    : m_RefCount(1), m_Device(device), m_Kind(kind), m_Width(w), m_Height(h),
      m_Format(fmt) {
  if (m_Device)
    m_Device->AddRef();
}

MetalSurface8::~MetalSurface8() {
  if (m_LockedData) {
    free(m_LockedData);
    m_LockedData = nullptr;
  }
  if (m_Device) {
    m_Device->Release();
    m_Device = nullptr;
  }
}

// IUnknown
STDMETHODIMP MetalSurface8::QueryInterface(REFIID riid, void **ppvObj) {
  if (!ppvObj)
    return E_POINTER;
  *ppvObj = this;
  AddRef();
  return D3D_OK;
}

STDMETHODIMP_(ULONG) MetalSurface8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalSurface8::Release() {
  if (--m_RefCount == 0) {
    delete this;
    return 0;
  }
  return m_RefCount;
}

// IDirect3DResource8
STDMETHODIMP MetalSurface8::GetDevice(IDirect3DDevice8 **ppDevice) {
  if (!ppDevice)
    return E_POINTER;
  *ppDevice = m_Device;
  if (m_Device)
    m_Device->AddRef();
  return D3D_OK;
}

STDMETHODIMP MetalSurface8::SetPrivateData(REFGUID g, CONST void *d, DWORD s,
                                           DWORD f) {
  return D3D_OK;
}
STDMETHODIMP MetalSurface8::GetPrivateData(REFGUID g, void *d, DWORD *s) {
  return E_NOTIMPL;
}
STDMETHODIMP MetalSurface8::FreePrivateData(REFGUID g) { return D3D_OK; }
STDMETHODIMP_(DWORD) MetalSurface8::SetPriority(DWORD p) { return 0; }
STDMETHODIMP_(DWORD) MetalSurface8::GetPriority() { return 0; }
STDMETHODIMP_(void) MetalSurface8::PreLoad() {}
STDMETHODIMP_(D3DRESOURCETYPE) MetalSurface8::GetType() {
  return D3DRTYPE_SURFACE;
}

// IDirect3DSurface8
STDMETHODIMP MetalSurface8::GetContainer(REFIID riid, void **ppContainer) {
  if (!ppContainer)
    return E_POINTER;
  *ppContainer = nullptr;
  return E_NOTIMPL;
}

STDMETHODIMP MetalSurface8::GetDesc(D3DSURFACE_DESC *pDesc) {
  if (!pDesc)
    return E_POINTER;
  pDesc->Format = m_Format;
  pDesc->Type = D3DRTYPE_SURFACE;
  pDesc->Usage =
      (m_Kind == kDepth) ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET;
  pDesc->Pool = D3DPOOL_DEFAULT;
  pDesc->Size = m_Width * m_Height * 4;
  pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
  pDesc->Width = m_Width;
  pDesc->Height = m_Height;
  return D3D_OK;
}

STDMETHODIMP MetalSurface8::LockRect(D3DLOCKED_RECT *pLockedRect,
                                     CONST RECT *pRect, DWORD Flags) {
  if (!pLockedRect)
    return E_POINTER;
  if (m_LockedData)
    return D3DERR_INVALIDCALL; // already locked

  // Calculate bytes per pixel based on format
  UINT bpp = 4; // default: 32-bit
  switch (m_Format) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8:
    bpp = 4;
    break;
  case D3DFMT_R5G6B5:
  case D3DFMT_A1R5G5B5:
  case D3DFMT_A4R4G4B4:
  case D3DFMT_X1R5G5B5:
    bpp = 2;
    break;
  case D3DFMT_A8:
    bpp = 1;
    break;
  default:
    bpp = 4;
    break;
  }

  UINT pitch = m_Width * bpp;
  UINT dataSize = pitch * m_Height;

  m_LockedData = malloc(dataSize);
  if (!m_LockedData)
    return E_FAIL;

  memset(m_LockedData, 0, dataSize);
  m_LockedPitch = pitch;

  pLockedRect->pBits = m_LockedData;
  pLockedRect->Pitch = pitch;

  return D3D_OK;
}

STDMETHODIMP MetalSurface8::UnlockRect() {
  if (!m_LockedData)
    return D3DERR_INVALIDCALL;

  // In a full implementation we'd upload this data to a Metal texture.
  // For now we just free the staging buffer since the engine only uses
  // surface locking for tiny procedural textures (e.g. 1x1 white pixel).
  free(m_LockedData);
  m_LockedData = nullptr;
  m_LockedPitch = 0;

  return D3D_OK;
}
