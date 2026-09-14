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

// TheSuperHackers @refactor bobtista 10/04/2026 DX8Backend is the reference
// implementation of IRenderBackend that owns the DX8Wrapper lifecycle and
// forwards rendering methods to the existing DX8Wrapper static facade.

#pragma once

#include "WW3D2/IRenderBackend.h"

class DX8Backend : public IRenderBackend
{
public:
	static DX8Backend* Create(void* window, bool lite);

	virtual ~DX8Backend() override;

	virtual void Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit) override;

	virtual void Begin_Scene() override;
	virtual void End_Scene(bool flip_frame) override;
	virtual void Flip_To_Primary() override;
	virtual void Clear(bool clear_color, bool clear_z_stencil,
	                   const Vector3& color,
	                   float dest_alpha, float z, unsigned int stencil) override;
	virtual void Set_Viewport(const RenderBackendViewport& viewport) override;
	virtual void Invalidate_Cached_Render_States() override;

	virtual void Set_Ambient(const Vector3& color) override;
	virtual void Set_Light_Environment(LightEnvironmentClass* light_env) override;

private:
	explicit DX8Backend(bool lite);

	bool Lite;
};
