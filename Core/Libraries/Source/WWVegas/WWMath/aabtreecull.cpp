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
 *                     $Archive:: /Commando/Code/wwmath/aabtreecull.cpp                       $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/26/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "aabtreecull.h"
#include "chunkio.h"
#include "iostruct.h"
#include <string.h>
#include "sphere.h"
#include "colmath.h"
#include "colmathinlines.h"



/*
** Declare the pools
*/
DEFINE_AUTO_POOL(AABTreeLinkClass,256);
DEFINE_AUTO_POOL(AABTreeNodeClass,256);


/*
** Current version of the file format
*/
const uint32 AABTREE_CURRENT_VERSION = 0x00010000;


/*
** Chunk Id's used by the aabtree code to save itself into a file
*/
enum 
{
	AABTREE_CHUNK_VERSION					= 0x00000001,	// version wrapper, contains 32bit version #
	AABTREE_CHUNK_AABNODE					= 0x00000101,	// generic aab-node wrapper
		AABTREE_CHUNK_AABNODE_INFO,							// OBSOLETE! generic aab-node definition (IOAABNodeStruct)
		AABTREE_CHUNK_AABNODE_CONTENTS,						// wrapper around contents of the node
		AABTREE_CHUNK_AABNODE_VARIABLES,						// wrapper around variables for a node

	AABTREE_CHUNK_NODE_INDEX				= 0x00000200,	// wrapper around the node index for an object

	AABTREE_VARIABLE_NODESTRUCT			= 0x00,
	AABTREE_VARIABLE_USERDATA
};


/*
** IOAABNodeStruct
** Data structure for the contents of a node in the AAB-Tree
*/
#define AABNODE_ATTRIBUTE_FRONT_CHILD		0x00000001
#define AABNODE_ATTRIBUTE_BACK_CHILD		0x00000002

struct IOAABNodeStruct
{
	IOVector3Struct	Center;
	IOVector3Struct	Extent;
	uint32				Attributes;
};


/*************************************************************************
**
** Utility functions for walking the object list in an AABTree Node
**
*************************************************************************/
static inline CullableClass * get_first_object(AABTreeNodeClass * node)
{
	return node->Object;		
}

static inline CullableClass * get_next_object(CullableClass * obj)
{
	return ((AABTreeLinkClass *)obj->Get_Cull_Link())->NextObject;
}


/*************************************************************************
**
** AABTreeCullSystemClass Implementation
**
*************************************************************************/
AABTreeCullSystemClass::AABTreeCullSystemClass(void) :
	ObjectCount(0),
	NodeCount(0),
	IndexedNodes(NULL)
{
	RootNode = new AABTreeNodeClass;
	Re_Index_Nodes();
}

AABTreeCullSystemClass::~AABTreeCullSystemClass(void)
{
	// Delete all links and release-ref all cullables:
	int nidx;
	for (nidx = 0; nidx < NodeCount; nidx++) {
		while(IndexedNodes[nidx]->Object) Remove_Object_Internal(IndexedNodes[nidx]->Object);
	}

	// Delete node tree (deleting the root recursively deletes all nodes)
	delete RootNode;

	// Delete indexed node pointer array
	if (IndexedNodes) {
		delete[] IndexedNodes;
		IndexedNodes = NULL;
	}
}


void AABTreeCullSystemClass::Add_Object_Internal(CullableClass * obj,int node_index)
{
	WWASSERT_PRINT
	(
		(obj->Get_Culling_System() == NULL),
		"AABTreeCullSystemClass::Add -- Object is already in another culling system!\n"
	);

	AABTreeLinkClass * new_link = new AABTreeLinkClass(this);
	obj->Set_Cull_Link(new_link);

	if (node_index == -1) {
		Add_Object_Recursive(RootNode,obj);
	} else {
		WWASSERT(node_index < NodeCount);
		IndexedNodes[node_index]->Add_Object(obj,false);
		ObjectCount++;
	}

	obj->Add_Ref();
}


void AABTreeCullSystemClass::Remove_Object_Internal(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link);

	AABTreeNodeClass * node = link->Node;
	WWASSERT(node);

	node->Remove_Object(obj);
	link->Set_Culling_System(NULL);
	delete link;
	obj->Set_Cull_Link(NULL);
	
	ObjectCount--;
	WWASSERT(ObjectCount >= 0);
	obj->Release_Ref();
}

void AABTreeCullSystemClass::Update_Culling(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	// unlink it from the node it is currently in
	AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link);
	AABTreeNodeClass * node = link->Node;
	WWASSERT(node);
	node->Remove_Object(obj);
	// decrement the object counter, the node can't
	// decrement it for us...
	ObjectCount--;

	// drop it into the tree again
	Add_Object_Recursive(RootNode,obj);
}

void AABTreeCullSystemClass::Collect_Objects(const Vector3 & point)
{
	Collect_Objects_Recursive(RootNode,point);
}

void AABTreeCullSystemClass::Collect_Objects(const AABoxClass & box)
{
	Collect_Objects_Recursive(RootNode,box);
}

void AABTreeCullSystemClass::Collect_Objects(const OBBoxClass & box)
{
	Collect_Objects_Recursive(RootNode,box);
}

void AABTreeCullSystemClass::Collect_Objects(const FrustumClass & frustum)
{
	Collect_Objects_Recursive(RootNode,frustum,0);
}

void AABTreeCullSystemClass::Collect_Objects(const SphereClass & sphere)
{
	Collect_Objects_Recursive(RootNode,sphere);
}

int AABTreeCullSystemClass::Partition_Node_Count(void) const
{
	return Partition_Node_Count_Recursive(RootNode);
}

int AABTreeCullSystemClass::Partition_Tree_Depth(void) const
{
	int get_max_depth = 0;
	Partition_Tree_Depth_Recursive(RootNode,0,get_max_depth);
	return get_max_depth;
}

int AABTreeCullSystemClass::Object_Count(void) const
{
	return ObjectCount;
}

int AABTreeCullSystemClass::Partition_Node_Count_Recursive(AABTreeNodeClass * node) const
{
	int curcount = 1;
	if (node->Front) {
		curcount += Partition_Node_Count_Recursive(node->Front);
	}
	if (node->Back) {
		curcount += Partition_Node_Count_Recursive(node->Back);
	}
	return curcount;
}

void AABTreeCullSystemClass::Partition_Tree_Depth_Recursive(AABTreeNodeClass * node,int cur_depth,int & max_depth) const
{
	cur_depth++;
	if (cur_depth > max_depth) {
		max_depth = cur_depth;
	}
	if (node->Front) {
		Partition_Tree_Depth_Recursive(node->Front,cur_depth,max_depth);
	} 
	if (node->Back) {
		Partition_Tree_Depth_Recursive(node->Back,cur_depth,max_depth);
	}
}

void AABTreeCullSystemClass::Add_Object_Recursive(AABTreeNodeClass * node,CullableClass * obj)
{
	// order the children in terms of size
	AABTreeNodeClass * big_child = node->Front;
	AABTreeNodeClass * small_child = node->Back;

	if (big_child && small_child && (big_child->Compute_Volume() < small_child->Compute_Volume())) {
		AABTreeNodeClass * tmp = big_child;
		big_child = small_child;
		small_child = tmp;
	}
	
	// Can we fit in the smaller child?
	if (small_child && small_child->Box.Contains(obj->Get_Cull_Box())) {
		Add_Object_Recursive(small_child,obj);
		return;
	}

	// Can we fit in the bigger child?
	if (big_child && big_child->Box.Contains(obj->Get_Cull_Box())) {
		Add_Object_Recursive(big_child,obj);
		return;
	}

	// Ok, we have to fit in this node.
	node->Add_Object(obj);
	ObjectCount++;
}

void AABTreeCullSystemClass::Add_Loaded_Object(AABTreeNodeClass * node,CullableClass * obj)
{
	WWASSERT(node);
	WWASSERT(obj);

	WWASSERT_PRINT
	(
		(obj->Get_Culling_System() == NULL),
		"AABTreeCullSystemClass::Add_Loaded_Object -- Object is already in another culling system!\n"
	);

	AABTreeLinkClass * new_link = new AABTreeLinkClass(this);
	obj->Set_Cull_Link(new_link);

	node->Add_Object(obj);
	ObjectCount++;
	obj->Add_Ref();
}

void AABTreeCullSystemClass::Re_Partition(void)
{
	/*
	** transfer all objects to a temporary node
	*/
	AABTreeNodeClass * dummy_node = new AABTreeNodeClass;
	RootNode->Transfer_Objects(dummy_node);

	/*
	** delete the old tree and make the dummy node the new root
	*/
	delete RootNode;
	RootNode = dummy_node;

	/*
	** partition the objects
	*/
	RootNode->Partition();

	/*
	** re-index the nodes
	*/
	Re_Index_Nodes();

	/*
	** reset the statistics
	*/
	Reset_Statistics();

}

void AABTreeCullSystemClass::Re_Partition(const AABoxClass & bounds,SimpleDynVecClass<AABoxClass> & boxes)
{
	/*
	** transfer all objects to a temporary node
	*/
	AABTreeNodeClass * dummy_node = new AABTreeNodeClass;
	RootNode->Transfer_Objects(dummy_node);

	/*
	** delete the old tree
	*/
	delete RootNode;

	/*
	** allocate a new root node and tell it to partition the given array of boxes
	*/
	RootNode = new AABTreeNodeClass;
	RootNode->Partition(bounds,boxes);

	/*
	** re-index the nodes
	*/
	Re_Index_Nodes();

	/*
	** reset statistics
	*/
	Reset_Statistics();

	/*
	** re-insert all objects and delete the temporary node
	*/
	dummy_node->Box.Extent.Set(0,0,0);
	CullableClass * obj = get_first_object(dummy_node);
	while (obj != NULL) {
		Update_Culling(obj);
		obj = get_first_object(dummy_node);
	}

	delete dummy_node;

	/*
	** Modify the root node so that any object can be added into the tree
	*/
	RootNode->Box.Extent.Set(FLT_MAX,FLT_MAX,FLT_MAX);
}

void AABTreeCullSystemClass::Update_Bounding_Boxes(void)
{
	Update_Bounding_Boxes_Recursive(RootNode);
}

const AABoxClass & AABTreeCullSystemClass::Get_Bounding_Box(void)
{
	WWASSERT(RootNode);
	return RootNode->Box;
}

void AABTreeCullSystemClass::Get_Node_Bounds(int node_id,AABoxClass * set_bounds)
{
	if ((node_id >= 0) && (node_id < NodeCount)) {
		*set_bounds = IndexedNodes[node_id]->Box;
	} else {
		*set_bounds = IndexedNodes[0]->Box;
	}
}

void AABTreeCullSystemClass::Reset_Statistics(void)
{
	Stats.NodeCount = NodeCount;
	Stats.NodesAccepted = 0;
	Stats.NodesTriviallyAccepted = 0;
	Stats.NodesRejected = 0;
}

const AABTreeCullSystemClass::StatsStruct & AABTreeCullSystemClass::Get_Statistics(void)
{
	return Stats;
}

void AABTreeCullSystemClass::Collect_Objects_Recursive(AABTreeNodeClass * node)
{
	/*
	** Collect any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			Add_To_Collection(obj);
			obj = get_next_object(obj);
		}
	}

	/*
	** Statistics
	*/
	NODE_TRIVIALLY_ACCEPTED();

	/*
	** Descend into the children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front);
	}
}


void AABTreeCullSystemClass::Collect_Objects_Recursive(AABTreeNodeClass * node,const Vector3 & point)
{
	/*
	** Is the point inside this volume?
	*/
	if (node->Box.Contains(point) == false) {
		NODE_REJECTED();
		return;
	} 

	NODE_ACCEPTED();

	/*
	** Collect any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			if (obj->Get_Cull_Box().Contains(point)) {
				Add_To_Collection(obj);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Descend into the children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back,point);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front,point);
	}
}

void AABTreeCullSystemClass::Collect_Objects_Recursive(AABTreeNodeClass * node,const AABoxClass & box)
{
	/*
	** Cull the given box against the bounding volume of this node
	** If it is culled, stop descending the tree.  If the current node is
	** completely contained inside the given box, jump into the collection function
	** that doesn't do any more volume checking...
	*/
	CollisionMath::OverlapType overlap = CollisionMath::Overlap_Test(box,node->Box);
	if (overlap == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	} else if (overlap == CollisionMath::INSIDE) {
		Collect_Objects_Recursive(node);
		return;
	}
	
	NODE_ACCEPTED();

	/*
	** Test any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			if (CollisionMath::Overlap_Test(box,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Recurse into any children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back,box);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front,box);
	}
}


void AABTreeCullSystemClass::Collect_Objects_Recursive(AABTreeNodeClass * node,const OBBoxClass & box)
{
	/*
	** Cull the given box against the bounding volume of this node
	** If it is culled, stop descending the tree.  If the current node is
	** completely contained inside the given box, jump into the collection function
	** that doesn't do any more volume checking...
	*/
	CollisionMath::OverlapType overlap = CollisionMath::Overlap_Test(box,node->Box);
	if (overlap == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	} else if (overlap == CollisionMath::INSIDE) {
		Collect_Objects_Recursive(node);
		return;
	}
	
	NODE_ACCEPTED();

	/*
	** Test any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			if (CollisionMath::Overlap_Test(box,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Recurse into any children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back,box);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front,box);
	}
}

void AABTreeCullSystemClass::Collect_Objects_Recursive
(
	AABTreeNodeClass * node,
	const FrustumClass & frustum,
	int planes_passed
)
{
	/*
	** Cull the bounding volume of this node against the frustum.
	** If it is culled, stop descending the tree.
	*/
	CollisionMath::OverlapType overlap = CollisionMath::Overlap_Test(frustum,node->Box,planes_passed);
	if (overlap == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	} else if (overlap == CollisionMath::INSIDE) {
		Collect_Objects_Recursive(node);
		return;
	}

	NODE_ACCEPTED();
	
	/*
	** Test any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			if (CollisionMath::Overlap_Test(frustum,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Recurse into any children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back,frustum,planes_passed);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front,frustum,planes_passed);
	}
}


void AABTreeCullSystemClass::Collect_Objects_Recursive(AABTreeNodeClass * node,const SphereClass & sphere)
{
	/*
	** Is the point inside this volume?
	*/	
	if (CollisionMath::Overlap_Test (node->Box, sphere) == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	}
	NODE_ACCEPTED();

	/*
	** Collect any objects in this node
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			if (CollisionMath::Overlap_Test (obj->Get_Cull_Box(), sphere) != CollisionMath::OUTSIDE) {
				Add_To_Collection(obj);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Descend into the children
	*/
	if (node->Back) {
		Collect_Objects_Recursive(node->Back,sphere);
	}
	if (node->Front) {
		Collect_Objects_Recursive(node->Front,sphere);
	}
}

void AABTreeCullSystemClass::Update_Bounding_Boxes_Recursive(AABTreeNodeClass * node)
{
	MinMaxAABoxClass minmaxbox(node->Box);

	/*
	** Update child boxes first and ensure that we bound them
	*/
	if (node->Front) {
		Update_Bounding_Boxes_Recursive(node->Front);
		minmaxbox.Add_Box(node->Front->Box);
	}
	if (node->Back) {
		Update_Bounding_Boxes_Recursive(node->Back);
		minmaxbox.Add_Box(node->Back->Box);
	}

	/*
	** Make sure we bound our contained objects
	*/
	if (node->Object) {
		CullableClass * obj = get_first_object(node);
		while (obj) {
			minmaxbox.Add_Box(obj->Get_Cull_Box());
			obj = get_next_object(obj);
		}
	}

	node->Box.Init_Min_Max(minmaxbox.MinCorner,minmaxbox.MaxCorner);
}


void AABTreeCullSystemClass::Load(ChunkLoadClass & cload)
{
	WWASSERT_PRINT(Object_Count() == 0, "Remove all objects from AAB-Culling system before loading!\n"); 
	
	delete RootNode;
	RootNode = new AABTreeNodeClass;

	// The first chunk should be a version chunk
	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() != AABTREE_CHUNK_VERSION) {
		WWDEBUG_SAY(("Attempting to read an obsolete AAB-Tree!"));
		cload.Close_Chunk();
		return;
	}

	// read in the version and verify that it is the current version
	uint32 version;
	cload.Read(&version,sizeof(version));
	if (version != AABTREE_CURRENT_VERSION) {
		WWDEBUG_SAY(("Attempting to read an obsolete AAB-Tree!"));
		cload.Close_Chunk();
		return;
	}
	cload.Close_Chunk();

	// read in the tree
	Load_Nodes(RootNode,cload);

	// re-index all nodes
	Re_Index_Nodes();

	// reset the statistics
	Reset_Statistics();
}

void AABTreeCullSystemClass::Load_Nodes(AABTreeNodeClass * node,ChunkLoadClass & cload)
{
	// Open the node description
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == AABTREE_CHUNK_AABNODE);
	
	// Load the node description.  
	// Older files will contain a chunk named AABTREE_CHUNK_AABNODE_INFO while newer
	// files will contain AABTREE_CHUNK_AABNODE_VARIABLES which contains the IOAABNodeStruct
	// in a micro-chunk.
	IOAABNodeStruct node_desc;
	memset(&node_desc,0,sizeof(IOAABNodeStruct));
	
	cload.Open_Chunk(); 
	if (cload.Cur_Chunk_ID() == AABTREE_CHUNK_AABNODE_INFO) {

		// Loading the legacy format...
		WWASSERT(cload.Cur_Chunk_ID() == AABTREE_CHUNK_AABNODE_INFO);
		cload.Read(&node_desc,sizeof(node_desc));

	} else if (cload.Cur_Chunk_ID() == AABTREE_CHUNK_AABNODE_VARIABLES) {
	
		// Loading the new format, contains micro chunks...
		while (cload.Open_Micro_Chunk()) {
			switch(cload.Cur_Micro_Chunk_ID()) {
				READ_MICRO_CHUNK(cload,AABTREE_VARIABLE_NODESTRUCT,node_desc);
				READ_MICRO_CHUNK(cload,AABTREE_VARIABLE_USERDATA,node->UserData);
			}
			cload.Close_Micro_Chunk();	
		}
	}
	cload.Close_Chunk();

	// Initialize the node bounds.
	node->Box.Center.X = node_desc.Center.X;
	node->Box.Center.Y = node_desc.Center.Y;
	node->Box.Center.Z = node_desc.Center.Z;

	node->Box.Extent.X = node_desc.Extent.X;
	node->Box.Extent.Y = node_desc.Extent.Y;
	node->Box.Extent.Z = node_desc.Extent.Z;

	// Load the contents of the node.
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == AABTREE_CHUNK_AABNODE_CONTENTS);
	Load_Node_Contents(node,cload);
	cload.Close_Chunk();

	// Close the node description
	cload.Close_Chunk(); 

	// if we are supposed to have a front child, load it
	if (node_desc.Attributes & AABNODE_ATTRIBUTE_FRONT_CHILD) {
		WWASSERT(node->Front == NULL);
		node->Front = new AABTreeNodeClass();
		node->Front->Parent = node;
		Load_Nodes(node->Front,cload);
	}
	
	// if we have a back child, load it
	if (node_desc.Attributes & AABNODE_ATTRIBUTE_BACK_CHILD) {
		WWASSERT(node->Back == NULL);
		node->Back = new AABTreeNodeClass();
		node->Back->Parent = node;
		Load_Nodes(node->Back,cload);
	}
}

void AABTreeCullSystemClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(AABTREE_CHUNK_VERSION);
	uint32 version = AABTREE_CURRENT_VERSION;
	csave.Write(&version,sizeof(uint32));
	csave.End_Chunk();

	Save_Nodes(RootNode,csave);
}

void AABTreeCullSystemClass::Save_Nodes(AABTreeNodeClass * node,ChunkSaveClass & csave)
{
	WWASSERT(node);
	csave.Begin_Chunk(AABTREE_CHUNK_AABNODE);

	csave.Begin_Chunk(AABTREE_CHUNK_AABNODE_VARIABLES);
	IOAABNodeStruct node_desc;
	memset(&node_desc,0,sizeof(node_desc));

	node_desc.Center.X = node->Box.Center.X;
	node_desc.Center.Y = node->Box.Center.Y;
	node_desc.Center.Z = node->Box.Center.Z;

	node_desc.Extent.X = node->Box.Extent.X;
	node_desc.Extent.Y = node->Box.Extent.Y;
	node_desc.Extent.Z = node->Box.Extent.Z;

	if (node->Front) {
		node_desc.Attributes |= AABNODE_ATTRIBUTE_FRONT_CHILD;
	}

	if (node->Back) {
		node_desc.Attributes |= AABNODE_ATTRIBUTE_BACK_CHILD;
	}
	
	WRITE_MICRO_CHUNK(csave,AABTREE_VARIABLE_NODESTRUCT,node_desc);
	WRITE_MICRO_CHUNK(csave,AABTREE_VARIABLE_USERDATA,node->UserData);
	csave.End_Chunk();

	csave.Begin_Chunk(AABTREE_CHUNK_AABNODE_CONTENTS);
	Save_Node_Contents(node,csave);
	csave.End_Chunk();

	csave.End_Chunk();

	if (node->Front) {
		Save_Nodes(node->Front,csave);
	}

	if (node->Back) {
		Save_Nodes(node->Back,csave);
	}
}

void AABTreeCullSystemClass::Load_Object_Linkage(ChunkLoadClass & cload,CullableClass * obj)
{
	uint32 index;
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == AABTREE_CHUNK_NODE_INDEX);
	cload.Read(&index,sizeof(index));
	cload.Close_Chunk();

	Add_Object_Internal(obj,index);
}

void AABTreeCullSystemClass::Save_Object_Linkage(ChunkSaveClass & csave,CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link);

	AABTreeNodeClass * node = link->Node;
	WWASSERT(node);

	uint32 index = node->Index;
	csave.Begin_Chunk(AABTREE_CHUNK_NODE_INDEX);
	csave.Write(&index,sizeof(index));
	csave.End_Chunk();
}


void AABTreeCullSystemClass::Re_Index_Nodes(void)
{
	if (IndexedNodes != NULL) {
		delete[] IndexedNodes;
		IndexedNodes = NULL;
	}
	NodeCount = Partition_Node_Count();
	WWASSERT(NodeCount > 0);
	IndexedNodes = new AABTreeNodeClass *[NodeCount];
	
	int counter = 0;
	Re_Index_Nodes_Recursive(RootNode,counter);
	WWASSERT(counter == NodeCount);
}


void AABTreeCullSystemClass::Re_Index_Nodes_Recursive(AABTreeNodeClass * node,int & counter)
{
	node->Index = counter;
	IndexedNodes[counter] = node;
	counter++;

	if (node->Front) {
		Re_Index_Nodes_Recursive(node->Front,counter);
	}
	if (node->Back) {
		Re_Index_Nodes_Recursive(node->Back,counter);
	}
}



/*************************************************************************
**
** AABTreeNodeClass Implementation
**
*************************************************************************/
AABTreeNodeClass::AABTreeNodeClass(void) :
	Index(0),	
	Box(Vector3(0,0,0),Vector3(0,0,0)),
	Parent(NULL),
	Front(NULL),
	Back(NULL),
	Object(NULL),
	UserData(0)
{
}

AABTreeNodeClass::~AABTreeNodeClass(void)
{
	// objects should be removed before deleting the partition tree
	WWASSERT(Object == NULL);
	
	// delete our children
	if (Front) {
		delete Front;
		Front = NULL;
	}
	if (Back) {
		delete Back;
		Back = NULL;
	}
}

void AABTreeNodeClass::Compute_Bounding_Box(void)
{
	/*
	** make the children update their boxes first
	*/
	if (Front) {
		Front->Compute_Bounding_Box();
	}

	if (Back) { 
		Back->Compute_Bounding_Box();
	}

	Compute_Local_Bounding_Box();
}

void AABTreeNodeClass::Compute_Local_Bounding_Box(void)
{
	/*
	** Now make sure we bound our children
	*/
	MinMaxAABoxClass box(Vector3(FLT_MAX,FLT_MAX,FLT_MAX),Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX));
	
	if (Front) {
		box.Add_Box(Front->Box);
	}

	if (Back) {
		box.Add_Box(Back->Box);
	}

	/*
	** bound the objects in this node
	*/
	CullableClass * obj = Object;
	while (obj) {
		box.Add_Box(obj->Get_Cull_Box());
		AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
		obj = link->NextObject;
	}

	Box.Init(box);
}

float AABTreeNodeClass::Compute_Volume(void)
{
	return Box.Volume();
}

void AABTreeNodeClass::Add_Object(CullableClass * obj,bool update_bounds)
{
	AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link);
	
	link->Node = this;
	link->NextObject = Object;
	Object = obj;

	if (update_bounds) {
		// if this is the only object and we have no children, just copy 
		// the object's bounding box, otherwise, add it to what we have
		if ((Object_Count() == 1) && (Front == NULL) && (Back == NULL)) {
			Box = obj->Get_Cull_Box();
		} else {
			Box.Add_Box(obj->Get_Cull_Box());
		}
	}
}

void AABTreeNodeClass::Remove_Object(CullableClass * obj)
{
	WWASSERT(obj);

	// find the given object in our linked list
	CullableClass * prevobj = NULL;
	CullableClass * curobj = Object;
	
	while (curobj) {

		AABTreeLinkClass * link = (AABTreeLinkClass *)curobj->Get_Cull_Link();

		if (curobj == obj) {
			// found the object, unlink it.
			if (prevobj) {
				AABTreeLinkClass * prevlink = (AABTreeLinkClass *)prevobj->Get_Cull_Link();
				prevlink->NextObject = link->NextObject;
			} else {
				Object = link->NextObject;
			}

			link->NextObject = NULL;
			link->Node = NULL;
			return;
		}

		// move to the next object
		prevobj = curobj;
		curobj = link->NextObject;
	}
}

void AABTreeNodeClass::Transfer_Objects(AABTreeNodeClass * dummy_node)
{
	// unlink all of our objects, relinking them to the dummy_node
	while (Object) {
		CullableClass * obj = Object;
		Remove_Object(obj);
		dummy_node->Add_Object(obj);
	}

	// do the same with our children
	if (Front) {
		Front->Transfer_Objects(dummy_node);
	}
	
	if (Back) {
		Back->Transfer_Objects(dummy_node);
	}
}

int AABTreeNodeClass::Object_Count(void)
{
	CullableClass * obj = Object;
	int count = 0;

	while (obj) {
		count++;
		obj = ((AABTreeLinkClass *)obj->Get_Cull_Link())->NextObject;
	}

	return count;
}

CullableClass * AABTreeNodeClass::Peek_Object(int index)
{
	int count = 0;
	CullableClass * obj = Object;
	WWASSERT(obj != NULL);

	while (obj && (count != index)) {
		count++;				
		obj = ((AABTreeLinkClass *)obj->Get_Cull_Link())->NextObject;
	}
	WWASSERT(count == index);
	return obj;
}


/******************************************************************************************
**
** Partitioning code which partitions the objects in the tree and passes them into
** the new children
**
******************************************************************************************/

void AABTreeNodeClass::Partition(void)
{
	/*
	** if we're down to only 2 objects, we're done
	*/
	int obj_count = Object_Count();
	if (obj_count <= 2) return;

	/*
	** Create an array of the bounding boxes of our objects
	*/
	SimpleDynVecClass<AABoxClass> boxes(obj_count);
	CullableClass * obj = Object;
	while (obj != NULL) {
		boxes.Add(obj->Get_Cull_Box());		
		obj = get_next_object(obj);
	}

	/*
	** Select and assign the splitting plane
	** De-allocate the array of boxes to conserve memory
	*/
	SplitChoiceStruct sc;
	Select_Splitting_Plane(&sc,boxes);
	boxes.Resize(0);
	
	/*
	** If there was no good split, just leave all of
	** the objects in this node
	*/
	if (sc.Cost == FLT_MAX) {
		return;
	}

	/*
	** Split the tiles 
	*/
	AABTreeNodeClass * front = new AABTreeNodeClass;
	AABTreeNodeClass * back = new AABTreeNodeClass;
	Split_Objects(sc,front,back);

	/*
	** Build a front tree if necessary.
	*/
	if (front->Object_Count() > 0) {
		Front = front;
		Front->Parent = this;
		Front->Partition();
	} else {
		delete front;
		front = NULL;
	}
	
	/*
	** Build a back tree if necessary. 
	*/
	if (back->Object_Count() > 0) {
		Back = back;
		Back->Parent = this;
		Back->Partition();
	} else {
		delete back;
		back = NULL;
	}
}



void AABTreeNodeClass::Split_Objects(const AABTreeNodeClass::SplitChoiceStruct & sc,AABTreeNodeClass * front,AABTreeNodeClass * back)
{
	// This function assumes that this node is a leaf
	WWASSERT(Front == NULL);
	WWASSERT(Back == NULL);
	WWASSERT(Object_Count() == sc.FrontCount + sc.BackCount);

	int fcount = 0;
	int bcount = 0;
	
	// unlink all of our objects, relinking them to the appropriate node
	while (Object) {
		
		// pull the object out of this node
		CullableClass * obj = Object;
		Remove_Object(Object);
		
		// decide which node to add the object to, 
		// NOTE: we have to use the same convention as Compute_Score!
		const AABoxClass & box = obj->Get_Cull_Box();

		if (CollisionMath::Overlap_Test(sc.Plane,box.Center) == CollisionMath::FRONT) {

			front->Add_Object(obj);
			fcount++;

		} else {

			back->Add_Object(obj);
			bcount++;

		}
	}

	// copy the bounding boxes
	front->Box = sc.FrontBox;
	front->Box.Extent += Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);
	back->Box = sc.BackBox;
	back->Box.Extent += Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);

	// when we are all done, the counts should match. 
	WWASSERT(fcount == sc.FrontCount);
	WWASSERT(bcount == sc.BackCount);
}




/******************************************************************************************
**
** Partitioning code which generates the tree based on a set of input boxes
**
******************************************************************************************/
void AABTreeNodeClass::Partition(const AABoxClass & bounds,SimpleDynVecClass<AABoxClass> & boxes)
{
	Box = bounds;

	/*
	** if we're down to only 1 box, we're done
	*/
	if (boxes.Count() <= 1) return;

	/*
	** Select and assign the splitting plane
	*/
	SplitChoiceStruct sc;
	Select_Splitting_Plane(&sc,boxes);
	
	/*
	** If there was no good split, just leave all of
	** the objects in this node
	*/
	if (sc.Cost == FLT_MAX) {
		return;
	}

	/*
	** Split the boxes
	** Deallocate the input box array to conserve RAM
	*/
	SimpleDynVecClass<AABoxClass> frontboxes(sc.FrontCount);
	SimpleDynVecClass<AABoxClass> backboxes(sc.BackCount);
	Split_Boxes(sc,boxes,frontboxes,backboxes);
	boxes.Delete_All();

	/*
	** Build a front tree if necessary.
	*/
	if (frontboxes.Count() > 0) {
		Front = new AABTreeNodeClass;
		Front->Parent = this;
		Front->Partition(sc.FrontBox,frontboxes);
	} else {
		Front = NULL;
	}
	
	/*
	** Build a back tree if necessary. 
	*/
	if (backboxes.Count() > 0) {
		Back = new AABTreeNodeClass;
		Back->Parent = this;
		Back->Partition(sc.BackBox,backboxes);
	} else {
		Back = NULL;
	}
}

void AABTreeNodeClass::Split_Boxes
(
	const AABTreeNodeClass::SplitChoiceStruct & sc,
	SimpleDynVecClass<AABoxClass> & boxes,
	SimpleDynVecClass<AABoxClass> & frontboxes,
	SimpleDynVecClass<AABoxClass> & backboxes
)
{
	WWASSERT(boxes.Count() == sc.FrontCount + sc.BackCount);

	// copy each box in the input array into the appropriate output array
	for (int i=0; i<boxes.Count(); i++) {
		
		const AABoxClass & box = boxes[i];

		if (CollisionMath::Overlap_Test(sc.Plane,box.Center) == CollisionMath::FRONT) {

			frontboxes.Add(box);

		} else {

			backboxes.Add(box);

		}
	}

	// when we are all done, the counts should match. 
	WWASSERT(frontboxes.Count() == sc.FrontCount);
	WWASSERT(backboxes.Count() == sc.BackCount);
}



/******************************************************************************************
**
** Partitioning code which searches for the best split for a given set of boxes.  This
** code is re-used by both types of partitioning (partitioning contained objects or
** partitioning based on a set of "seed" boxes.)
**
******************************************************************************************/

void AABTreeNodeClass::Select_Splitting_Plane
(
	SplitChoiceStruct * sc,
	SimpleDynVecClass<AABoxClass> & boxes
)
{
	const int NUM_TRYS = 300;

	/*
	** Try putting axis-aligned planes through some random vertices
	*/
	int objcount = boxes.Count();	
	int trys = 0;
	for (trys = 0; trys < MIN(NUM_TRYS,objcount); trys++) {

		int obj_index;
		SplitChoiceStruct test;
		
		/*
		** Select a random object
		*/
		obj_index = rand() % objcount;
		const AABoxClass & box = boxes[obj_index];

		/*
		** Select a random plane which co-incides with one of the faces
		** of the object's bounding box
		*/
		switch(rand() % 6) {
			case 0:	test.Plane.Set(AAPlaneClass::XNORMAL,box.Center.X + box.Extent.X);	break;
			case 1:	test.Plane.Set(AAPlaneClass::XNORMAL,box.Center.X - box.Extent.X);	break;
			case 2:	test.Plane.Set(AAPlaneClass::YNORMAL,box.Center.Y + box.Extent.Y);	break;
			case 3:	test.Plane.Set(AAPlaneClass::YNORMAL,box.Center.Y - box.Extent.Y);	break;
			case 4:	test.Plane.Set(AAPlaneClass::ZNORMAL,box.Center.Z + box.Extent.Z);	break;
			case 5:	test.Plane.Set(AAPlaneClass::ZNORMAL,box.Center.Z - box.Extent.Z);	break;
		};

		/*
		** Get the score for this plane
		*/
		Compute_Score(&test,boxes);
		if (test.Cost < sc->Cost) {
			*sc = test;
		}

	}

	/*
	** Still haven't found a valid splitting plane, uh-oh.
	*/
	if ((trys >= MIN(NUM_TRYS,objcount)) && (sc->Cost == FLT_MAX)) {
		Select_Splitting_Plane_Brute_Force(sc,boxes);
		return;
	}
}

void AABTreeNodeClass::Select_Splitting_Plane_Brute_Force
(
	AABTreeNodeClass::SplitChoiceStruct * sc,
	SimpleDynVecClass<AABoxClass> & boxes
)
{ 
	/*
	** Try putting axis-aligned planes along each face of each box
	*/
	int objcount = boxes.Count();	
	for (int obj_index = 0; obj_index < objcount; obj_index++) {

		AAPlaneClass plane;
		const AABoxClass & box = boxes[obj_index];

		/*
		** Try each face of this box 
		*/
		for (int plane_index = 0; plane_index < 6; plane_index++) {
			SplitChoiceStruct test;
			switch(plane_index % 6) {
				case 0:	test.Plane.Set(AAPlaneClass::XNORMAL,box.Center.X + box.Extent.X);	break;
				case 1:	test.Plane.Set(AAPlaneClass::XNORMAL,box.Center.X - box.Center.Y);	break;
				case 2:	test.Plane.Set(AAPlaneClass::YNORMAL,box.Center.Y + box.Center.Y);	break;
				case 3:	test.Plane.Set(AAPlaneClass::YNORMAL,box.Center.Y - box.Center.Y);	break;
				case 4:	test.Plane.Set(AAPlaneClass::ZNORMAL,box.Center.Z + box.Center.Z);	break;
				case 5:	test.Plane.Set(AAPlaneClass::ZNORMAL,box.Center.Z - box.Center.Z);	break;
			};

			test.FrontBox.Init_Empty();
			test.BackBox.Init_Empty();

			/*
			** Get the score for this plane
			*/
			Compute_Score(&test,boxes);
			if (test.Cost < sc->Cost) {
				*sc = test;
			}
		}
	}

	/*
	** Notify user that we couldn't split this node
	*/
#ifdef WWDEBUG
	if (sc->Cost == FLT_MAX) {		
		WWDEBUG_SAY(("Unable to split node!  objcount = %d. (%.2f,%.2f,%.2f)",objcount,Box.Center.X, Box.Center.Y, Box.Center.Z));
	}
#endif
}


void AABTreeNodeClass::Compute_Score
(
	AABTreeNodeClass::SplitChoiceStruct * sc,
	SimpleDynVecClass<AABoxClass> & boxes
)
{
	/*
	** Suitability of a plane as a partition plane is based on the following factors:
	** - How many "tiles" are on each side of the plane,
	*/
	for (int i=0; i<boxes.Count(); i++) {

		const AABoxClass & box = boxes[i];

		if (CollisionMath::Overlap_Test(sc->Plane,box.Center) == CollisionMath::FRONT) {

			sc->FrontCount++;	
			sc->FrontBox.Add_Box(box);	

		} else {
			
			sc->BackCount++;	
			sc->BackBox.Add_Box(box);	

		}
	}

	/*
	** Compute the cost.
	*/
	float back_cost = sc->BackBox.Volume() * sc->BackCount;
	float front_cost = sc->FrontBox.Volume() * sc->FrontCount;
	
	sc->Cost = front_cost + back_cost;
	
	if ((sc->FrontCount == 0) || (sc->BackCount == 0)) {
		sc->Cost = FLT_MAX;
	}
}



/**************************************************************************************

  AABTreeIterator Implemenation

**************************************************************************************/


AABTreeIterator::AABTreeIterator(AABTreeCullSystemClass * tree) :
	Tree(tree),
	CurNodeIndex(0)
{
	WWASSERT(Tree != NULL);
}
	
void AABTreeIterator::Reset(void)
{
	CurNodeIndex = 0;
}

bool AABTreeIterator::Enter_Parent(void)						
{
	validate();
	if (CurNodeIndex != 0) {
		CurNodeIndex = Tree->IndexedNodes[CurNodeIndex]->Parent->Index;
		return true;
	} else {
		return false;
	}
}

bool AABTreeIterator::Enter_Sibling(void)
{
	validate();
	if (CurNodeIndex != 0) {
		
		/*
		** find which child of our parent we are
		*/
		AABTreeNodeClass * parent = Tree->IndexedNodes[CurNodeIndex]->Parent;
		AABTreeNodeClass * parent_front = parent->Front;
		AABTreeNodeClass * parent_back = parent->Back;

		/*
		** if our parent doesn't have two children, we don't have a sibling
		*/
		if ((parent_front == NULL) || (parent_back == NULL)) {
			return false;
		}

		/*
		** if we our our parent's front child, go to its back child
		*/
		if ((int)parent_front->Index == CurNodeIndex) {
			CurNodeIndex = parent_back->Index;
			return true;
		}

		/*
		** if we our our parent's back child, go to its front child
		*/
		if ((int)parent_back->Index == (int)CurNodeIndex) {
			CurNodeIndex = parent_front->Index;
			return true;
		}
	}
	return false;
}

bool AABTreeIterator::Has_Front_Child(void)
{
	validate();
	return (Tree->IndexedNodes[CurNodeIndex]->Front != NULL);
}

bool AABTreeIterator::Enter_Front_Child(void)
{
	validate();
	if (Has_Front_Child()) {
		CurNodeIndex = Tree->IndexedNodes[CurNodeIndex]->Front->Index;
		return true;
	}
	return false;
}

bool AABTreeIterator::Has_Back_Child(void)
{
	validate();
	return (Tree->IndexedNodes[CurNodeIndex]->Back != NULL);
}

bool AABTreeIterator::Enter_Back_Child(void)
{
	if (Has_Back_Child()) {
		CurNodeIndex = Tree->IndexedNodes[CurNodeIndex]->Back->Index;
		return true;
	}
	return false;
}

int AABTreeIterator::Get_Current_Node_Index(void)
{
	return CurNodeIndex;
}

void AABTreeIterator::Get_Current_Box(AABoxClass * set_box)
{
	Tree->Get_Node_Bounds(CurNodeIndex,set_box);	
}

void AABTreeIterator::validate(void)
{
	if ((CurNodeIndex < 0) || (CurNodeIndex >= Tree->NodeCount)) {
		CurNodeIndex = 0;
	}
}

/*

  Can we make a more compact AABTree?

  Here is the existing data in each Node:

	uint32					Index;				// Index of this node
	AABoxClass				Box;					// Bounding box of the node
	AABTreeNodeClass *	Parent;				// parent of this node
	AABTreeNodeClass *	Front;				// front node
	AABTreeNodeClass *	Back;					// back node
	CullableClass *		Object;				// objects in this node
	uint32					UserData;			// 32bit field for the user, initialized to 0

	Total Size:				48 bytes

	Here is a possible replacement:

	uint16					Index;
	short						x,y,z,w,l,h;		// Need origin and scale factors for the boxes stored in tree
	uint16					ParentIndex;
	uint16					FrontIndex;
	uint16					BackIndex;
	CullableClass *		Object;
	uint32					UserData;
	
	Total Size:				28 bytes

*/