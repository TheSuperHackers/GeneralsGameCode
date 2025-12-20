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

/* $Header: /Commando/Code/Tools/max2w3d/vxllayer.h 3     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vxllayer.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/26/97 1:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>


#ifndef BITTYPE_H
#include "BITTYPE.h"
#endif

#ifndef NODELIST_H
#include "nodelist.h"
#endif

const sint8 VOXEL_VISIBLE = 0;			// voxels that are "outside" the object
const sint8 VOXEL_SOLID = 1;				// voxels that are part of the object
const sint8 VOXEL_UNKNOWN = -1;			// either inside or outside, don't know yet
const int max_bitmap_width = 256;
const int max_bitmap_height = 256;


class VoxelLayerClass
{
public:

	VoxelLayerClass();

	VoxelLayerClass
	(
		INodeListClass		& objectlist,
		TimeValue			time,
		Matrix3				parenttm,
		Point3				offset,
		Point3				scale,
		float					slicez,
		float					sliceh,
		int					bmwidth,
		int					bmheight
	);

	~VoxelLayerClass() {};

	BOOL Is_Visible( int x, int y )
	{
		if (x < 0 || x >= bitmap_width || y < 0 || y >= bitmap_height) {
			return TRUE;
		}

		if (Solid[x][y] == 0) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	BOOL Is_Solid( int x, int y )
	{
		if (x < 0 || x >= bitmap_width || y < 0 || y >= bitmap_height) {
			return FALSE;
		}

		if (Solid[x][y] == VOXEL_SOLID) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	unsigned int Get_Width(void) { return bitmap_width; }
	unsigned int Get_Height(void) { return bitmap_height; }

protected:

	void Set_Visible ( int x, int y )
	{
		if (x >= 0 && x < bitmap_width && y >= 0 && y < bitmap_height &&	Solid[x][y] != VOXEL_SOLID )
		{
			Solid[x][y] = VOXEL_VISIBLE;
		}
	}

	void Add_Solid(int x,int y)
	{
		// check if the point is outside the bitmap:
		if (x >= 0 && x < bitmap_width && y >= 0 && y < bitmap_height) {
			Solid[x][y] = VOXEL_SOLID;
		}
	}

	// draw a line of voxels into the bitmap
	void Draw_Line(double x0,double y0,double x1,double y1);

	// draw the intersection between the triangle and the voxel plane
	void Intersect_Triangle(Point3 a,Point3 b,Point3 c,float z);

	// scan convert the polygon fragment in this voxel slab
	void Scan_Triangle(Point3 a,Point3 b,Point3 c);

	sint8		Solid[max_bitmap_width][max_bitmap_height];

	float		SliceZ;
	float		SliceH;
	float		SliceZ0;
	float		SliceZ1;

	int		bitmap_width;
	int		bitmap_height;
};
