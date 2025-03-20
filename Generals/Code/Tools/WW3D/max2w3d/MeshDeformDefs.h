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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : MeshDeformDefs.h                                             * 
 *                                                                                             * 
 *                   Programmer : Patrick Smith                                                * 
 *                                                                                             * 
 *                   Start Date : 04/28/99                                                     * 
 *                                                                                             * 
 *                  Last Update : 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __MESH_DEFORM_DEFS_H
#define __MESH_DEFORM_DEFS_H

#include <max.h>
#include "Vector.H"

///////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////
typedef enum
{
	VERT_POSITION		= 1,
	VERT_COLORS			= 2,
	BOTH					= VERT_POSITION | VERT_COLORS
} DEFORM_CHANNELS;


///////////////////////////////////////////////////////////////////////////
//
//	Structures
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
//	VERT_INFO
//
//	Used to represent position or color information for a vertex.
//
///////////////////////////////////////////////////////////////////////////
typedef struct _VERT_INFO
{
	_VERT_INFO (void)
		:	index (0),
			color_index (0),
			value (0,0,0)				{ }

	_VERT_INFO (int vert_index, const Point3 &point, int vert_color_index = 0)
		:	index (vert_index),
			color_index (vert_color_index),
			value (point)				{ }

	UINT			index;
	UINT			color_index;
	Point3		value;

	// Don't care, DynamicVectorClass needs these
	bool operator== (const _VERT_INFO &src) { return false; }
	bool operator!= (const _VERT_INFO &src) { return true; }
} VERT_INFO;


///////////////////////////////////////////////////////////////////////////
//
//	Typedefs
//
///////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<VERT_INFO> DEFORM_LIST;


#endif //__MESH_DEFORM_DEFS_H

