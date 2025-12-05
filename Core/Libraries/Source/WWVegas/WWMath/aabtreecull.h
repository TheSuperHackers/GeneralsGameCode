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
 *                     $Archive:: /Commando/Code/wwmath/aabtreecull.h                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/24/01 10:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "cullsys.h"
#include "aaplane.h"
#include "wwmath.h"
#include "mempool.h"
#include "simplevec.h"
#include <math.h>
#include <float.h>

class AABTreeNodeClass;
class ChunkLoadClass;
class ChunkSaveClass;
class SphereClass;

/**
** AABTreeCullSystemClass
** Derived culling system that uses an Axis-Aligned Bounding Box Tree
*/
class AABTreeCullSystemClass : public CullSystemClass
{
public:

	AABTreeCullSystemClass(void);
	virtual ~AABTreeCullSystemClass(void);

	/*
	** Re-partition the tree.  Two methods can be used to accomplish this.  The
	** first re-partitions the tree based on the objects contained within, the second
	** re-partitions the tree based solely on a set of input "seed" boxes.  Each seed
	** box will become a leaf; then the objects will be re-inserted in the new tree.
	*/
	void					Re_Partition(void);
	void					Re_Partition(const AABoxClass & bounds,SimpleDynVecClass<AABoxClass> & boxes);

	/*
	** Update_Bounding_Boxes.  This function causes all bounding boxes in the tree to update themselves.
	** If any box is found to not bound the objects it is supposed to contain, the box is updated
	** Note that this is normally not necessary, the reason this function existsis due to the fact
	** that the renegade level editor tries to do everything possible to not discard the precalculated
	** visibilty data for a level.  In some cases, we want to load geometry that has been edited back
	** into the same AABTree without re-partitioning.
	*/
	void					Update_Bounding_Boxes(void);

	/*
	** Re-insert an object into the tree
	*/
	virtual void		Update_Culling(CullableClass * obj);

	/*
	** Statistics about the AAB-Tree
	*/
	int					Partition_Node_Count(void) const;
	int					Partition_Tree_Depth(void) const;
	int					Object_Count(void) const;

	/*
	** Collect objects which overlap the given primitive
	*/
	virtual void		Collect_Objects(const Vector3 & point);
	virtual void		Collect_Objects(const AABoxClass & box);
	virtual void		Collect_Objects(const OBBoxClass & box);
	virtual void		Collect_Objects(const FrustumClass & frustum);
	virtual void		Collect_Objects(const SphereClass & sphere);

	/*
	** Load and Save a description of this AAB-Tree and its contents
	*/
	virtual void		Load(ChunkLoadClass & cload);
	virtual void		Save(ChunkSaveClass & csave);

	/*
	** Save an objects linkage, load the linkage and re-link the object
	*/
	void					Load_Object_Linkage(ChunkLoadClass & cload,CullableClass * obj);
	void					Save_Object_Linkage(ChunkSaveClass & csave,CullableClass * obj);

	/*
	** Bounding box of the entire tree
	*/
	const AABoxClass & Get_Bounding_Box(void);
	void					Get_Node_Bounds(int node_id,AABoxClass * set_bounds);

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

protected:

	/*
	** Internal stat tracking
	*/
#ifdef WWDEBUG
	void	NODE_ACCEPTED(void)					{ Stats.NodesAccepted ++; }
	void	NODE_TRIVIALLY_ACCEPTED(void)		{ Stats.NodesTriviallyAccepted ++; }
	void	NODE_REJECTED(void)					{ Stats.NodesRejected ++; }
#else
	void	NODE_ACCEPTED(void)					{ }
	void	NODE_TRIVIALLY_ACCEPTED(void)		{ }
	void	NODE_REJECTED(void)					{ }
#endif

	/*
	** Internal functions
	*/
	void					Add_Object_Internal(CullableClass * obj,int node_index = -1);
	void					Remove_Object_Internal(CullableClass * obj);

	void					Re_Index_Nodes(void);
	void					Re_Index_Nodes_Recursive(AABTreeNodeClass * node,int & counter);

	int					Partition_Node_Count_Recursive(AABTreeNodeClass * node) const;
	void					Partition_Tree_Depth_Recursive(AABTreeNodeClass * node,int cur_depth,int & max_depth) const;
	void					Add_Object_Recursive(AABTreeNodeClass * node,CullableClass * obj);
	void					Add_Loaded_Object(AABTreeNodeClass * node,CullableClass * obj);

	void					Collect_Objects_Recursive(AABTreeNodeClass * node);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const Vector3 & point);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const AABoxClass & box);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const OBBoxClass & box);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const FrustumClass & frustum);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const FrustumClass & frustum,int planes_passed);
	void					Collect_Objects_Recursive(AABTreeNodeClass * node,const SphereClass & sphere);

	void					Update_Bounding_Boxes_Recursive(AABTreeNodeClass * node);

	void					Load_Nodes(AABTreeNodeClass * node,ChunkLoadClass & cload);
	void					Save_Nodes(AABTreeNodeClass * node,ChunkSaveClass & csave);

	virtual void		Load_Node_Contents(AABTreeNodeClass * /*node*/,ChunkLoadClass & /*cload*/) { }
	virtual void		Save_Node_Contents(AABTreeNodeClass * /*node*/,ChunkSaveClass & /*csave*/) { }

	AABTreeNodeClass *	RootNode;			// root of the AAB-Tree
	int						ObjectCount;		// number of objects in the system

	int						NodeCount;			// number of nodes
	AABTreeNodeClass **	IndexedNodes;		// index access to the nodes

	StatsStruct				Stats;

	friend class AABTreeIterator;
};


/**
** AABTreeIterator
** This iterator allows the user to walk a tree.  It can return the index of the current
** node and the bounds of the current node.
*/
class AABTreeIterator
{
public:
	AABTreeIterator(AABTreeCullSystemClass * tree);

	void					Reset(void);
	bool					Enter_Parent(void);
	bool					Enter_Sibling(void);
	bool					Has_Front_Child(void);
	bool					Enter_Front_Child(void);
	bool					Has_Back_Child(void);
	bool					Enter_Back_Child(void);

	int					Get_Current_Node_Index(void);
	void					Get_Current_Box(AABoxClass * set_box);

private:

	void					validate(void);

	AABTreeCullSystemClass * Tree;
	int					CurNodeIndex;

};


/**
** TypedAABTreeCullSystemClass
** This template adds type-safety to an AABTree.  It allows you to create trees
** containing a particular type of object (the class must be derived from CullableClass though)
*/
template <class T> class TypedAABTreeCullSystemClass : public AABTreeCullSystemClass
{
public:

	virtual void		Add_Object(T * obj,int node_index=-1)	{ Add_Object_Internal(obj,node_index); }
	virtual void		Remove_Object(T * obj)						{ Remove_Object_Internal(obj); }

	T *					Get_First_Collected_Object(void)		{ return (T*)Get_First_Collected_Object_Internal(); }
	T *					Get_Next_Collected_Object(T * obj)	{ return (T*)Get_Next_Collected_Object_Internal(obj); }
	T *					Peek_First_Collected_Object(void)	{ return (T*)Peek_First_Collected_Object_Internal(); }
	T *					Peek_Next_Collected_Object(T * obj)	{ return (T*)Peek_Next_Collected_Object_Internal(obj); }
};




/**
** AABTreeNodeClass - the aab-tree is built out of these objects
** CullableClass's can be linked into any of these nodes. Whenever the
** tree is re-built, all objects will end up in the leaf nodes.  Between
** re-builds, as objects are added, they will be placed as deep into the
** tree as possible.
*/
class AABTreeNodeClass : public AutoPoolClass<AABTreeNodeClass,256>
{

public:

	AABTreeNodeClass(void);
	~AABTreeNodeClass(void);

	void						Add_Object(CullableClass * obj,bool update_bounds = true);
	void						Remove_Object(CullableClass * obj);
	int						Object_Count(void);
	CullableClass *		Peek_Object(int index);

	uint32					Index;				// Index of this node
	AABoxClass				Box;					// Bounding box of the node
	AABTreeNodeClass *	Parent;				// parent of this node
	AABTreeNodeClass *	Front;				// front node
	AABTreeNodeClass *	Back;					// back node
	CullableClass *		Object;				// objects in this node
	uint32					UserData;			// 32bit field for the user, initialized to 0

	/*
	** Construction support:
	*/
	struct SplitChoiceStruct
	{
		SplitChoiceStruct(void) : Cost(FLT_MAX),FrontCount(0),BackCount(0),Plane(AAPlaneClass::XNORMAL,0.0f)
		{
			FrontBox.Init_Empty();
			BackBox.Init_Empty();
		}

		float					Cost;
		int					FrontCount;
		int					BackCount;
		MinMaxAABoxClass	FrontBox;
		MinMaxAABoxClass	BackBox;
		AAPlaneClass		Plane;
	};

	void						Compute_Bounding_Box(void);
	void						Compute_Local_Bounding_Box(void);
	float						Compute_Volume(void);
	void						Transfer_Objects(AABTreeNodeClass * dummy_node);

	/*
	** Partition the tree based on the objects contained.
	*/
	void						Partition(void);
	void						Split_Objects(	const SplitChoiceStruct & sc,
													AABTreeNodeClass * front,
													AABTreeNodeClass * back);

	/*
	** Partition the tree based on a set of input "seed" boxes.
	*/
	void						Partition(const AABoxClass & bounds,SimpleDynVecClass<AABoxClass> & boxes);
	void						Split_Boxes(	const SplitChoiceStruct & sc,
													SimpleDynVecClass<AABoxClass> & boxes,
													SimpleDynVecClass<AABoxClass> & frontboxes,
													SimpleDynVecClass<AABoxClass> & backboxes);

	/*
	** Functions used by both partitioning algorithms
	*/
	void						Select_Splitting_Plane(SplitChoiceStruct * sc,SimpleDynVecClass<AABoxClass> & boxes);
	void						Select_Splitting_Plane_Brute_Force(SplitChoiceStruct * sc,SimpleDynVecClass<AABoxClass> & boxes);
	void						Compute_Score(SplitChoiceStruct * sc,SimpleDynVecClass<AABoxClass> & boxes);
};


/*
** AABTreeLinkClass
** This structure is used to link objects into an AAB-Tree culling system.
*/
class AABTreeLinkClass : public CullLinkClass, public AutoPoolClass<AABTreeLinkClass,256>
{
public:
	AABTreeLinkClass(AABTreeCullSystemClass * system) : CullLinkClass(system),Node(nullptr), NextObject(nullptr) { }

	AABTreeNodeClass *				Node;					// partition node containing this object
	CullableClass *					NextObject;			// next object in the node
};
