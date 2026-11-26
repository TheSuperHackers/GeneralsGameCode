/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#pragma once

#include "WW3D2/dx8wrapper.h"

// D3D8's integer ZBIAS (0..16) pulls coplanar polygons toward the camera.
// Keep depth testing enabled; never alter depth writes or the world position.
// Only use the smallest nonzero bias, preserving an existing stronger bias.
class ScopedTerrainDecalDepthBias
{
public:
    ScopedTerrainDecalDepthBias()
    {
        if (!DX8Wrapper::_Get_D3D_Device8() ||
            !DX8Wrapper::Get_Current_Caps()->Support_ZBias())
            return;

        const unsigned previous = DX8Wrapper::Get_DX8_Render_State(D3DRS_ZBIAS);
        // An uninitialised cache value or a preexisting nonzero bias is not ours to change.
        if (previous != 0)
            return;

        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 1);
        changed = true;
    }

    ~ScopedTerrainDecalDepthBias()
    {
        if (changed)
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
    }

    ScopedTerrainDecalDepthBias(const ScopedTerrainDecalDepthBias &) = delete;
    ScopedTerrainDecalDepthBias &operator=(const ScopedTerrainDecalDepthBias &) = delete;

private:
    bool changed = false;
};
