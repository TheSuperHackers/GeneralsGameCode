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
 *                     $Archive:: /Commando/Code/ww3d2/aabtreebuilder.cpp                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 5:49p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AABTreeBuilderClass::AABTreeBuilderClass -- Constructor                                   *
 *   AABTreeBuilderClass::~AABTreeBuilderClass -- Destructor                                   *
 *   AABTreeBuilderClass::Reset -- reset the builder, delete all arrays                        *
 *   AABTreeBuilderClass::Build_AABTree -- Build an AABTree for the given mesh.                *
 *   AABTreeBuilderClass::Build_Tree -- recursivly builds the culling tree                     *
 *   AABTreeBuilderClass::Select_Splitting_Plane -- select a partition for the given polys     *
 *   AABTreeBuilderClass::Compute_Plane_Score -- evaluate the suitability of a partition plane *
 *   AABTreeBuilderClass::Which_Side -- which side of a plane is the given poly                *
 *   AABTreeBuilderClass::Split_Polys -- partition the polys with a plane                      *
 *   AABTreeBuilderClass::Compute_Bounding_Box -- compute bounding boxes for the cull nodes    *
 *   AABTreeBuilderClass::Assign_Index -- assign an array index to each node                   *
 *   AABTreeBuilderClass::Node_Count -- Count the nodes in the tree                            *
 *   AABTreeBuilderClass::Poly_Count -- returns number of polys                                *
 *   AABTreeBuilderClass::Node_Count_Recursive -- internal implementation of Node_Count        *
 *   AABTreeBuilderClass::Submit_Tree -- install nodes into an AABTreeClass                    *
 *   AABTreeBuilderClass::Submit_Tree_Recursive -- internal implementation of Submit_Tree      *
 *   AABTreeBuilderClass::Update_Min -- ensure given vector is < min of the poly               *
 *   AABTreeBuilderClass::Update_Max -- ensure given vector is > max of poly                   *
 *   AABTreeBuilderClass::Update_Min_Max -- ensure given vector is in min max of poly          *
 *   AABTreeBuilderClass::Export -- Saves this AABTree into a W3D chunk                        *
 *   AABTreeBuilderClass::Build_W3D_AABTree_Recursive -- Build array of indices and W3dMeshAAB *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "aabtreebuilder.h"
#include "chunkio.h"
#include "w3d_file.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#undef WWASSERT
#define WWASSERT	assert					// can't use WWASSERT because we use this module in the MAX plugin...
const float COINCIDENCE_EPSILON = 0.001f;


/***********************************************************************************************
 * AABTreeBuilderClass::AABTreeBuilderClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AABTreeBuilderClass::AABTreeBuilderClass(void) :
	Root(NULL),
	CurPolyIndex(0),
	PolyCount(0),
	Polys(NULL),
	VertCount(0),
	Verts(NULL)
{
}


/***********************************************************************************************
 * AABTreeBuilderClass::~AABTreeBuilderClass -- Destructor                                     *
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
AABTreeBuilderClass::~AABTreeBuilderClass(void)
{
	Reset();
}


/***********************************************************************************************
 * AABTreeBuilderClass::Reset -- reset the builder, delete all arrays                          *
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
void AABTreeBuilderClass::Reset(void)
{
	if (Root) {
		delete Root; Root = NULL;
	}

	if (Verts != NULL) {
		delete[] Verts;
		Verts = NULL;
	}

	if (Polys != NULL) {
		delete[] Polys;
		Polys = NULL;
	}
}

/***********************************************************************************************
 * AABTreeBuilderClass::Build_AABTree -- Build an AABTree for the given mesh.                  *
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
void AABTreeBuilderClass::Build_AABTree(int polycount,TriIndex * polys,int vertcount,Vector3 * verts)
{
	WWASSERT(polycount > 0);
	WWASSERT(vertcount > 0);
	WWASSERT(polys != NULL);
	WWASSERT(verts != NULL);

	/*
	** If we already have allocated data, release it
	*/
	Reset();

	/*
	** Copy the mesh data
	*/
	VertCount = vertcount;
	PolyCount = polycount;
	Verts = W3DNEWARRAY Vector3[VertCount];
	Polys = W3DNEWARRAY TriIndex[PolyCount];

	for (int vi=0; vi<VertCount; vi++) {
		Verts[vi] = verts[vi];
	}
	for (int pi=0; pi<PolyCount; pi++) {
		Polys[pi] = polys[pi];
	}

	/*
	** First, create a list of all of the poly indices
	*/
	int * polyindices = W3DNEWARRAY int[PolyCount];
	for (int i=0; i<PolyCount; i++) {
		polyindices[i] = i;
	}

	/*
	** Build the tree, note that the array of poly indices will be
	** deleted by the Build_Tree function.
	*/
	Root = W3DNEW CullNodeStruct;
	Build_Tree(Root,PolyCount,polyindices);
	polyindices = NULL;

	/*
	** fill in the remaining information needed in the tree:
	** for example: bounding boxes, index assignments
	*/
	Compute_Bounding_Box(Root);
	Assign_Index(Root,0);

}


/***********************************************************************************************
 * AABTreeBuilderClass::Build_Tree -- recursivly builds the culling tree                       *
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
void AABTreeBuilderClass::Build_Tree(CullNodeStruct * node,int polycount,int * polyindices)
{
	/*
	** First, if there are only a few polys left, just terminate the tree
	*/
	if (polycount <= MIN_POLYS_PER_NODE) {
		node->PolyCount = polycount;
		node->PolyIndices = polyindices;
		return;
	}

	/*
	** Try to find a suitable partitioning plane.
	*/
	SplitChoiceStruct sc;

	sc = Select_Splitting_Plane(polycount,polyindices);

	/*
	** If the algorithm could not separate any polys, just install the polys
	** in this node and terminate.  TODO: explore how this happens.
	*/
	if (sc.FrontCount + sc.BackCount != polycount) {
		node->PolyCount = polycount;
		node->PolyIndices = polyindices;
		return;
	}

	/*
	** Decide whether to actually partition this node.  If the partitioning
	** will not gain us anything, just install the polys in this node and terminate
	** the tree.
	*/
#if 0
	if (sc.Cost == MAX_COST) {
		node->PolyCount = polycount;
		node->PolyIndices = polyindices;
		return;
	}
#endif

	/*
	** Ok, split the polys
	*/
	SplitArraysStruct arrays;
	Split_Polys(polycount,polyindices,sc,&arrays);

	/*
	** Free the memory in use by the input tile-list
	*/
	delete[] polyindices;

	/*
	** Build a front tree if necessary.  Remember that the Build function
	** deletes the poly array.
	*/
	if (arrays.FrontCount) {
		WWASSERT(arrays.FrontPolys != NULL);
		node->Front = W3DNEW CullNodeStruct;
		Build_Tree(node->Front,arrays.FrontCount,arrays.FrontPolys);
		arrays.FrontPolys = NULL;
	}

	/*
	** Build a back tree if necessary. Remember that the build function
	** deletes the tile array.
	*/
	if (arrays.BackCount) {
		WWASSERT(arrays.BackPolys != NULL);

		node->Back = W3DNEW CullNodeStruct;
		Build_Tree(node->Back,arrays.BackCount,arrays.BackPolys);
		arrays.BackPolys = NULL;
	}

}


/***********************************************************************************************
 * AABTreeBuilderClass::Select_Splitting_Plane -- select a partition for the given polys       *
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
AABTreeBuilderClass::SplitChoiceStruct
AABTreeBuilderClass::Select_Splitting_Plane(int polycount,int * polyindices)
{
	WWASSERT(polyindices != NULL);

	const int NUM_TRYS = 50;

	SplitChoiceStruct best_plane_stats;
	SplitChoiceStruct considered_plane_stats;

	/*
	** Try putting axis-aligned planes through some random vertices
	*/
	for (int trys = 0; trys < MIN(NUM_TRYS,polycount); trys++) {

		AAPlaneClass plane;

		/*
		** Select a random poly and vertex index;
		*/
		int poly_index = polyindices[rand() % polycount];
		int vert_index = rand() % 3;
		const TriIndex * polyverts = Polys + poly_index;
		const Vector3 * vert = Verts + (*polyverts)[vert_index];

		/*
		** Select a random plane
		*/
		switch(rand() % 3) {
			case 0:	plane.Set(AAPlaneClass::XNORMAL,vert->X);	break;
			case 1:	plane.Set(AAPlaneClass::YNORMAL,vert->Y);	break;
			case 2:	plane.Set(AAPlaneClass::ZNORMAL,vert->Z);	break;
		};

		/*
		** Get the score for this plane
		*/
		considered_plane_stats = Compute_Plane_Score(polycount,polyindices,plane);

		if (considered_plane_stats.Cost < best_plane_stats.Cost) {
			best_plane_stats = considered_plane_stats;
		}
	}

	return best_plane_stats;
}


/***********************************************************************************************
 * AABTreeBuilderClass::Compute_Plane_Score -- evaluate the suitability of a partition plane   *
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
AABTreeBuilderClass::SplitChoiceStruct
AABTreeBuilderClass::Compute_Plane_Score(int polycount,int * polyindices,const AAPlaneClass & plane)
{
	/*
	** The score of a splitting plane is based on the following factors:
	** - the volumes of the resulting two children volumes,
	** - the number of polys in each child volume
	*/
	SplitChoiceStruct sc;
	sc.Plane = plane;

	for (int i=0; i<polycount; i++) {

		switch(Which_Side(plane,polyindices[i])) {

			case FRONT:
			case ON:
			case BOTH:
				{
					sc.FrontCount++;
					Update_Min_Max(polyindices[i],sc.FMin,sc.FMax );
					break;
				}

			case BACK:
				{
					sc.BackCount++;
					Update_Min_Max(polyindices[i],sc.BMin,sc.BMax );
					break;
				}
		}
	}

	/*
	** Inflate the box a tiny amount so that we never
	** get volumes of zero!
	*/
	sc.BMin -= Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);
	sc.BMax += Vector3(WWMATH_EPSILON,WWMATH_EPSILON,WWMATH_EPSILON);

	/*
	** Compute the cost.
	*/
	float back_cost = (sc.BMax.X - sc.BMin.X) * (sc.BMax.Y - sc.BMin.Y) * (sc.BMax.Z - sc.BMin.Z) * sc.BackCount;
	float front_cost = (sc.FMax.X - sc.FMin.X) * (sc.FMax.Y - sc.FMin.Y) * (sc.FMax.Z - sc.FMin.Z) * sc.FrontCount;
	sc.Cost = front_cost + back_cost;

	if ((sc.FrontCount == 0) || (sc.BackCount == 0)) {
		sc.Cost = FLT_MAX;
	}

	return sc;
}


/***********************************************************************************************
 * AABTreeBuilderClass::Which_Side -- which side of a plane is the given poly                  *
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
AABTreeBuilderClass::OverlapType
AABTreeBuilderClass::Which_Side(const AAPlaneClass & plane,int poly_index)
{
	/*
	** Check each vertex to see if it is in front, behind or on the plane
	*/
	int mask = 0;
	for (int vi=0; vi<3; vi++) {

		const Vector3 & point = Verts[ Polys[poly_index][vi] ];
		float delta = point[plane.Normal] - plane.Dist;

		if (delta > COINCIDENCE_EPSILON) {
			mask |= POS;
		}
		if (delta < -COINCIDENCE_EPSILON) {
			mask |= NEG;
		}
		mask |= ON;
	}

	/*
	** Now evaluate the status of all of the verts to determine whether the
	** triangle is in front, behind, on or overlapping the plane
	*/

	/*
	** If all verts were ON the plane, the triangle is ON the plane
	*/
	if (mask == ON) {
		return ON;
	}

	/*
	** If all verts were POS or ON, the triangle is POS (IN_FRONT)
	*/
	if ((mask & ~(POS | ON)) == 0) {
		return POS;
	}

	/*
	** If all verts were NEG or ON, the triangle is NEG (BEHIND)
	*/
	if ((mask & ~(NEG | ON)) == 0) {
		return NEG;
	}

	/*
	** Otherwise, the triangle spans the plane
	*/
	return BOTH;
}


/***********************************************************************************************
 * AABTreeBuilderClass::Split_Polys -- partition the polys with a plane                        *
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
void AABTreeBuilderClass::Split_Polys
(
	int									polycount,
	int *									polyindices,
	const SplitChoiceStruct &		sc,
	SplitArraysStruct *				arrays
)
{
	/*
	** Note that this routine arrays of polygons. The caller is then responsible for keeping
	** track of the memory this routine allocates.
	*/
	if (sc.FrontCount > 0) {
		arrays->FrontPolys = W3DNEWARRAY int[sc.FrontCount];
	}

	if (sc.BackCount > 0) {
		arrays->BackPolys = W3DNEWARRAY int[sc.BackCount];
	}

	arrays->FrontCount = 0;
	arrays->BackCount = 0;

	for (int i=0; i<polycount; i++) {

		switch(Which_Side(sc.Plane,polyindices[i])) {

			case FRONT:
			case ON:
			case BOTH:
				arrays->FrontPolys[arrays->FrontCount++] = polyindices[i];
				break;

			case BACK:
				arrays->BackPolys[arrays->BackCount++] = polyindices[i];
				break;
		}
	}

	/*
	** when we are all done, the counts should match.
	*/
	WWASSERT(arrays->FrontCount == sc.FrontCount);
	WWASSERT(arrays->BackCount == sc.BackCount);
}


/***********************************************************************************************
 * AABTreeBuilderClass::Compute_Bounding_Box -- compute bounding boxes for the cull nodes      *
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
void AABTreeBuilderClass::Compute_Bounding_Box(CullNodeStruct * node)
{
	/*
	** compute bounding volumes of the children
	*/
	if (node->Front) {
		Compute_Bounding_Box(node->Front);
	}

	if (node->Back) {
		Compute_Bounding_Box(node->Back);
	}

	/*
	** compute bounding volume for the polys in this node
	*/
	node->Min.Set(100000.0f,100000.0f,100000.0f);
	node->Max.Set(-100000.0f,-100000.0f,-100000.0f);

	for (int poly_index = 0; poly_index < node->PolyCount; poly_index++) {
		Update_Min_Max(node->PolyIndices[poly_index],node->Min,node->Max );
	}

	/*
	** bound the polys in the front child node
	*/
	if (node->Front) {
		if (node->Front->Min.X < node->Min.X) node->Min.X = node->Front->Min.X;
		if (node->Front->Max.X > node->Max.X) node->Max.X = node->Front->Max.X;

		if (node->Front->Min.Y < node->Min.Y) node->Min.Y = node->Front->Min.Y;
		if (node->Front->Max.Y > node->Max.Y) node->Max.Y = node->Front->Max.Y;

		if (node->Front->Min.Z < node->Min.Z) node->Min.Z = node->Front->Min.Z;
		if (node->Front->Max.Z > node->Max.Z) node->Max.Z = node->Front->Max.Z;
	}

	/*
	** bound the polys in the back child node
	*/
	if (node->Back) {
		if (node->Back->Min.X < node->Min.X) node->Min.X = node->Back->Min.X;
		if (node->Back->Max.X > node->Max.X) node->Max.X = node->Back->Max.X;

		if (node->Back->Min.Y < node->Min.Y) node->Min.Y = node->Back->Min.Y;
		if (node->Back->Max.Y > node->Max.Y) node->Max.Y = node->Back->Max.Y;

		if (node->Back->Min.Z < node->Min.Z) node->Min.Z = node->Back->Min.Z;
		if (node->Back->Max.Z > node->Max.Z) node->Max.Z = node->Back->Max.Z;
	}

	WWASSERT(node->Min.X != 100000.0f);
	WWASSERT(node->Min.Y != 100000.0f);
	WWASSERT(node->Min.Z != 100000.0f);
	WWASSERT(node->Max.X != -100000.0f);
	WWASSERT(node->Max.Y != -100000.0f);
	WWASSERT(node->Max.Z != -100000.0f);
}


/***********************************************************************************************
 * AABTreeBuilderClass::Assign_Index -- assign an array index to each node                     *
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
int AABTreeBuilderClass::Assign_Index(CullNodeStruct * node,int index)
{
	/*
	** This function is used to assign a sequential index to
	** each node in the tree.  The AABTree stores its nodes in
	** an array so this index is used to determine which slot
	** in the array to put each node into.
	*/
	WWASSERT(node);
	node->Index = index;
	index++;

	if (node->Front) {
		index = Assign_Index(node->Front,index);
	}

	if (node->Back) {
		index = Assign_Index(node->Back,index);
	}

	return index;
}


/***********************************************************************************************
 * AABTreeBuilderClass::Node_Count -- Count the nodes in the tree                              *
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
int AABTreeBuilderClass::Node_Count(void)
{
	if (Root) {
		return Node_Count_Recursive(Root,0);
	} else {
		return 0;
	}
}


/***********************************************************************************************
 * AABTreeBuilderClass::Poly_Count -- returns number of polys                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int AABTreeBuilderClass::Poly_Count(void)
{
	return PolyCount;
}

/***********************************************************************************************
 * AABTreeBuilderClass::Node_Count_Recursive -- internal implementation of Node_Count          *
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
int AABTreeBuilderClass::Node_Count_Recursive(CullNodeStruct * node,int curcount)
{
	curcount++;
	if (node->Front) {
		curcount = Node_Count_Recursive(node->Front,curcount);
	}
	if (node->Back) {
		curcount = Node_Count_Recursive(node->Back,curcount);
	}
	return curcount;
}


/***********************************************************************************************
 * AABTreeBuilderClass::Update_Min -- ensure given vector is < min of the poly                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeBuilderClass::Update_Min(int poly_index,Vector3 & min)
{
	for (int vert_index = 0; vert_index < 3; vert_index++) {

		const TriIndex * polyverts = Polys + poly_index;
		const Vector3 * point = Verts + (*polyverts)[vert_index];

		if (point->X  < min.X) min.X = point->X;
		if (point->Y  < min.Y) min.Y = point->Y;
		if (point->Z  < min.Z) min.Z = point->Z;
	}
}


/***********************************************************************************************
 * AABTreeBuilderClass::Update_Max -- ensure given vector is > max of poly                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/22/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void AABTreeBuilderClass::Update_Max(int poly_index,Vector3 & max)
{
	for (int vert_index = 0; vert_index < 3; vert_index++) {

		const TriIndex * polyverts = Polys + poly_index;
		const Vector3 * point = Verts + (*polyverts)[vert_index];

		if (point->X  > max.X) max.X = point->X;
		if (point->Y  > max.Y) max.Y = point->Y;
		if (point->Z  > max.Z) max.Z = point->Z;
	}
}


/***********************************************************************************************
 * AABTreeBuilderClass::Update_Min_Max -- ensure given vector is in min max of poly            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/98    BMG : Created.                                                                 *
 *=============================================================================================*/
void	AABTreeBuilderClass::Update_Min_Max(int poly_index, Vector3 & min, Vector3 & max)
{
	for (int vert_index = 0; vert_index < 3; vert_index++) {

		const TriIndex * polyverts = Polys + poly_index;
		const Vector3 * point = Verts + (*polyverts)[vert_index];

		if (point->X  < min.X) min.X = point->X;
		if (point->Y  < min.Y) min.Y = point->Y;
		if (point->Z  < min.Z) min.Z = point->Z;

		if (point->X  > max.X) max.X = point->X;
		if (point->Y  > max.Y) max.Y = point->Y;
		if (point->Z  > max.Z) max.Z = point->Z;
	}
}



/***********************************************************************************************
 * AABTreeBuilderClass::Export -- Saves this AABTree into a W3D chunk                          *
 *                                                                                             *
 *    This function will export the AABTree into a W3D chunk so that it can be loaded by its   *
 *    sister class "AABTreeClass" in the WW3D library.                                         *
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
void AABTreeBuilderClass::Export(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_AABTREE);

	/*
	** Pack the tree into an array of W3dMeshAABTreeNode's and polygon indices
	*/
	W3dMeshAABTreeNode * nodes = W3DNEWARRAY W3dMeshAABTreeNode[Node_Count()];
	uint32 * poly_indices = W3DNEWARRAY uint32[Poly_Count()];

	int cur_node = 0;
	int cur_poly = 0;
	Build_W3D_AABTree_Recursive(Root,nodes,poly_indices,cur_node,cur_poly);

	/*
	** Write out the header
	*/
	csave.Begin_Chunk(W3D_CHUNK_AABTREE_HEADER);
	W3dMeshAABTreeHeader header;
	memset(&header,0,sizeof(header));
	header.NodeCount = Node_Count();
	header.PolyCount = Poly_Count();
	csave.Write(&header,sizeof(header));
	csave.End_Chunk();

	/*
	** Write out the array of polygon indices
	*/
	csave.Begin_Chunk(W3D_CHUNK_AABTREE_POLYINDICES);
	csave.Write(poly_indices,Poly_Count() * sizeof(uint32));
	csave.End_Chunk();

	/*
	** Write out the array of nodes
	*/
	csave.Begin_Chunk(W3D_CHUNK_AABTREE_NODES);
	for (int ni=0; ni<Node_Count(); ni++) {
		csave.Write(&(nodes[ni]),sizeof(W3dMeshAABTreeNode));
	}
	csave.End_Chunk();

	csave.End_Chunk(); // W3D_CHUNK_AABTREE done
}


/***********************************************************************************************
 * AABTreeBuilderClass::Build_W3D_AABTree_Recursive -- Build array of indices and W3dMeshAABTr *
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
void AABTreeBuilderClass::Build_W3D_AABTree_Recursive
(
	AABTreeBuilderClass::CullNodeStruct *	node,
	W3dMeshAABTreeNode *							w3d_nodes,
	uint32 *											poly_indices,
	int &												cur_node,
	int &												cur_poly
)
{
	/*
	** Copy data from the builder's node into our node
	*/
	W3dMeshAABTreeNode * newnode = &(w3d_nodes[node->Index]);
	newnode->Min.X = node->Min.X;
	newnode->Min.Y = node->Min.Y;
	newnode->Min.Z = node->Min.Z;
	newnode->Max.X = node->Max.X;
	newnode->Max.Y = node->Max.Y;
	newnode->Max.Z = node->Max.Z;

	/*
	** If this is a non-leaf node, set up the child indices, otherwise set up the polygon indices
	*/
	if (node->Front != NULL) {

		WWASSERT(node->Back != NULL);		// if we have one child, we better have both!
		newnode->FrontOrPoly0 = node->Front->Index;
		newnode->BackOrPolyCount = node->Back->Index;

	} else {

		newnode->FrontOrPoly0 = cur_poly | 0x80000000;
		newnode->BackOrPolyCount = node->PolyCount;

	}

	/*
	** Copy the polygon indices for this node into our array
	*/
	for (int pcounter = 0; pcounter < node->PolyCount; pcounter++) {
		poly_indices[cur_poly++] = node->PolyIndices[pcounter];
	}

	/*
	** Install the children
	*/
	if (node->Front) {
		Build_W3D_AABTree_Recursive(node->Front,w3d_nodes,poly_indices,cur_node,cur_poly);
	}
	if (node->Back) {
		Build_W3D_AABTree_Recursive(node->Back,w3d_nodes,poly_indices,cur_node,cur_poly);
	}
}




