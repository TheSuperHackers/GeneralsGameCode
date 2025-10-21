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

/***************************************************************************
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : G                                        *
 *                                                                         *
 *                     $Archive:: /Commando/Code/ww3d2/line3d.h           $*
 *                                                                         *
 *                      $Author:: Hector_y                                $*
 *                                                                         *
 *                     $Modtime:: 2/16/01 3:52p                           $*
 *                                                                         *
 *                    $Revision:: 2                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef LINE3D_H
#define LINE3D_H

#include "always.h"
#include "rendobj.h"
#include "vector3.h"
#include "vector4.h"
#include "shader.h"

class	VertexMaterialClass;
class RenderInfoClass;

/*
** Line3DClass -- Render3DObject for rendering 3D line segments.
** These are conceptually cylinders with a given width - some approximation
** of this will be rendered. (The current approximation assumes that
** Line3DCLass objects are unlit, therefore only the sihouette needs to be
** approximated).
*/
class Line3DClass : public W3DMPO, public RenderObjClass
{
	W3DMPO_GLUE(Line3DClass)

	public:

		Line3DClass (const Vector3 & start, const Vector3 & end, float width,
			float r, float g, float b, float opacity = 1.0f);
		Line3DClass(const Line3DClass & src);
		Line3DClass & operator = (const Line3DClass & that);
		virtual ~Line3DClass(void);
		virtual RenderObjClass * Clone(void) const;

		// class id of this render object
		virtual int Class_ID(void) const { return CLASSID_LINE3D; }

		virtual void		Render(RenderInfoClass & rfinfo);

		// scale the 3D line symmetrically about its center.
		virtual void		Scale(float scale);
		virtual void		Scale(float scalex, float scaley, float scalez);

		// returns the number of polygons in the render object
		virtual int Get_Num_Polys(void) const;

      // Get the object space bounding volumes
      virtual void	Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
      virtual void	Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

		// The following functions are unique to Line3DClass:

		// Reset the line start and end points
		void Reset(const Vector3 & new_start, const Vector3 & new_end);

		// Reset line start and end points, and the line width
		void Reset(const Vector3 & new_start, const Vector3 & new_end, float new_width);

		// Reset the line color
		void Re_Color(float r, float g, float b);

		// Reset the line opacity
		void Set_Opacity(float opacity);

		// For non-opaque lines, allow them to render last.
		void							Set_Sort_Level(int level)									{ SortLevel = level; }
		int							Get_Sort_Level(void) const									{ return SortLevel; }

	protected:

		// This is kept to facilitate changing the line endpoints.
		float Length;

		// This is kept to facilitate changing the line width.
		float Width;

		// shader
		ShaderClass				Shader;
		// vertices
		Vector3						vert[8];
		// color
		Vector4						Color;
		char														SortLevel;
};

#endif
