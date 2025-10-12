#include "DX9Device.h"
#include "DX9Device.h"
#include <iostream> // For basic logging
#include <Windows.h> // Required for HWND

DX9Device::DX9Device()
    : m_pD3D(nullptr), m_pD3DDevice(nullptr)
{
    // Constructor
}

DX9Device::~DX9Device()
{
    Shutdown();
}

bool DX9Device::Init(HWND hWnd)
{
    std::cout << "Initializing DirectX 9 Device..." << std::endl;

    // Create the D3D object.
    if (nullptr == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        std::cerr << "Failed to create Direct3D9 object!" << std::endl;
        return false;
    }

    // Placeholder for device creation parameters.
    // In a real scenario, these would come from configuration or window properties.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Use desktop format

    // Create the D3DDevice
    if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                   D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                   &d3dpp, &m_pD3DDevice)))
    {
        std::cerr << "Failed to create Direct3D9 device!" << std::endl;
        m_pD3D->Release();
        m_pD3D = nullptr;
        return false;
    }

    std::cout << "DirectX 9 Device initialized successfully." << std::endl;
    return true;
}

void DX9Device::Shutdown()
{
    std::cout << "Shutting down DirectX 9 Device..." << std::endl;
    if (m_pD3DDevice != nullptr)
    {
        m_pD3DDevice->Release();
        m_pD3DDevice = nullptr;
    }
    if (m_pD3D != nullptr)
    {
        m_pD3D->Release();
        m_pD3D = nullptr;
    }
    std::cout << "DirectX 9 Device shut down." << std::endl;
}
