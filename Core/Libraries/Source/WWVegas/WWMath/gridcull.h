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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/gridcull.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/10/01 10:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GridCullSystemClass::clamp_indices_to_grid -- constrains indices to be a valid location   *
 *   GridCullSystemClass::map_point_to_cell -- determines which cell  the point is in          *
 *   GridCullSystemClass::map_point_to_address -- determines the address of a point in the gri *
 *   GridCullSystemClass::map_indices_to_address -- computes the address for given index tripl *
 *   GridCullSystemClass::total_cell_count -- returns the total number of cells in the grid    *
 *   GridCullSystemClass::compute_box -- computes the bounding box for a grid cell             *
 *   GridCullSystemClass::compute_box -- computes bounding box for a range of grid cells       *
 *   GridCullSystemClass::init_volume -- inits volume to contain the given range               *
 *   GridCullSystemClass::init_volume -- inits volume to contain the given line segment        *
 *   GridCullSystemClass::init_volume -- inits volume to contain the given box                 *
 *   GridCullSystemClass::init_volume -- inits volume to contain the given oriented box        *
 *   GridCullSystemClass::init_volume -- inits volume to contain the given frustum             *
 *   GridCullSystemClass::VolumeStruct::VolumeStruct -- constructor                            *
 *   GridCullSystemClass::VolumeStruct::VolumeStruct -- constructor                            *
 *   GridCullSystemClass::VolumeStruct::Is_Leaf -- check if volume is a leaf                   *
 *   GridCullSystemClass::VolumeStruct::Is_Empty -- check if volume is empty (or invalid)      *
 *   GridCullSystemClass::VolumeStruct::Split -- split this volume                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#include "cullsys.h"
#include "mempool.h"
#include "frustum.h"
#include "aabox.h"
#include "lineseg.h"
#include "obbox.h"
#include <string.h>

class ChunkLoadClass;
class ChunkSaveClass;

/*
** GridCullSystemClass
** This is a culling system designed for dynamic objects (objects which are moving or
** changing bounding box size).  It features O(1) insertion as opposed to
** the AABTree, QuadTree, or Octree insertion times which are O(logn).  It's disadvantages
** compared to the above mentioned systems are that it must uniformly divide space.  The
** AABTree conforms to the geometry placed in it and can therefore cull things more
** efficiently.  In actual use, this system is like an AAB-tree which is uniformly subdivided,
** its just that we can jump straight to the leaves of the tree for insertion...
**
** The bounding volume for each grid cell is considered to be the volume of the cell, expanded
** by the maximum object size.  Inserting an object into the grid is a matter of determining
** which cell its center point is in.  Objects which are larger than the maximum size are
** allowed but they are simply put into a linked list which is iterated with each call to
** the culling system (linearly culled rather than logarithmic...)
*/
class GridCullSystemClass : public CullSystemClass
{

public:

	GridCullSystemClass(void);
	~GridCullSystemClass(void);

	virtual void		Collect_Objects(const Vector3 & point);
	virtual void		Collect_Objects(const AABoxClass & box);
	virtual void		Collect_Objects(const OBBoxClass & box);
	virtual void		Collect_Objects(const FrustumClass & frustum);

	virtual void		Re_Partition(const Vector3 & min,const Vector3 & max,float objdim);
	virtual void		Update_Culling(CullableClass * obj);

	virtual void		Load(ChunkLoadClass & cload);
	virtual void		Save(ChunkSaveClass & csave);

	virtual int			Get_Object_Count (void) const { return ObjCount; }

	/*
	** Statistics
	*/
	struct StatsStruct
	{
		int				NodeCount;
		int				NodesAccepted;
		int				NodesTriviallyAccepted;
		int				NodesRejected;
	};

	void					Reset_Statistics(void);
	const StatsStruct & Get_Statistics(void);

	void					Get_Min_Cell_Size (Vector3 &size) const	{ size = MinCellSize; }
	void					Set_Min_Cell_Size (const Vector3 &size)	{ MinCellSize = size; }

	int					Get_Termination_Count (void) const	{ return TerminationCellCount; }
	void					Set_Termination_Count (int count)	{ TerminationCellCount = count; }

protected:

	void					Collect_And_Unlink_All(void);
	void					Add_Object_Internal(CullableClass * obj);
	void					Remove_Object_Internal(CullableClass * obj);

	enum
	{
		TERMINATION_CELL_COUNT	= 16384,			// algorithm terminates if we ever have more than this many cells.
		UNGRIDDED_ADDRESS			= 0xFFFFFFFF	// address given to objs that didn't fit in grid
	};

	// Constants which control the division of space:
	Vector3				MinCellSize;			// min dimensions for a cell (don't go below this...)
	float					MaxObjExtent;			// max extent/radius (objects bigger than this are just put in a list)
	int					TerminationCellCount;

	// Constants that define the division of space
	Vector3				Origin;
	Vector3				CellDim;
	Vector3				OOCellDim;
	int					CellCount[3];

	//	3D array of pointers to objects in each cell
	CullableClass **	Cells;

	// list of objs that were outside or too big for the grid.
	CullableClass *	NoGridList;

	// number of objects in the system
	int					ObjCount;

	// statistics
	StatsStruct				Stats;

	// Structure used to define a volume in the grid.  The volume spans from the cell indexed
	// by Min[0],Min[1],Min[2] to the cell indexed by Max[0]-1,Max[1]-1,Max[2]-1.
	struct VolumeStruct
	{
		VolumeStruct(void);
		VolumeStruct(int i0,int j0,int k0,int i1,int j1,int k1);
		bool Is_Leaf(void) const;
		bool Is_Empty(void) const;
		void Split(VolumeStruct & v0,VolumeStruct & v1) const;

		int Min[3];
		int Max[3];
	};

	void					link_object(CullableClass * obj);
	void					link_object(CullableClass * obj,int address);
	void					unlink_object(CullableClass * obj);
	void					link_object_to_list(CullableClass ** head,CullableClass * obj);
	void					unlink_object_from_list(CullableClass ** head,CullableClass * obj);

	bool					map_point_to_cell(const Vector3 & pt,int & set_i,int & set_j,int & set_k);
	bool					map_point_to_address(const Vector3 & pt,int & set_address);
	WWINLINE int			map_indices_to_address(int i,int j,int k);
	void					clamp_indices_to_grid(int * i,int * j,int * k);

	int					total_cell_count(void);
	void					compute_box(int i,int j,int k,AABoxClass * set_box);
	void					compute_box(const VolumeStruct & area, AABoxClass * set_box);

	void					init_volume(const Vector3 & bound_min,const Vector3 & bound_max,VolumeStruct * set_volume);
	void					init_volume(const Vector3 & point,VolumeStruct * set_volume);
	void					init_volume(const LineSegClass & line,VolumeStruct * set_volume);
	void					init_volume(const AABoxClass & box,VolumeStruct * set_volume);
	void					init_volume(const OBBoxClass & box,VolumeStruct * set_volume);
	void					init_volume(const FrustumClass & frustum,VolumeStruct * set_volume);

	void					collect_objects_in_leaf(const Vector3 & point,CullableClass * head);
	void					collect_objects_in_leaf(const AABoxClass & aabox,CullableClass * head);
	void					collect_objects_in_leaf(const OBBoxClass & obbox,CullableClass * head);
	void					collect_objects_in_leaf(const FrustumClass & frustum,CullableClass * head);
};

/*
** Macros for gathering statistics.  Placed here in the header file so that
** derived classes can use them as well.
*/
#ifdef WWDEBUG

#define GRIDCULL_NODE_ACCEPTED						Stats.NodesAccepted ++;
#define GRIDCULL_NODE_TRIVIALLY_ACCEPTED			Stats.NodesTriviallyAccepted ++;
#define GRIDCULL_NODE_REJECTED						Stats.NodesRejected ++;

#else

#define GRIDCULL_NODE_ACCEPTED
#define GRIDCULL_NODE_TRIVIALLY_ACCEPTED
#define GRIDCULL_NODE_REJECTED

#endif


/*
** TypedGridCullSystemClass
** This class simply enforces that a certain type of object is inserted into the grid cull system.
** It exposes the add, remove, and collection iterating functions and is intended to be the class
** actually used by the end user.
*/
template <class T> class TypedGridCullSystemClass : public GridCullSystemClass
{
public:

	virtual void		Add_Object(T * obj)						{ Add_Object_Internal(obj); }
	virtual void		Remove_Object(T * obj)					{ Remove_Object_Internal(obj); }

	T *					Get_First_Collected_Object(void)		{ return (T*)Get_First_Collected_Object_Internal(); }
	T *					Get_Next_Collected_Object(T * obj)	{ return (T*)Get_Next_Collected_Object_Internal(obj); }

};


/*
** GridLinkClass
** This structure is used to link cullable objects into a Grid culling system
** This class is should only be used by classes which derive from GridCullSystemClass
** not normal users.
*/
class GridLinkClass : public CullLinkClass, public AutoPoolClass<GridLinkClass,256>
{
public:
	GridLinkClass(GridCullSystemClass * system);
	virtual ~GridLinkClass(void);

	int									GridAddress;		// address in the grid.
	CullableClass *					Prev;					// prev object in this cell
	CullableClass *					Next;					// next object in this cell
};


/*
** GridListIterator
** This is just a simple iterator that contains the code for traversing the
** list of objects either in a cell in the grid or in the NoGridList...
** This class should only be used by classes which derive from GridCullSystemClass
** not normal users.
*/
class GridListIterator
{
public:

	GridListIterator(CullableClass * head)		{ First(head); }

	void			First(CullableClass * head)	{ Head = head; CurObj = head; }
	void			First(void)							{ CurObj = Head; }
	void			Next(void)							{ if (CurObj) { CurObj = ((GridLinkClass *)CurObj->Get_Cull_Link())->Next; } }
	void			Prev(void)							{ if (CurObj) { CurObj = ((GridLinkClass *)CurObj->Get_Cull_Link())->Prev; } }
	bool			Is_Done(void)						{ return (CurObj == NULL); }

	CullableClass *	Get_Obj(void)				{ if (CurObj) { CurObj->Add_Ref(); } return CurObj; }
	CullableClass *	Peek_Obj(void)				{ return CurObj; }

private:

	CullableClass *	Head;				// head of the list we're working in
	CullableClass *	CurObj;			// node we're currently at.

};



/***********************************************************************************************
 * GridCullSystemClass::clamp_indices_to_grid -- constrains indices to be a valid location     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::clamp_indices_to_grid(int * i,int * j,int * k)
{
	if (*i < 0) *i = 0;
	if (*i >= CellCount[0]) *i = CellCount[0] - 1;
	if (*j < 0) *j = 0;
	if (*j >= CellCount[1]) *j = CellCount[1] - 1;
	if (*k < 0) *k = 0;
	if (*k >= CellCount[2]) *k = CellCount[2] - 1;
}


/***********************************************************************************************
 * GridCullSystemClass::map_point_to_cell -- determines which cell  the point is in            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool GridCullSystemClass::map_point_to_cell(const Vector3 & pt,int & set_i,int & set_j,int & set_k)
{
	Vector3 dp = pt - Origin;
	set_i = floor(dp.X * OOCellDim.X);
	set_j = floor(dp.Y * OOCellDim.Y);
	set_k = floor(dp.Z * OOCellDim.Z);

	if (	(set_i >= 0) && (set_j >= 0) && (set_k >= 0) &&
			(set_i < CellCount[0]) && (set_j < CellCount[1]) && (set_k < CellCount[2])	)
	{
		return true;
	}
	return false;
}


/***********************************************************************************************
 * GridCullSystemClass::map_point_to_address -- determines the address of a point in the grid  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool GridCullSystemClass::map_point_to_address(const Vector3 & pt,int &set_address)
{
	int i,j,k;
	bool res = map_point_to_cell(pt,i,j,k);

	if (res) {
		set_address = map_indices_to_address(i,j,k);
		return true;
	} else {
		set_address = UNGRIDDED_ADDRESS;
		return false;
	}
}


/***********************************************************************************************
 * GridCullSystemClass::map_indices_to_address -- computes the address for given index triplet *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE int GridCullSystemClass::map_indices_to_address(int i,int j,int k)
{
	return i + j*CellCount[0] + k*CellCount[0]*CellCount[1];
}


/***********************************************************************************************
 * GridCullSystemClass::total_cell_count -- returns the total number of cells in the grid      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE int GridCullSystemClass::total_cell_count(void)
{
	return CellCount[0] * CellCount[1] * CellCount[2];
}


/***********************************************************************************************
 * GridCullSystemClass::compute_box -- computes the bounding box for a grid cell               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void	GridCullSystemClass::compute_box(int i,int j,int k,AABoxClass * set_box)
{
	WWASSERT(set_box != NULL);
	WWASSERT((i >= 0) && (j >= 0) && (k >= 0));
	WWASSERT((i < CellCount[0]) && (j < CellCount[1]) && (k < CellCount[2]));

	Vector3 min,max;

	min.X = Origin.X + i * CellDim.X - MaxObjExtent;
	min.Y = Origin.Y + j * CellDim.Y - MaxObjExtent;
	min.Z = Origin.Z + k * CellDim.Z - MaxObjExtent;

	max.X = min.X + CellDim.X + 2.0f*MaxObjExtent;
	max.Y = min.Y + CellDim.Y + 2.0f*MaxObjExtent;
	max.Z = min.Z + CellDim.Z + 2.0f*MaxObjExtent;

	set_box->Init((min+max)*0.5f, (min-max)*0.5f);
}


/***********************************************************************************************
 * GridCullSystemClass::compute_box -- computes bounding box for a range of grid cells         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void	GridCullSystemClass::compute_box(const GridCullSystemClass::VolumeStruct & vol, AABoxClass * set_box)
{
	WWASSERT(set_box != NULL);
	WWASSERT((vol.Min[0] >= 0) && (vol.Min[1] >= 0) && (vol.Min[2] >= 0));
	WWASSERT((vol.Max[0] <= CellCount[0]) && (vol.Max[1] <= CellCount[1]) && (vol.Max[2] <= CellCount[2]));

	Vector3 min,max;

	min.X = Origin.X + vol.Min[0] * CellDim.X - MaxObjExtent;
	min.Y = Origin.Y + vol.Min[1] * CellDim.Y - MaxObjExtent;
	min.Z = Origin.Z + vol.Min[2] * CellDim.Z - MaxObjExtent;

	max.X = Origin.X + vol.Max[0] * CellDim.X + MaxObjExtent;
	max.Y = Origin.Y + vol.Max[1] * CellDim.Y + MaxObjExtent;
	max.Z = Origin.Z + vol.Max[2] * CellDim.Z + MaxObjExtent;

	Vector3 center((max.X+min.X)*0.5f,(max.Y+min.Y)*0.5f,(max.Z+min.Z)*0.5f);
	Vector3 extent((max.X-min.X)*0.5f,(max.Y-min.Y)*0.5f,(max.Z-min.Z)*0.5f);
	set_box->Init(center,extent);
}


/***********************************************************************************************
 * GridCullSystemClass::init_volume -- inits volume to contain the given range                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::init_volume
(
	const Vector3 &		bound_min,
	const Vector3 &		bound_max,
	VolumeStruct *			set_vol
)
{
	// expand the box by the maximum size of any object
	Vector3 grid_min = bound_min;
	grid_min.X -= MaxObjExtent;
	grid_min.Y -= MaxObjExtent;
	grid_min.Z -= MaxObjExtent;

	Vector3 grid_max = bound_max;
	grid_max.X += MaxObjExtent;
	grid_max.Y += MaxObjExtent;
	grid_max.Z += MaxObjExtent;

	// now compute the grid coordinates of the corners of the box
	GridCullSystemClass::map_point_to_cell(grid_min,set_vol->Min[0],set_vol->Min[1],set_vol->Min[2]);
	GridCullSystemClass::map_point_to_cell(grid_max,set_vol->Max[0],set_vol->Max[1],set_vol->Max[2]);

	// now clamp the volume to the actual grid
	clamp_indices_to_grid(&(set_vol->Min[0]),&(set_vol->Min[1]),&(set_vol->Min[2]));
	clamp_indices_to_grid(&(set_vol->Max[0]),&(set_vol->Max[1]),&(set_vol->Max[2]));

	// increment the outer edge of the box due to the way we traverse the grid...
	set_vol->Max[0] ++;
	set_vol->Max[1] ++;
	set_vol->Max[2] ++;
}


/***********************************************************************************************
 * GridCullSystemClass::init_volume -- inits volume to contain the given line segment          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::init_volume(const LineSegClass & line,VolumeStruct * set_volume)
{
	Vector3 min_pt,max_pt;
	min_pt.X = WWMath::Min(line.Get_P0().X,line.Get_P1().X);
	max_pt.X = WWMath::Max(line.Get_P0().X,line.Get_P1().X);
	min_pt.Y = WWMath::Min(line.Get_P0().Y,line.Get_P1().Y);
	max_pt.Y = WWMath::Max(line.Get_P0().Y,line.Get_P1().Y);
	min_pt.Z = WWMath::Min(line.Get_P0().Z,line.Get_P1().Z);
	max_pt.Z = WWMath::Max(line.Get_P0().Z,line.Get_P1().Z);
	init_volume(min_pt,max_pt,set_volume);
}


/***********************************************************************************************
 * GridCullSystemClass::init_volume -- inits volume to contain the given box                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::init_volume(const AABoxClass & box,VolumeStruct * set_volume)
{
	init_volume(box.Center - box.Extent,box.Center + box.Extent,set_volume);
}


/***********************************************************************************************
 * GridCullSystemClass::init_volume -- inits volume to contain the given oriented box          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::init_volume(const OBBoxClass & box,VolumeStruct * set_volume)
{
	Vector3 aaextent;
	box.Compute_Axis_Aligned_Extent(&aaextent);
	init_volume(box.Center - aaextent,box.Center + aaextent,set_volume);
}


/***********************************************************************************************
 * GridCullSystemClass::init_volume -- inits volume to contain the given frustum               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::init_volume(const FrustumClass & frustum,VolumeStruct * set_volume)
{
	init_volume(frustum.Get_Bound_Min(),frustum.Get_Bound_Max(),set_volume);
}


/***********************************************************************************************
 * GridCullSystemClass::VolumeStruct::VolumeStruct -- constructor                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
WWINLINE GridCullSystemClass::VolumeStruct::VolumeStruct(void)
{
}


/***********************************************************************************************
 * GridCullSystemClass::VolumeStruct::VolumeStruct -- constructor                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE GridCullSystemClass::VolumeStruct::VolumeStruct(int i0,int j0,int k0,int i1,int j1,int k1)
{
	Min[0] = i0;
	Min[1] = j0;
	Min[2] = k0;
	Max[0] = i1;
	Max[1] = j1;
	Max[2] = k1;
	WWASSERT(Max[0] > Min[0]);
	WWASSERT(Max[1] > Min[1]);
	WWASSERT(Max[2] > Min[2]);
}


/***********************************************************************************************
 * GridCullSystemClass::VolumeStruct::Is_Leaf -- check if volume is a leaf                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool GridCullSystemClass::VolumeStruct::Is_Leaf(void) const
{
	return ((Max[0]-Min[0] == 1) && (Max[1]-Min[1] == 1) && (Max[2]-Min[2] == 1));
}


/***********************************************************************************************
 * GridCullSystemClass::VolumeStruct::Is_Empty -- check if volume is empty (or invalid)        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool GridCullSystemClass::VolumeStruct::Is_Empty(void) const
{
	return ((Max[0]-Min[0] <= 0) || (Max[1]-Min[1] <= 0) || (Max[2]-Min[2] <= 0));
}


/***********************************************************************************************
 * GridCullSystemClass::VolumeStruct::Split -- split this volume                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void GridCullSystemClass::VolumeStruct::Split(VolumeStruct & v0,VolumeStruct & v1) const
{
	// find the longest dimension
	int split_axis = 0;
	int delta[3];

	delta[0] = Max[0] - Min[0];
	delta[1] = Max[1] - Min[1];
	delta[2] = Max[2] - Min[2];

	if (delta[1] > delta[split_axis]) split_axis = 1;
	if (delta[2] > delta[split_axis]) split_axis = 2;

	WWASSERT(delta[split_axis] > 0);

	// split the volume perpendicularly to that dimension
	memcpy(&v0,this,sizeof(VolumeStruct));
	memcpy(&v1,this,sizeof(VolumeStruct));

	v0.Max[split_axis] = v1.Min[split_axis] = Min[split_axis] + (delta[split_axis] >> 1);
}
