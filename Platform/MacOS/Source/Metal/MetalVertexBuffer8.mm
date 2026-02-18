#import "MetalVertexBuffer8.h"
#import "MetalIndexBuffer8.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#import <Metal/Metal.h>
#include <windows.h>  // macOS Win32 type shim
#include <cstdio>

// MetalVertexBuffer8 implementation

MetalVertexBuffer8::MetalVertexBuffer8(unsigned int fvf, unsigned short count,
                                       unsigned int size)
    : m_FVF(fvf), m_VertexCount(count), m_VertexSize(size), m_RefCount(1),
      m_MTLBuffer(nullptr), m_IsDirty(true) {
  m_SysMemCopy = new uint8_t[count * size];
}

MetalVertexBuffer8::~MetalVertexBuffer8() {
  delete[] m_SysMemCopy;
  if (m_MTLBuffer) {
    id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)m_MTLBuffer;
    buf = nil;
  }
}

STDMETHODIMP MetalVertexBuffer8::QueryInterface(REFIID riid, void **ppvObj) {
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MetalVertexBuffer8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalVertexBuffer8::Release() {
  if (m_RefCount > 0)
    --m_RefCount;
  // Don't delete this — lifetime managed by DX8VertexBufferClass destructor
  // which calls VertexBuffer->Release() then frees via VertexBufferClass dtor
  return m_RefCount;
}

STDMETHODIMP MetalVertexBuffer8::GetDevice(IDirect3DDevice8 **ppDevice) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalVertexBuffer8::SetPrivateData(REFGUID guid, const void *pData,
                                                DWORD SizeOfData, DWORD Flags) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalVertexBuffer8::GetPrivateData(REFGUID guid, void *pData,
                                                DWORD *pSizeOfData) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalVertexBuffer8::FreePrivateData(REFGUID guid) {
  return E_NOTIMPL;
}

STDMETHODIMP_(DWORD) MetalVertexBuffer8::SetPriority(DWORD PriorityNew) {
  return 0;
}

STDMETHODIMP_(DWORD) MetalVertexBuffer8::GetPriority() { return 0; }

STDMETHODIMP_(void) MetalVertexBuffer8::PreLoad() {}

STDMETHODIMP_(D3DRESOURCETYPE) MetalVertexBuffer8::GetType() {
  return D3DRTYPE_VERTEXBUFFER;
}

STDMETHODIMP MetalVertexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock,
                                      BYTE **ppbData, DWORD Flags) {
  if (ppbData) {
    *ppbData = m_SysMemCopy + OffsetToLock;
    return D3D_OK;
  }
  return E_POINTER;
}

// Helper to get global device - need to declare this as external since it's
// defined in MetalDevice8.mm or related files
extern id<MTLDevice> GetGlobalMetalDevice(); // We need a way to get the device

STDMETHODIMP MetalVertexBuffer8::Unlock() {
  if (m_MTLBuffer) {
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MTLBuffer;
    memcpy([buf contents], m_SysMemCopy, m_VertexCount * m_VertexSize);
  } else {
    // If not created yet in GetMTLBuffer, mark as dirty so it gets created with
    // fresh data
    m_IsDirty = true;
  }
  return D3D_OK;
}

void *MetalVertexBuffer8::GetMTLBuffer() {
  if (!m_MTLBuffer) {
    // Use the device from MetalDevice8 (set during InitMetal)
    extern void *g_MetalMTLDevice;
    id<MTLDevice> device = g_MetalMTLDevice
                               ? (__bridge id<MTLDevice>)g_MetalMTLDevice
                               : MTLCreateSystemDefaultDevice();

    if (device) {
      id<MTLBuffer> buf =
          [device newBufferWithBytes:m_SysMemCopy
                              length:m_VertexCount * m_VertexSize
                             options:MTLResourceStorageModeShared];
      m_MTLBuffer = (__bridge_retained void *)buf;
      m_IsDirty = false;
    }
  } else if (m_IsDirty) {
    // Update existing buffer
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MTLBuffer;
    memcpy([buf contents], m_SysMemCopy, m_VertexCount * m_VertexSize);
    m_IsDirty = false;
  }
  return m_MTLBuffer;
}

STDMETHODIMP MetalVertexBuffer8::GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
  if (pDesc) {
    pDesc->Format = D3DFMT_UNKNOWN;
    pDesc->Type = D3DRTYPE_VERTEXBUFFER;
    pDesc->Usage = 0;
    pDesc->Pool = D3DPOOL_MANAGED;
    pDesc->Size = m_VertexCount * m_VertexSize;
    pDesc->FVF = m_FVF;
    return D3D_OK;
  }
  return E_POINTER;
}
