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
 *                     $Archive:: /Commando/Code/WWMath/tri.cpp                               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/03/01 3:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TriClass::Find_Dominant_Plane -- returns indices of the axes of the dominant plane        *
 *   TriClass::Contains_Point -- performs 2D point-in-triangle test.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "tri.h"
#include "vector2.h"


struct FDPRec 
{ 
	int axis1; 
	int axis2; 
	int axis3; 
};

static inline void find_dominant_plane_fast(const TriClass & tri, FDPRec& info)
{
	static const FDPRec dominance[3] =
	{
		{ 1, 2, 0 },		// Dominant is the X axis
		{ 0, 2, 1 },		// Dominant is the Y axis
		{ 0, 1, 2 }			// Dominant is the Z axis
	};

	/*
	** Find the largest component of the normal
	*/
	float x = WWMath::Fabs(tri.N->X);
	float y = WWMath::Fabs(tri.N->Y);
	float z = WWMath::Fabs(tri.N->Z);

	float val = x;
	int ni = 0;

	if (y > val) 
	{
		ni = 1;
		val = y;
	}

	if (z > val) 
	{
		ni = 2;
	}

	info = dominance[ni];
}

static inline void find_dominant_plane(const TriClass & tri, int * axis1,int * axis2,int * axis3)
{
	/*
	** Find the largest component of the normal
	*/
	int ni = 0;
	float x = WWMath::Fabs(tri.N->X);
	float y = WWMath::Fabs(tri.N->Y);
	float z = WWMath::Fabs(tri.N->Z);
	float val = x;

	if (y > val) {
		ni = 1;
		val = y;
	}

	if (z > val) {
		ni = 2;
	}

	/*
	** return the indices of the two axes perpendicular
	*/
	switch (ni) 
	{
	case 0: 
		// Dominant is the X axis
		*axis1 = 1;
		*axis2 = 2;
		*axis3 = 0;
		break;
	case 1: 
		// Dominant is the Y axis
		*axis1 = 0;
		*axis2 = 2;
		*axis3 = 1;
		break;
	case 2: 
		// Dominant is the Z axis
		*axis1 = 0;
		*axis2 = 1;
		*axis3 = 2;
		break;
	}
}



/***********************************************************************************************
 * TriClass::Find_Dominant_Plane -- returns indices of the axes of the dominant plane          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void TriClass::Find_Dominant_Plane(int * axis1,int * axis2) const
{
	/*
	** Find the largest component of the normal
	*/
	int ni = 0;
	float x = WWMath::Fabs(N->X);
	float y = WWMath::Fabs(N->Y);
	float z = WWMath::Fabs(N->Z);
	float val = x;

	if (y > val) {
		ni = 1;
		val = y;
	}

	if (z > val) {
		ni = 2;
	}

	/*
	** return the indices of the two axes perpendicular
	*/
	switch (ni) 
	{
	case 0: 
		// Dominant is the X axis
		*axis1 = 1;
		*axis2 = 2;
		break;
	case 1: 
		// Dominant is the Y axis
		*axis1 = 0;
		*axis2 = 2;
		break;
	case 2: 
		// Dominant is the Z axis
		*axis1 = 0;
		*axis2 = 1;
		break;
	}
}


/***********************************************************************************************
 * TriClass::Contains_Point -- performs 2D point-in-triangle test.                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Assumes that the point is in the plane of the triangle... use this after you've intersected *
 * a ray with the plane of the triangle.                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool TriClass::Contains_Point(const Vector3 & ipoint) const
{
#if 0
	/*
	** Perform the test in 2d on the plane which the normal
	** is most perpendicular to. (copied from E.Cosky's intersection code)
	*/
	int axis1 = 0;
	int axis2 = 0;
	Find_Dominant_Plane(&axis1,&axis2);

#if 1
	unsigned char flags;	// dummy variable passed into function and not used here
	return Point_In_Triangle_2D(*V[0], *V[1], *V[2], ipoint, axis1, axis2, flags);
#else
	float u0 = ipoint[axis1] - (*V[0])[axis1];
	float v0 = ipoint[axis2] - (*V[0])[axis2];

	/*
	** determine the 2d vectors on the dominant plane from the first vertex to the other two
	*/
	float u1 = (*V[1])[axis1] - (*V[0])[axis1]; 
	float v1 = (*V[1])[axis2] - (*V[0])[axis2];
	float	u2 = (*V[2])[axis1] - (*V[0])[axis1]; 
	float	v2 = (*V[2])[axis2] - (*V[0])[axis2];

	float alpha, beta;
	bool intersect = false; 

	// calculate alpha and beta as normalized (0..1) percentages across the 2d projected triangle
	// and do bounds checking (sum <= 1)  to determine whether or not the triangle intersection occurs.
	if (u1 == 0)    {
	  beta = u0 / u2;
	  if ((beta >= 0) && (beta <= 1)) {
			alpha = (v0 - beta * v2) / v1;
			intersect = ((alpha >= 0) && ((alpha + beta) <= 1 + WWMATH_EPSILON));
	  }
	} else {
	  beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
	  if ((beta >= 0) && (beta <= 1)) {
			alpha = (u0 - beta * u2) / u1;
			intersect = ((alpha >= 0) && ((alpha + beta) <= 1 + WWMATH_EPSILON));
	  }
	}

	return intersect;
#endif
#endif
/*
** New cross-product based point-containment
*/
#if 0
	int vi;
	int axis3 = 0;

	for (vi=0; vi<3; vi++) {
		if ((axis1 != vi) && (axis2 != vi))	axis3 = vi;
	}

	Vector3 test_point = ipoint;
	test_point[axis3] = 0.0f;

	Vector3 points[3];
	for (vi=0; vi<3; vi++) {
		points[vi] = *(V[vi]);
		points[vi][axis3] = 0.0f;
	}
	
	bool side[3];
	Vector3 edge;
	Vector3 cross;
	Vector3 dp;

	for (vi=0; vi<3; vi++) {
		edge = points[(vi+1)%3] - points[vi];
		dp = test_point - points[vi];
	
		Vector3::Cross_Product(dp,edge,&cross);
		side[vi] = (cross[axis3] > 0.0f);
	}

	bool my_intersect = ((side[0] == side[1]) && (side[1] == side[2]));
	return my_intersect;
#endif


/*
** "Optimized" version
*/
#if 1
#if 0

	// srj opto version
	/// @todo srj -- profile me to see if this is actually faster

	FDPRec info;
	find_dominant_plane_fast(*this, info);

	/*
	** Compute the 2D cross product of edge0 with a vector to the point
	*/

	float edge_x, edge_y, dp_x, dp_y, cross;
	bool side0, side1, side2;

	edge_x = (*V[1])[info.axis1] - (*V[0])[info.axis1];
	edge_y = (*V[1])[info.axis2] - (*V[0])[info.axis2];
	dp_x = ipoint[info.axis1] - (*V[0])[info.axis1];
	dp_y = ipoint[info.axis2] - (*V[0])[info.axis2];
	cross = edge_x * dp_y - edge_y * dp_x;
	side0 = (cross >= 0.0f);

	edge_x = (*V[2])[info.axis1] - (*V[1])[info.axis1];
	edge_y = (*V[2])[info.axis2] - (*V[1])[info.axis2];
	dp_x = ipoint[info.axis1] - (*V[1])[info.axis1];
	dp_y = ipoint[info.axis2] - (*V[1])[info.axis2];
	cross = edge_x * dp_y - edge_y * dp_x;
	side1 = (cross >= 0.0f);

	edge_x = (*V[0])[info.axis1] - (*V[2])[info.axis1];
	edge_y = (*V[0])[info.axis2] - (*V[2])[info.axis2];
	dp_x = ipoint[info.axis1] - (*V[2])[info.axis1];
	dp_y = ipoint[info.axis2] - (*V[2])[info.axis2];
	cross = edge_x * dp_y - edge_y * dp_x;
	side2 = (cross >= 0.0f);

	bool my_intersect = ((side0 == side1) && (side1 == side2));
	return my_intersect;

#else
	int vi;
	int axis1 = 0;
	int axis2 = 0;
	int axis3 = 0;

	find_dominant_plane(*this,&axis1,&axis2,&axis3);

	bool side[3];

	/*
	** Compute the 2D cross product of edge0 with a vector to the point
	*/
	Vector2 edge;
	Vector2 dp;

	for (vi=0; vi<3; vi++) {
		
		int va=vi;
		int vb=(vi+1)%3;
		
		edge.Set((*V[vb])[axis1] - (*V[va])[axis1] , (*V[vb])[axis2] - (*V[va])[axis2]);
		dp.Set(ipoint[axis1] - (*V[va])[axis1] , ipoint[axis2] - (*V[va])[axis2]);
		float cross = edge.X * dp.Y - edge.Y * dp.X;
		side[vi] = (cross >= 0.0f);
	}

	bool my_intersect = ((side[0] == side[1]) && (side[1] == side[2]));
	return my_intersect;
#endif
#endif


}
