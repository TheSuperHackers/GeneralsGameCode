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
 *                     $Archive:: /Commando/Code/ww3d2/aabtreebuilder.h                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef AABTREEBUILDER_H
#define AABTREEBUILDER_H

#include "always.h"
#include "vector3.h"
#include "Vector3i.h"
#include "aaplane.h"
#include "bittype.h"
#include "meshgeometry.h"
#include <float.h>

class AABTreeClass;
class ChunkSaveClass;
struct W3dMeshAABTreeNode;

/*
** AABTreeBuilderClass
** This class serves simply to build AABTreeClasses.  It first builds a tree
** which uses an easier to manage data structure (but uses more memory).  Then
** the tree is converted into the representation used in the AABTreeClass.
*/
class AABTreeBuilderClass
{
public:

	AABTreeBuilderClass(void);
	~AABTreeBuilderClass(void);

	void					Build_AABTree(int polycount,TriIndex * polys,int vertcount,Vector3 * verts);
	void					Export(ChunkSaveClass & csave);

	int					Node_Count(void);
	int					Poly_Count(void);

	enum
	{
		MIN_POLYS_PER_NODE =		4,
		SMALL_VERTEX =				-100000,
		BIG_VERTEX =				100000
	};

private:

	/*
	** This CullNodeStruct is used in building the AABTree.  It is much more
	** wasteful in terms of memory footprint and number of allocations than the
	** streamlined version found in the actual AABTreeClass.
	*/
	struct CullNodeStruct
	{
		CullNodeStruct(void) : Index(0),Min(0,0,0),Max(0,0,0),Front(NULL),Back(NULL),PolyCount(0),PolyIndices(NULL) {}
		~CullNodeStruct(void)
		{
			delete Front;
			delete Back;
			delete[] PolyIndices;
		}

		int						Index;
		Vector3					Min;
		Vector3					Max;
		CullNodeStruct *		Front;
		CullNodeStruct *		Back;
		int						PolyCount;
		int *						PolyIndices;
	};

	/*
	** SplitChoiceStruct - encapsulates the results of evaluating the suitability of a partition
	*/
	struct SplitChoiceStruct
	{
		SplitChoiceStruct(void) :
			Cost(FLT_MAX),
			FrontCount(0),
			BackCount(0),
			BMin(BIG_VERTEX,BIG_VERTEX,BIG_VERTEX),
			BMax(SMALL_VERTEX,SMALL_VERTEX,SMALL_VERTEX),
			FMin(BIG_VERTEX,BIG_VERTEX,BIG_VERTEX),
			FMax(SMALL_VERTEX,SMALL_VERTEX,SMALL_VERTEX),
			Plane(AAPlaneClass::XNORMAL,0)
		{
		}

		float						Cost;				// try to minimize this!
		int						FrontCount;		// number of polys in front of the plane
		int						BackCount;		// number of polys behind the plane
		Vector3					BMin;				// min of the bounding box of the "back" child
		Vector3					BMax;				// max of the bounding box of the "back" child
		Vector3					FMin;				// min of the bounding box of the "front" child
		Vector3					FMax;				// max of the bounding box of the "front" child
		AAPlaneClass			Plane;			// partitioning plane
	};

	struct SplitArraysStruct
	{
		SplitArraysStruct(void) :
			FrontCount(0),
			BackCount(0),
			FrontPolys(NULL),
			BackPolys(NULL)
		{
		}

		int						FrontCount;
		int						BackCount;
		int *						FrontPolys;
		int *						BackPolys;
	};

	enum OverlapType
	{
		POS				= 0x01,
		NEG				= 0x02,
		ON					= 0x04,
		BOTH				= 0x08,
		OUTSIDE			= POS,
		INSIDE			= NEG,
		OVERLAPPED		= BOTH,
		FRONT				= POS,
		BACK				= NEG,
	};


	/*
	** Internal functions
	*/
	void								Reset();
	void								Build_Tree(CullNodeStruct * node,int polycount,int * polyindices);
	SplitChoiceStruct				Select_Splitting_Plane(int polycount,int * polyindices);
	SplitChoiceStruct				Compute_Plane_Score(int polycont,int * polyindices,const AAPlaneClass & plane);
	void								Split_Polys(int polycount,int * polyindices,const SplitChoiceStruct & sc,SplitArraysStruct *	arrays);
	OverlapType						Which_Side(const AAPlaneClass & plane,int poly_index);
	void								Compute_Bounding_Box(CullNodeStruct * node);
	int								Assign_Index(CullNodeStruct * node,int index);
	int								Node_Count_Recursive(CullNodeStruct * node,int curcount);
	void								Update_Min(int poly_index,Vector3 & set_min);
	void								Update_Max(int poly_index,Vector3 & set_max);
	void								Update_Min_Max(int poly_index, Vector3 & set_min, Vector3 & set_max);

	void								Build_W3D_AABTree_Recursive(CullNodeStruct *	node,
											W3dMeshAABTreeNode * w3dnodes,
											uint32 * poly_indices,
											int & cur_node,
											int &	cur_poly);
	/*
	** Tree
	*/
	CullNodeStruct *				Root;
	int								CurPolyIndex;

	/*
	** Mesh data
	*/
	int								PolyCount;
	TriIndex *						Polys;
	int								VertCount;
	Vector3 *						Verts;

	friend class AABTreeClass;
};




#endif //AABTREEBUILDER_H

