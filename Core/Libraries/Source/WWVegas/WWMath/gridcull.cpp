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
 *                     $Archive:: /Commando/Code/wwmath/gridcull.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/27/00 7:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "gridcull.h"
#include "colmath.h"
#include "colmathinlines.h"



/*
** Declare the pool for GridLinks
*/
DEFINE_AUTO_POOL(GridLinkClass,256);


/*************************************************************************
**
** Utility functions for walking the object list in an AABTree Node
**
*************************************************************************/
static inline CullableClass * get_next_object(CullableClass * obj)
{
	return ((GridLinkClass *)obj->Get_Cull_Link())->Next;
}

/*************************************************************************
**
** GridLinkClass Implementation
**
*************************************************************************/
GridLinkClass::GridLinkClass(GridCullSystemClass * system) :
	CullLinkClass(system),
	GridAddress(-1),
	Prev(nullptr),
	Next(nullptr)
{
}

GridLinkClass::~GridLinkClass()
{
}





/*************************************************************************
**
** GridCullSystemClass Implementation
**
*************************************************************************/


GridCullSystemClass::GridCullSystemClass() :
	MinCellSize(10,10,10),
	MaxObjExtent(15),
	Origin(-100,-100,-100),
	CellDim(10,10,10),
	Cells(nullptr),
	NoGridList(nullptr),
	ObjCount(0),
	TerminationCellCount(TERMINATION_CELL_COUNT)
{
	CellCount[0] = CellCount[1] = CellCount[2] = 0;
	Re_Partition(Vector3(-100,-100,-100),Vector3(100,100,100),15);
	Reset_Statistics();
}


GridCullSystemClass::~GridCullSystemClass()
{
	delete Cells;
	Cells = nullptr;
}


void GridCullSystemClass::Collect_Objects(const Vector3 & point)
{
	/*
	** Collect the objects in the grid
	*/
	VolumeStruct vol;
	init_volume(point,point,&vol);
	if (!vol.Is_Empty()) {

		int delta_x = vol.Max[0] - vol.Min[0];
		int i,j,k;
		int address = map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2]);

		for (k=vol.Min[2]; k<vol.Max[2]; k++) {
			for (j=vol.Min[1]; j<vol.Max[1]; j++) {
				for (i=vol.Min[0]; i<vol.Max[0]; i++) {
					GRIDCULL_NODE_TRIVIALLY_ACCEPTED;
					collect_objects_in_leaf(point,Cells[address]);
					address++;
				}
				address -= delta_x;
				address += CellCount[0];
			}
			address = map_indices_to_address(vol.Min[0],vol.Min[1],k+1);
		}
	}

	/*
	** Collect the objects in the no-grid-list
	*/
	collect_objects_in_leaf(point,NoGridList);
}


void GridCullSystemClass::Collect_Objects(const AABoxClass & box)
{
	/*
	** Collect the objects in the grid
	*/
	VolumeStruct vol;
	init_volume(box,&vol);

	if (!vol.Is_Empty()) {

		int delta_x = vol.Max[0] - vol.Min[0];
		int i,j,k;
		int address = map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2]);

		for (k=vol.Min[2]; k<vol.Max[2]; k++) {
			for (j=vol.Min[1]; j<vol.Max[1]; j++) {
				for (i=vol.Min[0]; i<vol.Max[0]; i++) {
					GRIDCULL_NODE_TRIVIALLY_ACCEPTED;
					collect_objects_in_leaf(box,Cells[address]);
					address++;
				}
				address -= delta_x;
				address += CellCount[0];
			}
			address = map_indices_to_address(vol.Min[0],vol.Min[1],k+1);
		}
	}

	/*
	** Collect the objects in the no-grid-list
	*/
	collect_objects_in_leaf(box,NoGridList);
}


void GridCullSystemClass::Collect_Objects(const OBBoxClass & box)
{
	/*
	** Collect the objects in the grid
	*/
	VolumeStruct vol;
	init_volume(box,&vol);

	if (!vol.Is_Empty()) {

		int delta_x = vol.Max[0] - vol.Min[0];
		int i,j,k;
		int address = map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2]);

		for (k=vol.Min[2]; k<vol.Max[2]; k++) {
			for (j=vol.Min[1]; j<vol.Max[1]; j++) {
				for (i=vol.Min[0]; i<vol.Max[0]; i++) {
					GRIDCULL_NODE_TRIVIALLY_ACCEPTED;
					collect_objects_in_leaf(box,Cells[address]);
					address++;
				}
				address -= delta_x;
				address += CellCount[0];
			}
			address = map_indices_to_address(vol.Min[0],vol.Min[1],k+1);
		}
	}

	/*
	** Collect the objects in the no-grid-list
	*/
	collect_objects_in_leaf(box,NoGridList);
}


void GridCullSystemClass::Collect_Objects(const FrustumClass & frustum)
{
	/*
	** Collect the objects in the grid
	*/
	VolumeStruct vol;
	init_volume(frustum,&vol);

	if (!vol.Is_Empty()) {

		int delta_x = vol.Max[0] - vol.Min[0];
		int i,j,k;
		int address = map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2]);

		for (k=vol.Min[2]; k<vol.Max[2]; k++) {
			for (j=vol.Min[1]; j<vol.Max[1]; j++) {
				for (i=vol.Min[0]; i<vol.Max[0]; i++) {
					GRIDCULL_NODE_TRIVIALLY_ACCEPTED;
					collect_objects_in_leaf(frustum,Cells[address]);
					address++;
				}
				address -= delta_x;
				address += CellCount[0];
			}
			address = map_indices_to_address(vol.Min[0],vol.Min[1],k+1);
		}
	}

	/*
	** Collect the objects in the no-grid-list
	*/
	collect_objects_in_leaf(frustum,NoGridList);
}


void GridCullSystemClass::Re_Partition(const Vector3 & input_min,const Vector3 & input_max,float objdim)
{
	/*
	** grab all objects into the collection list and unlink them from the grid
	*/
	Reset_Collection();
	Collect_And_Unlink_All();

	/*
	** Do a sanity check on the input parameters
	*/
	Vector3 min = input_min;
	Vector3 max = input_max;

	if (max.X - min.X < 1.0f) {
		max.X += MinCellSize.X;
		min.X -= MinCellSize.X;
	}
	if (max.Y - min.Y < 1.0f) {
		max.Y += MinCellSize.Y;
		min.Y -= MinCellSize.Y;
	}
	if (max.Z - min.Z < 1.0f) {
		max.Z += MinCellSize.Z;
		min.Z -= MinCellSize.Z;
	}

	/*
	** Compute the grid parameters
	*/
	Origin = min;
	Vector3 world_dim = max - min;
	MaxObjExtent = objdim;

	WWASSERT(world_dim.X > 0.0f);
	WWASSERT(world_dim.Y > 0.0f);
	WWASSERT(world_dim.Z > 0.0f);

	/*
	** how many cells should we use on each dimension?
	*/
	CellCount[0] = CellCount[1] = CellCount[2] = 1;
	CellDim = world_dim;
	bool done = false;

	while (!done) {

		/*
		** find biggest dimension
		*/
		int bigdim = 0;
		if (CellDim[1]/MinCellSize[1] > CellDim[bigdim]/MinCellSize[bigdim]) bigdim = 1;
		if (CellDim[2]/MinCellSize[2] > CellDim[bigdim]/MinCellSize[bigdim]) bigdim = 2;

		/*
		** split dimension in two if possible
		*/
		if (CellDim[bigdim] >= 2.0f * MinCellSize[bigdim]) {
			CellDim[bigdim] /= 2.0f;
			CellCount[bigdim] *= 2;
		}

		/*
		** check termination conditions
		*/
		if (total_cell_count() >= TerminationCellCount) {
			done = true;
		}

		if (	(CellDim[0] < 2.0*MinCellSize[0]) &&
				(CellDim[1] < 2.0*MinCellSize[1]) &&
				(CellDim[2] < 2.0*MinCellSize[2]) ) {
			done = true;
		}
	}

	OOCellDim.X = 1.0f / CellDim.X;
	OOCellDim.Y = 1.0f / CellDim.Y;
	OOCellDim.Z = 1.0f / CellDim.Z;

	delete[] Cells;
	Cells = W3DNEWARRAY CullableClass * [total_cell_count()];
	memset(&(Cells[0]),0,total_cell_count() * sizeof(CullableClass *));

	/*
	** iterate the collection list and re-insert all objects into the grid
	*/
	CullableClass * obj;
	for (	obj = Get_First_Collected_Object_Internal();
			obj != nullptr;
			obj = Get_Next_Collected_Object_Internal(obj))
	{
		link_object(obj);
	}
}


// This is used when re-partitioning the grid. All objects are pulled out and linked into the
// collection list. When the grid has been re-initialized, the objects are put back in.
void GridCullSystemClass::Collect_And_Unlink_All()
{
	Reset_Collection();

	/*
	** pull all objects out of the grid
	*/
	for (int k=0; k<CellCount[2]; k++) {
		for (int j=0; j<CellCount[1]; j++) {
			for (int i=0; i<CellCount[0]; i++) {

				/*
				** collect and unlink all objects in cell(i,j,k)
				*/
				CullableClass * obj = Cells[map_indices_to_address(i,j,k)];
				while (obj) {
					CullableClass * nextobj = get_next_object(obj);

					unlink_object(obj);
					Add_To_Collection(obj);

					obj = nextobj;
				}

			}
		}
	}

	/*
	** pull all objects out of the "no-grid" list
	*/
	CullableClass * obj = NoGridList;
	while (obj) {
		CullableClass * nextobj = get_next_object(obj);

		unlink_object(obj);
		Add_To_Collection(obj);

		obj = nextobj;
	}
}


void GridCullSystemClass::Update_Culling(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	int address;
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();
	map_point_to_address(obj->Get_Cull_Box().Center,address);

	if (address != link->GridAddress) {
		unlink_object(obj);
		link_object(obj,address);
	}
}


void GridCullSystemClass::Reset_Statistics()
{
	// number of (virtual) nodes = 2n-1
	Stats.NodeCount = 2 * (CellCount[0] * CellCount[1] * CellCount[2]) - 1;
	Stats.NodesAccepted = 0;
	Stats.NodesTriviallyAccepted = 0;
	Stats.NodesRejected = 0;
}


const GridCullSystemClass::StatsStruct & GridCullSystemClass::Get_Statistics()
{
	return Stats;
}


void GridCullSystemClass::Add_Object_Internal(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == nullptr);

	GridLinkClass * link = new GridLinkClass(this);
	obj->Set_Cull_Link(link);
	link_object(obj);

	ObjCount++;
	obj->Add_Ref();
}


void GridCullSystemClass::Remove_Object_Internal(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();

	unlink_object(obj);
	link->Set_Culling_System(nullptr);
	delete link;
	obj->Set_Cull_Link(nullptr);

	ObjCount--;
	obj->Release_Ref();
}


void GridCullSystemClass::link_object(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	int address;
	map_point_to_address(obj->Get_Cull_Box().Center,address);

	link_object(obj,address);
}

void GridCullSystemClass::link_object(CullableClass * obj,int address)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link != nullptr);

	/*
	** if obj cannot be inserted into the grid, add it to the NoGridList
	** otherwise, insert it into the cell
	*/
	const AABoxClass & box = obj->Get_Cull_Box();
	if (
			(box.Extent.X > MaxObjExtent) ||
			(box.Extent.Y > MaxObjExtent) ||
			(box.Extent.Z > MaxObjExtent) ||
			(address == UNGRIDDED_ADDRESS)
		)
	{
		link->GridAddress = UNGRIDDED_ADDRESS;
		link_object_to_list(&NoGridList,obj);
	} else {
		link->GridAddress = address;
		link_object_to_list(&(Cells[address]),obj);
	}
}

void GridCullSystemClass::unlink_object(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();

	if (link->GridAddress == UNGRIDDED_ADDRESS) {
		unlink_object_from_list(&NoGridList,obj);
	} else {
		unlink_object_from_list(&(Cells[link->GridAddress]),obj);
	}
}


void GridCullSystemClass::link_object_to_list(CullableClass ** head,CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();

	/*
	** Insert this object as the new head of the list.
	*/
	link->Next = *head;
	link->Prev = nullptr;

	if (link->Next != nullptr) {
		GridLinkClass * next_link = (GridLinkClass *)link->Next->Get_Cull_Link();
		WWASSERT(next_link != nullptr);
		next_link->Prev = obj;
	}

	*head = obj;
}


void GridCullSystemClass::unlink_object_from_list(CullableClass ** head,CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);
	GridLinkClass * link = (GridLinkClass *)obj->Get_Cull_Link();

	/*
	** check to see that the object is actually in this list
	*/
#ifdef DEBUG_CRASHING
	CullableClass * tmp = *head;
	bool found = false;
	while (tmp && !found) {
		if (tmp == obj) found = true;
		tmp = ((GridLinkClass *)(tmp->Get_Cull_Link()))->Next;
	}
	WWASSERT(found);
#endif

	/*
	** If we were the head of the list, make the head point to the next object
	*/
	if (obj == *head) {
		*head = link->Next;
	}

	/*
	** Link the object previous to us to our next...
	*/
	if (link->Prev) {
		GridLinkClass * prev_link = (GridLinkClass *)link->Prev->Get_Cull_Link();
		prev_link->Next = link->Next;
	}

	/*
	** Link the objects after us to our previous...
	*/
	if (link->Next) {
		GridLinkClass * next_link = (GridLinkClass *)link->Next->Get_Cull_Link();
		next_link->Prev = link->Prev;
	}

	link->Prev = nullptr;
	link->Next = nullptr;
}



/*************************************************************************
**
** GridCullSystem Internal Leaf-iterating collection functions
**
*************************************************************************/
void GridCullSystemClass::collect_objects_in_leaf(const Vector3 & point,CullableClass * head)
{
	if (head != nullptr) {
		GridListIterator it(head);
		for (;!it.Is_Done(); it.Next()) {
			CullableClass * obj = it.Peek_Obj();
			if (obj->Get_Cull_Box ().Contains (point) == true) {
				Add_To_Collection(obj);
			}
		}
	}
}

void GridCullSystemClass::collect_objects_in_leaf(const AABoxClass & box,CullableClass * head)
{
	if (head != nullptr) {
		GridListIterator it(head);
		for (;!it.Is_Done(); it.Next()) {
			CullableClass * obj = it.Peek_Obj();
			if (CollisionMath::Overlap_Test(box,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
		}
	}
}

void GridCullSystemClass::collect_objects_in_leaf(const OBBoxClass & obbox,CullableClass * head)
{
	if (head != nullptr) {
		GridListIterator it(head);
		for (;!it.Is_Done(); it.Next()) {
			CullableClass * obj = it.Peek_Obj();
			if (CollisionMath::Overlap_Test(obbox,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
		}
	}
}

void GridCullSystemClass::collect_objects_in_leaf(const FrustumClass & frustum,CullableClass * head)
{
	if (head != nullptr) {
		GridListIterator it(head);
		for (;!it.Is_Done(); it.Next()) {
			CullableClass * obj = it.Peek_Obj();
			if (CollisionMath::Overlap_Test(frustum,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
		}
	}
}
