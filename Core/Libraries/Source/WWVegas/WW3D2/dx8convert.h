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

#include "WWMath/vector3.h"
#include <d3d8types.h>

inline D3DVECTOR To_D3DVECTOR(const Vector3& v)
{
	D3DVECTOR dxv;
	dxv.x = v.X;
	dxv.y = v.Y;
	dxv.z = v.Z;
	return dxv;
}

inline D3DCOLORVALUE To_D3DCOLORVALUE(const Vector3& rgb, float a)
{
	D3DCOLORVALUE dxc;
	dxc.r = rgb.X;
	dxc.g = rgb.Y;
	dxc.b = rgb.Z;
	dxc.a = a;
	return dxc;
}
