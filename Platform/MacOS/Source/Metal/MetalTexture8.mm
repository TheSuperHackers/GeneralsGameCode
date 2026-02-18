#include "MetalTexture8.h"
#include "MetalDevice8.h"
#include "MetalSurface8.h"
#include <map>

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL E_FAIL
#endif

// Helper: Get Bytes Per Pixel or Block Size
UINT BytesPerPixelFromD3D(D3DFORMAT fmt) {
  switch (fmt) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8:
    return 4;
  case D3DFMT_R5G6B5:
  case D3DFMT_X1R5G5B5:
  case D3DFMT_A1R5G5B5:
  case D3DFMT_A4R4G4B4:
    return 2;
  case D3DFMT_R8G8B8:
    return 3;
  case D3DFMT_A8:
  case D3DFMT_L8:
  case D3DFMT_P8:
    return 1;
  case D3DFMT_DXT1:
    return 8; // Per 4x4 block (8 bytes)
  case D3DFMT_DXT2:
  case D3DFMT_DXT3:
  case D3DFMT_DXT4:
  case D3DFMT_DXT5:
    return 16; // Per 4x4 block (16 bytes)
  default:
    return 4; // Fallback
  }
}

// Helper: Metal Pixel Format
MTLPixelFormat MetalFormatFromD3D(D3DFORMAT fmt) {
  switch (fmt) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8: // Ignore alpha channel in shader if needed
    return MTLPixelFormatBGRA8Unorm;

  // macOS Metal supports BC compression
  case D3DFMT_DXT1:
    return MTLPixelFormatBC1_RGBA;
  case D3DFMT_DXT3: // DXT2 is premultiplied alpha DXT3, usually same storage
    return MTLPixelFormatBC2_RGBA;
  case D3DFMT_DXT5: // DXT4 is premultiplied alpha DXT5
    return MTLPixelFormatBC3_RGBA;

    // 16-bit formats often need conversion on macOS Desktop (no B5G6R5 native
    // support typically) Converting to BGRA8 on CPU is safer, but let's assume
    // keys are handled or return Invalid for now. Actually, we can return
    // BGRA8Unorm and rely on LockRect/UnlockRect to convert if we implement
    // conversion logic. For basic support, let's map to BGRA8Unorm and warn if
    // mismatch. But D3D will try to write 16-bit data. We'll stick to 32-bit
    // and Compressed for Stage 3.

  default:
    return MTLPixelFormatBGRA8Unorm;
  }
}

MetalTexture8::MetalTexture8(MetalDevice8 *device, UINT width, UINT height,
                             UINT levels, DWORD usage, D3DFORMAT format,
                             D3DPOOL pool)
    : m_RefCount(1), m_Device(device), m_Width(width), m_Height(height),
      m_Levels(levels), m_Usage(usage), m_Format(format), m_Pool(pool) {

  if (m_Device)
    m_Device->AddRef();

  if (m_Levels == 0)
    m_Levels = 1; // Actually 0 means "all levels", typically mipmapping. Metal
                  // needs specific count.
  // Typically we'd calculate log2(max(w, h)) + 1. But for now clamp to 1 if 0.
  // IDirect3DTexture8::GetLevelCount returns actual count.

  MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
  desc.pixelFormat = MetalFormatFromD3D(format);
  desc.width = width;
  desc.height = height;
  desc.mipmapLevelCount = m_Levels;
  desc.usage = MTLTextureUsageShaderRead;
  if (usage & D3DUSAGE_RENDERTARGET) {
    desc.usage |= MTLTextureUsageRenderTarget;
  }

  id<MTLDevice> mtlDev = (__bridge id<MTLDevice>)m_Device->GetMTLDevice();
  id<MTLTexture> tex = [mtlDev newTextureWithDescriptor:desc];
  m_Texture = (__bridge_retained void *)tex; // Retain manual ref
}

MetalTexture8::MetalTexture8(MetalDevice8 *device, void *mtlTexture,
                             D3DFORMAT format)
    : m_RefCount(1), m_Device(device), m_Width(0), m_Height(0), m_Levels(1),
      m_Usage(0), m_Format(format), m_Pool(D3DPOOL_DEFAULT) {

  if (m_Device)
    m_Device->AddRef();

  id<MTLTexture> tex = (__bridge id<MTLTexture>)mtlTexture;
  if (tex) {
    // We want to RETAIN it because we will Release it in destructor.
    // __bridge_retained transfers +1.
    // If tex is __bridge (no transfer), we need a retain.
    // If we cast `void*` -> `id`, it's __bridge.
    // `tex` is just a pointer usage.
    // We want to store it as `m_Texture` (void*) with +1 ref count.
    m_Texture = (__bridge_retained void *)tex;

    m_Width = (UINT)tex.width;
    m_Height = (UINT)tex.height;
    m_Levels = (UINT)tex.mipmapLevelCount;
  } else {
    m_Texture = nullptr;
  }
}

MetalTexture8::~MetalTexture8() {
  if (m_Texture) {
    CFRelease(m_Texture); // Specific matching retain/release
    m_Texture = nullptr;
  }
  if (m_Device)
    m_Device->Release();
}

STDMETHODIMP MetalTexture8::QueryInterface(REFIID riid, void **ppvObj) {
  if (!ppvObj)
    return E_POINTER;
  *ppvObj = nullptr;
  // Basic IUnknown check (omitting UUID check for brevity/uuid lib missing)
  *ppvObj = this;
  AddRef();
  return D3D_OK;
}

STDMETHODIMP_(ULONG) MetalTexture8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalTexture8::Release() {
  if (--m_RefCount == 0) {
    delete this;
    return 0;
  }
  return m_RefCount;
}

// IDirect3DResource8
STDMETHODIMP MetalTexture8::GetDevice(IDirect3DDevice8 **ppDevice) {
  if (ppDevice) {
    *ppDevice = m_Device;
    m_Device->AddRef();
    return D3D_OK;
  }
  return D3DERR_INVALIDCALL;
}

STDMETHODIMP MetalTexture8::SetPrivateData(REFGUID refguid, CONST void *pData,
                                           DWORD SizeOfData, DWORD Flags) {
  return D3D_OK;
}
STDMETHODIMP MetalTexture8::GetPrivateData(REFGUID refguid, void *pData,
                                           DWORD *pSizeOfData) {
  return D3DERR_NOTFOUND;
}
STDMETHODIMP MetalTexture8::FreePrivateData(REFGUID refguid) { return D3D_OK; }
STDMETHODIMP_(DWORD) MetalTexture8::SetPriority(DWORD PriorityNew) { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetPriority() { return 0; }
STDMETHODIMP_(void) MetalTexture8::PreLoad() {}
STDMETHODIMP_(D3DRESOURCETYPE) MetalTexture8::GetType() {
  return D3DRTYPE_TEXTURE;
}

// IDirect3DBaseTexture8
STDMETHODIMP_(DWORD) MetalTexture8::SetLOD(DWORD LODNew) { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetLOD() { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetLevelCount() { return m_Levels; }

// IDirect3DTexture8
STDMETHODIMP MetalTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
  if (!pDesc)
    return D3DERR_INVALIDCALL;
  if (Level >= m_Levels)
    return D3DERR_INVALIDCALL;

  pDesc->Format = m_Format;
  pDesc->Type = D3DRTYPE_SURFACE;
  pDesc->Usage = m_Usage;
  pDesc->Pool = m_Pool;
  pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
  pDesc->Width = std::max(1u, m_Width >> Level);
  pDesc->Height = std::max(1u, m_Height >> Level);
  pDesc->Size = 0; // Not used often
  return D3D_OK;
}

STDMETHODIMP
MetalTexture8::GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel) {
  if (!ppSurfaceLevel)
    return E_POINTER;
  if (Level >= m_Levels) {
    *ppSurfaceLevel = nullptr;
    return D3DERR_INVALIDCALL;
  }

  UINT w = std::max(1u, m_Width >> Level);
  UINT h = std::max(1u, m_Height >> Level);

  // Create a lightweight surface wrapper for this mip level.
  // MetalSurface8 ctor sets refcount to 1, which the caller will Release.
  auto *surface =
      new MetalSurface8(m_Device, MetalSurface8::kColor, w, h, m_Format);
  *ppSurfaceLevel = surface;
  return D3D_OK;
}

STDMETHODIMP MetalTexture8::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect,
                                     CONST RECT *pRect, DWORD Flags) {
  if (Level >= m_Levels || !pLockedRect)
    return D3DERR_INVALIDCALL;

  // Check if checks already locked
  if (m_LockedLevels.count(Level))
    return D3DERR_INVALIDCALL; // Already locked

  // Allocate staging memory
  UINT width = std::max(1u, m_Width >> Level);
  UINT height = std::max(1u, m_Height >> Level);
  UINT bpp = BytesPerPixelFromD3D(m_Format);

  UINT pitch = 0;
  UINT dataSize = 0;

  bool isCompressed = (m_Format == D3DFMT_DXT1 || m_Format == D3DFMT_DXT2 ||
                       m_Format == D3DFMT_DXT3 || m_Format == D3DFMT_DXT4 ||
                       m_Format == D3DFMT_DXT5);

  if (isCompressed) {
    // Blocks are 4x4
    UINT blocksWide = std::max(1u, (width + 3) / 4);
    UINT blocksHigh = std::max(1u, (height + 3) / 4);
    pitch = blocksWide * bpp; // bpp is bytes per block (8 or 16)
    dataSize = pitch * blocksHigh;
  } else {
    pitch = width * bpp;
    dataSize = pitch * height;
  }

  void *data = malloc(dataSize);
  if (!data)
    return D3DERR_OUTOFVIDEOMEMORY;

  pLockedRect->pBits = data;
  pLockedRect->Pitch = pitch;

  LockedLevel lvl;
  lvl.ptr = data;
  lvl.pitch = pitch;
  lvl.bytesPerPixel = bpp;

  m_LockedLevels[Level] = lvl;

  return D3D_OK;
}

STDMETHODIMP MetalTexture8::UnlockRect(UINT Level) {
  auto it = m_LockedLevels.find(Level);
  if (it == m_LockedLevels.end())
    return D3DERR_INVALIDCALL;

  LockedLevel &lvl = it->second;

  // Upload to Metal Texture
  id<MTLTexture> tex = (__bridge id<MTLTexture>)m_Texture;

  UINT width = std::max(1u, m_Width >> Level);
  UINT height = std::max(1u, m_Height >> Level);

  MTLRegion region = MTLRegionMake2D(0, 0, width, height);

  [tex replaceRegion:region
         mipmapLevel:Level
           withBytes:lvl.ptr
         bytesPerRow:lvl.pitch];

  free(lvl.ptr);
  m_LockedLevels.erase(it);

  return D3D_OK;
}

STDMETHODIMP MetalTexture8::AddDirtyRect(CONST RECT *pDirtyRect) {
  return D3D_OK;
}
