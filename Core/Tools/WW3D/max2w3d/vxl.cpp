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

/* $Header: /Commando/Code/Tools/max2w3d/vxl.cpp 4     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G Math Library                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vxl.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/14/97 3:07p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vxl.h"
#include "errclass.h"


/*

	This module will voxelize one or more meshes.  It is only used to compute
	some things like Moment of Inertia and Center of Mass for the object. 
	Much of the code which was doing the shading and lighting computations
	has been stripped out.

*/

static void compute_dimensions(
											INodeListClass &	meshlist,
											const Matrix3 & parenttm,
											TimeValue curtime,
											Point3 *	min,
											Point3 * max
										);

#define VIS_UNKNOWN		0
#define VIS_SOLID			1
#define VIS_VISIBLE		2

/************************************************************************
* VoxelClass Constructor
*
* Voxelize a list of meshes.
*
* INPUTS:
*
* OUTPUS:
*
************************************************************************/
VoxelClass::VoxelClass
(
	INodeListClass &				meshlist,
	int								resolution,
	Matrix3							parenttm,
	TimeValue						time,
	Progress_Meter_Class &		meter
)
{
	Resolution = resolution;
	ParentTM = parenttm;
	CurTime = time;

	XDim = resolution + 1;
	YDim = resolution + 1;
	ZDim = resolution + 1;
	
	// assert that none of the dimensions
	// were too big.  (if this happened, VoxelData is going
	// to be a *HUMONGOUS* amount of memory...)
	assert(XDim < 256);
	assert(YDim < 256);
	assert(ZDim < 256);
	
	// Allocate visibility flags array
	VisData = new uint8[XDim * YDim * ZDim];
	if (VisData == NULL) {
		throw ErrorClass("out of memory!");
	}

	memset(VisData,0,XDim*YDim*ZDim);

	/*
	** compute the two corners of the bounding box of
	** these meshes.  Note that these coordinates are
	** be specified in the space defined by ParentTM.
	*/
	Point3 min;
	Point3 max;
	compute_dimensions(meshlist,ParentTM,CurTime,&min,&max);


	/*
	** The voxelizer uses three values: offset, size,
	** and scale:
	**	
	** offset - the position of the "most negative" corner
	**	size - dimensions of the box
	** scale - scale factor to apply to vertices after they've
	**         been translated by -offset
	**
	*/
	Size = max - min;
	Offset = min;
	Scale.x = Resolution / Size.x;
	Scale.y = Resolution / Size.y;
	Scale.z = Resolution / Size.z;


	/*
	** Dimensions of a single voxel block
	*/
	BlockXDim = Size.x / Resolution;
	BlockYDim = Size.y / Resolution;
	BlockZDim = Size.z / Resolution;

	
	/*
	** Voxelize the meshes!
	*/
	Quantize_Meshes
	(
		meshlist,
		meter
	);

}


/************************************************************************
* VoxelClass Destructor
*
* De-Allocates memory used by the voxel object
*
* INPUTS:
* none
*
* OUTPUS:
* none
*
************************************************************************/
VoxelClass::~VoxelClass()
{
	if (VisData != NULL) delete[] VisData;
}


/************************************************************************
* VoxelClass::Quantize_Meshes
*
* Generataes voxel data from the list of meshes passed in.
*
* INPUTS:
* meshlist - list of meshes to "voxelize"
* meter - progress meter object
*
* OUTPUS:
* none
*
************************************************************************/
void VoxelClass::Quantize_Meshes
(
	INodeListClass & meshlist,
	Progress_Meter_Class & meter
)
{
	/*
	** Progress meter updating
	*/
	meter.Finish_In_Steps(2);
	Progress_Meter_Class slabmeter(meter,meter.Increment);
	slabmeter.Finish_In_Steps(ZDim);
	
	/*
	** Generate the Voxel Layer for each slice of the model
	*/
	float min_z = Offset.z;
	float max_z = Offset.z + Size.z;
	float sliceh = Size.z / (float)ZDim;

	for (int slicecount = 0; slicecount < ZDim; slicecount++ )
	{
		float slicez = min_z + (max_z - min_z) * ((float)slicecount/(float)(ZDim-1));

		VoxelLayerClass * vlayer = new VoxelLayerClass
		(
			meshlist,
			CurTime,
			ParentTM,
			Offset,
			Scale,
			slicez,
			sliceh,
			XDim,
			YDim
		);	 

		Set_Layer(*vlayer,slicecount);

		slabmeter.Add_Increment();
		if (slabmeter.Cancelled()) throw ErrorClass("Export Cancelled");

		delete vlayer;
	}

	meter.Add_Increment();

	// 3D visibility calculations
	Progress_Meter_Class vismeter(meter,meter.Increment);
	Compute_Visiblity(vismeter);

	meter.Add_Increment();

	// Compute the voxel bounding box
	Compute_Bounding_Box(Size,Offset);
}

/************************************************************************
* VoxelClass::Set_Layer
*
* Sets a layer of the Voxel data according to contents of the passed
* bitmap.
*
* INPUTS:
* bitmap - bitmap to use as the source data
* z - z co-ordinate of the layer to set
*
* OUTPUS:
* none
*
************************************************************************/
void VoxelClass::Set_Layer
(
	VoxelLayerClass & vlayer,
	uint32 z
)
{
	// bitmap must have same x&y dimensions as the voxel space
	if (vlayer.Get_Width() != (unsigned)XDim) return;
	if (vlayer.Get_Height() != (unsigned)YDim) return;


	// Copy the solid voxels into our voxel cube
	for (unsigned j=0; j<vlayer.Get_Height(); j++) {
		for (unsigned i=0; i<vlayer.Get_Width(); i++) {
			if (!vlayer.Is_Solid(i,j)) {
				raw_set_vis(i,j,z,VIS_UNKNOWN);
			} else {
				raw_set_vis(i,j,z,VIS_SOLID);
			}				
		}
	}
}

/************************************************************************
* VoxelClass::Compute_Bounding_Box
*
* Given the information which the MeshObjList gives us regarding the
* size and offset of the group of meshes we are processing, compute the
* scaled co-ordinates of the eight corners of the bounding box.
*
* INPUTS:
* size - width, length, and height of the bounding box
* offset - offset to the first point
* scale - scale factors for each axis.
*
* OUTPUS:
* none
*
************************************************************************/
void VoxelClass::Compute_Bounding_Box
(
	Point3 size,
	Point3 offset
)
{
	int i;

	// First we just set the bounding box values in the
	// original scale. The corners must be specified in the
	// order expected.

	// +x +y -z
	BoxCorner[0].x = offset.x + size.x;
	BoxCorner[0].y = offset.y + size.y;
	BoxCorner[0].z = offset.z;
	
	// +x -y -z
	BoxCorner[1].x = offset.x + size.x;
	BoxCorner[1].y = offset.y;
	BoxCorner[1].z = offset.z;

	// -x -y -z
	BoxCorner[2].x = offset.x;
	BoxCorner[2].y = offset.y;
	BoxCorner[2].z = offset.z;
	
	// -x +y -z
	BoxCorner[3].x = offset.x;
	BoxCorner[3].y = offset.y + size.y;
	BoxCorner[3].z = offset.z;
	
	// +x +y +z
	BoxCorner[4].x = offset.x + size.x;
	BoxCorner[4].y = offset.y + size.y;
	BoxCorner[4].z = offset.z + size.z;
	
	// +x -y +z
	BoxCorner[5].x = offset.x + size.x;
	BoxCorner[5].y = offset.y;
	BoxCorner[5].z = offset.z + size.z;

	// -x -y +z
	BoxCorner[6].x = offset.x;
	BoxCorner[6].y = offset.y;
	BoxCorner[6].z = offset.z + size.z;
	
	// -x +y +z
	BoxCorner[7].x = offset.x;
	BoxCorner[7].y = offset.y + size.y;
	BoxCorner[7].z = offset.z + size.z;
	
	// Now, scale all of them into the voxel co-ordinate system
	for (i=0; i<8; i++) {
		BoxCorner[i].x *= Scale.x;
		BoxCorner[i].y *= Scale.y;
		BoxCorner[i].z *= Scale.z;
	}
}


/************************************************************************
* VoxelClass::Compute_Visibility
*
* Detects the outer shell of the voxel object.
* "Tunnels" are drilled into the voxel space from all directions, all 
* voxels that are encountered before hitting the outer shell are marked 
* as VIS_VISIBLE.  After doing all of the tunnels, all voxels which are
* not now marked as VIS_VISIBLE are marked as VIS_SOLID.  This way we
* can absorb all internal geometry into one big blob.  Next we will 
* suck out the internals of that blob :-)
*
* INPUTS:
* none
*
* OUTPUS:
* none
*
************************************************************************/
void VoxelClass::Compute_Visiblity
(
	Progress_Meter_Class & meter
)
{
	int x,y,z;

	meter.Finish_In_Steps(ZDim + XDim + ZDim);
	
	/////////////////////////////////////////////////////////////
	// First, I'm going to loop through the layers (X-Y planes)
	// For each layer, I'll tunnel in the x and y direction 
	// from each point along its edge.  This is identical to 
	// the way James does his 2d visibility
	/////////////////////////////////////////////////////////////
	for (z = 0; z < (int)ZDim; z++) {

		// Tunneling in the X direction
		for (y = 0; y < (int)YDim; y++)	{

			for (x = 0; x < (int)XDim; x++) {
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}

			for (x = (int)XDim-1; x >= 0; x--)	{
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}
		}

		// Tunneling in the Y direction
		for (x = 0; x < (int)XDim; x++) {

			for (y = 0; y < (int)YDim; y++) {
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}

			for (y = (int)YDim-1; y >= 0; y--) {
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}
		}

		meter.Add_Increment();
		if (meter.Cancelled()) throw ErrorClass("Export Cancelled");
	} // done with the X-Y layers

	
	/////////////////////////////////////////////////////////////
	// Now I'm going to tunnel up and down through the object.
	// To do this, I will loop across the width of the object
	// (the X direction) and at each step tunnel through the
	// Y-Z plane from all points along the top and bottom.
	/////////////////////////////////////////////////////////////
	for (x = 0; x < (int)XDim; x++) {

		// Tunneling in the Z direction
		for (y = 0; y < (int)YDim; y++)	{

			for (z = 0; z < (int)ZDim; z++) {
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}

			for (z = (int)ZDim-1; z >= 0; z--)	{
				if (raw_read_vis(x,y,z) == VIS_SOLID) break;
				raw_set_vis(x,y,z,VIS_VISIBLE);
			}
		}

		meter.Add_Increment();
		if (meter.Cancelled()) throw ErrorClass("Export Cancelled");
	} // done with the X-Z layers


	///////////////////////////////////////////////////////////
	// Now, we search for all of the VIS_UNKNOWN voxels and
	// set them to VIS_SOLID and we are done voxelizing
	///////////////////////////////////////////////////////////
	for (z = 0; z < (int)ZDim; z++) {
		for (y = 0; y < (int)YDim; y++) {
			for (x = 0; x < (int)XDim; x++) {

				int vis = raw_read_vis(x,y,z);
				if (vis == VIS_UNKNOWN) {
					raw_set_vis(x,y,z,VIS_SOLID);
				}

			}
		}
		meter.Add_Increment();
		if (meter.Cancelled()) throw ErrorClass("Export Cancelled");
	}
}


/************************************************************************
* VoxelClass::raw_read_vis
*
* safe read of the visiblity data at i,j,k
*
* INPUTS:
* i,j,k - integer indices of the visiblity data to read
*
* OUTPUS:
* none
*
************************************************************************/
uint8  VoxelClass::raw_read_vis
(
	int i,
	int j,
	int k
)
{
	if (i<0) return 0;
	if (j<0) return 0;
	if (k<0) return 0;
	if (i>=(int)XDim) return 0;
	if (j>=(int)YDim) return 0;
	if (k>=(int)ZDim) return 0;

	return VisData[i + j*XDim + k*XDim*YDim];
}

/************************************************************************
* VoxelClass::raw_set_vis
*
* safe set of the visibility data at i,j,k
*
* INPUTS:
* i,j,k - integer indices of the visibility data to set
* val - value to set.
*
* OUTPUS:
* none
*
************************************************************************/
void VoxelClass::raw_set_vis(
	int i,
	int j,
	int k,
	uint8 val
)
{
	if (i<0) return;
	if (j<0) return;
	if (k<0) return;
	if (i>=(int)XDim) return;
	if (j>=(int)YDim) return;
	if (k>=(int)ZDim) return;

	VisData[i + j*XDim + k*XDim*YDim] = val;
	return;
}


void compute_dimensions
(  
	INodeListClass &	meshlist,
	const Matrix3 &	parenttm,
	TimeValue			curtime,
	Point3 *				set_min,
	Point3 *				set_max
)
{
	// Find the minimum and maximum extents in the X, Y, and Z directions.
	// Also find the total surface area.
	Point3 min;
	Point3 max;
	float surface_area = 0.0;

	BOOL first = TRUE;

	for ( unsigned i = 0; i < meshlist.Num_Nodes() ; ++ i )
	{
		
		//	Get the relavent data from the INode
		INode *        n   = meshlist[i];
		Object *       obj = n->EvalWorldState(curtime).obj;
		TriObject *    tri = (TriObject *)obj->ConvertToType(curtime, triObjectClassID);
		Mesh *         mesh = &(tri->mesh);
		Matrix3        tm   = n->GetObjTMAfterWSM(curtime);

		
		// Compute a matrix which takes vertices of this mesh into the
		// specified parent space.
		Matrix3			delta = tm * Inverse(parenttm);				
		
		unsigned verts = mesh->getNumVerts();
		unsigned faces = mesh->getNumFaces();

		for ( unsigned vert_index = 0; vert_index < verts; ++ vert_index )
		{
			Point3 p = delta * mesh->verts [vert_index];

			if ( first )
			{
				first = FALSE;
				min = max = p;
			}
			else
			{
				if ( p.x < min.x ) min.x = p.x;
				if ( p.y < min.y ) min.y = p.y;
				if ( p.z < min.z ) min.z = p.z;

				if ( p.x > max.x ) max.x = p.x;
				if ( p.y > max.y ) max.y = p.y;
				if ( p.z > max.z ) max.z = p.z;
			}
		}

		for ( unsigned face_index = 0; face_index < faces; ++ face_index )
		{
			Face face = mesh->faces [ face_index ];

			Point3 a = mesh->verts [ face.v[0] ];
			Point3 b = mesh->verts [ face.v[1] ];
			Point3 c = mesh->verts [ face.v[2] ];

			double area = 0.5 * Length ( CrossProd ( b - a, c - a ) );

			surface_area += (float) area;
		}
	}

	// In the odd case that there are no vertices....
	if ( first )
	{
		min = max = Point3 (0,0,0);
	}

	*set_min = min;
	*set_max = max;
}

uint8	VoxelClass::Is_Solid(int i,int j,int k)
{
	return (raw_read_vis(i,j,k) == VIS_SOLID); 
}

void VoxelClass::Compute_Physical_Properties(double Volume[1],double CM[3],double I[9])
{
	int i,j,k;
	
	// volume of a single voxel block:
	double bvol = BlockXDim * BlockYDim * BlockZDim;
	
	// volume of object
	double volume = 0.0;
	Point3 cm(0.0,0.0,0.0);
	int numblocks = 0;

	////////////////////////////////////////////////////////////////////////
	// compute the volume and the center of mass
	////////////////////////////////////////////////////////////////////////
	for (k=0; k < ZDim; k++) {
		for (j=0; j < YDim; j++) {
			for (i=0; i < XDim; i++) {
				if (Is_Solid(i,j,k)) {

					// Add this block's volume to the total
					volume += bvol;
	
					// Add this block's position to the CM computation
					cm += Voxel_Position(i,j,k);
					numblocks++;
 				}
			}
		}
	}

	cm.x = cm.x / (double)numblocks;
	cm.y = cm.y / (double)numblocks;
	cm.z = cm.z / (double)numblocks;

	CM[0] = cm.x;
	CM[1] = cm.y;
	CM[2] = cm.z;

	Volume[0] = volume;

	////////////////////////////////////////////////////////////////////////
	// compute the inertia tensor assuming constant density and factoring
	// density out:
	//
	//
	//			  ( ( (   2	  2
	// 		  | | |	y + z		-(xy)		-(xz)
	// 		  | | |
	// 		  | | |				 2	  2
	//	 I= den*| | |	-(xy)	   x + z		-(yz)			dx dy dz
	// 		  | | |
	//			  | | |							 2	  2
	// 		  | | |	-(xz)		-(yz)	   x + y
	//			  ) ) )
	//
	////////////////////////////////////////////////////////////////////////

	for (i=0; i < 9; i++) {
		I[i] = 0.0;
	}

	for (k=0; k < ZDim; k++) {
		for (j=0; j < YDim; j++) {
			for (i=0; i < XDim; i++) {
				if (Is_Solid(i,j,k)) {
	
					// position of block, relative to the CM
					Point3 pos = Voxel_Position(i,j,k) - cm;

					// moments of inertia
					double y2z2 = pos.y * pos.y + pos.z * pos.z;
					double x2z2 = pos.x * pos.x + pos.z * pos.z;
					double x2y2 = pos.x * pos.x + pos.y * pos.y;

					// products of inertia
					double xy = pos.x * pos.y;
					double xz = pos.x * pos.z;
					double yz = pos.y * pos.z;
					
					// add to the running total!
					I[0] += y2z2 * bvol;
					I[1] += -xy * bvol;
					I[2] += -xz * bvol;

					I[3] += -xy * bvol;
					I[4] += x2z2 * bvol;
					I[5] += -yz * bvol;

					I[6] += -xz * bvol;
					I[7] += -yz * bvol;
					I[8] += x2y2 * bvol;
				}
			}
		}
	}
}

Point3 VoxelClass::Voxel_Position(int i,int j,int k)
{
	// returns the coordinates of the center of block(i,j,k)
	return Point3(
		Offset.x + i * BlockXDim + BlockXDim / 2.0,
		Offset.y + j * BlockYDim + BlockYDim / 2.0,
		Offset.z + k * BlockZDim + BlockZDim / 2.0
	);
}