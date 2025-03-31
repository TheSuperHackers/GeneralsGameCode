/*
**	Command & Conquer Generals Zero Hour(tm)
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

 /***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shd6bumpdiff.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 7/11/02 10:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "assetmgr.h"
#include "rinfo.h"
#include "camera.h"

#include "shdbumpdiff.h"
#include "shd6bumpdiff.h"
#include "shd6bumpdiff_constants.h"
#include "shdclassids.h"

// shader code declarations
#include "shd6bumpdiff.vsh_code.h"


ShdHWVertexShader		Shd6BumpDiffClass::Vertex_Shader;
Matrix4x4				Shd6BumpDiffClass::View_Projection_Matrix;

Shd6BumpDiffClass::Shd6BumpDiffClass(const ShdDefClass* def)
:	ShdInterfaceClass(def,SHDDEF_CLASSID_BUMPDIFF),
	Texture(NULL)
{
	ShdBumpDiffDefClass* Definition=(ShdBumpDiffDefClass*)def;

	Texture=WW3DAssetManager::Get_Instance()->Get_Texture
	(
		Definition->Get_Texture_Name()
	);

	const Vector3& a=Definition->Get_Ambient();
	Ambient.Set(a.X,a.Y,a.Z,1.0f);

	const Vector3& d=Definition->Get_Diffuse();
	Diffuse.Set(d.X,d.Y,d.Z,1.0f);
}

Shd6BumpDiffClass::~Shd6BumpDiffClass()
{
	REF_PTR_RELEASE(Texture);
}

void Shd6BumpDiffClass::Init()
{
	// Create vertex shader
	DWORD vertex_shader_declaration[]=
	{
		D3DVSD_STREAM(0),
		(D3DVSD_REG(0, D3DVSDT_FLOAT3)), // vertex position
		(D3DVSD_REG(1, D3DVSDT_FLOAT3)), // vertex normal
		(D3DVSD_REG(2, D3DVSDT_D3DCOLOR)), // vertex color
		(D3DVSD_REG(3, D3DVSDT_FLOAT2)), // vertex texture coords
		D3DVSD_END()
	};

	Vertex_Shader.Create
	(
		shd6bumpdiff_vsh_code,
		vertex_shader_declaration
	);
}

void Shd6BumpDiffClass::Shutdown()
{
	Vertex_Shader.Destroy();
}


/**********************************************************************************************
//! Apply shared states for 1 pass DX6 (no bump)
/*! 7/10/02 5:39p KJM Created
*/
void Shd6BumpDiffClass::Apply_Shared(int pass, RenderInfoClass& rinfo)
{
	// vertex processing behavior
	DX8Wrapper::Set_DX8_Render_State(D3DRS_SOFTWAREVERTEXPROCESSING,!Vertex_Shader.Is_Using_Hardware());

	// fixed function uses pass through by default
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

	// set vertex shader
	DX8Wrapper::Set_Vertex_Shader(Vertex_Shader.Peek_Shader());

	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	// set constants
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_CONST, D3DXVECTOR4(0.0f, 1.0f, 0.5f, 2.0f), 1);

	// calculate shader view projection matrix
	Matrix4x4 view_matrix;
	DX8Wrapper::Get_Transform(D3DTS_VIEW, view_matrix);

	Matrix4x4 proj_matrix;
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION, proj_matrix);

	Matrix4x4::Multiply(proj_matrix, view_matrix, &View_Projection_Matrix);
}

//**********************************************************************************************
//! Apply per instance states for 1 pass DX6 specular with gloss map (no bump)
/*! 7/10/02 5:39p KJM Created
*/
void Shd6BumpDiffClass::Apply_Instance(int cur_pass, RenderInfoClass& rinfo)
{
	DX8Wrapper::Set_Texture(0, Texture);

	// set vertex shader constants
	Matrix4x4 world;
	DX8Wrapper::Get_Transform(D3DTS_WORLD, world);

	Matrix4x4 world_view_proj_matrix;

	Matrix4x4::Multiply(View_Projection_Matrix,world,&world_view_proj_matrix);

	DX8Wrapper::Set_Vertex_Shader_Constant(CV_WORLD_VIEW_PROJECTION, &world_view_proj_matrix, 4);
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_WORLD, &world, 4);

	ShdHWVertexShader::Light
	(
		rinfo,
		Ambient,
		Diffuse
	);
}

unsigned Shd6BumpDiffClass::Get_Vertex_Stream_Count() const
{
	return 1;
}

unsigned Shd6BumpDiffClass::Get_Vertex_Size(unsigned stream) const
{
	return sizeof(VertexFormatXYZNDUV1);
}

void Shd6BumpDiffClass::Copy_Vertex_Stream
(
	unsigned stream, 
	void* dest_buffer, 
	const VertexStreamStruct& vss, 
	unsigned vertex_count
)
{
   VertexFormatXYZNDUV1* verts=(VertexFormatXYZNDUV1*)dest_buffer;

	for (unsigned i=0; i<vertex_count; ++i) 
	{
		if (vss.Locations) 
		{
			verts[i].x=vss.Locations[i][0];
			verts[i].y=vss.Locations[i][1];
			verts[i].z=vss.Locations[i][2];
		}
		else 
		{
			verts[i].x=0.0f;
			verts[i].y=0.0f;
			verts[i].z=0.0f;
		}

		if (vss.DiffuseInt) 
		{
			verts[i].diffuse=vss.DiffuseInt[i];
		}
		else 
		{
			verts[i].diffuse=0xffffffff;
		}
	
		if (vss.Normals) 
		{
			verts[i].nx=vss.Normals[i][0];
			verts[i].ny=vss.Normals[i][1];
			verts[i].nz=vss.Normals[i][2];
		}
		else 
		{
			verts[i].nx=0.0f;
			verts[i].ny=0.0f;
			verts[i].nz=0.0f;
		}

		if (vss.UV[0])
		{
			verts[i].u1=vss.UV[0][i].U;
			verts[i].v1=vss.UV[0][i].V;
		}
		else 
		{
			verts[i].u1=0.0f;
			verts[i].v1=0.0f;
		}
	}
}

