#ifndef DX9DEVICE_H
#define DX9DEVICE_H

// Placeholder for DirectX 9 Device header
// This file will contain the interface for the new DirectX 9 rendering device.

#include <d3d9.h> // Include DirectX 9 headers

class DX9Device
{
public:
    DX9Device();
    ~DX9Device();

    bool Init(HWND hWnd);
    void Shutdown();

    // Placeholder for device-specific functions
    IDirect3D9* GetDirect3D() const { return m_pD3D; }
    IDirect3DDevice9* GetDirect3DDevice() const { return m_pD3DDevice; }

private:
    IDirect3D9*       m_pD3D;
    IDirect3DDevice9* m_pD3DDevice;
};

#endif // DX9DEVICE_H
