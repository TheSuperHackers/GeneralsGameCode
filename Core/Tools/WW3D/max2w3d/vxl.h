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

/* $Header: /Commando/Code/Tools/max2w3d/vxl.h 4     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vxl.h                          $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/26/97 1:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ALWAYS_H
#include "always.h"
#endif

#include <max.h>

#ifndef NODELIST_H
#include "nodelist.h"
#endif

#ifndef VXLLAYER_H
#include "vxllayer.h"
#endif

#ifndef PROGRESS_H
#include "PROGRESS.H"
#endif


/*
	This class is used to compute approximate physical properties of a polygon
	mesh or meshes.  Some of these properties are listed below:

		- Moment of Inertia Tensor (with density factored out)
		- Center of Mass
		- Volume

	It is a cannibalized version of the code from the voxel plugin.
*/

class VoxelClass
{
public:

	VoxelClass(
		INodeListClass &				meshlist,
		int								resolution,
		Matrix3							parenttm,
		TimeValue						time,
		Progress_Meter_Class &		meter
	);

	~VoxelClass(void);

	int		Get_Width() { return XDim; }
	int		Get_Height() { return YDim; }
	int		Num_Layers() { return ZDim; }
	uint8		Is_Solid(int i,int j,int k);

	void		Compute_Physical_Properties(double Volume[1],double CM[3],double I[9]);

private:

	int		XDim;
	int		YDim;
	int		ZDim;

	double	BlockXDim;
	double	BlockYDim;
	double	BlockZDim;

	unsigned char	* VisData;

	float				Resolution;			// resolution of the voxel grid
	TimeValue		CurTime;
	Point3			Offset;
	Point3			Size;
	Point3			Scale;				// three scale values to fit the meshes into the desired grid

	Point3			BoxCorner[8];		// World-Space corners of the bounding box of the voxel space
	Matrix3			ParentTM;			// coordinate system of the parent of this object.

	void   raw_set_vis(int i,int j,int k,uint8 val);
	uint8  raw_read_vis(int i,int j,int k);

	int    voxel_touches_space(int i,int j,int k);
	void   purge_interior(void);

	void Quantize_Meshes
	(
		INodeListClass &			meshlist,
		Progress_Meter_Class	&	meter
	);

	// set one layer of the voxel object
	void Set_Layer
	(
		VoxelLayerClass &		layer,
		uint32					z
	);

	// compute the bounding box
	void Compute_Bounding_Box(Point3 size,Point3 offset);

	// 3D visibility
	void Compute_Visiblity(Progress_Meter_Class & meter);

	// returns the position of the center of voxel(i,j,k)
	Point3 Voxel_Position(int i,int j,int k);
};
