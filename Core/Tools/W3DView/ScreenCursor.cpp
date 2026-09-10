import Graphics.Frame.AttachmentBindings;
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
#include "WW3D2/WW3D.h"
import Graphics.Materials.State;
#include "WW3D2/Scene.h"
#include "WW3D2/RInfo.h"
#include "WW3D2/Texture.h"
#include <array>
#include "WWMath/matrix4.h"
import Graphics.Scene.Surfaces.Geometry;


///////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::ScreenCursorClass ()
	:	m_ScreenPos (0, 0),
		m_Width (0),
		m_Height (0),
		m_hWnd (nullptr)
{
	Initialize ();
}


///////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::ScreenCursorClass (const ScreenCursorClass &src)
	:	m_ScreenPos (0, 0),
		m_hWnd (nullptr),
		m_Width (0),
		m_Height (0),
		RenderObjClass (src)
{
	Initialize ();
}


///////////////////////////////////////////////////////////////////
//
//	~ScreenCursorClass
//
///////////////////////////////////////////////////////////////////
ScreenCursorClass::~ScreenCursorClass ()
{
}


///////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Initialize ()
{
	// Create default vertex material

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
}


///////////////////////////////////////////////////////////////////
//
//	Set_Texture
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Set_Texture (TextureClass *texture)
{
	m_pTexture.Assign_Add_Ref (texture);

	// Find the dimensions of the texture:
	if (m_pTexture != nullptr) {
		m_Width	= m_pTexture->Get_Width();
		m_Height	= m_pTexture->Get_Height();
	}
}


///////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::On_Frame_Update ()
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
	const auto& screen = Graphics::Get_Attachment_Bindings().Default().viewport;
	const unsigned screen_cx = screen.width;
	const unsigned screen_cy = screen.height;

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
}


///////////////////////////////////////////////////////////////////
//
//	Render
//
///////////////////////////////////////////////////////////////////
void
ScreenCursorClass::Render (RenderInfoClass &rinfo)
{
    std::array<Graphics::SurfaceVertex, 4> vertices{};
    std::array<unsigned, 6> indices{};
    for (unsigned i = 0; i < vertices.size(); ++i) {
        auto& vertex = vertices[i];
        vertex.position[0] = m_Verticies[i].X;
        vertex.position[1] = m_Verticies[i].Y;
        vertex.position[2] = m_Verticies[i].Z;
        vertex.color = {1,1,1,1};
        vertex.uv[0] = m_UVs[i].X;
        vertex.uv[1] = m_UVs[i].Y;
    }
    for (unsigned i = 0; i < 2; ++i)
        for (unsigned corner = 0; corner < 3; ++corner)
            indices[i * 3 + corner] = m_Triangles[i][corner];
    if (!Draw_Graphics_Prelit_Geometry(vertices, indices, Matrix4x4(true),
        Graphics::MaterialState::ATestBlend2D(), m_pTexture.Peek()))
        DEBUG_LOG(("Viewer cursor graphics submission failed.\n"));

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
}
