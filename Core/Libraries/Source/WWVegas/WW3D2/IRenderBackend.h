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

#include "ww3dformat.h"

// Forward declarations keep this header includable without pulling in the full
// WW3D2 header graph. All W3D types below are passed by pointer or reference.

class ShaderClass;
class VertexMaterialClass;
class TextureBaseClass;
class TextureClass;
class ZTextureClass;
class VertexBufferClass;
class IndexBufferClass;
class DynamicVBAccessClass;
class DynamicIBAccessClass;
class LightClass;
class LightEnvironmentClass;
class Matrix4x4;
class Matrix3D;
class Vector3;

enum TransformKind
{
    RB_TRANSFORM_WORLD,
    RB_TRANSFORM_VIEW,
    RB_TRANSFORM_PROJECTION
};

struct RenderBackendViewport
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    float min_z;
    float max_z;
};

// Exposes the high-level subset of DX8Wrapper's public API: the calls that take
// and return W3D types (ShaderClass, TextureBaseClass, Matrix4x4, etc.). The
// low-level D3D8-specific entry points on DX8Wrapper are not exposed here and
// remain reachable only through DX8Wrapper's static methods.
//
// Method names intentionally match the existing DX8Wrapper names so migrating a
// caller is a mechanical DX8Wrapper::X(...) -> g_renderBackend->X(...) rewrite.

class IRenderBackend
{
public:
    virtual ~IRenderBackend() {}

    // Optional device lifecycle. DX8Wrapper owns the render device and calls
    // these after the backend is constructed and before it is destroyed. A
    // backend that drives its own device creates it in Initialize and releases
    // it in Shutdown; the DX8 reference backend leaves them as no-ops.
    virtual void Initialize(void * window, int width, int height) {}
    virtual void Shutdown() {}

    virtual bool Is_Device_Lost() const = 0;
    virtual bool Has_Stencil() = 0;
    virtual WW3DFormat Get_Back_Buffer_Format() = 0;
    virtual void Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit) = 0;

    virtual void Begin_Scene() = 0;
    virtual void End_Scene(bool flip_frame) = 0;
    virtual void Flip_To_Primary() = 0;
    virtual void Clear(bool clear_color, bool clear_z_stencil,
                       const Vector3 & color,
                       float dest_alpha, float z, unsigned int stencil) = 0;
    virtual void Set_Viewport(const RenderBackendViewport & viewport) = 0;

    virtual void Set_Vertex_Buffer(const VertexBufferClass * vb, unsigned int stream) = 0;
    virtual void Set_Vertex_Buffer(const DynamicVBAccessClass & vba) = 0;
    virtual void Set_Index_Buffer(const IndexBufferClass * ib, unsigned int index_base_offset) = 0;
    virtual void Set_Index_Buffer(const DynamicIBAccessClass & iba, unsigned int index_base_offset) = 0;
    virtual void Set_Index_Buffer_Index_Offset(unsigned int offset) = 0;

    virtual void Set_Shader(const ShaderClass & shader) = 0;
    virtual void Get_Shader(ShaderClass & shader) = 0;
    virtual void Set_Material(const VertexMaterialClass * material) = 0;
    virtual void Set_Texture(unsigned int stage, TextureBaseClass * texture) = 0;

    virtual void Apply_Render_State_Changes() = 0;
    virtual void Apply_Default_State() = 0;
    virtual void Invalidate_Cached_Render_States() = 0;

    virtual void Set_Transform(TransformKind transform, const Matrix4x4 & m) = 0;
    virtual void Set_Transform(TransformKind transform, const Matrix3D & m) = 0;
    virtual void Get_Transform(TransformKind transform, Matrix4x4 & m) = 0;
    virtual void Set_World_Identity() = 0;
    virtual void Set_View_Identity() = 0;
    virtual bool Is_World_Identity() = 0;
    virtual bool Is_View_Identity() = 0;
    virtual void Set_Projection_Transform_With_Z_Bias(const Matrix4x4 & matrix,
                                                      float znear, float zfar) = 0;

    virtual void Set_Light(unsigned int index, const LightClass & light) = 0;
    virtual void Set_Ambient(const Vector3 & color) = 0;
    virtual const Vector3 & Get_Ambient() const = 0;
    virtual void Set_Fog(bool enable, const Vector3 & color, float start, float end) = 0;
    virtual bool Get_Fog_Enable() const = 0;
    virtual void Set_Light_Environment(LightEnvironmentClass * light_env) = 0;
    virtual LightEnvironmentClass * Get_Light_Environment() const = 0;

    // Ranges are unsigned int rather than the DX8 era unsigned short so the
    // interface does not pin every future backend to 16-bit draw counts. The DX8
    // reference backend narrows on the way through to DX8Wrapper.

    virtual void Draw_Triangles(unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count) = 0;

    virtual void Draw_Triangles(unsigned int buffer_type,
                                unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count) = 0;

    virtual void Draw_Strip(unsigned int start_index,
                            unsigned int index_count,
                            unsigned int min_vertex_index,
                            unsigned int vertex_count) = 0;

    // Legacy fixed function vertex format code or shader handle. Always 32 bits
    // wide; unsigned long would be 64 bits on targets where long is 64-bit.
    virtual void Set_Vertex_Shader(unsigned int vertex_shader) = 0;
    virtual void Set_Pixel_Shader(unsigned int pixel_shader) = 0;
    virtual void Set_Vertex_Shader_Constant(int reg, const void * data, int count) = 0;
    virtual void Set_Pixel_Shader_Constant(int reg, const void * data, int count) = 0;

    virtual TextureClass * Create_Render_Target(int width, int height, WW3DFormat format) = 0;
    virtual void Set_Render_Target_With_Z(TextureClass * texture, ZTextureClass * ztexture) = 0;
    virtual bool Is_Render_To_Texture() = 0;
    virtual void Set_Shadow_Map(int idx, ZTextureClass * ztex) = 0;
    virtual ZTextureClass * Get_Shadow_Map(int idx) = 0;
};
