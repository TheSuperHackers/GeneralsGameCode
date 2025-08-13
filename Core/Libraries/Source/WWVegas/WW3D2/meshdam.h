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
 *                     $Archive:: /Commando/Code/ww3d2/meshdam.h                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MESHDAM_H
#define MESHDAM_H

#include "always.h"
#include "vector3.h"
#include "bittype.h"
#include "w3derr.h"

class MeshModelClass;
class ChunkLoadClass;

struct RGBStruct
{
	uint8	R,G,B;
};

struct DamageVertexStruct
{
	int		VertexIdx;		// index of the vertex to "damage"
	Vector3	Vertex0;			// original vertex position
	Vector3	Vertex1;			// damaged vertex position
};

struct DamageColorStruct
{
	int			VertexIdx;	// index of the vertex to damage.
	RGBStruct	Color0;		// original color.
	RGBStruct	Color1;		// damaged color.
};


/*
** DamageClass - this class encapsulates the information needed
** to apply damage to meshes.  It contains replacement vertices,
** vertex colors and materials
*/
class DamageClass
{
public:

	DamageClass(void);
	~DamageClass(void);

	WW3DErrorType				Load_W3D(ChunkLoadClass & cload,MeshModelClass * basemesh);

protected:

	WW3DErrorType				read_vertices(ChunkLoadClass & cload,MeshModelClass * basemesh);
	WW3DErrorType				read_colors(ChunkLoadClass & cload,MeshModelClass * basemesh);

	int							DamageIndex;
	int							NumMaterials;
	int							NumVerts;
	int							NumColors;

	DamageVertexStruct *		Verts;
	DamageColorStruct *		Colors;

	friend class MeshClass;
};


#endif


#endif //0
