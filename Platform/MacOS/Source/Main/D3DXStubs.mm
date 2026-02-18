/**
 * D3DXStubs.mm — D3DX8 helper function implementations for macOS
 *
 * All texture loading goes through MetalDevice8 (DX8-compatible adapter).
 * The old MacOSRenderDevice pipeline is no longer used.
 */
#include "d3d8.h"
#include "d3dx8.h"
#include <windows.h>  // macOS Win32 type shim
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#import <Metal/Metal.h>

#include "MetalDevice8.h"
#include "MetalTexture8.h"

// Forward declarations
extern "C" IDirect3D8 *CreateMetalInterface8();
extern "C" IDirect3DDevice8 *CreateMetalDevice8();

// File system for loading from .big archives
#include "Common/File.h"
#include "Common/FileSystem.h"

// ═══════════════════════════════════════════════════════════════
//  TGA/DDS loading helpers (moved from MacOSRenderer.mm)
// ═══════════════════════════════════════════════════════════════

#pragma pack(push, 1)
struct TGAHeader {
  uint8_t IDLength;
  uint8_t ColorMapType;
  uint8_t ImageType;
  uint16_t CMapStart;
  uint16_t CMapLength;
  uint8_t CMapDepth;
  uint16_t XOffset;
  uint16_t YOffset;
  uint16_t Width;
  uint16_t Height;
  uint8_t PixelDepth;
  uint8_t ImageDescriptor;
};

struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};

struct DDS_HEADER {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwHeight;
  uint32_t dwWidth;
  uint32_t dwPitchOrLinearSize;
  uint32_t dwDepth;
  uint32_t dwMipMapCount;
  uint32_t dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t dwCaps;
  uint32_t dwCaps2;
  uint32_t dwCaps3;
  uint32_t dwCaps4;
  uint32_t dwReserved2;
};
#pragma pack(pop)

static void DecompressDXT1(int w, int h, const uint8_t *src, uint8_t *dst) {
  int bw = (w + 3) / 4, bh = (h + 3) / 4;
  for (int by = 0; by < bh; by++) {
    for (int bx = 0; bx < bw; bx++) {
      uint16_t c0 = src[0] | (src[1] << 8);
      uint16_t c1 = src[2] | (src[3] << 8);
      uint32_t bits = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
      src += 8;
      uint8_t colors[4][4];
      colors[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
      colors[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
      colors[0][2] = (c0 & 0x1F) * 255 / 31;
      colors[0][3] = 255;
      colors[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
      colors[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
      colors[1][2] = (c1 & 0x1F) * 255 / 31;
      colors[1][3] = 255;
      if (c0 > c1) {
        for (int i = 0; i < 3; i++) {
          colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
          colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
        }
        colors[2][3] = colors[3][3] = 255;
      } else {
        for (int i = 0; i < 3; i++)
          colors[2][i] = (colors[0][i] + colors[1][i]) / 2;
        colors[2][3] = 255;
        colors[3][0] = colors[3][1] = colors[3][2] = 0;
        colors[3][3] = 0;
      }
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          int x = bx * 4 + px, y = by * 4 + py;
          if (x >= w || y >= h) {
            bits >>= 2;
            continue;
          }
          int idx = bits & 3;
          bits >>= 2;
          int off = (y * w + x) * 4;
          // Output BGRA (for MTLPixelFormatBGRA8Unorm)
          dst[off + 0] = colors[idx][2]; // B
          dst[off + 1] = colors[idx][1]; // G
          dst[off + 2] = colors[idx][0]; // R
          dst[off + 3] = colors[idx][3]; // A
        }
      }
    }
  }
}

static void DecompressDXT5(int w, int h, const uint8_t *src, uint8_t *dst) {
  int bw = (w + 3) / 4, bh = (h + 3) / 4;
  for (int by = 0; by < bh; by++) {
    for (int bx = 0; bx < bw; bx++) {
      uint8_t a0 = src[0], a1 = src[1];
      uint64_t abits = 0;
      for (int i = 0; i < 6; i++)
        abits |= (uint64_t)src[2 + i] << (8 * i);
      src += 8;
      uint8_t alphas[8];
      alphas[0] = a0;
      alphas[1] = a1;
      if (a0 > a1) {
        for (int i = 1; i <= 6; i++)
          alphas[1 + i] = ((7 - i) * a0 + i * a1) / 7;
      } else {
        for (int i = 1; i <= 4; i++)
          alphas[1 + i] = ((5 - i) * a0 + i * a1) / 5;
        alphas[6] = 0;
        alphas[7] = 255;
      }
      uint16_t c0 = src[0] | (src[1] << 8);
      uint16_t c1 = src[2] | (src[3] << 8);
      uint32_t bits = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
      src += 8;
      uint8_t colors[4][3];
      colors[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
      colors[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
      colors[0][2] = (c0 & 0x1F) * 255 / 31;
      colors[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
      colors[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
      colors[1][2] = (c1 & 0x1F) * 255 / 31;
      for (int i = 0; i < 3; i++) {
        colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
        colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
      }
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          int x = bx * 4 + px, y = by * 4 + py;
          if (x >= w || y >= h) {
            bits >>= 2;
            abits >>= 3;
            continue;
          }
          int ci = bits & 3;
          bits >>= 2;
          int ai = abits & 7;
          abits >>= 3;
          int off = (y * w + x) * 4;
          dst[off + 0] = colors[ci][2]; // B
          dst[off + 1] = colors[ci][1]; // G
          dst[off + 2] = colors[ci][0]; // R
          dst[off + 3] = alphas[ai];    // A
        }
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  File loading — reads from .big archives via TheFileSystem
// ═══════════════════════════════════════════════════════════════
static bool LoadFileData(const char *filename,
                         std::vector<unsigned char> &data) {
  const char *prefixes[] = {"",
                            "Art/",
                            "Art/Textures/",
                            "Data/",
                            "Data/English/Art/Textures/",
                            "Data/Window/",
                            "Window/",
                            "assets/"};
  for (int i = 0; i < 8; i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s%s", prefixes[i], filename);
    for (char *c = path; *c; c++)
      if (*c == '\\')
        *c = '/';
    if (TheFileSystem) {
      File *gf = TheFileSystem->openFile(path);
      if (gf) {
        size_t s = gf->size();
        data.resize(s);
        gf->read(data.data(), s);
        gf->close();
        return true;
      }
    }
  }
  return false;
}

// ═══════════════════════════════════════════════════════════════
//  Texture cache (avoids reloading same file)
// ═══════════════════════════════════════════════════════════════
static std::map<std::string, IDirect3DTexture8 *> s_TextureCache;

// ═══════════════════════════════════════════════════════════════
//  D3DX Functions
// ═══════════════════════════════════════════════════════════════

extern "C" {

HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DTexture8 **ppTexture) {
  if (!ppTexture || !pDevice)
    return E_POINTER;
  return pDevice->CreateTexture(Width, Height, MipLevels, Usage, Format, Pool,
                                ppTexture);
}

HRESULT WINAPI D3DXCreateTextureFromFileExA(
    IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width, UINT Height,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter,
    DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo, const void *pPalette,
    IDirect3DTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  *ppTexture = nullptr;

  if (!pSrcFile || !pSrcFile[0])
    return E_FAIL;

  // Check cache first
  auto it = s_TextureCache.find(pSrcFile);
  if (it != s_TextureCache.end()) {
    it->second->AddRef();
    *ppTexture = it->second;
    return D3D_OK;
  }

  // Load file data from .big archives or filesystem
  std::vector<unsigned char> fileData;
  if (!LoadFileData(pSrcFile, fileData)) {
    return E_FAIL;
  }

  // Detect format and load
  // DDS?
  if (fileData.size() >= 4 && fileData[0] == 'D' && fileData[1] == 'D' &&
      fileData[2] == 'S' && fileData[3] == ' ') {
    DDS_HEADER *dh = (DDS_HEADER *)(fileData.data() + 4);
    int dw = dh->dwWidth, dh_h = dh->dwHeight;
    uint32_t fourcc = dh->ddspf.dwFourCC;
    const uint32_t FOURCC_DXT1 = 0x31545844;
    const uint32_t FOURCC_DXT3 = 0x33545844;
    const uint32_t FOURCC_DXT5 = 0x35545844;

    if (fourcc == FOURCC_DXT1 || fourcc == FOURCC_DXT3 ||
        fourcc == FOURCC_DXT5) {
      // Decompress to BGRA8 and upload
      std::vector<uint8_t> rgba(dw * dh_h * 4);
      const uint8_t *src = (const uint8_t *)(dh + 1);
      if (fourcc == FOURCC_DXT1)
        DecompressDXT1(dw, dh_h, src, rgba.data());
      else if (fourcc == FOURCC_DXT5)
        DecompressDXT5(dw, dh_h, src, rgba.data());
      else {
        // DXT3 — use DXT5 path as approximation for now
        DecompressDXT5(dw, dh_h, src, rgba.data());
      }

      IDirect3DTexture8 *tex = nullptr;
      HRESULT hr = pDevice->CreateTexture(dw, dh_h, 1, 0, D3DFMT_A8R8G8B8,
                                          D3DPOOL_MANAGED, &tex);
      if (FAILED(hr) || !tex)
        return E_FAIL;

      D3DLOCKED_RECT lr;
      if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
        for (int y = 0; y < dh_h; y++) {
          memcpy((uint8_t *)lr.pBits + y * lr.Pitch, rgba.data() + y * dw * 4,
                 dw * 4);
        }
        tex->UnlockRect(0);
      }
      s_TextureCache[pSrcFile] = tex;
      tex->AddRef(); // one for cache, one for caller
      *ppTexture = tex;
      return D3D_OK;
    }

    // Uncompressed DDS - check if it has RGB data
    if (dh->ddspf.dwFlags & 0x40) { // DDPF_RGB
      int dw2 = dh->dwWidth, dh2 = dh->dwHeight;
      int bpp = dh->ddspf.dwRGBBitCount / 8;
      const uint8_t *src = (const uint8_t *)(dh + 1);

      IDirect3DTexture8 *tex = nullptr;
      HRESULT hr = pDevice->CreateTexture(dw2, dh2, 1, 0, D3DFMT_A8R8G8B8,
                                          D3DPOOL_MANAGED, &tex);
      if (FAILED(hr) || !tex)
        return E_FAIL;

      D3DLOCKED_RECT lr;
      if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
        for (int y = 0; y < dh2; y++) {
          const uint8_t *sline = src + y * dw2 * bpp;
          uint8_t *dline = (uint8_t *)lr.pBits + y * lr.Pitch;
          for (int x = 0; x < dw2; x++) {
            dline[x * 4 + 0] = sline[x * bpp + 0]; // B
            dline[x * 4 + 1] = sline[x * bpp + 1]; // G
            dline[x * 4 + 2] = sline[x * bpp + 2]; // R
            dline[x * 4 + 3] = (bpp >= 4) ? sline[x * bpp + 3] : 255;
          }
        }
        tex->UnlockRect(0);
      }
      s_TextureCache[pSrcFile] = tex;
      tex->AddRef();
      *ppTexture = tex;
      return D3D_OK;
    }

    return E_FAIL;
  }

  // TGA
  if (fileData.size() < sizeof(TGAHeader))
    return E_FAIL;

  TGAHeader *hdr = (TGAHeader *)fileData.data();
  int w = hdr->Width, h = hdr->Height, bpp = hdr->PixelDepth;
  int type = hdr->ImageType;

  if (w <= 0 || h <= 0 || (bpp != 8 && bpp != 24 && bpp != 32))
    return E_FAIL;

  unsigned char *src_ptr = (unsigned char *)(hdr + 1) + hdr->IDLength;

  // Palette for 8-bit
  struct RGBCol {
    uint8_t b, g, r, a;
  };
  std::vector<RGBCol> palette(256);
  if (hdr->ColorMapType == 1) {
    int len = hdr->CMapLength;
    int depth = hdr->CMapDepth;
    int db = depth / 8;
    unsigned char *p_ptr = src_ptr;
    src_ptr += len * db;
    for (int i = 0; i < len; i++) {
      palette[i].b = p_ptr[i * db + 0];
      palette[i].g = p_ptr[i * db + 1];
      palette[i].r = p_ptr[i * db + 2];
      palette[i].a = (db == 4) ? p_ptr[i * db + 3] : 255;
    }
  }

  int bv = bpp / 8;
  std::vector<unsigned char> decomp;
  if (type == 9 || type == 10 || type == 11) { // RLE
    decomp.resize(w * h * bv);
    unsigned char *dst = decomp.data();
    int pixels = 0, total = w * h;
    while (pixels < total) {
      unsigned char ch = *src_ptr++;
      if (ch & 0x80) {
        int n = (ch & 0x7F) + 1;
        for (int j = 0; j < n && pixels < total; j++) {
          memcpy(dst + pixels * bv, src_ptr, bv);
          pixels++;
        }
        src_ptr += bv;
      } else {
        int n = ch + 1;
        for (int j = 0; j < n && pixels < total; j++) {
          memcpy(dst + pixels * bv, src_ptr, bv);
          src_ptr += bv;
          pixels++;
        }
      }
    }
    src_ptr = decomp.data();
  }

  // Convert to BGRA
  std::vector<unsigned char> rgba(w * h * 4);
  bool flipY = !(hdr->ImageDescriptor & 0x20);
  for (int y = 0; y < h; y++) {
    int sy = flipY ? (h - 1 - y) : y;
    const unsigned char *sline = src_ptr + (sy * w * bv);
    unsigned char *dline = rgba.data() + (y * w * 4);
    for (int x = 0; x < w; x++) {
      if (bv == 1 && hdr->ColorMapType == 1) {
        uint8_t idx = sline[x];
        dline[x * 4 + 0] = palette[idx].b;
        dline[x * 4 + 1] = palette[idx].g;
        dline[x * 4 + 2] = palette[idx].r;
        dline[x * 4 + 3] = palette[idx].a;
      } else {
        dline[x * 4 + 0] = sline[x * bv + 0]; // B
        dline[x * 4 + 1] = sline[x * bv + 1]; // G
        dline[x * 4 + 2] = sline[x * bv + 2]; // R
        dline[x * 4 + 3] = (bv == 4) ? sline[x * bv + 3] : 255;
      }
    }
  }

  IDirect3DTexture8 *tex = nullptr;
  HRESULT hr = pDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8,
                                      D3DPOOL_MANAGED, &tex);
  if (FAILED(hr) || !tex)
    return E_FAIL;

  D3DLOCKED_RECT lr;
  if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
    for (int y = 0; y < h; y++) {
      memcpy((uint8_t *)lr.pBits + y * lr.Pitch, rgba.data() + y * w * 4,
             w * 4);
    }
    tex->UnlockRect(0);
  }

  s_TextureCache[pSrcFile] = tex;
  tex->AddRef();
  *ppTexture = tex;
  return D3D_OK;
}

} // extern "C"

// Legacy wrappers — no longer needed
IDirect3DTexture8 *CreateMacOSD3DTextureWrapper(void *tex) { return nullptr; }

IDirect3DSurface8 *CreateMacOSD3DSurfaceWrapper(void *tex, int level) {
  return nullptr;
}

// ═══════════════════════════════════════════════════════════════
//  More D3DX Functions
// ═══════════════════════════════════════════════════════════════

extern "C" {

HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture8 *pBaseTexture,
                                 const void *pPalette, UINT SrcLevel,
                                 DWORD Filter) {
  return D3D_OK;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *pDestSurface,
                                          const void *pDestPalette,
                                          const RECT *pDestRect,
                                          IDirect3DSurface8 *pSrcSurface,
                                          const void *pSrcPalette,
                                          const RECT *pSrcRect, DWORD Filter,
                                          DWORD ColorKey) {
  D3DLOCKED_RECT srcLR, destLR;
  if (pSrcSurface->LockRect(&srcLR, pSrcRect, 0) == 0) {
    if (pDestSurface->LockRect(&destLR, pDestRect, 0) == 0) {
      D3DSURFACE_DESC desc;
      pDestSurface->GetDesc(&desc);
      for (unsigned int y = 0; y < desc.Height; ++y) {
        memcpy((char *)destLR.pBits + y * destLR.Pitch,
               (char *)srcLR.pBits + y * srcLR.Pitch, desc.Width * 4);
      }
      pDestSurface->UnlockRect();
    }
    pSrcSurface->UnlockRect();
  }
  return D3D_OK;
}

HRESULT WINAPI D3DXCreateCubeTexture(IDirect3DDevice8 *pDevice, UINT Size,
                                     UINT MipLevels, DWORD Usage,
                                     D3DFORMAT Format, D3DPOOL Pool,
                                     IDirect3DCubeTexture8 **ppCubeTexture) {
  return E_NOTIMPL;
}

HRESULT WINAPI D3DXCreateVolumeTexture(
    IDirect3DDevice8 *pDevice, UINT Width, UINT Height, UINT Depth,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    IDirect3DVolumeTexture8 **ppVolumeTexture) {
  return E_NOTIMPL;
}

HRESULT WINAPI D3DXGetErrorStringA(HRESULT hr, char *pBuffer, UINT BufferSize) {
  if (pBuffer && BufferSize > 0) {
    snprintf(pBuffer, BufferSize, "D3D Error 0x%08X", (unsigned int)hr);
  }
  return D3D_OK;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF) {
  UINT size = 0;

  DWORD posType = FVF & 0x00E;
  if (FVF & D3DFVF_XYZRHW) {
    size += 16;
  } else if (posType >= D3DFVF_XYZB1 && posType <= D3DFVF_XYZB5) {
    size += 12;
    int numWeights = (posType - D3DFVF_XYZ) / 2;
    size += numWeights * 4;
  } else if (FVF & D3DFVF_XYZ) {
    size += 12;
  }

  if (FVF & D3DFVF_NORMAL)
    size += 12;
  if (FVF & D3DFVF_PSIZE)
    size += 4;
  if (FVF & D3DFVF_DIFFUSE)
    size += 4;
  if (FVF & D3DFVF_SPECULAR)
    size += 4;

  UINT nTex = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (UINT i = 0; i < nTex; i++) {
    UINT texFmt = (FVF >> (i * 2 + 16)) & 0x3;
    switch (texFmt) {
    case D3DFVF_TEXTUREFORMAT1:
      size += 4;
      break;
    case D3DFVF_TEXTUREFORMAT2:
      size += 8;
      break;
    case D3DFVF_TEXTUREFORMAT3:
      size += 12;
      break;
    case D3DFVF_TEXTUREFORMAT4:
      size += 16;
      break;
    }
  }
  return size;
}

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors) {
  return E_NOTIMPL;
}

} // extern "C"

// ═══════════════════════════════════════════════════════════════
//  Entry Points — called from dx8wrapper.cpp
// ═══════════════════════════════════════════════════════════════

extern "C" IDirect3D8 *CreateMacOSD3D8() { return CreateMetalInterface8(); }

extern "C" IDirect3DDevice8 *CreateMacOSD3DDevice8() {
  return CreateMetalDevice8();
}

// ═══════════════════════════════════════════════════════════════
//  Stubs for removed MacOSRenderDevice pipeline
// ═══════════════════════════════════════════════════════════════

// Previously defined in MacOSRenderer.mm — no longer needed since
// all rendering goes through MetalDevice8 (DX8 pipeline).
extern "C" void MacOS_InitRenderer(void *windowHandle) {
  // No-op: old MacOSRenderDevice initialization removed
}

extern "C" void MacOS_Render() {
  // No-op: old MacOSRenderDevice render removed
}
