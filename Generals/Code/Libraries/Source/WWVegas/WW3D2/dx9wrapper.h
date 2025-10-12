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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX9_WRAPPER_H
#define DX9_WRAPPER_H

#include "always.h"
#include <d3d9.h>

class DX9Wrapper
{
public:
    DX9Wrapper();
    ~DX9Wrapper();

    bool Init(IDirect3DDevice9* device);
    void Shutdown();

    IDirect3DDevice9* GetDevice() const { return m_pD3DDevice; }

private:
    IDirect3DDevice9* m_pD3DDevice;
};

extern DX9Wrapper TheDX9Wrapper;

#endif // DX9_WRAPPER_H
