/**
 * MetalInterface8 — IDirect3D8 implementation on Apple Metal
 *
 * This represents the "Direct3D8" object that enumerates adapters
 * and creates devices. On macOS, there's always one adapter (Metal GPU).
 */
#pragma once

#ifdef __APPLE__

#include <windows.h>  // macOS Win32 type shim
#include <d3d8.h>

class MetalInterface8 : public IDirect3D8 {
public:
  MetalInterface8();
  virtual ~MetalInterface8();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override;
  STDMETHOD_(ULONG, AddRef)() override;
  STDMETHOD_(ULONG, Release)() override;

  // IDirect3D8
  STDMETHOD(RegisterSoftwareDevice)(void *pInitializeFunction) override;
  STDMETHOD_(UINT, GetAdapterCount)() override;
  STDMETHOD(GetAdapterIdentifier)(UINT Adapter, DWORD Flags,
                                  D3DADAPTER_IDENTIFIER8 *pIdentifier) override;
  STDMETHOD_(UINT, GetAdapterModeCount)(UINT Adapter) override;
  STDMETHOD(EnumAdapterModes)(UINT Adapter, UINT Mode,
                              D3DDISPLAYMODE *pMode) override;
  STDMETHOD(GetAdapterDisplayMode)(UINT Adapter,
                                   D3DDISPLAYMODE *pMode) override;
  STDMETHOD(CheckDeviceType)(UINT Adapter, DWORD CheckType,
                             D3DFORMAT DisplayFormat,
                             D3DFORMAT BackBufferFormat,
                             BOOL Windowed) override;
  STDMETHOD(CheckDeviceFormat)(UINT Adapter, DWORD DeviceType,
                               D3DFORMAT AdapterFormat, DWORD Usage,
                               DWORD RType, D3DFORMAT CheckFormat) override;
  STDMETHOD(CheckDeviceMultiSampleType)(UINT Adapter, DWORD DeviceType,
                                        D3DFORMAT SurfaceFormat, BOOL Windowed,
                                        DWORD MultiSampleType) override;
  STDMETHOD(CheckDepthStencilMatch)(UINT Adapter, DWORD DeviceType,
                                    D3DFORMAT AdapterFormat,
                                    D3DFORMAT RenderTargetFormat,
                                    D3DFORMAT DepthStencilFormat) override;
  STDMETHOD(GetDeviceCaps)(UINT Adapter, DWORD DeviceType,
                           D3DCAPS8 *pCaps) override;
  STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT Adapter) override;
  STDMETHOD(CreateDevice)(
      UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
      DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters,
      IDirect3DDevice8 **ppReturnedDeviceInterface) override;

private:
  ULONG m_RefCount;
};

#endif // __APPLE__
