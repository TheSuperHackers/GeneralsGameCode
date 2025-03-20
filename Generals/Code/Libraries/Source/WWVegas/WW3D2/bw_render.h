/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/bw_render.h                            $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/26/01 4:53p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BW_RENDER_H__
#define BW_RENDER_H__

#include "always.h"
#include "vector2.h"
#include "vector3.h"
#include "Vector3i.h"

class BW_Render
{
	// Internal pixel buffer used by the triangle renderer
	// The buffer is not allocated or freed by this class.
	class Buffer
	{
		unsigned char* buffer;
		int scale;
		int minv;
		int maxv;
	public:
		Buffer(unsigned char* buffer, int scale);
		~Buffer();

		void Set_H_Line(int start_x, int end_x, int y);
		void Fill(unsigned char c);
		inline int Scale() const { return scale; }
	} pixel_buffer;

	Vector2* vertices;

	void Render_Preprocessed_Triangle(Vector3& xcf,Vector3i& yci);

public:
	BW_Render(unsigned char* buffer, int scale);
	~BW_Render();

	void Fill(unsigned char c);
	void Set_Vertex_Locations(Vector2* vertices,int count); // Warning! Contents are modified!
	void Render_Triangles(const unsigned long* indices,int index_count);
	void Render_Triangle_Strip(const unsigned long* indices,int index_count);
};


#endif // BW_RENDER_H__
