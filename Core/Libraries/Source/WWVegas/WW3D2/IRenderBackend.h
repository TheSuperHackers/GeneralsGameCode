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

// TheSuperHackers @refactor bobtista 10/04/2026 Abstract W3D-facing rendering
// interface so WW3D2 rendering can be re-targeted to other backends while the
// existing DX8 path stays as the reference implementation.

#pragma once

// Forward declarations keep this header includable without pulling in the full
// WW3D2 header graph. All W3D types below are passed by pointer or reference.

class LightEnvironmentClass;
class Vector3;

struct RenderBackendViewport
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	float min_z;
	float max_z;
};

// A method appears here once a caller routes through it, not in anticipation of
// one. The set below is what current callers route through; the rest of the
// DX8Wrapper API stays reachable through DX8Wrapper's static methods until a
// caller migrates, at which point the method it needs moves here.
//
// Method names intentionally match the existing DX8Wrapper names so migrating a
// caller is a mechanical DX8Wrapper::X(...) -> Get_Render_Backend()->X(...)
// rewrite.

class IRenderBackend
{
public:
	virtual ~IRenderBackend() {}

	virtual void Set_Gamma(float gamma, float bright, float contrast, bool calibrate = true, bool uselimit = true) = 0;

	virtual void Begin_Scene() = 0;
	virtual void End_Scene(bool flip_frame = true) = 0;
	virtual void Flip_To_Primary() = 0;
	virtual void Clear(bool clear_color, bool clear_z_stencil,
	                   const Vector3& color,
	                   float dest_alpha = 0.0f, float z = 1.0f, unsigned int stencil = 0) = 0;
	virtual void Set_Viewport(const RenderBackendViewport& viewport) = 0;
	virtual void Invalidate_Cached_Render_States() = 0;

	virtual void Set_Ambient(const Vector3& color) = 0;
	virtual void Set_Light_Environment(LightEnvironmentClass* light_env) = 0;
};
