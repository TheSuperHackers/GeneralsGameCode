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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/shd7bumpspec.cpp                           $*
 *                                                                                             *
 *                       $Author:: Kenny_m
 *																																	
 *								$Modtime:: 6/06/02 11:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 * 06/06/02 KM added software vertex shader fallback check
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "assetmgr.h"
#include "rinfo.h"
#include "camera.h"

#include "shdbumpspec.h"
#include "shd7bumpspec.h"
#include "shd7bumpspec_constants.h"
#include "shdclassids.h"

// shader code declarations
#include "shd7bumpspecpass0.vsh_code.h"
#include "shd7bumpspecpass1.vsh_code.h"

ShdHWVertexShader		Shd7BumpSpecClass::Pass_0_Vertex_Shader;
ShdHWVertexShader		Shd7BumpSpecClass::Pass_1_Vertex_Shader;
Matrix4x4				Shd7BumpSpecClass::View_Projection_Matrix;


Shd7BumpSpecClass::Shd7BumpSpecClass(const ShdDefClass* def)
:	ShdInterfaceClass(def,SHDDEF_CLASSID_BUMPSPEC),
	Texture(NULL),
	NormalMap(NULL)
{
	ShdBumpSpecDefClass* Definition=(ShdBumpSpecDefClass*)def;

	Texture=WW3DAssetManager::Get_Instance()->Get_Texture
	(
		Definition->Get_Texture_Name()
	);

	NormalMap=WW3DAssetManager::Get_Instance()->Get_Texture
	(
		Definition->Get_Bump_Map_Name()
	);

	const Vector3& a=Definition->Get_Ambient();
	Ambient.Set(a.X,a.Y,a.Z,1.0f);

	const Vector3& d=Definition->Get_Diffuse();
	Diffuse.Set(d.X,d.Y,d.Z,1.0f);

	const Vector3& s=Definition->Get_Specular();
	Specular.Set(s.X,s.Y,s.Z,1.0f);

	const Vector2& db=Definition->Get_Diffuse_Bumpiness();
	const Vector2& ds=Definition->Get_Specular_Bumpiness();
	Bumpiness.Set(db.X,db.Y,ds.X,ds.Y);
}

Shd7BumpSpecClass::~Shd7BumpSpecClass()
{
	REF_PTR_RELEASE(Texture);
	REF_PTR_RELEASE(NormalMap);
}

void Shd7BumpSpecClass::Init()
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

	Pass_0_Vertex_Shader.Create
	(
		shd7bumpspecpass0_vsh_code,
		vertex_shader_declaration
	);

	Pass_1_Vertex_Shader.Create
	(
		shd7bumpspecpass1_vsh_code,
		vertex_shader_declaration
	);

}

void Shd7BumpSpecClass::Shutdown()
{
	Pass_0_Vertex_Shader.Destroy();
	Pass_1_Vertex_Shader.Destroy();
}


//**********************************************************************************************
//! Apply shared states for 2 pass DX7 bump specular with gloss map
/*! 5/27/02 5:39p KJM Created
/*! 06/06/02 KM added software vertex shader fallback check
*/
void Shd7BumpSpecClass::Apply_Shared(int pass, RenderInfoClass& rinfo)
{
	// vertex processing behavior
	DX8Wrapper::Set_DX8_Render_State(D3DRS_SOFTWAREVERTEXPROCESSING,!Pass_0_Vertex_Shader.Is_Using_Hardware());

	// fixed function uses pass through by default
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

	if (pass==0)
	{
		// set vertex shader
		DX8Wrapper::Set_Vertex_Shader(Pass_0_Vertex_Shader.Peek_Shader());

		// set texture stage settings
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP,   D3DTOP_DOTPRODUCT3 );
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_SPECULAR );

		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP,   D3DTOP_ADDSMOOTH );
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	}
	else
	{
		// specular pass

		// set vertex shader
		DX8Wrapper::Set_Vertex_Shader(Pass_1_Vertex_Shader.Peek_Shader());

		const Matrix3D& cam=rinfo.Camera.Get_Transform();

		// set constants
		DX8Wrapper::Set_Vertex_Shader_Constant
		(
			CV_EYE_WORLD, 
			D3DXVECTOR4
			(
				cam.Get_X_Translation(),
				cam.Get_Y_Translation(),
				cam.Get_Z_Translation(),
				1.0f
			), 
			1
		);

		// set texture stage settings
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP,   D3DTOP_DOTPRODUCT3 );
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_CURRENT );
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_SPECULAR );

		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ONE);
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

//**********************************************************************************************
//! Apply per instance states for 2 pass DX7 bump specular with gloss map
/*! 5/27/02 5:39p KJM Created
*/
void Shd7BumpSpecClass::Apply_Instance(int cur_pass, RenderInfoClass& rinfo)
{
	DX8Wrapper::Set_Texture(0, NormalMap);
	DX8Wrapper::Set_Texture(1, Texture);

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
		Diffuse,
		Specular
	);

	DX8Wrapper::Set_Vertex_Shader_Constant(CV_BUMPINESS, &Bumpiness, 1);
}

unsigned Shd7BumpSpecClass::Get_Vertex_Stream_Count() const
{
	return 1;
}

unsigned Shd7BumpSpecClass::Get_Vertex_Size(unsigned stream) const
{
	return sizeof(VertexFormatXYZNDUV1TG3);
}

void Shd7BumpSpecClass::Copy_Vertex_Stream
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

