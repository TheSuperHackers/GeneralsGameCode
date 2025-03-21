/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/ww3d2/aabtree.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/14/01 9:42a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef AABTREE_H
#define AABTREE_H

#include "always.h"
#include "refcount.h"
#include "simplevec.h"
#include "vector3.h"
#include "Vector3i.h"
#include "aaplane.h"
#include "bittype.h"
#include "colmath.h"
#include "wwdebug.h"
#include "aabtreebuilder.h"
#include "obbox.h"
#include <tri.h>
#include <float.h>


class MeshClass;
class CameraClass;
class RayCollisionTestClass;
class AABoxCollisionTestClass;
class OBBoxCollisionTestClass;
class OBBoxIntersectionTestClass;
class ChunkLoadClass;
class ChunkSaveClass;
class MeshGeometryClass;
class OBBoxClass;
class ChunkLoadClass;

struct BoxRayAPTContextStruct;

#define AABTREE_LEAF_FLAG 0x80000000


/*
** AABTreeClass
** This class encapsulates an Axis-Aligned Bounding Box Tree for a mesh.  This tree
** can be used to perform hierarchical culling for collision detection.  Note that
** this class is constructed using the AABTreeBuilderClass; these two classes are
** very tightly coupled.  Pretty much the only code which needs to know about the AABTreeClass
** is in MeshGeometryClass.  I moved these out into a separate file just to reduce the
** size of meshmdl.cpp.
*/
class AABTreeClass : public W3DMPO, public RefCountClass
{
	W3DMPO_GLUE(AABTreeClass)
public:

	AABTreeClass(void);
	AABTreeClass(AABTreeBuilderClass * builder);
	AABTreeClass(const AABTreeClass & that);
	~AABTreeClass(void);

	void						Load_W3D(ChunkLoadClass & cload);

	int						Get_Node_Count(void) { return NodeCount; }
	int						Get_Poly_Count(void) { return PolyCount; }
	int						Compute_Ram_Size(void);
	void						Generate_APT(const OBBoxClass & box,SimpleDynVecClass<uint32> & apt);
	void						Generate_APT(const OBBoxClass & box,const Vector3 & viewdir,SimpleDynVecClass<uint32> & apt);

	bool						Cast_Ray(RayCollisionTestClass & raytest);
	int						Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 & start_point,
									int axis_dir, unsigned char & flags);
	bool						Cast_AABox(AABoxCollisionTestClass & boxtest);
	bool						Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	bool						Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

private:
	
	AABTreeClass &			operator = (const AABTreeClass & that);

	void						Read_Poly_Indices(ChunkLoadClass & cload);
	void						Read_Nodes(ChunkLoadClass & cload);
	
	void						Build_Tree_Recursive(AABTreeBuilderClass::CullNodeStruct * node,int &curpolyindex);
	void						Reset(void);
	void						Set_Mesh(MeshGeometryClass * mesh);
	void						Update_Bounding_Boxes(void);
	void						Update_Min_Max(int index,Vector3 & min,Vector3 & max);

	/*
	** CullNodeStruct - the culling tree is built out of an array of these structures
	** They contain the extents of an axis-aligned box, indices to children nodes,
	** and indices into the polygon array
	** (05/22/2000 gth - changed this structure to support either child nodes -or-
	** a polygon array but not both at the same time.  Also switched to 32bit indices
	** so that the code doesn't become useless as quickly )
	*/
	struct CullNodeStruct
	{
		Vector3				Min;
		Vector3				Max;
		
		uint32				FrontOrPoly0;
		uint32				BackOrPolyCount;

		// accessors
		inline bool			Is_Leaf(void);				
		
		inline int			Get_Back_Child(void);		// returns index of back child (only call for non-LEAFs!!!)
		inline int			Get_Front_Child(void);		// returns index of front child (only call for non-LEAFs!!!)
		inline int			Get_Poly0(void);				// returns index of first polygon (only call on LEAFs)
		inline int			Get_Poly_Count(void);		// returns polygon count (only call on LEAFs)

		// initialization
		inline void			Set_Front_Child(uint32 index);
		inline void			Set_Back_Child(uint32 index);
		inline void			Set_Poly0(uint32 index);
		inline void			Set_Poly_Count(uint32 count);
	};

	/*
	** OBBoxAPTContextStruct - this is a temporary datastructure used in building
	** an APT by culling the mesh to an oriented bounding box.
	*/
	struct OBBoxAPTContextStruct
	{
		OBBoxAPTContextStruct(const OBBoxClass & box,SimpleDynVecClass<uint32> & apt) : 
			Box(box), APT(apt)
		{ }

		OBBoxClass							Box;
		SimpleDynVecClass<uint32> &	APT;
	};

	/**
	** OBBoxRayAPTContextStruct - temporary datastructure used in building an APT
	** by culling the mesh to a oriented box and eliminating backfaces to a ray.
	*/
	struct OBBoxRayAPTContextStruct
	{
		OBBoxRayAPTContextStruct(const OBBoxClass & box,const Vector3 & viewdir,SimpleDynVecClass<uint32> & apt) :
			Box(box),
			ViewVector(viewdir),
			APT(apt)
		{ }

		OBBoxClass							Box;
		Vector3								ViewVector;
		SimpleDynVecClass<uint32> &	APT;
	};

	void						Generate_OBBox_APT_Recursive(CullNodeStruct * node,OBBoxAPTContextStruct & context);
	void						Generate_OBBox_APT_Recursive(CullNodeStruct * node, OBBoxRayAPTContextStruct & context);

	bool						Cast_Ray_Recursive(CullNodeStruct * node,RayCollisionTestClass & raytest);
	int						Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(CullNodeStruct * node, const Vector3 & start_point,
									int axis_r, int axis_1, int axis_2, int direction, unsigned char & flags);
	bool						Cast_AABox_Recursive(CullNodeStruct * node,AABoxCollisionTestClass & boxtest);
	bool						Cast_OBBox_Recursive(CullNodeStruct * node,OBBoxCollisionTestClass & boxtest);
	bool						Intersect_OBBox_Recursive(CullNodeStruct * node,OBBoxIntersectionTestClass & boxtest);

	bool						Cast_Ray_To_Polys(CullNodeStruct * node,RayCollisionTestClass & raytest);
	int						Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(CullNodeStruct * node, const Vector3 & start_point,
									int axis_r, int axis_1, int axis_2, int direction, unsigned char & flags);
	bool						Cast_AABox_To_Polys(CullNodeStruct * node,AABoxCollisionTestClass & boxtest);
	bool						Cast_OBBox_To_Polys(CullNodeStruct * node,OBBoxCollisionTestClass & boxtest);
	bool						Intersect_OBBox_With_Polys(CullNodeStruct * node,OBBoxIntersectionTestClass & boxtest);

	void						Update_Bounding_Boxes_Recursive(CullNodeStruct * node);

	int						NodeCount;			// number of nodes in the tree
	CullNodeStruct *		Nodes;				// array of nodes
	int						PolyCount;			// number of polygons in the parent mesh (and the number of indexes in our array)
	uint32 *					PolyIndices;		// linear array of polygon indices, nodes index into this array
	MeshGeometryClass *	Mesh;					// pointer to the parent mesh (non-ref-counted; we are a member of this mesh)

	friend class MeshClass;
	friend class MeshGeometryClass;
	friend class AuxMeshDataClass;
	friend class AABTreeBuilderClass;

};

inline int AABTreeClass::Compute_Ram_Size(void) 
{ 
	return	NodeCount * sizeof(CullNodeStruct) +
				PolyCount * sizeof(int) +
				sizeof(AABTreeClass);
}

inline bool AABTreeClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	WWASSERT(Nodes != NULL);
	return Cast_Ray_Recursive(&(Nodes[0]),raytest);
}

inline int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 & start_point,
	int axis_dir, unsigned char & flags)
{
	// These tables translate between the axis_dir representation (which is an integer in which 0
	// indicates a ray along the positive x axis, 1 along the negative x axis, 2 the positive y
	// axis, 3 negative y axis, 4 positive z axis, 5 negative z axis) and a four-integer
	// representation (axis_r is the axis number - 0, 1 or 2 - of the axis along which the ray is
	// cast; axis_1 and axis_2 are the axis numbers of the other two axes; direction is 0 for
	// negative and 1 for positive direction of the ray).
	static const int axis_r[6] =		{ 0, 0, 1, 1, 2, 2 };
	static const int axis_1[6] =		{ 1, 1, 2, 2, 0, 0 };
	static const int axis_2[6] =		{ 2, 2, 0, 0, 1, 1 };
	static const int direction[6] =	{ 1, 0, 1, 0, 1, 0 };
	WWASSERT(Nodes != NULL);
	WWASSERT(axis_dir >= 0);
	WWASSERT(axis_dir < 6);

	// The functions called after this point will 'or' bits into this variable, so it needs to
	// be initialized here to TRI_RAYCAST_FLAG_NONE.
	flags = TRI_RAYCAST_FLAG_NONE;
	
	return Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(&(Nodes[0]), start_point,
		axis_r[axis_dir], axis_1[axis_dir], axis_2[axis_dir], direction[axis_dir], flags);
}

inline bool AABTreeClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	WWASSERT(Nodes != NULL);
	return Cast_AABox_Recursive(&(Nodes[0]),boxtest);
}

inline bool AABTreeClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	WWASSERT(Nodes != NULL);
	return Cast_OBBox_Recursive(&(Nodes[0]),boxtest);
}

inline bool AABTreeClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	WWASSERT(Nodes != NULL);
	return Intersect_OBBox_Recursive(&(Nodes[0]),boxtest);
}

inline void AABTreeClass::Update_Bounding_Boxes(void)
{
	WWASSERT(Nodes != NULL);
	Update_Bounding_Boxes_Recursive(&(Nodes[0]));
}


/***********************************************************************************************

  AABTreeClass::CullNodeStruct implementation

  These nodes can be either leaf nodes or non-leaf nodes.  If they are leaf nodes, they
  will contain an index to their first polygon index and a polygon count.  If they are
  non-leafs they will contain indices to their front and back children.  Since I'm re-using
  the same variables for the child indices and the polygon indices, you have to call
  the Is_Leaf function then only call the appropriate functions.  The flag indicating whether
  this node is a leaf is stored in the MSB of the FrontOrPoly0 variable.  It will always
  be stripped off by these accessor functions

***********************************************************************************************/

inline bool AABTreeClass::CullNodeStruct::Is_Leaf(void)
{
	return ((FrontOrPoly0 & AABTREE_LEAF_FLAG) != 0);
}

inline int AABTreeClass::CullNodeStruct::Get_Front_Child(void)
{
	WWASSERT(!Is_Leaf());
	return FrontOrPoly0;		// we shouldn't be calling this on a leaf and the leaf bit should be zero...
}

inline int AABTreeClass::CullNodeStruct::Get_Back_Child(void)
{
	WWASSERT(!Is_Leaf());
	return BackOrPolyCount;
}

inline int AABTreeClass::CullNodeStruct::Get_Poly0(void)
{
	WWASSERT(Is_Leaf());
	return (FrontOrPoly0 & ~AABTREE_LEAF_FLAG);
}

inline int AABTreeClass::CullNodeStruct::Get_Poly_Count(void)
{
	WWASSERT(Is_Leaf());
	return BackOrPolyCount;
}

inline void AABTreeClass::CullNodeStruct::Set_Front_Child(uint32 index)
{
	WWASSERT(index < 0x7FFFFFFF);
	FrontOrPoly0 = index;
}

inline void AABTreeClass::CullNodeStruct::Set_Back_Child(uint32 index)
{
	WWASSERT(index < 0x7FFFFFFF);
	BackOrPolyCount = index;
}

inline void AABTreeClass::CullNodeStruct::Set_Poly0(uint32 index)
{
	WWASSERT(index < 0x7FFFFFFF);
	FrontOrPoly0 = (index | AABTREE_LEAF_FLAG);
}

inline void AABTreeClass::CullNodeStruct::Set_Poly_Count(uint32 count)
{
	WWASSERT(count < 0x7FFFFFFF);
	BackOrPolyCount = count;
}

#endif
