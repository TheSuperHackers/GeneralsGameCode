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
 *                     $Archive:: wwshade/shd8bumpdiff.cpp                           $*
 *                                                                                             *
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 07/08/02 3:40p                                               $*
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
#include "shdmesh.h"
#include "texproject.h"

#include "shdbumpdiff.h"
#include "shd8bumpdiff.h"
#include "shd8bumpdiff_constants.h"
#include "shdclassids.h"

// shader code declarations
#include "shd8bumpdiff.vsh_code.h"
#include "shd8bumpdiff.psh_code.h"
#include "shd8ssbumpdiff.vsh_code.h"
#include "shd8ssbumpdiff.psh_code.h"


ShdHWVertexShader		Shd8BumpDiffClass::Vertex_Shader;
ShdHWPixelShader		Shd8BumpDiffClass::Pixel_Shader;
ShdHWPixelShader		Shd8BumpDiffClass::Self_Shadow_Pixel_Shader;
ShdHWVertexShader		Shd8BumpDiffClass::Self_Shadow_Vertex_Shader;
Matrix4x4				Shd8BumpDiffClass::View_Projection_Matrix;

Shd8BumpDiffClass::Shd8BumpDiffClass(const ShdDefClass* def)
:	ShdInterfaceClass(def,SHDDEF_CLASSID_BUMPDIFF),
	Texture(NULL),
	NormalMap(NULL)
{
	ShdBumpDiffDefClass* Definition=(ShdBumpDiffDefClass*)def;

	Texture=WW3DAssetManager::Get_Instance()->Get_Texture
	(
		Definition->Get_Texture_Name()
	);

	NormalMap=WW3DAssetManager::Get_Instance()->Get_Texture
	(
		Definition->Get_Bump_Map_Name()
	);

	const Vector3& a=Definition->Get_Ambient();
	Ambient.Set(a.X,a.Y,a.Z,0.0f);

	const Vector3& d=Definition->Get_Diffuse();
	Diffuse.Set(d.X,d.Y,d.Z,0.0f);

	const Vector2& db=Definition->Get_Diffuse_Bumpiness();
	Bumpiness.Set(db.X,db.Y,0,0);
}

Shd8BumpDiffClass::~Shd8BumpDiffClass(void)
{
	REF_PTR_RELEASE(Texture);
	REF_PTR_RELEASE(NormalMap);
}

void Shd8BumpDiffClass::Init(void)
{
	// Create vertex shader
	DWORD vertex_shader_declaration[]=
	{
		D3DVSD_STREAM(0),
		(D3DVSD_REG(0, D3DVSDT_FLOAT3)), // vertex position
		(D3DVSD_REG(1, D3DVSDT_FLOAT3)), // vertex normal
		(D3DVSD_REG(2, D3DVSDT_D3DCOLOR)), // vertex color
		(D3DVSD_REG(3, D3DVSDT_FLOAT2)), // vertex texture coords
		(D3DVSD_REG(4, D3DVSDT_FLOAT3)), // vertex S basis
		(D3DVSD_REG(5, D3DVSDT_FLOAT3)), // vertex T basis
		(D3DVSD_REG(6, D3DVSDT_FLOAT3)), // vertex SxT basis
		D3DVSD_END()
	};

	Pixel_Shader.Create(shd8bumpdiff_psh_code);

	Vertex_Shader.Create
	(
		shd8bumpdiff_vsh_code,
		vertex_shader_declaration
	);

	Self_Shadow_Pixel_Shader.Create(shd8ssbumpdiff_psh_code);
	Self_Shadow_Vertex_Shader.Create
	(
		shd8ssbumpdiff_vsh_code,
		vertex_shader_declaration
	);
}

void Shd8BumpDiffClass::Shutdown(void)
{
	Vertex_Shader.Destroy();
	Pixel_Shader.Destroy();
	Self_Shadow_Pixel_Shader.Destroy();
	Self_Shadow_Vertex_Shader.Destroy();
}

void Shd8BumpDiffClass::Apply_Shared(int cur_pass, RenderInfoClass& rinfo)
{
	// set vertex shader
	if (cur_pass==0)
	{
		DX8Wrapper::Set_Vertex_Shader(Vertex_Shader.Peek_Shader());
	}
	else
	{
		DX8Wrapper::Set_Vertex_Shader(Self_Shadow_Vertex_Shader.Peek_Shader());
	}

	// set vertex shader constants
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_CONST, D3DXVECTOR4(0.0f, 1.0f, 0.5f, 2.0f), 1);

	// set pixel shader
	if (cur_pass==0)
	{
		DX8Wrapper::Set_Pixel_Shader(Pixel_Shader.Peek_Shader());
	}
	else
	{
		DX8Wrapper::Set_Pixel_Shader(Self_Shadow_Pixel_Shader.Peek_Shader());
	}

	// set constants
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_CONST, D3DXVECTOR4(0.0f, 1.0f, 0.5f, 2.0f), 1);

	// calculate shader view projection matrix
	Matrix4x4 view_matrix;
	DX8Wrapper::Get_Transform(D3DTS_VIEW, view_matrix);

	Matrix4x4 proj_matrix;
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION, proj_matrix);

	Matrix4x4::Multiply(proj_matrix, view_matrix, &View_Projection_Matrix);
}

void Shd8BumpDiffClass::Apply_Instance(int cur_pass, RenderInfoClass& rinfo)
{
	if (cur_pass==0)
	{
		DX8Wrapper::Set_Texture(0, NormalMap);
		DX8Wrapper::Set_Texture(1, Texture);
	}
	else
	{
		ZTextureClass* ztex=DX8Wrapper::Get_Shadow_Map(0);


		DX8Wrapper::Set_Texture(0, Texture);

		DX8Wrapper::Set_Vertex_Shader_Constant(CV_TEXMAP,&Self_Shadow_Transform,4);
	}

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

	DX8Wrapper::Set_Vertex_Shader_Constant(CV_BUMPINESS, &Bumpiness, 1);
}

unsigned Shd8BumpDiffClass::Get_Vertex_Stream_Count() const
{
	return 1;
}

unsigned Shd8BumpDiffClass::Get_Vertex_Size(unsigned stream) const
{
	return sizeof(VertexFormatXYZNDUV1TG3);
}

void Shd8BumpDiffClass::Copy_Vertex_Stream
(
	unsigned stream, 
	void* dest_buffer, 
	const VertexStreamStruct& vss, 
	unsigned vertex_count
)
{
   VertexFormatXYZNDUV1TG3* verts=(VertexFormatXYZNDUV1TG3*)dest_buffer;

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

		if (vss.S)
		{
			verts[i].Sx=vss.S[i].X;
			verts[i].Sy=vss.S[i].Y;
			verts[i].Sz=vss.S[i].Z;
		}
		else
		{
			verts[i].Sx=0.0f;
			verts[i].Sy=0.0f;
			verts[i].Sz=0.0f;
		}

		if (vss.T)
		{
			verts[i].Tx=vss.T[i].X;
			verts[i].Ty=vss.T[i].Y;
			verts[i].Tz=vss.T[i].Z;
		}
		else
		{
			verts[i].Tx=0.0f;
			verts[i].Ty=0.0f;
			verts[i].Tz=0.0f;
		}

		if (vss.SxT)
		{
			verts[i].SxTx=vss.SxT[i].X;
			verts[i].SxTy=vss.SxT[i].Y;
			verts[i].SxTz=vss.SxT[i].Z;
		}
		else
		{
			verts[i].SxTx=0.0f;
			verts[i].SxTy=0.0f;
			verts[i].SxTz=0.0f;
		}
	}
}

bool Shd8BumpDiffClass::Pass_Selection
(
	ShdMeshClass* mesh, 
	RenderInfoClass* rinfo,
	int pass
)
{
	if (mesh->Is_Applying_Shadow_Map())
	{
		if (pass==1) // just do once
		{
			// is rendering self shadowed object
			Setup_Self_Shadow_Info(*mesh,*rinfo);
		}
		return (pass==1);
	}

	return (pass==0);
}


/**********************************************************************************************
//! Set up transform for shadow maps
/*! 06/07/02 KM created
*/
void Shd8BumpDiffClass::Setup_Self_Shadow_Info(ShdMeshClass& mesh, RenderInfoClass& rinfo)
{
	Matrix4x4 tex_mat=mesh.Peek_Texture_Projector()->Peek_Mapper()->Get_Texture_Transform();

	//set special texture matrix for shadow mapping
	ZTextureClass* smap=DX8Wrapper::Get_Shadow_Map(0); // todo KJM assign shadow map from rinfo

	if (!smap) return;

	float off_x=0.5f+(0.5f/(float)smap->Get_Width());
	float off_y=0.5f+(0.5f/(float)smap->Get_Height());

	unsigned int bits=Get_Num_Depth_Bits(smap->Get_Texture_Format());
	float range=(float)(0xFFFFFFFF>>(32-bits));
	float bias=0.0f;//-0.001f*range;
	Matrix4x4 sb_mat
	(
		Vector4(0.5f,     0.0f,     0.0f,	0.0f),
		Vector4(0.0f,    -0.5f,     0.0f,	0.0f),
		Vector4(0.0f,     0.0f,    range,	0.0f),
		Vector4(off_x,	  off_y,	    bias,	1.0f) 
	);

	Matrix4x4 view2tex;
	Matrix4x4::Multiply(sb_mat,tex_mat,&view2tex);

	Matrix4x4 world;
	world.Init(mesh.Get_Transform());

	Matrix4x4 view;
	view.Init(rinfo.Camera.Get_View_Matrix());

	Matrix4x4 vw_mat;
	Matrix4x4::Multiply(world,view,&vw_mat);

	Matrix4x4::Multiply(view2tex,vw_mat,&Self_Shadow_Transform);
}
