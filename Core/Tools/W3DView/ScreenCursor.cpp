/*
**	Command & Conquer Renegade(tm)
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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/ScreenCursor.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "ScreenCursor.h"
#include "Utils.h"
#include "ww3d.h"
#include "vertmaterial.h"
#include "shader.h"
#include "scene.h"
#include "rinfo.h"
#include "texture.h"
#include "dx8wrapper.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "sortingrenderer.h"


///////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::ScreenCursorClass (void)
	:	m_ScreenPos (0, 0),
		m_pTexture (nullptr),
		m_pVertMaterial (nullptr),
		m_Width (0),
		m_Height (0),
		m_hWnd (nullptr)
{
	Initialize ();
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::ScreenCursorClass (const ScreenCursorClass &src)
	:	m_ScreenPos (0, 0),
		m_pTexture (nullptr),
		m_hWnd (nullptr),
		m_pVertMaterial (nullptr),
		m_Width (0),
		m_Height (0),
		RenderObjClass (src)
{
	Initialize ();
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	~ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::~ScreenCursorClass (void)
{
	REF_PTR_RELEASE (m_pTexture);
	REF_PTR_RELEASE (m_pVertMaterial);
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Initialize (void)
{
	REF_PTR_RELEASE(m_pVertMaterial);

	// Create default vertex material
	m_pVertMaterial = NEW_REF( VertexMaterialClass, ());
	m_pVertMaterial->Set_Diffuse (1.0F, 1.0F, 1.0F);
	m_pVertMaterial->Set_Emissive (0.0F, 0.0F, 0.0F);
	m_pVertMaterial->Set_Specular (1.0F, 1.0F, 1.0F);
	m_pVertMaterial->Set_Ambient (1.0F, 1.0F, 1.0F);

	m_Triangles[0].I = 0;
	m_Triangles[0].J = 1;
	m_Triangles[0].K = 2;
	m_Triangles[1].I = 1;
	m_Triangles[1].J = 2;
	m_Triangles[1].K = 3;

	m_Normals[0].X = 0;
	m_Normals[0].Y = 0;
	m_Normals[0].Z = -1;
	m_Normals[1].X = 0;
	m_Normals[1].Y = 0;
	m_Normals[1].Z = -1;
	m_Normals[2].X = 0;
	m_Normals[2].Y = 0;
	m_Normals[2].Z = -1;
	m_Normals[3].X = 0;
	m_Normals[3].Y = 0;
	m_Normals[3].Z = -1;

	m_UVs[0].X = 0;
	m_UVs[0].Y = 0;
	m_UVs[1].X = 1.0F;
	m_UVs[1].Y = 0;
	m_UVs[2].X = 0;
	m_UVs[2].Y = 1.0F;
	m_UVs[3].X = 1.0F;
	m_UVs[3].Y = 1.0F;
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Set_Texture
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Set_Texture (TextureClass *texture)
{
	REF_PTR_SET (m_pTexture, texture);

	// Find the dimensions of the texture:
	if (m_pTexture != nullptr) {
		m_Width	= m_pTexture->Get_Width();
		m_Height	= m_pTexture->Get_Height();
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::On_Frame_Update (void)
{
	//
	//	Get the current cursor position in screen coords
	//
	POINT point = { 0 };
	::GetCursorPos (&point);

	if (m_hWnd != nullptr) {

		//
		//	Normalize the screen position
		//
		RECT rect = { 0 };
		::GetClientRect (m_hWnd, &rect);
		::ScreenToClient (m_hWnd, &point);
		m_ScreenPos.X = ((float)point.x) / ((float)rect.right);
		m_ScreenPos.Y = ((float)point.y) / ((float)rect.bottom);

	} else {

		//
		//	Normalize the screen position
		//
		m_ScreenPos.X = ((float)point.x) / ((float)::GetSystemMetrics (SM_CXSCREEN));
		m_ScreenPos.Y = ((float)point.y) / ((float)::GetSystemMetrics (SM_CYSCREEN));
	}

	//
	//	Determine the current display resolution
	//
	int screen_cx = 0;
	int screen_cy = 0;
	int bits = 0;
	bool windowed = false;
	WW3D::Get_Device_Resolution (screen_cx, screen_cy, bits, windowed);

	//
	//	Calculate the 3D position
	//
	float normal_width = ((float)m_Width) / (float)screen_cx;
	float normal_height = ((float)m_Height) / (float)screen_cy;
	float x_pos = floor(m_ScreenPos.X * ((float)screen_cx) + 0.5F) / ((float)screen_cx);
	float y_pos = floor(m_ScreenPos.Y * ((float)screen_cy) + 0.5F) / ((float)screen_cy);
	float z_pos = 0;

	//
	//	Convert the 3D position to normalized 'view' coords
	//
	float x_max		= ((x_pos + normal_width) * 2) - 1;
	float y_max		= 1 - ((y_pos + normal_height) * 2);
	x_pos				= (x_pos * 2) - 1;
	y_pos				= 1 - (y_pos * 2);
	z_pos				= 0;

	//
	//	Build the vertices from the position and extents
	//
	m_Verticies[0].X = x_pos;
	m_Verticies[0].Y = y_pos;
	m_Verticies[0].Z = z_pos;

	m_Verticies[1].X = x_max;
	m_Verticies[1].Y = y_pos;
	m_Verticies[1].Z = z_pos;

	m_Verticies[2].X = x_pos;
	m_Verticies[2].Y = y_max;
	m_Verticies[2].Z = z_pos;

	m_Verticies[3].X = x_max;
	m_Verticies[3].Y = y_max;
	m_Verticies[3].Z = z_pos;
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Render
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Render (RenderInfoClass &rinfo)
{
	const int VERTEX_COUNT = 4;
	const int FACE_COUNT = 2;
	/*
	** Dump the vertices into the dynamic sorting vertex buffer.
	*/
	DynamicVBAccessClass vbaccess(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,VERTEX_COUNT);
	{
		DynamicVBAccessClass::WriteLockClass lock(&vbaccess);
		VertexFormatXYZNDUV2* vb=lock.Get_Formatted_Vertex_Array();

		for (int i=0; i<VERTEX_COUNT; i++) {

			// Locations
			vb->x=m_Verticies[i].X;
			vb->y=m_Verticies[i].Y;
			vb->z=m_Verticies[i].Z;

			// Normals
			vb->nx=m_Normals[i].X;
			vb->ny=m_Normals[i].Y;
			vb->nz=m_Normals[i].Z;

			// UV coordinates
			vb->u1=m_UVs[i].X;
			vb->v1=m_UVs[i].Y;

			vb++;
		}
	}

	/*
	** Dump the faces into the dynamic sorting index buffer.
	*/
	DynamicIBAccessClass ibaccess(BUFFER_TYPE_DYNAMIC_SORTING,FACE_COUNT*3);
	{
		DynamicIBAccessClass::WriteLockClass lock(&ibaccess);
		unsigned short * indices = lock.Get_Index_Array();
		for (int i=0; i<FACE_COUNT; i++) {
			indices[3*i+0] = m_Triangles[i][0];
			indices[3*i+1] = m_Triangles[i][1];
			indices[3*i+2] = m_Triangles[i][2];
		}
	}

	/*
	** Apply the shader and material
	*/
	DX8Wrapper::Set_Material(m_pVertMaterial);
	DX8Wrapper::Set_Shader(ShaderClass::_PresetATestBlend2DShader);
	DX8Wrapper::Set_Texture(0,m_pTexture);

	DX8Wrapper::Set_Vertex_Buffer(vbaccess);
	DX8Wrapper::Set_Index_Buffer(ibaccess,0);

	SphereClass sphere;
	Get_Obj_Space_Bounding_Sphere(sphere);

	SortingRendererClass::Insert_Triangles(
		sphere,
		0,
		FACE_COUNT*3,
		0,
		VERTEX_COUNT*2);

	return ;
}


//////////////////////////////////////////////////////////////
//
//	Get_Obj_Space_Bounding_Sphere
//
//////////////////////////////////////////////////////////////
void
ScreenCursorClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center = Get_Transform().Get_Translation();
	sphere.Radius = max (m_Width, m_Height);
}


//////////////////////////////////////////////////////////////
//
//	Get_Obj_Space_Bounding_Box
//
//////////////////////////////////////////////////////////////
void
ScreenCursorClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	Matrix3D transform = Get_Transform ();
	box.Center = transform.Get_Translation ();
	box.Extent.Set(0.1F, m_Width, m_Height);
}


//////////////////////////////////////////////////////////////
//
//	Notify_Added
//
//////////////////////////////////////////////////////////////
void
ScreenCursorClass::Notify_Added (SceneClass * scene)
{
	if (scene != nullptr) {
		scene->Register (this, SceneClass::ON_FRAME_UPDATE);
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//	Notify_Removed
//
//////////////////////////////////////////////////////////////
void
ScreenCursorClass::Notify_Removed (SceneClass * scene)
{
	if (scene != nullptr) {
		scene->Unregister (this, SceneClass::ON_FRAME_UPDATE);
	}

	return ;
}
