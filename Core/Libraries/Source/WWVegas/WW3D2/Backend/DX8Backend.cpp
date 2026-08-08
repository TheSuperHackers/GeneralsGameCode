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

// TheSuperHackers @refactor bobtista 10/04/2026 DX8Backend forwarding adapter.
// Every method in this file is a one-line trampoline to the existing
// DX8Wrapper static API. Keep it that way — if behavior needs to change it
// should change in DX8Wrapper, not here.

#include "DX8Backend.h"

#include "WW3D2/dx8wrapper.h"
#include "WWMath/vector3.h"
#include "WWMath/matrix4.h"
#include "WWMath/matrix3d.h"
#include "WW3D2/light.h"
#include "WW3D2/lightenvironment.h"

DX8Backend::DX8Backend()
{
}

DX8Backend::~DX8Backend()
{
}

bool DX8Backend::Is_Device_Lost() const
{
    return DX8Wrapper::Is_Device_Lost();
}

bool DX8Backend::Has_Stencil()
{
    return DX8Wrapper::Has_Stencil();
}

WW3DFormat DX8Backend::Get_Back_Buffer_Format()
{
    return DX8Wrapper::getBackBufferFormat();
}

void DX8Backend::Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit)
{
    DX8Wrapper::Set_Gamma(gamma, bright, contrast, calibrate, uselimit);
}

void DX8Backend::Begin_Scene()
{
    DX8Wrapper::Begin_Scene();
}

void DX8Backend::End_Scene(bool flip_frame)
{
    DX8Wrapper::End_Scene(flip_frame);
}

void DX8Backend::Flip_To_Primary()
{
    DX8Wrapper::Flip_To_Primary();
}

void DX8Backend::Clear(bool clear_color, bool clear_z_stencil,
                       const Vector3 & color,
                       float dest_alpha, float z, unsigned int stencil)
{
    DX8Wrapper::Clear(clear_color, clear_z_stencil, color, dest_alpha, z, stencil);
}

void DX8Backend::Set_Viewport(const RenderBackendViewport & viewport)
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

void DX8Backend::Set_Vertex_Buffer(const VertexBufferClass * vb, unsigned int stream)
{
    DX8Wrapper::Set_Vertex_Buffer(vb, stream);
}

void DX8Backend::Set_Vertex_Buffer(const DynamicVBAccessClass & vba)
{
    DX8Wrapper::Set_Vertex_Buffer(vba);
}

void DX8Backend::Set_Index_Buffer(const IndexBufferClass * ib, unsigned int index_base_offset)
{
    DX8Wrapper::Set_Index_Buffer(ib, static_cast<unsigned short>(index_base_offset));
}

void DX8Backend::Set_Index_Buffer(const DynamicIBAccessClass & iba, unsigned int index_base_offset)
{
    DX8Wrapper::Set_Index_Buffer(iba, static_cast<unsigned short>(index_base_offset));
}

void DX8Backend::Set_Index_Buffer_Index_Offset(unsigned int offset)
{
    DX8Wrapper::Set_Index_Buffer_Index_Offset(offset);
}

void DX8Backend::Set_Shader(const ShaderClass & shader)
{
    DX8Wrapper::Set_Shader(shader);
}

void DX8Backend::Get_Shader(ShaderClass & shader)
{
    DX8Wrapper::Get_Shader(shader);
}

void DX8Backend::Set_Material(const VertexMaterialClass * material)
{
    DX8Wrapper::Set_Material(material);
}

void DX8Backend::Set_Texture(unsigned int stage, TextureBaseClass * texture)
{
    DX8Wrapper::Set_Texture(stage, texture);
}

void DX8Backend::Apply_Render_State_Changes()
{
    DX8Wrapper::Apply_Render_State_Changes();
}

void DX8Backend::Apply_Default_State()
{
    DX8Wrapper::Apply_Default_State();
}

void DX8Backend::Invalidate_Cached_Render_States()
{
    DX8Wrapper::Invalidate_Cached_Render_States();
}

static D3DTRANSFORMSTATETYPE To_D3D_Transform(TransformKind transform)
{
    switch (transform)
    {
    case RB_TRANSFORM_VIEW:
        return D3DTS_VIEW;
    case RB_TRANSFORM_PROJECTION:
        return D3DTS_PROJECTION;
    case RB_TRANSFORM_WORLD:
    default:
        return D3DTS_WORLD;
    }
}

void DX8Backend::Set_Transform(TransformKind transform, const Matrix4x4 & m)
{
    DX8Wrapper::Set_Transform(To_D3D_Transform(transform), m);
}

void DX8Backend::Set_Transform(TransformKind transform, const Matrix3D & m)
{
    DX8Wrapper::Set_Transform(To_D3D_Transform(transform), m);
}

void DX8Backend::Get_Transform(TransformKind transform, Matrix4x4 & m)
{
    DX8Wrapper::Get_Transform(To_D3D_Transform(transform), m);
}

void DX8Backend::Set_World_Identity()
{
    DX8Wrapper::Set_World_Identity();
}

void DX8Backend::Set_View_Identity()
{
    DX8Wrapper::Set_View_Identity();
}

bool DX8Backend::Is_World_Identity()
{
    return DX8Wrapper::Is_World_Identity();
}

bool DX8Backend::Is_View_Identity()
{
    return DX8Wrapper::Is_View_Identity();
}

void DX8Backend::Set_Projection_Transform_With_Z_Bias(const Matrix4x4 & matrix, float znear, float zfar)
{
    DX8Wrapper::Set_Projection_Transform_With_Z_Bias(matrix, znear, zfar);
}

void DX8Backend::Set_Light(unsigned int index, const LightClass & light)
{
    DX8Wrapper::Set_Light(index, light);
}

void DX8Backend::Set_Ambient(const Vector3 & color)
{
    DX8Wrapper::Set_Ambient(color);
}

const Vector3 & DX8Backend::Get_Ambient() const
{
    return DX8Wrapper::Get_Ambient();
}

void DX8Backend::Set_Fog(bool enable, const Vector3 & color, float start, float end)
{
    DX8Wrapper::Set_Fog(enable, color, start, end);
}

bool DX8Backend::Get_Fog_Enable() const
{
    return DX8Wrapper::Get_Fog_Enable();
}

void DX8Backend::Set_Light_Environment(LightEnvironmentClass * light_env)
{
    DX8Wrapper::Set_Light_Environment(light_env);
}

LightEnvironmentClass * DX8Backend::Get_Light_Environment() const
{
    return DX8Wrapper::Get_Light_Environment();
}

void DX8Backend::Draw_Triangles(unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count)
{
    DX8Wrapper::Draw_Triangles(static_cast<unsigned short>(start_index),
                               static_cast<unsigned short>(polygon_count),
                               static_cast<unsigned short>(min_vertex_index),
                               static_cast<unsigned short>(vertex_count));
}

void DX8Backend::Draw_Triangles(unsigned int buffer_type,
                                unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count)
{
    DX8Wrapper::Draw_Triangles(buffer_type,
                               static_cast<unsigned short>(start_index),
                               static_cast<unsigned short>(polygon_count),
                               static_cast<unsigned short>(min_vertex_index),
                               static_cast<unsigned short>(vertex_count));
}

void DX8Backend::Draw_Strip(unsigned int start_index,
                            unsigned int index_count,
                            unsigned int min_vertex_index,
                            unsigned int vertex_count)
{
    DX8Wrapper::Draw_Strip(static_cast<unsigned short>(start_index),
                           static_cast<unsigned short>(index_count),
                           static_cast<unsigned short>(min_vertex_index),
                           static_cast<unsigned short>(vertex_count));
}

void DX8Backend::Set_Vertex_Shader(unsigned int vertex_shader)
{
    DX8Wrapper::Set_Vertex_Shader(static_cast<DWORD>(vertex_shader));
}

void DX8Backend::Set_Pixel_Shader(unsigned int pixel_shader)
{
    DX8Wrapper::Set_Pixel_Shader(static_cast<DWORD>(pixel_shader));
}

void DX8Backend::Set_Vertex_Shader_Constant(int reg, const void * data, int count)
{
    DX8Wrapper::Set_Vertex_Shader_Constant(reg, data, count);
}

void DX8Backend::Set_Pixel_Shader_Constant(int reg, const void * data, int count)
{
    DX8Wrapper::Set_Pixel_Shader_Constant(reg, data, count);
}

TextureClass * DX8Backend::Create_Render_Target(int width, int height, WW3DFormat format)
{
    return DX8Wrapper::Create_Render_Target(width, height, format);
}

void DX8Backend::Set_Render_Target_With_Z(TextureClass * texture, ZTextureClass * ztexture)
{
    DX8Wrapper::Set_Render_Target_With_Z(texture, ztexture);
}

bool DX8Backend::Is_Render_To_Texture()
{
    return DX8Wrapper::Is_Render_To_Texture();
}

void DX8Backend::Set_Shadow_Map(int idx, ZTextureClass * ztex)
{
    DX8Wrapper::Set_Shadow_Map(idx, ztex);
}

ZTextureClass * DX8Backend::Get_Shadow_Map(int idx)
{
    return DX8Wrapper::Get_Shadow_Map(idx);
}
