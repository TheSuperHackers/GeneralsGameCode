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

// TheSuperHackers @refactor bobtista 08/10/2026 Static rendering interface so WW3D2
// rendering can be re-targeted to other backends. The build links exactly one
// backend, and that backend defines the methods below. The DX8 backend is the
// reference implementation.

#pragma once

#include "WW3D2/ww3dformat.h"

// Forward declarations keep this header includable without pulling in the full
// WW3D2 header graph. All W3D classes below are passed by pointer or reference.

class DynamicIBAccessClass;
class DynamicVBAccessClass;
class IndexBufferClass;
class LightClass;
class LightEnvironmentClass;
class Matrix3D;
class Matrix4x4;
class ShaderClass;
class TextureBaseClass;
class Vector3;
class VertexBufferClass;
class VertexMaterialClass;

struct RenderViewport
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    float min_z;
    float max_z;
};

enum RenderBackendTransform
{
    RB_TRANSFORM_WORLD,
    RB_TRANSFORM_VIEW,
    RB_TRANSFORM_PROJECTION
};

// The interface holds only the methods that callers route through. The rest of
// the DX8Wrapper API is called directly.

class Renderer
{
public:
    // Initialized in WW3D::Init and shut down in WW3D::Shutdown.
    static bool Init(void * window, bool lite);
    static void Shutdown();

    static int Get_Render_Device_Count();
    static int Get_Render_Device();
    static const char * Get_Render_Device_Name(int device_index);
    static void Get_Device_Resolution(int & width, int & height, int & bits, bool & windowed);
    static void Get_Render_Target_Resolution(int & width, int & height, int & bits, bool & windowed);
    static int Get_Device_Resolution_Width();
    static int Get_Device_Resolution_Height();
    static bool Is_Windowed();
    static int Get_Texture_Bitdepth();
    static int Get_Swap_Interval();
    static bool Has_Stencil();
    static WW3DFormat Get_Back_Buffer_Format();

    static void Set_Gamma(float gamma, float bright, float contrast, bool calibrate = true, bool uselimit = true);

    static void Begin_Scene();
    static void End_Scene(bool flip_frame = true);
    static void Flip_To_Primary();
    static void Clear(bool clear_color, bool clear_z_stencil,
                      const Vector3 & color,
                      float dest_alpha = 0.0f, float z = 1.0f, unsigned int stencil = 0);
    static void Set_Viewport(const RenderViewport & viewport);
    static void Invalidate_Cached_Render_States();

    static void Set_Shader(const ShaderClass & shader);
    static void Set_Material(const VertexMaterialClass * material);
    static void Set_Texture(unsigned stage, TextureBaseClass * texture);
    static void Apply_Render_State_Changes();

    static void Set_Vertex_Buffer(const VertexBufferClass * vb, unsigned stream = 0);
    static void Set_Vertex_Buffer(const DynamicVBAccessClass & vba);
    static void Set_Index_Buffer(const IndexBufferClass * ib, unsigned short index_base_offset);
    static void Set_Index_Buffer(const DynamicIBAccessClass & iba, unsigned short index_base_offset);
    static void Set_Index_Buffer_Index_Offset(unsigned offset);

    static void Draw_Triangles(unsigned buffer_type, unsigned short start_index, unsigned short polygon_count,
                               unsigned short min_vertex_index, unsigned short vertex_count);
    static void Draw_Triangles(unsigned short start_index, unsigned short polygon_count,
                               unsigned short min_vertex_index, unsigned short vertex_count);
    static void Draw_Strip(unsigned short start_index, unsigned short polygon_count,
                           unsigned short min_vertex_index, unsigned short vertex_count);

    static void Set_Transform(RenderBackendTransform transform, const Matrix4x4 & m);
    static void Set_Transform(RenderBackendTransform transform, const Matrix3D & m);
    static void Get_Transform(RenderBackendTransform transform, Matrix4x4 & m);
    static void Set_World_Identity();
    static void Set_View_Identity();
    static void Set_Projection_Transform_With_Z_Bias(const Matrix4x4 & matrix, float znear, float zfar);

    static void Set_Ambient(const Vector3 & color);
    static void Set_Light_Environment(LightEnvironmentClass * light_env);
    static void Set_Light(unsigned index, const LightClass & light);
    static void Clear_Light(unsigned index);
    static void Set_Fog(bool enable, const Vector3 & color, float start, float end);
};
