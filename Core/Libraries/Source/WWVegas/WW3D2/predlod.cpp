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
 *                     $Archive:: /Commando/Code/ww3d2/predlod.cpp        $*
 *                                                                         *
 *                      $Author:: Jani_p                                  $*
 *                                                                         *
 *                     $Modtime:: 9/20/01 10:10a                          $*
 *                                                                         *
 *                    $Revision:: 5                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   PredictiveLODOptimizerClass::Clear -- clear object list and total cost*
 *   PredictiveLODOptimizerClass::Add_Object -- adds object to list, cost  *
 *   PredictiveLODOptimizerClass::Optimize_LODs -- does LOD optimization   *
 *   PredictiveLODOptimizerClass::Free -- releases all memory used.        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "predlod.h"
#include <memory.h>

/* NOTE: The LODHeapNode and LODHeap classes are defined here for use in the
 * Optimize_LODs() member function. */

// A node entry for a heap. It has no son/father pointers since it will be
// used in an array implementation of a heap.

/*
**	NOTE: LODHeapNodes contain pointers to RenderObjClass, but these pointers
** are NOT tracked via refcounting. This is because the heaps are created and
** destroyed within one function, so all references created are necessary;
** and performing Add_Ref and Release_Ref every time a pointer is copied
** would hurt performance.
*/

class LODHeapNode {
	public:
		LODHeapNode(void)												{ Item = NULL; }
		LODHeapNode (float key)										{ Item = NULL; Key = key; }
		LODHeapNode (RenderObjClass * item, float key)		{ Item = item; Key = key; }

		~LODHeapNode(void)											{ }

		RenderObjClass *	Get_Item(void)							{ return(Item); }

		float						Get_Key(void)						{ return(Key); }
		void						Set_Key(float key)				{ Key = key; }

		int operator <		(const LODHeapNode& node)	{ return(Key < node.Key); }
		int operator <=	(const LODHeapNode& node)	{ return(Key <= node.Key); }
		int operator >		(const LODHeapNode& node)	{ return(Key > node.Key); }
		int operator >=	(const LODHeapNode& node)	{ return(Key >= node.Key); }
		int operator ==	(const LODHeapNode& node)	{ return(Key == node.Key); }
		int operator !=	(const LODHeapNode& node)	{ return(Key != node.Key); }

	private:
		RenderObjClass *Item;
		float Key;
};

// A Heap implemented as a complete binary tree in an array.
class LODHeap {
	public:
		// This constructor receives an array of HeapNodes and arranges it to
		// fulfil the heap condition. Note: the array will be used inside the
		// heap, so it should never be used or deleted outside. The resulting
		// heap is full - no nodes can be added until some are removed.
		LODHeap(int count, LODHeapNode *NodeArray) {
			Nodes = NodeArray;
			Num = Max = count;
			// Now build a heap from the array by working backwards, building
			// subheaps from the bottom up. (starting at the middle of the array
			// since the single-node subtrees at the leaves are already heaps)
			int index;
			for (index = Num/2; index >= 1; index--) Downheap(index);
		}

		~LODHeap(void) {
//			delete []Nodes;
		}

		LODHeapNode	*Top(void) {
			return &(Nodes[1]);
		}

		// This changes the key value of the entry at the top of the heap and
		// adjusts the heap accordingly.
		void Change_Key_Top(float new_key) {
			Nodes[1].Set_Key(new_key);
			Downheap(1);
		}

		// This searches for an entry which has a certain Item value, and
		// changes its key to a new one. The heap is then adjusted accordingly.
		void Change_Key(RenderObjClass *item, float new_key) {
			for (int i=1; i <= Num; i++) {
				if (Nodes[i].Get_Item() == item) {
					float old_key = Nodes[i].Get_Key();
					Nodes[i].Set_Key(new_key);
					// If the key has been decreased, adjust the node downwards.
					// Otherwise, adjust it upwards.
					if (new_key < old_key) Downheap(i);
					else Upheap(i);
					break;
				}
			}
		}

	private:
		LODHeap(void) {}	// Just to ensure the default constructor is not used.

		// The node and key arrays have one extra entry because entry [0] is
		// reserved for various uses.
		LODHeapNode *		Nodes;	// The nodes

		int					Max;		// The maximum number of nodes
		int					Num;		// the current number of nodes

		// Two utility methods used by various other methods: both take a
		// single entry which violates the heap condition and moves it in the
		// heap until the heap condition is fulfilled.

		// Upheap takes an entry with a (possibly) overlarge key and moves it
		// up until the heap condition is satisfied. (this is a private
		// method, so no error checking is needed).
		// Note that Upheap puts a sentinel in Nodes[0].
		void Upheap(int index) {
			LODHeapNode node = Nodes[index];
			Nodes[0].Set_Key(FLT_MAX);
			while (Nodes[index/2] <= node) {
				Nodes[index] = Nodes[index/2];
				index = index/2;
			}
			Nodes[index] = node;
		}

		// Downheap takes an entry with a (possibly) oversmall key and moves it
		// down until the heap condition is satisfied. (this is a private
		// method, so no error checking is needed).
		void Downheap(int index) {
			LODHeapNode node = Nodes[index];
			while (index <= Num/2) {
				int child_index = index + index;
				if ((child_index < Num) && (Nodes[child_index] < Nodes[child_index+1])) child_index++;
				if (node >= Nodes[child_index]) break;
				Nodes[index] = Nodes[child_index];
				index = child_index;
			}
			Nodes[index] = node;
		}
};

// Static PredictiveLODOptimizerClass data members:
RenderObjClass **	PredictiveLODOptimizerClass::ObjectArray = NULL;
int					PredictiveLODOptimizerClass::ArraySize = 0;
int					PredictiveLODOptimizerClass::NumObjects = 0;
float					PredictiveLODOptimizerClass::TotalCost = 0.0f;
LODHeapNode *		PredictiveLODOptimizerClass::VisibleObjArray1;
LODHeapNode	*		PredictiveLODOptimizerClass::VisibleObjArray2;
int					PredictiveLODOptimizerClass::VisibleObjArraySize;


/**************************************************************************
 * PredictiveLODOptimizerClass::Clear -- clear object list and total cost *
 *                                                                        *
 * INPUT:	                                                              *
 *                                                                        *
 * OUTPUT:	                                                              *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/12/1999 NH  : Created.                                            *
 *========================================================================*/
void PredictiveLODOptimizerClass::Clear(void)
{
	if (ObjectArray) {
		// Release refs to all objects in the list:
		for (int i = 0; i < NumObjects; i++) {
			if (ObjectArray[i]) {
				ObjectArray[i]->Release_Ref();
				ObjectArray[i] = NULL;
			}
		}
	}

	TotalCost = 0.0f;
	NumObjects = 0;
}


/**************************************************************************
 * PredictiveLODOptimizerClass::Add_Object -- adds object to list, cost   *
 *                                                                        *
 * INPUT:	                                                              *
 *                                                                        *
 * OUTPUT:	                                                              *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/12/1999 NH  : Created.                                            *
 *========================================================================*/
void PredictiveLODOptimizerClass::Add_Object(RenderObjClass *robj)
{
	// If array present but too small, free it and copy it to new array.
	if (ObjectArray) {
		if (ArraySize <= NumObjects) {
			int new_array_size = NumObjects + 100;
			RenderObjClass **new_array = W3DNEWARRAY RenderObjClass *[new_array_size];
			memcpy(new_array, ObjectArray, sizeof(RenderObjClass *) * NumObjects);
			delete [] ObjectArray;
			ObjectArray = new_array;
			ArraySize = new_array_size;
		}
	} else {
		// Create new object array.
		ObjectArray = W3DNEWARRAY RenderObjClass *[100];
	}

	// Copy pointer and add ref
	ObjectArray[NumObjects] = robj;
	ObjectArray[NumObjects]->Add_Ref();
	NumObjects++;

	float cost = robj->Get_Cost();
	// Some sanity checking so one object doesn't mess up the entire scene
	WWASSERT (cost >= 0.0f);
	WWASSERT (cost < 1.0e6);
	TotalCost += cost;
}


/**************************************************************************
 * PredictiveLODOptimizerClass::Optimize_LODs -- does LOD optimization    *
 *                                                                        *
 * INPUT:	float max_cost - the upper bound on the total scene Cost.     *
 *                                                                        *
 * OUTPUT:	none.                                                         *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   12/08/1997 NH  : Created.                                            *
 *   04/23/1997 NH  : Ported to SR 1.3.                                   *
 *   03/12/1999 NH  : Moved to PredictiveLODOptimizerClass.               *
 *                                                                        *
 * COMMENTS:                                                              *
 *   This function implements the algorithm outlined in "Adaptive         *
 *   Display Algorithm for Interactive Frame Rates During Visualization   *
 *   of Complex Virtual Environments", Thomas Funkhouser & Carlo Sequin,  *
 *   SIGGRAPH '93 Proceedings, pp. 247-253.                               *
 *   Modifications have been made to support screensize clamping of LODs. *
 *========================================================================*/
void PredictiveLODOptimizerClass::Optimize_LODs(float max_cost)
{
	if (!ObjectArray || NumObjects == 0) return;

	AllocVisibleObjArrays(NumObjects);

	// Allocate and fill arrays. (one extra entry since the zeroth entry is not used).
//	LODHeapNode *visible_obj_array1 = W3DNEWARRAY LODHeapNode[NumObjects + 1];
//	LODHeapNode *visible_obj_array2 = W3DNEWARRAY LODHeapNode[NumObjects + 1];

	// Insert objects into arrays: (0th entry reserved for sentinel values)
	for (int i = 0; i < NumObjects; i++) {
		RenderObjClass *robj = ObjectArray[i];
		// We use minus Value for the first queue to make it ordered by minimum Value.
		VisibleObjArray1[i + 1] = LODHeapNode(robj, -(robj->Get_Value()));
		VisibleObjArray2[i + 1] = LODHeapNode(robj, robj->Get_Post_Increment_Value());
	}

	// Build priority queues:
	LODHeap min_current_value_queue(NumObjects, VisibleObjArray1);
	LODHeap max_post_increment_value_queue(NumObjects, VisibleObjArray2);
	// These memory areas now are pointed to within the heaps:
//	visible_obj_array1 = NULL;
//	visible_obj_array2 = NULL;

	// Main loop: iteratively increment/decrement tuples.
	bool done = false;
	RenderObjClass *max_data, *min_data;

	while (!done) {
		// Initialize max_data and min_data so comparison at end of loop uses correct values.
		max_data = NULL;
		min_data = NULL;

		// Increment incrementable tuple with maximum next value.
		if (TotalCost <= max_cost) {
			// If tuple with maximum next value is already at maximum lod, all
			// tuples are (since AT_MAX_LOD is smaller than any Value), so stop.
			if (max_post_increment_value_queue.Top()->Get_Key() == RenderObjClass::AT_MAX_LOD) {
				done = true;
				break;
			}

			// Get (incrementable) tuple with maximum next value.
		 	max_data = max_post_increment_value_queue.Top()->Get_Item();

			// Increment tuple (and update TotalCost accordingly).
			TotalCost -= max_data->Get_Cost();
			max_data->Increment_LOD();
			TotalCost += max_data->Get_Cost();

			// Update priority queues with incremented tuple.
			max_post_increment_value_queue.Change_Key_Top(max_data->Get_Post_Increment_Value());
			min_current_value_queue.Change_Key(max_data, -(max_data->Get_Value()));
		}

		// Decrement decerementable tuples with minimum current value.
		while (TotalCost > max_cost) {
			// If tuple with minimum current value is already at minimum lod, all
			// tuples are (since AT_MIN_LOD is smaller than any (negated) Value),
			// so stop.
			if (min_current_value_queue.Top()->Get_Key() == -RenderObjClass::AT_MIN_LOD) {
				done = true;
				break;
			}

			// Get (decrementable) tuple with minimum current value.
		 	min_data = min_current_value_queue.Top()->Get_Item();

			// Decrement tuple (and update TotalCost accordingly).
			TotalCost -= min_data->Get_Cost();
			min_data->Decrement_LOD();
			TotalCost += min_data->Get_Cost();

			// Update priority queues with incremented tuple.
			min_current_value_queue.Change_Key_Top(-(min_data->Get_Value()));
			max_post_increment_value_queue.Change_Key(min_data, min_data->Get_Post_Increment_Value());

			// Check termination criterion (same tuple incremented and decremented).
			if (max_data == min_data) {
				done = true;
				break;
			}
		}
	}

	// Clear optimizer:
	Clear();
}


/**************************************************************************
 * PredictiveLODOptimizerClass::Free -- releases all memory used.         *
 *                                                                        *
 * INPUT:	                                                              *
 *                                                                        *
 * OUTPUT:	                                                              *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/12/1999 NH  : Created.                                            *
 *========================================================================*/
void PredictiveLODOptimizerClass::Free(void)
{
	Clear();

	if (ObjectArray) {
		delete [] ObjectArray;
		ObjectArray = NULL;
		ArraySize = 0;
	}

	// Only the array number one has been allocated...
	if (VisibleObjArray1) delete[] VisibleObjArray1;
	VisibleObjArray1=NULL;
	VisibleObjArray2=NULL;
	VisibleObjArraySize = 0;
}

void PredictiveLODOptimizerClass::AllocVisibleObjArrays(int num_objects)
{
	if (VisibleObjArraySize<num_objects) {
		VisibleObjArraySize=num_objects;
		if (VisibleObjArray1) delete[] VisibleObjArray1;	// Only the first array is actually allocated
		VisibleObjArray1=W3DNEWARRAY LODHeapNode[2*(num_objects + 1)];
		VisibleObjArray2=VisibleObjArray1+(num_objects + 1);
	}
}

