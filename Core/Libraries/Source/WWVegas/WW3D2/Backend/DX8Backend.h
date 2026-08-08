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
// implementation of IRenderBackend that forwards every virtual method to the
// existing DX8Wrapper static facade. Pure adaptation, no new rendering logic.

#pragma once

#include "WW3D2/IRenderBackend.h"

class DX8Backend : public IRenderBackend
{
public:
    DX8Backend();
    virtual ~DX8Backend() override;

    virtual bool Is_Device_Lost() const override;
    virtual bool Has_Stencil() override;
    virtual WW3DFormat Get_Back_Buffer_Format() override;
    virtual void Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit) override;

    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame) override;
    virtual void Flip_To_Primary() override;
    virtual void Clear(bool clear_color, bool clear_z_stencil,
                       const Vector3 & color,
                       float dest_alpha, float z, unsigned int stencil) override;
    virtual void Set_Viewport(const RenderBackendViewport & viewport) override;

    virtual void Set_Vertex_Buffer(const VertexBufferClass * vb, unsigned int stream) override;
    virtual void Set_Vertex_Buffer(const DynamicVBAccessClass & vba) override;
    virtual void Set_Index_Buffer(const IndexBufferClass * ib, unsigned int index_base_offset) override;
    virtual void Set_Index_Buffer(const DynamicIBAccessClass & iba, unsigned int index_base_offset) override;
    virtual void Set_Index_Buffer_Index_Offset(unsigned int offset) override;

    virtual void Set_Shader(const ShaderClass & shader) override;
    virtual void Get_Shader(ShaderClass & shader) override;
    virtual void Set_Material(const VertexMaterialClass * material) override;
    virtual void Set_Texture(unsigned int stage, TextureBaseClass * texture) override;
    virtual void Apply_Render_State_Changes() override;
    virtual void Apply_Default_State() override;
    virtual void Invalidate_Cached_Render_States() override;

    virtual void Set_Transform(TransformKind transform, const Matrix4x4 & m) override;
    virtual void Set_Transform(TransformKind transform, const Matrix3D & m) override;
    virtual void Get_Transform(TransformKind transform, Matrix4x4 & m) override;
    virtual void Set_World_Identity() override;
    virtual void Set_View_Identity() override;
    virtual bool Is_World_Identity() override;
    virtual bool Is_View_Identity() override;
    virtual void Set_Projection_Transform_With_Z_Bias(const Matrix4x4 & matrix, float znear, float zfar) override;

    virtual void Set_Light(unsigned int index, const LightClass & light) override;
    virtual void Set_Ambient(const Vector3 & color) override;
    virtual const Vector3 & Get_Ambient() const override;
    virtual void Set_Fog(bool enable, const Vector3 & color, float start, float end) override;
    virtual bool Get_Fog_Enable() const override;
    virtual void Set_Light_Environment(LightEnvironmentClass * light_env) override;
    virtual LightEnvironmentClass * Get_Light_Environment() const override;

    virtual void Draw_Triangles(unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count) override;
    virtual void Draw_Triangles(unsigned int buffer_type,
                                unsigned int start_index,
                                unsigned int polygon_count,
                                unsigned int min_vertex_index,
                                unsigned int vertex_count) override;
    virtual void Draw_Strip(unsigned int start_index,
                            unsigned int index_count,
                            unsigned int min_vertex_index,
                            unsigned int vertex_count) override;

    virtual void Set_Vertex_Shader(unsigned int vertex_shader) override;
    virtual void Set_Pixel_Shader(unsigned int pixel_shader) override;
    virtual void Set_Vertex_Shader_Constant(int reg, const void * data, int count) override;
    virtual void Set_Pixel_Shader_Constant(int reg, const void * data, int count) override;

    virtual TextureClass * Create_Render_Target(int width, int height, WW3DFormat format) override;
    virtual void Set_Render_Target_With_Z(TextureClass * texture, ZTextureClass * ztexture) override;
    virtual bool Is_Render_To_Texture() override;
    virtual void Set_Shadow_Map(int idx, ZTextureClass * ztex) override;
    virtual ZTextureClass * Get_Shadow_Map(int idx) override;
};
