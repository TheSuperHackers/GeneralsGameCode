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

// TheSuperHackers @refactor bobtista 08/10/2026 DX8 definition of the Renderer
// methods. Init and Shutdown own the DX8Wrapper lifecycle. The rendering methods
// are one-line trampolines to the existing DX8Wrapper static API.

#include "WW3D2/Renderer.h"

#include "WW3D2/dx8wrapper.h"
#include "WW3D2/formconv.h"
#include "WWMath/vector3.h"
#include "WW3D2/lightenvironment.h"
#include "WWDebug/wwdebug.h"

static D3DTRANSFORMSTATETYPE To_D3D_Transform(RenderBackendTransform transform)
{
    switch (transform)
    {
    case RB_TRANSFORM_WORLD:
        return D3DTS_WORLD;
    case RB_TRANSFORM_VIEW:
        return D3DTS_VIEW;
    case RB_TRANSFORM_PROJECTION:
        return D3DTS_PROJECTION;
    }
    WWASSERT(0);
    return D3DTS_WORLD;
}

static bool Initialized = false;
static bool Lite = false;

bool Renderer::Init(void * window, bool lite)
{
    WWASSERT(!Initialized);
    if (Initialized)
    {
        return true;
    }

    Init_D3D_To_WW3_Conversion();
    WWDEBUG_SAY(("Init DX8Wrapper"));
    if (!DX8Wrapper::Init(window, lite))
    {
        return false;
    }

    Initialized = true;
    Lite = lite;
    return true;
}

void Renderer::Shutdown()
{
    if (Initialized && !Lite)
    {
        DX8Wrapper::Shutdown();
    }
    Initialized = false;
}

int Renderer::Get_Render_Device_Count()
{
    return DX8Wrapper::Get_Render_Device_Count();
}

int Renderer::Get_Render_Device()
{
    return DX8Wrapper::Get_Render_Device();
}

const char * Renderer::Get_Render_Device_Name(int device_index)
{
    return DX8Wrapper::Get_Render_Device_Name(device_index);
}

void Renderer::Get_Device_Resolution(int & width, int & height, int & bits, bool & windowed)
{
    DX8Wrapper::Get_Device_Resolution(width, height, bits, windowed);
}

void Renderer::Get_Render_Target_Resolution(int & width, int & height, int & bits, bool & windowed)
{
    DX8Wrapper::Get_Render_Target_Resolution(width, height, bits, windowed);
}

int Renderer::Get_Device_Resolution_Width()
{
    return DX8Wrapper::Get_Device_Resolution_Width();
}

int Renderer::Get_Device_Resolution_Height()
{
    return DX8Wrapper::Get_Device_Resolution_Height();
}

bool Renderer::Is_Windowed()
{
    return DX8Wrapper::Is_Windowed();
}

int Renderer::Get_Texture_Bitdepth()
{
    return DX8Wrapper::Get_Texture_Bitdepth();
}

int Renderer::Get_Swap_Interval()
{
    return DX8Wrapper::Get_Swap_Interval();
}

bool Renderer::Has_Stencil()
{
    return DX8Wrapper::Has_Stencil();
}

WW3DFormat Renderer::Get_Back_Buffer_Format()
{
    return DX8Wrapper::getBackBufferFormat();
}

void Renderer::Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit)
{
    DX8Wrapper::Set_Gamma(gamma, bright, contrast, calibrate, uselimit);
}

void Renderer::Begin_Scene()
{
    DX8Wrapper::Begin_Scene();
}

void Renderer::End_Scene(bool flip_frame)
{
    DX8Wrapper::End_Scene(flip_frame);
}

void Renderer::Flip_To_Primary()
{
    DX8Wrapper::Flip_To_Primary();
}

void Renderer::Clear(bool clear_color, bool clear_z_stencil,
                     const Vector3 & color,
                     float dest_alpha, float z, unsigned int stencil)
{
    DX8Wrapper::Clear(clear_color, clear_z_stencil, color, dest_alpha, z, stencil);
}

void Renderer::Set_Viewport(const RenderViewport & viewport)
{
    D3DVIEWPORT8 vp;
    vp.X      = viewport.x;
    vp.Y      = viewport.y;
    vp.Width  = viewport.width;
    vp.Height = viewport.height;
    vp.MinZ   = viewport.min_z;
    vp.MaxZ   = viewport.max_z;
    DX8Wrapper::Set_Viewport(&vp);
}

void Renderer::Invalidate_Cached_Render_States()
{
    DX8Wrapper::Invalidate_Cached_Render_States();
}

void Renderer::Set_Transform(RenderBackendTransform transform, const Matrix4x4 & m)
{
    DX8Wrapper::Set_Transform(To_D3D_Transform(transform), m);
}

void Renderer::Set_Transform(RenderBackendTransform transform, const Matrix3D & m)
{
    DX8Wrapper::Set_Transform(To_D3D_Transform(transform), m);
}

void Renderer::Get_Transform(RenderBackendTransform transform, Matrix4x4 & m)
{
    DX8Wrapper::Get_Transform(To_D3D_Transform(transform), m);
}

void Renderer::Set_World_Identity()
{
    DX8Wrapper::Set_World_Identity();
}

void Renderer::Set_View_Identity()
{
    DX8Wrapper::Set_View_Identity();
}

void Renderer::Set_Projection_Transform_With_Z_Bias(const Matrix4x4 & matrix, float znear, float zfar)
{
    DX8Wrapper::Set_Projection_Transform_With_Z_Bias(matrix, znear, zfar);
}

void Renderer::Set_Ambient(const Vector3 & color)
{
    DX8Wrapper::Set_Ambient(color);
}

void Renderer::Set_Light_Environment(LightEnvironmentClass * light_env)
{
    DX8Wrapper::Set_Light_Environment(light_env);
}

void Renderer::Set_Light(unsigned index, const LightClass & light)
{
    DX8Wrapper::Set_Light(index, light);
}

void Renderer::Clear_Light(unsigned index)
{
    DX8Wrapper::Set_Light(index, nullptr);
}

void Renderer::Set_Fog(bool enable, const Vector3 & color, float start, float end)
{
    DX8Wrapper::Set_Fog(enable, color, start, end);
}
