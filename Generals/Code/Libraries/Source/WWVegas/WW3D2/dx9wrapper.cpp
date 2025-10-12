/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dx9wrapper.h"
#include <iostream>

DX9Wrapper::DX9Wrapper()
    : m_pD3DDevice(nullptr)
{
}

DX9Wrapper::~DX9Wrapper()
{
    Shutdown();
}

bool DX9Wrapper::Init(IDirect3DDevice9* device)
{
    if (!device)
    {
        std::cerr << "DX9Wrapper::Init: Provided D3DDevice9 is null." << std::endl;
        return false;
    }
    m_pD3DDevice = device;
    m_pD3DDevice->AddRef(); // Take a reference to the device
    std::cout << "DX9Wrapper initialized with D3DDevice9." << std::endl;
    return true;
}

void DX9Wrapper::Shutdown()
{
    if (m_pD3DDevice)
    {
        m_pD3DDevice->Release();
        m_pD3DDevice = nullptr;
        std::cout << "DX9Wrapper shut down." << std::endl;
    }
}

DX9Wrapper TheDX9Wrapper;
