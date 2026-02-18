#pragma once

#include <d3d8.h>  // DX8 SDK header

class MetalDevice8;

// Minimal IDirect3DSurface8 implementation for render-target / depth-buffer
// token passing.  The engine stores default RT and depth surfaces to hand
// back to SetRenderTarget; it never actually reads pixel data from them.
class MetalSurface8 : public IDirect3DSurface8 {
  W3DMPO_GLUE(MetalSurface8)
public:
  enum SurfaceKind { kColor, kDepth };

  MetalSurface8(MetalDevice8 *device, SurfaceKind kind, UINT w, UINT h,
                D3DFORMAT fmt);
  virtual ~MetalSurface8();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IDirect3DResource8
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice);
  STDMETHOD(SetPrivateData)(REFGUID g, CONST void *d, DWORD s, DWORD f);
  STDMETHOD(GetPrivateData)(REFGUID g, void *d, DWORD *s);
  STDMETHOD(FreePrivateData)(REFGUID g);
  STDMETHOD_(DWORD, SetPriority)(DWORD p);
  STDMETHOD_(DWORD, GetPriority)();
  STDMETHOD_(void, PreLoad)();
  STDMETHOD_(D3DRESOURCETYPE, GetType)();

  // IDirect3DSurface8
  STDMETHOD(GetContainer)(REFIID riid, void **ppContainer);
  STDMETHOD(GetDesc)(D3DSURFACE_DESC *pDesc);
  STDMETHOD(LockRect)(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect,
                      DWORD Flags);
  STDMETHOD(UnlockRect)();

  SurfaceKind GetKind() const { return m_Kind; }

private:
  ULONG m_RefCount;
  MetalDevice8 *m_Device;
  SurfaceKind m_Kind;
  UINT m_Width;
  UINT m_Height;
  D3DFORMAT m_Format;
  void *m_LockedData = nullptr;
  UINT m_LockedPitch = 0;
};
