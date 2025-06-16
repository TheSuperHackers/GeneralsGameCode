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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/aabtree.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/24/01 5:34p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AABTreeClass::AABTreeClass -- Constructor                                                 *
 *   AABTreeClass::AABTreeClass -- Constructor                                                 *
 *   AABTreeClass::AABTreeClass -- copy constructor                                            *
 *   AABTreeClass::~AABTreeClass -- Destructor                                                 *
 *   AABTreeClass::operator =  -- assignment operator                                          *
 *   AABTreeClass::Reset -- reset this tree, releases all allocated resources                  *
 *   AABTreeClass::Build_Tree_Recursive -- Initializes this tree from the given builder        *
 *   AABTreeClass::Set_Mesh -- set the mesh pointer                                            *
 *   AABTreeClass::Generate_APT -- Generate an active poly table for the mesh                  *
 *   AABTreeClass::Generate_OBBox_APT_Recursive -- recursively generate the apt                *
 *   AABTreeClass::Cast_Ray_Recursive -- Internal implementation of Cast_Ray                   *
 *   AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive -- Internal implementation    *
 *   AABTreeClass::Cast_AABox_Recursive -- internal implementation of Cast_AABox               *
 *   AABTreeClass::Cast_OBBox_Recursive -- Internal implementation of Cast_OBBox               *
 *   AABTreeClass::Intersect_OBBox_Recursive -- internal implementation of Intersect_OBBox     *
 *   AABTreeClass::Cast_Ray_To_Polys -- cast the ray to polys in the given node                *
 *   AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys -- cast ray to polys in the nod*
 *   AABTreeClass::Cast_AABox_To_Polys -- cast aabox to polys in the given node                *
 *   AABTreeClass::Cast_OBBox_To_Polys -- cast obbox to polys in the given node                *
 *   AABTreeClass::Intersect_OBBox_With_Polys -- Test polys for intersection                   *
 *   AABTreeClass::Update_Bounding_Boxes_Recursive -- recompute the bounding boxes             *
 *   AABTreeClass::Update_Min_Max -- updates min and max given a polygon index                 *
 *   AABTreeClass::Load_W3D -- Load a W3D description of an AABTree                            *
 *   AABTreeClass::Read_Poly_Indices -- load the polygon index array                           *
 *   AABTreeClass::Read_Nodes -- Load the node array                                           *
 *   AABTreeClass::Generate_APT -- generate an apt from a box and viewdir                      *
 *   AABTreeClass::Generate_OBBox_APT_Recursive -- recurse, generate the apt for a box and vie *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "aabtree.h"
#include "aabtreebuilder.h"
#include "wwdebug.h"
#include "tri.h"
#include "meshgeometry.h"
#include "coltest.h"
#include "inttest.h"
#include "colmathinlines.h"
#include "w3d_file.h"
#include "chunkio.h"



/***********************************************************************************************
 * AABTreeClass::AABTreeClass -- Constructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AABTreeClass::AABTreeClass(void) :
	NodeCount(0),
	Nodes(NULL),
	PolyCount(0),
	PolyIndices(NULL),
	Mesh(NULL)
{
}

/***********************************************************************************************
 * AABTreeClass::AABTreeClass -- Constructor                                                   *
 *                                                                                             *
 *  Builds an AABTree from the contents of an AABTreeBuilderClass.                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *   5/23/2000  gth : Created.                                                                 *
 *=============================================================================================*/
AABTreeClass::AABTreeClass(AABTreeBuilderClass * builder)
{
	NodeCount = builder->Node_Count();
	Nodes = W3DNEWARRAY AABTreeClass::CullNodeStruct[NodeCount];

	PolyCount = builder->Poly_Count();
	PolyIndices = W3DNEWARRAY uint32[PolyCount];

	int curpolyindex = 0;
	Build_Tree_Recursive(builder->Root,curpolyindex);
}


/***********************************************************************************************
 * AABTreeClass::AABTreeClass -- copy constructor                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
AABTreeClass::AABTreeClass(const AABTreeClass & that) :
	NodeCount(0),
	Nodes(NULL),
	PolyCount(0),
	PolyIndices(0),
	Mesh(NULL)
{ 
	*this = that; 
}

/***********************************************************************************************
 * AABTreeClass::~AABTreeClass -- Destructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
AABTreeClass::~AABTreeClass(void)
{
	Reset();
}

/***********************************************************************************************
 * AABTreeClass::operator =  -- assignment operator                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
AABTreeClass & AABTreeClass::operator = (const AABTreeClass & that)
{
	Reset();
	
	NodeCount = that.NodeCount;
	if (NodeCount > 0) {
		Nodes = W3DNEWARRAY CullNodeStruct[NodeCount];
		memcpy(Nodes,that.Nodes,NodeCount * sizeof(CullNodeStruct));
	}

	PolyCount = that.PolyCount;
	if (PolyCount > 0) {
		PolyIndices = W3DNEWARRAY uint32[PolyCount];
		memcpy(PolyIndices,that.PolyIndices,PolyCount * sizeof(uint32));
	}

	Mesh = that.Mesh;

	return *this;
}


/***********************************************************************************************
 * AABTreeClass::Reset -- reset this tree, releases all allocated resources                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Reset(void)
{
	NodeCount = 0;
	if (Nodes) {
		delete[] Nodes;
		Nodes = NULL;
	}
	PolyCount = 0;
	if (PolyIndices) {
		delete[] PolyIndices;
		PolyIndices = NULL;
	}
	if (Mesh) {
		Mesh = NULL;
	}
}

/***********************************************************************************************
 * AABTreeClass::Scale - uniform scale																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/17/02    Jani : Created.                                                                *
 *=============================================================================================*/
void AABTreeClass::Scale(float f)
{
	for (int i=0;i<NodeCount;++i) {
		Nodes[i].Min*=f;
		Nodes[i].Max*=f;
	}
}


/***********************************************************************************************
 * AABTreeClass::Build_Tree_Recursive -- Initializes this tree from the given builder          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Build_Tree_Recursive(AABTreeBuilderClass::CullNodeStruct * node,int & curpolyindex)
{
	/*
	** Copy data from the builder's node into our node
	*/
	CullNodeStruct * newnode = &(Nodes[node->Index]);

	newnode->Min = node->Min;
	newnode->Max = node->Max;
	
	/*
	** If this is a non-leaf node, set up the child indices, otherwise set up the polygon indices
	*/
	if (node->Front != NULL) {

		WWASSERT(node->Back != NULL);		// if we have one child, we better have both!
		newnode->Set_Front_Child(node->Front->Index);
		newnode->Set_Back_Child(node->Back->Index);
	
	} else {

		newnode->Set_Poly0(curpolyindex);
		newnode->Set_Poly_Count(node->PolyCount);

	}

	/*
	** Copy the polygon indices for this node into our array
	*/
	for (int pcounter = 0; pcounter < node->PolyCount; pcounter++) {
		PolyIndices[curpolyindex++] = node->PolyIndices[pcounter];
	}

	/*
	** Install the children
	*/
	if (node->Front) {
		Build_Tree_Recursive(node->Front,curpolyindex);
	}
	if (node->Back) {
		Build_Tree_Recursive(node->Back,curpolyindex);
	}
}


/***********************************************************************************************
 * AABTreeClass::Set_Mesh -- set the mesh pointer                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Set_Mesh(MeshGeometryClass * mesh)
{
	Mesh = mesh;
}


/***********************************************************************************************
 * AABTreeClass::Generate_APT -- Generate an active poly table for the mesh                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 * box - oriented box to cull the mesh against (in the mesh's coordinate system!!!)            *
 * apt - vector of ints to fill the apt into                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void AABTreeClass::Generate_APT(const OBBoxClass & box,SimpleDynVecClass<uint32> & apt)
{
	OBBoxAPTContextStruct context(box,apt);
	Generate_OBBox_APT_Recursive(&(Nodes[0]),context);
}



/***********************************************************************************************
 * AABTreeClass::Generate_OBBox_APT_Recursive -- recursively generate the apt                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * node - current node in the recursion                                                        *
 * context - structure containing the results/current state                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Generate_OBBox_APT_Recursive(CullNodeStruct * node,OBBoxAPTContextStruct & context)
{
	/*
	** Test the volume against the bounding volume of this node
	** If it is outside, stop descending the tree.
	*/
	AABoxClass nodebox;
	nodebox.Init_Min_Max(node->Min,node->Max);				// NOTE: too bad we have to compute the nodebox!!!
	
	if (!CollisionMath::Intersection_Test(context.Box,nodebox)) {
		return;
	}
	
	/*
	** If this is a leaf, test its polygons, otherwise recurse into the children
	*/
	if (node->Is_Leaf()) {

		int polycount = node->Get_Poly_Count();
		int poly0 = node->Get_Poly0();

		if (polycount > 0) {
			TriClass tri;
			const Vector3 * loc = Mesh->Get_Vertex_Array();
			const TriIndex * polys = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
			const Vector4 * norms = Mesh->Get_Plane_Array();
#endif
			for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

				int poly_index = PolyIndices[poly0 + poly_counter];
				
				tri.V[0] = &(loc[ polys[poly_index][0] ]);
				tri.V[1] = &(loc[ polys[poly_index][1] ]);
				tri.V[2] = &(loc[ polys[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
				tri.N = (Vector3*)&(norms[poly_index]);
#else
				Vector3 normal;
				tri.N = &normal;
				tri.Compute_Normal();
#endif
				if (CollisionMath::Intersection_Test(context.Box,tri)) {;
					context.APT.Add(poly_index);
				} 
			}
		}

	} else {

		Generate_OBBox_APT_Recursive(&(Nodes[node->Get_Front_Child()]),context);
		Generate_OBBox_APT_Recursive(&(Nodes[node->Get_Back_Child()]),context);
	}
}


/***********************************************************************************************
 * AABTreeClass::Generate_APT -- generate an apt from a box and viewdir                        *
 *                                                                                             *
 * Indices of the polys which intersect the box and are not backfacing to the given viewdir    *
 * will be added to the passed in apt.                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/10/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Generate_APT
(
	const OBBoxClass & box,
	const Vector3 & viewdir,
	SimpleDynVecClass<uint32> & apt
)
{
	OBBoxRayAPTContextStruct context(box,viewdir,apt);
	Generate_OBBox_APT_Recursive(&(Nodes[0]),context);
}


/***********************************************************************************************
 * AABTreeClass::Generate_OBBox_APT_Recursive -- recurse, generate the apt for a box and viewd *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/10/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Generate_OBBox_APT_Recursive(CullNodeStruct * node, OBBoxRayAPTContextStruct & context)
{
	/*
	** Test the volume against the bounding volume of this node
	** If it is outside, stop descending the tree.
	*/
	AABoxClass nodebox;
	nodebox.Init_Min_Max(node->Min,node->Max);				// NOTE: too bad we have to compute the nodebox!!!
	
	if (!CollisionMath::Intersection_Test(context.Box,nodebox)) {
		return;
	}
	
	/*
	** If this is a leaf, test its polygons, otherwise recurse into the children
	*/
	if (node->Is_Leaf()) {

		int polycount = node->Get_Poly_Count();
		int poly0 = node->Get_Poly0();

		if (polycount > 0) {
			TriClass tri;
			const Vector3 * loc = Mesh->Get_Vertex_Array();
			const TriIndex * polys = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
			const Vector4 * norms = Mesh->Get_Plane_Array();
#endif

			for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

				int poly_index = PolyIndices[poly0 + poly_counter];
				
				tri.V[0] = &(loc[ polys[poly_index][0] ]);
				tri.V[1] = &(loc[ polys[poly_index][1] ]);
				tri.V[2] = &(loc[ polys[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
				tri.N = (Vector3*)&(norms[poly_index]);
#else
				Vector3 normal;
				tri.N = &normal;
				tri.Compute_Normal();
#endif
				if (Vector3::Dot_Product(*tri.N,context.ViewVector) < 0.0f) {
					if (CollisionMath::Intersection_Test(context.Box,tri)) {
						context.APT.Add(poly_index);
					} 
				}
			}
		}

	} else {

		Generate_OBBox_APT_Recursive(&(Nodes[node->Get_Front_Child()]),context);
		Generate_OBBox_APT_Recursive(&(Nodes[node->Get_Back_Child()]),context);
	}
}


/***********************************************************************************************
 * AABTreeClass::Cast_Ray_Recursive -- Internal implementation of Cast_Ray                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 * raytest - contains all of the ray test information                                          *
 * node - current cull node being processed                                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_Ray_Recursive(CullNodeStruct * node,RayCollisionTestClass & raytest)
{
	/*
	** Cull the collision test against the bounding volume of this node
	** If it is culled, stop descending the tree.
	*/
	if (raytest.Cull(node->Min,node->Max)) {
		return false;
	}
	
	bool res = false;
	if (node->Is_Leaf()) {
	
		return Cast_Ray_To_Polys(node,raytest);
	
	} else {

		res = res | Cast_Ray_Recursive(&(Nodes[node->Get_Front_Child()]),raytest);
		res = res | Cast_Ray_Recursive(&(Nodes[node->Get_Back_Child()]),raytest);

	}
	
	return res;
}


/***********************************************************************************************
 * AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive -- Internal implementation      *
 *                                                                                             *
 * INPUT:                                                                                      *
 * node - current cull node being processed                                                    *
 * start_point - starting point                                                                *
 * axis_r - the axis along which the ray is projected                                          *
 * axis_1, axis_2 - the other two axes                                                         *
 * direction - 0 means the ray goes toward the negative axis, 1 means positive.                *
 * flags - reference bitfield for result flags (ray hit edge, start point embedded in tri, etc)*
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/30/00    NH : Created.                                                                  *
 *=============================================================================================*/
int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(CullNodeStruct * node,
	const Vector3 & start_point, int axis_r, int axis_1, int axis_2, int direction,
	unsigned char & flags)
{
	/*
	** Cull the ray against the bounding volume of this node
	** If it is culled, stop descending the tree.
	** We do two main tests: a 2D test against axis1 and axis2 to see if the starting point (and
	** hence the ray) falls within the 2D projection of the bbox, and a 1D test to see if the ray
	** is completely above or below the bbox. The second test checks (start > max) or (start < min)
	** depending on the direction of the ray - we do this in a branchless fashion by turning
	** (start < min) into (-start > -min). Then we can use tables to perform the correct check.
	*/
	static const float sign[2] = { -1.0f, 1.0f };
	float bounds[2], start[2];
	bounds[0] = -node->Min[axis_r];
	bounds[1] = node->Max[axis_r];
	start[0] = -start_point[axis_r];
	start[1] = start_point[axis_r];
	if (	start_point[axis_1] < node->Min[axis_1] || start_point[axis_2] < node->Min[axis_2] ||
			start_point[axis_1] > node->Max[axis_1] || start_point[axis_2] > node->Max[axis_2] ||
			start[direction] > bounds[direction] ) {
		return 0;
	}

	int count = 0;
	if (node->Is_Leaf()) {
	
		return Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(node, start_point, axis_r, axis_1,
			axis_2, direction, flags);
	
	} else {

		count += Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(&(Nodes[node->Get_Front_Child()]),
			start_point, axis_r, axis_1, axis_2, direction, flags);
		count += Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(&(Nodes[node->Get_Back_Child()]),
			start_point, axis_r, axis_1, axis_2, direction, flags);

	}
	
	return count;
}


/***********************************************************************************************
 * AABTreeClass::Cast_AABox_Recursive -- internal implementation of Cast_AABox                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 * boxtest - contains description of the collision operation to be performed                   *
 * node - current cull node being processed                                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_AABox_Recursive(CullNodeStruct * node,AABoxCollisionTestClass & boxtest)
{
	/*
	** First, check the bounding box of the move against the bounding box
	** of this tree, if they don't intersect, bail out
	*/
	if (boxtest.Cull(node->Min,node->Max)) {
		return false;
	}

	bool res = false;
	if (node->Is_Leaf()) {

		return Cast_AABox_To_Polys(node,boxtest);

	} else {

		res = res | Cast_AABox_Recursive(&(Nodes[node->Get_Front_Child()]),boxtest);
		res = res | Cast_AABox_Recursive(&(Nodes[node->Get_Back_Child()]),boxtest);

	}
	
	return res;
}


/***********************************************************************************************
 * AABTreeClass::Cast_OBBox_Recursive -- Internal implementation of Cast_OBBox                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 * boxtest - contains description of the collision test to be performed                        *
 * node - current cull node being processed                                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_OBBox_Recursive(CullNodeStruct * node,OBBoxCollisionTestClass & boxtest)
{
	/*
	** First, check the bounding box of the move against the bounding box
	** of this tree, if they don't intersect, bail out
	*/
	if (boxtest.Cull(node->Min,node->Max)) {
		return false;
	}

	bool res = false;
	if (node->Is_Leaf()) {

		return Cast_OBBox_To_Polys(node,boxtest);
	
	} else {

		res = res | Cast_OBBox_Recursive(&(Nodes[node->Get_Front_Child()]),boxtest);
		res = res | Cast_OBBox_Recursive(&(Nodes[node->Get_Back_Child()]),boxtest);

	}

	return res;
}


/***********************************************************************************************
 * AABTreeClass::Intersect_OBBox_Recursive -- internal implementation of Intersect_OBBox       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/20/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Intersect_OBBox_Recursive(AABTreeClass::CullNodeStruct * node,OBBoxIntersectionTestClass & test)
{
	/*
	** Cull the collision test against the bounding volume of this node
	** If it is culled, stop descending the tree.
	*/
	if (test.Cull(node->Min,node->Max)) {
		return false;
	}

	bool res = false;
	if (node->Is_Leaf()) {

		return Intersect_OBBox_With_Polys(node,test);
		
	} else {

		res = res | Intersect_OBBox_Recursive(&(Nodes[node->Get_Front_Child()]),test);
		res = res | Intersect_OBBox_Recursive(&(Nodes[node->Get_Back_Child()]),test);
	}

	return res;
}

#ifdef RTS_DEBUG
#pragma optimize("", off)	 // We get an odd error when using optimized in the debug.  
// All optimized seems to work.  jba.
#endif
/***********************************************************************************************
 * AABTreeClass::Cast_Ray_To_Polys -- cast the ray to polys in the given node                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_Ray_To_Polys(CullNodeStruct * node,RayCollisionTestClass & raytest)
{
	if (node->Get_Poly_Count() > 0) {
		/*
		** Simply loop through the polys in this node, checking each for collision
		*/
		TriClass tri;

		const Vector3 * loc = Mesh->Get_Vertex_Array();
		const TriIndex * polyverts = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
		const Vector4 * norms = Mesh->Get_Plane_Array();
#endif

		int polyhit = -1;
		int poly0 = node->Get_Poly0();
		int polycount = node->Get_Poly_Count();

		for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

			int poly_index = PolyIndices[poly0 + poly_counter];

			tri.V[0] = &(loc[ polyverts[poly_index][0] ]);
			tri.V[1] = &(loc[ polyverts[poly_index][1] ]);
			tri.V[2] = &(loc[ polyverts[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
			tri.N = (Vector3*)&(norms[poly_index]);
#else
			Vector3 normal;
			tri.N = &normal;
			tri.Compute_Normal();
#endif
			if (CollisionMath::Collide(raytest.Ray,tri,raytest.Result)) {;
				polyhit = poly_index;
			}

			if (raytest.Result->StartBad) {
				return true;
			}
		}
		if (polyhit != -1) {
			raytest.Result->SurfaceType = Mesh->Get_Poly_Surface_Type (polyhit);
			return true;
		}
	}
	return false;
}
#ifdef RTS_DEBUG
#pragma optimize("", on)
#endif


/***********************************************************************************************
 * AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys -- cast ray to polys in the node *
 *                                                                                             *
 * INPUT:                                                                                      *
 * node - current cull node being processed                                                    *
 * start_point - starting point                                                                *
 * axis_r - the axis along which the ray is projected                                          *
 * axis_1, axis_2 - the other two axes                                                         *
 * direction - 0 means the ray goes toward the negative axis, 1 means positive.                *
 * flags - reference bitfield for result flags (ray hit edge, start point embedded in tri, etc)*
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/30/00    NH : Created.                                                                  *
 *=============================================================================================*/
int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(CullNodeStruct * node,
	const Vector3 & start_point, int axis_r, int axis_1, int axis_2, int direction,
	unsigned char & flags)
{
	int count = 0;

	if (node->Get_Poly_Count() > 0) {

		/*
		** Simply loop through the polys in this node, checking each for collision
		*/

		const Vector3 * loc = Mesh->Get_Vertex_Array();
		const TriIndex * polyverts = Mesh->Get_Polygon_Array();
		const Vector4 * plane = Mesh->Get_Plane_Array();
		int poly0 = node->Get_Poly0();
		int polycount = node->Get_Poly_Count();
		
		for (int poly_counter=0; poly_counter < polycount; poly_counter++) {

			int poly_index = PolyIndices[poly0 + poly_counter];

			const Vector3 &v0 = loc[ polyverts[poly_index][0] ];
			const Vector3 &v1 = loc[ polyverts[poly_index][1] ];
			const Vector3 &v2 = loc[ polyverts[poly_index][2] ];
			const Vector4 &tri_plane = plane[poly_index];

			// Since (int)true is defined as 1, and (int)false as 0:
			count += (unsigned int)Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(v0,	v1, v2,
				tri_plane, start_point, axis_r, axis_1, axis_2, direction, flags);
		}
	}

	return count;
}


/***********************************************************************************************
 * AABTreeClass::Cast_AABox_To_Polys -- cast aabox to polys in the given node                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_AABox_To_Polys(CullNodeStruct * node,AABoxCollisionTestClass & boxtest)
{
	if (node->Get_Poly_Count() > 0) {
		/*
		** Simply loop through the polys in this node, checking each for collision
		*/	  
		TriClass tri;

		const Vector3 * loc = Mesh->Get_Vertex_Array();
		const TriIndex * polyverts = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
		const Vector4 * norms = Mesh->Get_Plane_Array();
#endif

		int polyhit = -1;
		int poly0 = node->Get_Poly0();
		int polycount = node->Get_Poly_Count();

		for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

			int poly_index = PolyIndices[poly0 + poly_counter];

			tri.V[0] = &(loc[ polyverts[poly_index][0] ]);
			tri.V[1] = &(loc[ polyverts[poly_index][1] ]);
			tri.V[2] = &(loc[ polyverts[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
			tri.N = (Vector3*)&(norms[poly_index]);
#else
			Vector3 normal;
			tri.N = &normal;
			tri.Compute_Normal();
#endif
			if (CollisionMath::Collide(boxtest.Box,boxtest.Move,tri,boxtest.Result)) {
				polyhit = poly_index;
			}

			if (boxtest.Result->StartBad) {
				return true;
			}
		}
		if (polyhit != -1) {
			boxtest.Result->SurfaceType = Mesh->Get_Poly_Surface_Type (polyhit);
			return true;
		}
	}
	return false;
}


/***********************************************************************************************
 * AABTreeClass::Cast_OBBox_To_Polys -- cast obbox to polys in the given node                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Cast_OBBox_To_Polys(CullNodeStruct * node,OBBoxCollisionTestClass & boxtest)
{
	int polycount = node->Get_Poly_Count();
	int poly0 = node->Get_Poly0();

	if (polycount > 0) {
		/*
		** Simply loop through the polys in this node, checking each for collision
		*/	  
		TriClass tri;

		const Vector3 * loc = Mesh->Get_Vertex_Array();
		const TriIndex * polyverts = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
		const Vector4 * norms = Mesh->Get_Plane_Array();
#endif

		int polyhit = -1;

		for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

			int poly_index = PolyIndices[poly0 + poly_counter];

			tri.V[0] = &(loc[ polyverts[poly_index][0] ]);
			tri.V[1] = &(loc[ polyverts[poly_index][1] ]);
			tri.V[2] = &(loc[ polyverts[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
			tri.N = (Vector3*)&(norms[poly_index]);
#else
			Vector3 normal;
			tri.N = &normal;
			tri.Compute_Normal();
#endif

			if (CollisionMath::Collide(boxtest.Box,boxtest.Move,tri,Vector3(0,0,0),boxtest.Result)) {
				polyhit = poly_index;
			}

			if (boxtest.Result->StartBad) {
				return true;
			}
		}
		if (polyhit != -1) {
			boxtest.Result->SurfaceType = Mesh->Get_Poly_Surface_Type (polyhit);
			return true;
		}
	}
	return false;

}


/***********************************************************************************************
 * AABTreeClass::Intersect_OBBox_With_Polys -- Test polys for intersection                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/20/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool AABTreeClass::Intersect_OBBox_With_Polys
(
	CullNodeStruct * node,
	OBBoxIntersectionTestClass & test
)
{
	int poly0 = node->Get_Poly0();
	int polycount = node->Get_Poly_Count();

	if (polycount > 0) {
		/*
		** Simply loop through the polys in this node, checking each for collision
		*/	  
		TriClass tri;

		const Vector3 * loc = Mesh->Get_Vertex_Array();
		const TriIndex * polyverts = Mesh->Get_Polygon_Array();
#if (!OPTIMIZE_PLANEEQ_RAM)
		const Vector4 * norms = Mesh->Get_Plane_Array();
#endif

		for (int poly_counter=0; poly_counter<polycount; poly_counter++) {

			int poly_index = PolyIndices[poly0 + poly_counter];

			tri.V[0] = &(loc[ polyverts[poly_index][0] ]);
			tri.V[1] = &(loc[ polyverts[poly_index][1] ]);
			tri.V[2] = &(loc[ polyverts[poly_index][2] ]);
#if (!OPTIMIZE_PLANEEQ_RAM)
			tri.N = (Vector3*)&(norms[poly_index]);
#else
			Vector3 normal;
			tri.N = &normal;
			tri.Compute_Normal();
#endif

			if (CollisionMath::Intersection_Test(test.Box,tri)) {
				return true;
			}
		}
	}
	return false;
}


/***********************************************************************************************
 * AABTreeClass::Update_Bounding_Boxes_Recursive -- recompute the bounding boxes               *
 *                                                                                             *
 * Typically this function will be used when you have modified the mesh slightly and you need  *
 * to update the bounding boxes but you want to keep the structure of the tree.                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Update_Bounding_Boxes_Recursive(CullNodeStruct * node)
{
	node->Min.Set(100000.0f,100000.0f,100000.0f);
	node->Max.Set(-100000.0f,-100000.0f,-100000.0f);

	/*
	** Recurse to the children first
	*/
	if (node->Is_Leaf() == false) {

		Update_Bounding_Boxes_Recursive(&(Nodes[node->Get_Front_Child()]));
		Update_Bounding_Boxes_Recursive(&(Nodes[node->Get_Back_Child()]));

		/*
		** Bound our children
		*/
		int front = node->Get_Front_Child();
		int back = node->Get_Back_Child();

		if (Nodes[front].Min.X < node->Min.X) node->Min.X = Nodes[front].Min.X;
		if (Nodes[front].Max.X > node->Max.X) node->Max.X = Nodes[front].Max.X;

		if (Nodes[front].Min.Y < node->Min.Y) node->Min.Y = Nodes[front].Min.Y;
		if (Nodes[front].Max.Y > node->Max.Y) node->Max.Y = Nodes[front].Max.Y;
		
		if (Nodes[front].Min.Z < node->Min.Z) node->Min.Z = Nodes[front].Min.Z;
		if (Nodes[front].Max.Z > node->Max.Z) node->Max.Z = Nodes[front].Max.Z;

		if (Nodes[back].Min.X < node->Min.X) node->Min.X = Nodes[back].Min.X;
		if (Nodes[back].Max.X > node->Max.X) node->Max.X = Nodes[back].Max.X;

		if (Nodes[back].Min.Y < node->Min.Y) node->Min.Y = Nodes[back].Min.Y;
		if (Nodes[back].Max.Y > node->Max.Y) node->Max.Y = Nodes[back].Max.Y;
		
		if (Nodes[back].Min.Z < node->Min.Z) node->Min.Z = Nodes[back].Min.Z;
		if (Nodes[back].Max.Z > node->Max.Z) node->Max.Z = Nodes[back].Max.Z;

	} else {
		
		/*
		** Bound our polygons
		*/
		int poly0 = node->Get_Poly0();
		int polycount = node->Get_Poly_Count();

		for (int poly_index = 0; poly_index < polycount; poly_index++) {
			int pi = PolyIndices[poly0 + poly_index];
			Update_Min_Max(pi,node->Min,node->Max );
		}

	}

	WWASSERT(node->Min.X != 100000.0f);
	WWASSERT(node->Min.Y != 100000.0f);
	WWASSERT(node->Min.Z != 100000.0f);
	WWASSERT(node->Max.X != -100000.0f);
	WWASSERT(node->Max.Y != -100000.0f);
	WWASSERT(node->Max.Z != -100000.0f);
}


/***********************************************************************************************
 * AABTreeClass::Update_Min_Max -- updates min and max given a polygon index                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Update_Min_Max(int poly_index,Vector3 & min,Vector3 & max)
{
	for (int vert_index = 0; vert_index < 3; vert_index++) {

		const TriIndex * polyverts = Mesh->Get_Polygon_Array() + poly_index;
		const Vector3 * point = Mesh->Get_Vertex_Array() + (*polyverts)[vert_index];

		if (point->X  < min.X) min.X = point->X;
		if (point->Y  < min.Y) min.Y = point->Y;
		if (point->Z  < min.Z) min.Z = point->Z;

		if (point->X  > max.X) max.X = point->X;
		if (point->Y  > max.Y) max.Y = point->Y;
		if (point->Z  > max.Z) max.Z = point->Z;
	}
}


/***********************************************************************************************
 * AABTreeClass::Load_W3D -- Load a W3D description of an AABTree                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/22/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Load_W3D(ChunkLoadClass & cload)
{
	Reset();
	
	W3dMeshAABTreeHeader header;
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_AABTREE_HEADER);
	cload.Read(&header,sizeof(header));
	cload.Close_Chunk();

	NodeCount = header.NodeCount;
	PolyCount = header.PolyCount;
	Nodes = W3DNEWARRAY CullNodeStruct[NodeCount];
	PolyIndices = W3DNEWARRAY uint32[PolyCount];

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) 
		{
		case W3D_CHUNK_AABTREE_POLYINDICES:
			Read_Poly_Indices(cload);
			break;

		case W3D_CHUNK_AABTREE_NODES:
			Read_Nodes(cload);
			break;
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * AABTreeClass::Read_Poly_Indices -- load the polygon index array                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/22/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Read_Poly_Indices(ChunkLoadClass & cload)
{
	cload.Read(PolyIndices,sizeof(uint32) * PolyCount);
}


/***********************************************************************************************
 * AABTreeClass::Read_Nodes -- Load the node array                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/22/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AABTreeClass::Read_Nodes(ChunkLoadClass & cload)
{
	W3dMeshAABTreeNode w3dnode;

	for (int i=0; i<NodeCount; i++) {
		cload.Read(&w3dnode,sizeof(w3dnode));

		Nodes[i].Min.X = w3dnode.Min.X;
		Nodes[i].Min.Y = w3dnode.Min.Y;
		Nodes[i].Min.Z = w3dnode.Min.Z;

		Nodes[i].Max.X = w3dnode.Max.X;
		Nodes[i].Max.Y = w3dnode.Max.Y;
		Nodes[i].Max.Z = w3dnode.Max.Z;

		Nodes[i].FrontOrPoly0 = w3dnode.FrontOrPoly0;
		Nodes[i].BackOrPolyCount = w3dnode.BackOrPolyCount;
	}
}



