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
 *                     $Archive:: /Commando/Code/Tools/W3DView/ScreenCursor.h                                                                                                                                                                                                                                                                                                                               $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __SCREENCURSOR_H
#define __SCREENCURSOR_H

#include "resource.h"
#include "rendobj.h"
#include "Vector3i.h"

// Forward declarations
class VertexMaterialClass;

///////////////////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////////////////
class ScreenCursorClass : public RenderObjClass
{
	public:

		////////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		////////////////////////////////////////////////////////////////////////
		ScreenCursorClass (void);
		ScreenCursorClass (const ScreenCursorClass &src);
		virtual ~ScreenCursorClass (void);

		////////////////////////////////////////////////////////////////////////
		//	Public operators
		////////////////////////////////////////////////////////////////////////
		const ScreenCursorClass &operator= (const ScreenCursorClass &src);

		////////////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////////////
		void						Set_Window (HWND hwnd)						{ m_hWnd = hwnd; }
		void						Set_Texture (TextureClass *texture);

		////////////////////////////////////////////////////////////////////////
		//	Base class overrides
		////////////////////////////////////////////////////////////////////////
		RenderObjClass *		Clone (void) const								{ return new ScreenCursorClass (*this); }
		virtual int				Class_ID(void) const								{ return CLASSID_LAST + 103L; }
		virtual void			Render (RenderInfoClass &rinfo);
		virtual void			On_Frame_Update (void);
		virtual void			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
		virtual void			Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

		virtual void			Notify_Added(SceneClass * scene);
		virtual void			Notify_Removed(SceneClass * scene);

	protected:

		////////////////////////////////////////////////////////////////////////
		//	Protected methods
		////////////////////////////////////////////////////////////////////////
		void						Initialize (void);

	private:

		////////////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////////////
		HWND						m_hWnd;
		Vector2					m_ScreenPos;
		TextureClass *			m_pTexture;
		VertexMaterialClass *m_pVertMaterial;

		Vector3					m_Verticies[4];
		Vector3					m_Normals[4];
		Vector3i					m_Triangles[2];
		Vector2					m_UVs[4];

		int 						m_Width;
		int						m_Height;
};


#endif //__SCREENCURSOR_H
