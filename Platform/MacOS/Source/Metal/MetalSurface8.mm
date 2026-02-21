#import "MetalSurface8.h"
#import "MetalDevice8.h"
#import "MetalTexture8.h"
#include <cstdlib>
#include <cstring>
#import <Metal/Metal.h>

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL E_FAIL
#endif

MetalSurface8::MetalSurface8(MetalDevice8 *device, MetalSurface8::SurfaceKind kind, UINT w,
                             UINT h, D3DFORMAT fmt,
                             MetalTexture8 *parentTexture, UINT mipLevel)
    : m_RefCount(1), m_Device(device), m_Kind(kind), m_Width(w), m_Height(h),
      m_Format(fmt), m_ParentTexture(parentTexture), m_MipLevel(mipLevel) {
  if (m_Device)
    m_Device->AddRef();
  if (m_ParentTexture)
    m_ParentTexture->AddRef();
}

MetalSurface8::~MetalSurface8() {
  if (m_LockedData) {
    free(m_LockedData);
    m_LockedData = nullptr;
  }
  if (m_ParentTexture) {
    m_ParentTexture->Release();
    m_ParentTexture = nullptr;
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
  bool isCompressed = false;
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
  case D3DFMT_DXT1:
  case D3DFMT_DXT2:
  case D3DFMT_DXT3:
  case D3DFMT_DXT4:
  case D3DFMT_DXT5:
    isCompressed = true;
    bpp = (m_Format == D3DFMT_DXT1) ? 8 : 16;
    break;
  default:
    bpp = 4;
    break;
  }

  UINT pitch = 0;
  UINT dataSize = 0;

  if (isCompressed) {
    UINT blocksWide = std::max(1u, (m_Width + 3) / 4);
    UINT blocksHigh = std::max(1u, (m_Height + 3) / 4);
    pitch = blocksWide * bpp; // bpp holds bytesPerBlock
    dataSize = pitch * blocksHigh;
  } else {
    pitch = m_Width * bpp;
    dataSize = pitch * m_Height;
  }

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

  // If this surface came from GetSurfaceLevel, upload data to the parent texture
  if (m_ParentTexture && m_ParentTexture->GetMetalTexture()) {
    id<MTLTexture> tex = (__bridge id<MTLTexture>)m_ParentTexture->GetMetalTexture();
    
    // Calculate bytes per pixel matching the surface format
    UINT bpp = 4;
    switch (m_Format) {
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_R5G6B5:
    case D3DFMT_A4R4G4B4:
      bpp = 2;
      break;
    case D3DFMT_A8:
      bpp = 1;
      break;
    default:
      bpp = 4;
      break;
    }

    // The Metal texture may have a different pixel format than the D3D surface.
    // If so, we need to convert. Common case: D3D A1R5G5B5 (16-bit) → Metal BGRA8 (32-bit)
    MTLPixelFormat mtlFmt = tex.pixelFormat;
    UINT mtlBpp = 4; // Metal side bytes per pixel
    if (mtlFmt == MTLPixelFormatBGRA8Unorm || mtlFmt == MTLPixelFormatRGBA8Unorm) {
      mtlBpp = 4;
    } else if (mtlFmt == MTLPixelFormatR8Unorm) {
      mtlBpp = 1;
    }

    if (bpp == 2 && mtlBpp == 4 && m_Format == D3DFMT_A1R5G5B5) {
      // Convert A1R5G5B5 → BGRA8
      UINT pixelCount = m_Width * m_Height;
      uint8_t *converted = (uint8_t *)malloc(pixelCount * 4);
      uint16_t *src = (uint16_t *)m_LockedData;
      for (UINT i = 0; i < pixelCount; i++) {
        uint16_t px = src[i];
        uint8_t a = (px & 0x8000) ? 255 : 0;
        uint8_t r = ((px >> 10) & 0x1F) * 255 / 31;
        uint8_t g = ((px >> 5) & 0x1F) * 255 / 31;
        uint8_t b = (px & 0x1F) * 255 / 31;
        converted[i * 4 + 0] = b; // B
        converted[i * 4 + 1] = g; // G
        converted[i * 4 + 2] = r; // R
        converted[i * 4 + 3] = a; // A
      }
      MTLRegion region = MTLRegionMake2D(0, 0, m_Width, m_Height);
      [tex replaceRegion:region
             mipmapLevel:m_MipLevel
                   slice:0
               withBytes:converted
             bytesPerRow:m_Width * 4
           bytesPerImage:m_Width * m_Height * 4];
      free(converted);
    } else {
      bool isCompressed = (mtlFmt == MTLPixelFormatBC1_RGBA ||
                           mtlFmt == MTLPixelFormatBC2_RGBA ||
                           mtlFmt == MTLPixelFormatBC3_RGBA);

      if (isCompressed) {
        // DXT1 is 8 bytes per block, DXT3/5 are 16 bytes per block.
        // The D3D format determines bytes per pixel value used for block bpp in LockRect.
        // Wait, MetalFormatFromD3D gave us BC format. Let's infer block size.
        UINT bytesPerBlock = (mtlFmt == MTLPixelFormatBC1_RGBA) ? 8 : 16;
        UINT blocksWide = std::max(1u, (m_Width + 3) / 4);
        UINT blocksHigh = std::max(1u, (m_Height + 3) / 4);
        UINT bytesPerRow = blocksWide * bytesPerBlock;
        UINT bytesPerImage = bytesPerRow * blocksHigh;

        MTLRegion region = MTLRegionMake2D(0, 0, m_Width, m_Height);
        [tex replaceRegion:region
               mipmapLevel:m_MipLevel
                     slice:0
                 withBytes:m_LockedData
               bytesPerRow:bytesPerRow
             bytesPerImage:bytesPerImage];
      } else {
        // Direct upload (formats match or close enough, uncompressed)
        UINT uploadBpr = m_Width * mtlBpp;
        MTLRegion region = MTLRegionMake2D(0, 0, m_Width, m_Height);
        [tex replaceRegion:region
               mipmapLevel:m_MipLevel
                     slice:0
                 withBytes:m_LockedData
               bytesPerRow:uploadBpr
             bytesPerImage:uploadBpr * m_Height];
      }
    }

    // NOTE: Do NOT free m_LockedData here!
    // DirectX 8 pattern: callers (W3DShroud, TerrainTex) store pBits from
    // LockRect and continue writing to it after UnlockRect. The buffer
    // must stay alive until the surface is destroyed (handled in ~MetalSurface8).
  }

  return D3D_OK;
}
