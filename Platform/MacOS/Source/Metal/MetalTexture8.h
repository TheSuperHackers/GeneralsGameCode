#pragma once

#include <d3d8.h>  // DX8 SDK header
#include <Metal/Metal.h>

class MetalDevice8;

class MetalTexture8 : public IDirect3DTexture8 {
  W3DMPO_GLUE(MetalTexture8)
public:
  MetalTexture8(MetalDevice8 *device, UINT width, UINT height, UINT levels,
                DWORD usage, D3DFORMAT format, D3DPOOL pool);
  MetalTexture8(MetalDevice8 *device, void *mtlTexture,
                D3DFORMAT format); // For wrapping existing textures
  virtual ~MetalTexture8();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IDirect3DResource8
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice);
  STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void *pData,
                            DWORD SizeOfData, DWORD Flags);
  STDMETHOD(GetPrivateData)(REFGUID refguid, void *pData, DWORD *pSizeOfData);
  STDMETHOD(FreePrivateData)(REFGUID refguid);
  STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
  STDMETHOD_(DWORD, GetPriority)();
  STDMETHOD_(void, PreLoad)();
  STDMETHOD_(D3DRESOURCETYPE, GetType)();

  // IDirect3DBaseTexture8
  STDMETHOD_(DWORD, SetLOD)(DWORD LODNew);
  STDMETHOD_(DWORD, GetLOD)();
  STDMETHOD_(DWORD, GetLevelCount)();

  // IDirect3DTexture8
  STDMETHOD(GetLevelDesc)(UINT Level, D3DSURFACE_DESC *pDesc);
  STDMETHOD(GetSurfaceLevel)(UINT Level, IDirect3DSurface8 **ppSurfaceLevel);
  STDMETHOD(LockRect)(UINT Level, D3DLOCKED_RECT *pLockedRect,
                      CONST RECT *pRect, DWORD Flags);
  STDMETHOD(UnlockRect)(UINT Level);
  STDMETHOD(AddDirtyRect)(CONST RECT *pDirtyRect);

  // Metal Specific
  id<MTLTexture> GetMTLTexture() const {
    return (__bridge id<MTLTexture>)m_Texture;
  }
  void *GetMTLTextureVoid() const { return m_Texture; }

private:
  ULONG m_RefCount;
  MetalDevice8 *m_Device; // Weak ref? Or AddRef? Usually AddRef.

  void *m_Texture; // id<MTLTexture>

  UINT m_Width;
  UINT m_Height;
  UINT m_Levels;
  DWORD m_Usage;
  D3DFORMAT m_Format;
  D3DPOOL m_Pool;

  // Staging for LockRect (assuming single lock for now)
  // We might need a map of locked levels if multiple levels are locked
  // simultaneously. But typically game locks one level.
  struct LockedLevel {
    void *ptr;
    UINT pitch;
    UINT bytesPerPixel;
  };
  std::map<UINT, LockedLevel> m_LockedLevels;
};

// Internal Helper for Format Mapping
MTLPixelFormat MetalFormatFromD3D(D3DFORMAT fmt);
UINT BytesPerPixelFromD3D(D3DFORMAT fmt);
