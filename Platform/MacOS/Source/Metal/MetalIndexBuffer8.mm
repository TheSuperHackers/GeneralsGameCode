#import "MetalIndexBuffer8.h"
#import "MetalVertexBuffer8.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#import <Metal/Metal.h>
#include <windows.h>  // macOS Win32 type shim
#include <cstdio>

// MetalIndexBuffer8 implementation

MetalIndexBuffer8::MetalIndexBuffer8(unsigned int count, bool is32bit)
    : m_Count(count), m_Is32Bit(is32bit), m_RefCount(1), m_MTLBuffer(nullptr),
      m_IsDirty(true) {
  uint32_t size = count * (is32bit ? 4 : 2);
  m_SysMemCopy = new uint8_t[size];
}

MetalIndexBuffer8::~MetalIndexBuffer8() {
  delete[] m_SysMemCopy;
  if (m_MTLBuffer) {
    id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)m_MTLBuffer;
    buf = nil;
  }
}

STDMETHODIMP MetalIndexBuffer8::QueryInterface(REFIID riid, void **ppvObj) {
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MetalIndexBuffer8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalIndexBuffer8::Release() {
  if (m_RefCount > 0)
    --m_RefCount;
  // Don't delete this — lifetime managed by DX8IndexBufferClass destructor
  return m_RefCount;
}

STDMETHODIMP MetalIndexBuffer8::GetDevice(IDirect3DDevice8 **ppDevice) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalIndexBuffer8::SetPrivateData(REFGUID guid, const void *pData,
                                               DWORD SizeOfData, DWORD Flags) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalIndexBuffer8::GetPrivateData(REFGUID guid, void *pData,
                                               DWORD *pSizeOfData) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalIndexBuffer8::FreePrivateData(REFGUID guid) {
  return E_NOTIMPL;
}

STDMETHODIMP_(DWORD) MetalIndexBuffer8::SetPriority(DWORD PriorityNew) {
  return 0;
}

STDMETHODIMP_(DWORD) MetalIndexBuffer8::GetPriority() { return 0; }

STDMETHODIMP_(void) MetalIndexBuffer8::PreLoad() {}

STDMETHODIMP_(D3DRESOURCETYPE) MetalIndexBuffer8::GetType() {
  return D3DRTYPE_INDEXBUFFER;
}

STDMETHODIMP MetalIndexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock,
                                     BYTE **ppbData, DWORD Flags) {
  if (ppbData) {
    *ppbData = m_SysMemCopy + OffsetToLock;
    return D3D_OK;
  }
  return E_POINTER;
}

STDMETHODIMP MetalIndexBuffer8::Unlock() {
  if (m_MTLBuffer) {
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MTLBuffer;
    uint32_t size = m_Count * (m_Is32Bit ? 4 : 2);
    memcpy([buf contents], m_SysMemCopy, size);
  } else {
    m_IsDirty = true;
  }
  return D3D_OK;
}

void *MetalIndexBuffer8::GetMTLBuffer() {
  if (!m_MTLBuffer) {
    extern void *g_MetalMTLDevice;
    id<MTLDevice> device = g_MetalMTLDevice
                               ? (__bridge id<MTLDevice>)g_MetalMTLDevice
                               : MTLCreateSystemDefaultDevice();
    if (device) {
      uint32_t size = m_Count * (m_Is32Bit ? 4 : 2);
      id<MTLBuffer> buf =
          [device newBufferWithBytes:m_SysMemCopy
                              length:size
                             options:MTLResourceStorageModeShared];
      m_MTLBuffer = (__bridge_retained void *)buf;
      m_IsDirty = false;
    }
  } else if (m_IsDirty) {
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MTLBuffer;
    uint32_t size = m_Count * (m_Is32Bit ? 4 : 2);
    memcpy([buf contents], m_SysMemCopy, size);
    m_IsDirty = false;
  }
  return m_MTLBuffer;
}

STDMETHODIMP MetalIndexBuffer8::GetDesc(D3DINDEXBUFFER_DESC *pDesc) {
  if (pDesc) {
    pDesc->Format = m_Is32Bit ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    pDesc->Type = D3DRTYPE_INDEXBUFFER;
    pDesc->Usage = 0;
    pDesc->Pool = D3DPOOL_MANAGED;
    pDesc->Size = m_Count * (m_Is32Bit ? 4 : 2);
    return D3D_OK;
  }
  return E_POINTER;
}
