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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : MeshDeformData.cpp                                           * 
 *                                                                                             * 
 *                   Programmer : Patrick Smith                                                * 
 *                                                                                             * 
 *                   Start Date : 06/07/99                                                     * 
 *                                                                                             * 
 *                  Last Update : 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __MESH_DEFORM_SAVE_DEFS_H
#define __MESH_DEFORM_SAVE_DEFS_H

#include <max.h>

///////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////
typedef enum
{
	DEFORM_CHUNK_INFO					= 0x000000001,
	DEFORM_CHUNK_SET_INFO,
	DEFORM_CHUNK_KEYFRAME_INFO,
	DEFORM_CHUNK_POSITION_DATA,
	DEFORM_CHUNK_POSITION_VERTS,
	DEFORM_CHUNK_COLOR_DATA,
	DEFORM_CHUNK_COLOR_VERTS
} DEFORM_CHUNK_IDS;

///////////////////////////////////////////////////////////////////////////
//
//	Structures
//
///////////////////////////////////////////////////////////////////////////

//
// Deform information.  Each mesh can have sets of keyframes of
//	deform info associated with it.
// 
struct DeformChunk
{
	uint32					SetCount;
	uint32					reserved[4];
};

//
// Deform set information.  Each set is made up of a series
// of keyframes.
// 
struct DeformChunkSetInfo
{	
	uint32					KeyframeCount;
	uint32					flags;
	uint32					NumVerticies;
	uint32					NumVertexColors;
	uint32					reserved[2];
};

#define DEFORM_SET_MANUAL_DEFORM	0x00000001	// set is isn't applied during sphere or point tests.

//
// Deform keyframe information.  Each keyframe is made up of
// a set of per-vert deform data.
// 
struct DeformChunkKeyframeInfo
{
	float32					DeformPercent;
	uint32					VertexCount;
	uint32					ColorCount;
	uint32					reserved[2];
};

//
// Deform data.  Contains deform information about a vertex
// in the mesh.
// 
struct DeformDataChunk
{
	uint32					VertexIndex;
	uint32					ColorIndex;
	Point3					Value;
	uint32					reserved[2];
};


#endif //__MESH_DEFORM_SAVE_DEFS_H
