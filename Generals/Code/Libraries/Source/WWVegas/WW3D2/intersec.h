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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/intersec.h                             $*
 *                                                                                             *
 *                      $Author:: Naty_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/28/01 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*

	Most of the functions of this class are inline functions and if you use them you will need
	to include <intersec.inl>.

	Exceptions to this are the functions most commonly used by render objects:



	The constructors/destructors are inline & implemented here.

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef INTERSEC_H
#define INTERSEC_H

#include "always.h"
#include "matrix3d.h"
#include "layer.h"
#include "sphere.h"
#include "coltype.h"


class RenderObjClass;
typedef unsigned short POLYGONINDEX;


/*
**
*/
class IntersectionResultClass 
{
public:
	RenderObjClass *	IntersectedRenderObject;
	POLYGONINDEX		IntersectedPolygon;       

	Matrix3D				ModelMatrix;		// this is required for transforming mesh normals from model coords
	Vector3				ModelLocation;		// ditto
	Vector3				Intersection;		// location of intersection is placed here

	float					Range;				// distance from ray to point of intersection
	float					Alpha, Beta;		// stored for use by Process_Intersection_Results() - used to interpolate normal across a polygon
	bool					Intersects;			// could be in the intersection_type, but it is used in a number of bool operations
	int						CollisionType;	// mask for which object types to intersect with (added for 'Generals'. MW)

	enum INTERSECTION_TYPE {
		NONE = 0,
		GENERIC, 
		POLYGON
	} IntersectionType;
};


class IntersectionClass
{
	// member data

public:
	enum {
		MAX_POLY_INTERSECTION_COUNT = 1000, // arbitrary size of stack array used for storing intersection results within Intersect_Mesh(). Non-recursive function.
		MAX_HIERARCHY_NODE_COUNT = 256			
	};
	
	// this structure is used to store all intersections on the stack for sorting before 
	// copying the nearest intersection to IntersectionClassObject->Result.
	// note: the convex intersection functions return with the first intersection results.

	Vector3 *RayLocation; // note: these pointers must be Set() to valid pointers by external code
	Vector3 *RayDirection; 
	Vector3 *IntersectionNormal; // if non-zero then Process_Intersection_Results() will place the (perhaps vertex interpolated) surface normal here.

	// 2d screen coordinates for use by Intersect_Screen_Point...() routines.
	float ScreenX, ScreenY;

	// if true, then interpolate the normal for a polygon intersection from the vertex normals.
	// note: intersection routines below which take a FinalResult argument do not interpolate
	// the normal since they are intended to be used to find the nearest of several intersections
	// and interpolating the normal for intersections that are tossed would be wasteful.
	// If you need the normal interpolated in that case anyways then call Interpolate_Normal().
	bool InterpolateNormal; 

	// if true, then perform potentially much faster convex intersection test which will return 
	// the first valid intersection. Generally used for producing silhouettes.
	// If true, all intersections will be convex tests. If false, test mode will be determined (generally)
	// by the render object or whatever called the intersection functions and passes the Convex argument.
	bool ConvexTest;
						
	// do not consider intersections beyond this range as an intersection.
	// Note: Get_Screen_Ray sets this to scene->zstop.
	float MaxDistance; 

	// final intersection results are contained here
	IntersectionResultClass Result; 

	//
	// Implementation
	//

	// configures the member data to use the passed pointers
	inline void Set(Vector3 *location, Vector3 *direction, Vector3 *intersection_normal, bool interpolate_normal, float max_distance, bool convex_test = false)
	{
		RayLocation = location;
		RayDirection = direction;
		IntersectionNormal = intersection_normal;
		InterpolateNormal = interpolate_normal;
		MaxDistance = max_distance;
		ConvexTest = convex_test;
	}


	
	// this constructor uses static variables for the location/direction/normal variables
	// so can be only used one thread at a time unless the Set() function is used to 
	// set them to private vector3's
	inline IntersectionClass()
	: ConvexTest(false)
	{
		RayLocation = &_RayLocation;
		RayDirection = &_RayDirection;
		IntersectionNormal = &_IntersectionNormal;
		Result.CollisionType=COLL_TYPE_ALL;	//added for 'Generals'. MW
	}

	
	// This will be the most commonly used constructor
	inline IntersectionClass(Vector3 *location, Vector3 *direction, Vector3 *intersection_normal, bool interpolate_normal = false, float max_distance = WWMATH_FLOAT_MAX, bool convex_test = false)
	{
		Set(location, direction, intersection_normal, interpolate_normal, max_distance, convex_test);	
	}


	virtual ~IntersectionClass() {}


	// this copy routine is used when the model coords are needed to be copied along with the other information.
	inline void Copy_Results(IntersectionResultClass *Destination, IntersectionResultClass *Source) {
		Destination->ModelMatrix = Source->ModelMatrix;
		Destination->ModelLocation = Source->ModelLocation;
		Copy_Partial_Results(Destination, Source);
		Destination->IntersectedRenderObject = Source->IntersectedRenderObject;
	}


	inline void Copy_Results(IntersectionResultClass *Source) {
		Copy_Results(&Result, Source);
	}


	// this is called only for the nearest intersection. If the request passes a Interpolated_Normal pointer then it will be calculated.
	// otherwise the results are copied into the request structure.
	// This does not copy the matrix or location members; it is intended to be used during poly testing
	// where these values are identical between results, or as a completion function for Copy_Results()
	inline void Copy_Partial_Results(IntersectionResultClass *Destination, IntersectionResultClass *Source) 
	{
		Destination->IntersectedPolygon = Source->IntersectedPolygon;
		Destination->Intersection = Source->Intersection;
		Destination->Range = Source->Range;
		Destination->Alpha = Source->Alpha;
		Destination->Beta = Source->Beta;
		Destination->Intersects = true;
		Destination->IntersectionType = Source->IntersectionType;
	}


	// used for creating temporary copies
	inline IntersectionClass(IntersectionClass *source)
	{
		*this = source;
	}


	inline IntersectionClass *operator =(IntersectionClass *source)
	{
		Set(source->RayLocation, source->RayDirection, source->IntersectionNormal, source->InterpolateNormal, source->MaxDistance, source->ConvexTest);
		Copy_Results(&source->Result);
		return this;
	}



	// find the range to the intersection of the ray and sphere (if any)
	// note: Intersection_Request->RayDirection must be a unit vector
	// To find the actual intersection location and perhaps the intersection normal, use Intersection_Sphere() instead.
	// loosly based on code found in Graphics Gems I,  p388
	// this will only set the result's range if intersection occurs; it is intended to be used as a first pass intersection test
	// before intersecting the mesh polygons itself.
	// Note: Does NOT do Max_Distance testing
	inline bool Intersect_Sphere_Quick(SphereClass &Sphere, IntersectionResultClass *FinalResult) 
	{
		// make a unit vector from the ray origin to the sphere center
		Vector3 sphere_vector(Sphere.Center - *RayLocation);
		
		// get the dot product between the sphere_vector and the ray vector
		FinalResult->Alpha = Vector3::Dot_Product(sphere_vector, *RayDirection);

		FinalResult->Beta = Sphere.Radius * Sphere.Radius - (Vector3::Dot_Product(sphere_vector, sphere_vector) - FinalResult->Alpha * FinalResult->Alpha);

		if(FinalResult->Beta < 0.0f) {
			return FinalResult->Intersects = false;
		}
		return FinalResult->Intersects = true;
	}


	// this will find the intersection with the sphere and the intersection normal if needed.
	inline bool Intersect_Sphere(SphereClass &Sphere, IntersectionResultClass *FinalResult) 
	{
		if(!Intersect_Sphere_Quick(Sphere, FinalResult)) 
			return false;

		// determine range to intersection based on stored alpha/beta values
		float d = sqrtf(FinalResult->Beta);
		FinalResult->Range = FinalResult->Alpha - d;

		if(FinalResult->Range > MaxDistance) return false;

		FinalResult->Intersection = *RayLocation +  FinalResult->Range * (*RayDirection);
		
		if(IntersectionNormal != 0) {
			(*IntersectionNormal) = FinalResult->Intersection - Sphere.Center;	
		}
		return true;
	}


	// inline declarations
	// Usage of these functions requires including intersec.inl
	
	// determine location & direction for projected screen coordinate ray
	inline void Get_Screen_Ray(float ScreenX, float ScreenY, const LayerClass &Layer); 
	
	// uses the Result's range & the Ray_Direction to calculate the actual point of intersection.
	inline void Calculate_Intersection(IntersectionResultClass *Result);

	// interpolate the normal for a polygon intersection. Will ONLY work for polygon intersections,
	// and the Results.Intersection_Data must refer to a polygon with a valid ->mesh pointer.
	inline void Interpolate_Intersection_Normal(IntersectionResultClass *FinalResult);

	// various methods for performing intersections.
	inline bool Intersect_Plane(IntersectionResultClass *Result, Vector3 &Plane_Normal, Vector3 &Plane_Point);
	inline bool Intersect_Plane_Quick(IntersectionResultClass *Result, Vector3 &Plane_Normal, Vector3 &Plane_Point);
	inline bool Intersect_Polygon(IntersectionResultClass *Result, Vector3 &PolygonNormal, Vector3 &v1, Vector3 &v2, Vector3 &v3);
	inline bool Intersect_Polygon(IntersectionResultClass *Result, Vector3 &v1, Vector3 &v2, Vector3 &v3);
	inline bool Intersect_Polygon_Z(IntersectionResultClass *Result, Vector3 &PolygonNormal, Vector3 &v1, Vector3 &v2, Vector3 &v3);

	/*
	**	This function will fill the passed array with the set of points & uv values that represent
	**	the boolean operation of the anding of the ClipPoints with the TrianglePoints. The UV values	
	**	provided for the TrianglePoints triangle are used to generate accurate UV values for any
	**	new points created by this operation.
	**	The clipped points have Z values that make them sit on the ClipPoints triangle plane.
	*/
	static inline int _Intersect_Triangles_Z(	
		Vector3 ClipPoints[3], 
		Vector3 TrianglePoints[3], 
		Vector2 UV[3],
		Vector3 ClippedPoints[6], 
		Vector2 ClippedUV[6]
	);

	/*
	**	This function will find the z elevation for the passed Vector3 whose x/y components
	**	are defined, using the specified vertex & surface normal to determine the correct value
	*/
	static inline float _Get_Z_Elevation(Vector3 &Point, Vector3 &PlanePoint, Vector3 &PlaneNormal);


	// test a 2d screen area with the intersection's screen coords, assigning a GENERIC intersection 
	// to the specified object.
	inline bool Intersect_Screen_Object(IntersectionResultClass *Final_Result, Vector4 &Area, RenderObjClass *obj = 0);


	// non-inlined declarations


	// accumulates an object array for passing into Intersect_ObjectArray
	void Append_Object_Array(int MaxCount, int &CurrentCount, RenderObjClass **ObjectArray, RenderObjClass *Object);

	// traverses an RenderObjClass object and adds it's subobjects, potentially performing
	// a quick sphere intersection test before adding.
	void Append_Hierarchy_Objects(int MaxCount, int &CurrentCount, RenderObjClass **ObjectArray, RenderObjClass *Heirarchy, bool Test_Bounding_Spheres, bool Convex);

	// top level intersection routines, most store intersection results in the Intersection.Result
	// member structure and perform normal interpolation as a final step if indicated in the member data.

	bool Intersect_Object_Array(int ObjectCount, RenderObjClass **ObjectArray,IntersectionResultClass *FinalResult, bool Test_Bounding_Sphere, bool Convex);
	bool Intersect_Object_Array(int ObjectCount, RenderObjClass **ObjectArray,IntersectionResultClass *FinalResult, IntersectionResultClass *TemporaryResults, bool Test_Bounding_Sphere, bool Convex);
	bool Intersect_RenderObject(RenderObjClass *RObj, IntersectionResultClass *FinalResult = 0);
	bool Intersect_Screen_Point_RenderObject(float screen_x, float screen_y, const LayerClass &Layer, RenderObjClass *RObj, IntersectionResultClass *FinalResult);

	bool Intersect_Screen_Point_Layer_Range(float ScreenX, float ScreenY, const LayerClass &TopLayer, const LayerClass &BackLayer);
	bool Intersect_Screen_Point_Layer(float ScreenX, float ScreenY, const LayerClass &Layer);
	bool Intersect_Layer(const LayerClass &Layer, bool Test_All = true);

	// the various intersection routines, all of which store their intersection results
	// in the passed Intersection_Result strucuture.
	bool Intersect_Box(Vector3 &Box_Min, Vector3 &Box_Max, IntersectionResultClass *FinalResult);
	bool Intersect_Hierarchy(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult, bool Test_Bounding_Sphere = true,  bool Convex = false);
	bool Intersect_Hierarchy_Sphere(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult);
	bool Intersect_Hierarchy_Sphere_Quick(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult); 

	/*
	** Identifies exactly what sub object of a render object is under the screen space vector
	*/
	RenderObjClass *Intersect_Sub_Object(float screenx, float screeny, LayerClass &layer, RenderObjClass *robj, IntersectionResultClass *result);


	/*
	**	Functions related to determining if a 3d point is within a triangle.
	*/
	static inline void _Find_Polygon_Dominant_Plane(Vector3 &Normal, int &Axis_1, int &Axis_2);
	static inline int _Largest_Normal_Index(Vector3 &v);
	static inline bool _Point_In_Polygon(IntersectionResultClass *FinalResult, Vector3 &Normal, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3);
	static inline bool _Point_In_Polygon(IntersectionResultClass *FinalResult, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3, int axis_1, int axis_2);
	static inline bool _Point_In_Polygon(Vector3 &Point, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3, int axis_1, int axis_2,float &Alpha,float &Beta);
	static inline bool _Point_In_Polygon_Z(Vector3 &Point, Vector3 Corners[3]);
	static inline bool _Point_In_Polygon_Z(Vector3 &Point, Vector3 &Corner1, Vector3 &Corner2, Vector3 &Corner3);

protected:

	/*
	**	Find the intersection between two lines and interpolate the UV values for the intersection.
	**	Designed for use with _Intersect_Triangles_Z.
	*/
	//static inline void _Intersect_Lines_Z(Vector3 &A, Vector3 &B, Vector2 &UVStart, Vector2 &UVEnd, Vector3 &C, Vector3 &D, Vector3 ClippedPoints[6], Vector2 ClippedUV[6], int &DestIndex);
	static inline bool In_Front_Of_Line
	(
		const Vector3 & p,			// point to test
		const Vector3 & e0,			// point on edge
		const Vector3 & de			// direction of edge
	);

	static inline float Intersect_Lines
	(
		const Vector3 & p0,			// start of line segment
		const Vector3 & p1,			// end of line segment
		const Vector3 & e0,			// point on clipping edge
		const Vector3 & de			// direction of clipping edge
	);

	static inline int Clip_Triangle_To_LineXY(	
		int incount,
		Vector3 * InPoints,
		Vector2 * InUVs,
		Vector3 * OutPoints,
		Vector2 * OutUVs,
		const Vector3 & edge_point0,
		const Vector3 & edge_point1
	);



	inline float Plane_Z_Distance(Vector3 &PlaneNormal, Vector3 &PlanePoint);
	inline void Transform_Model_To_World_Coords(IntersectionResultClass *FinalResult);

	/*
	**	Static vars available for use by temporary intersection class objects.
	*/
	static Vector3 _RayLocation, _RayDirection, _IntersectionNormal;

};


#endif 
