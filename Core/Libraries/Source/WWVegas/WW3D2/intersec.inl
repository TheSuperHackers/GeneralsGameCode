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


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef INTERSEC_INL
#define INTERSEC_INL

#include "camera.h"


/// debug code that will be tossed

#ifdef DEBUG_NORMALS

#include "d:/g/app/main/debug_o.h"

inline bool Verify_Normal(Vector3 &Normal, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3)
{

	double d1 = Vector3::Dot_Product(Normal, loc1);
	double d2 = Vector3::Dot_Product(Normal, loc2);
	double d3 = Vector3::Dot_Product(Normal, loc3);

	double e1 = d1 - d2;
	double e2 = d2 - d3;
	double e3 = d3 - d1;
	
	if((fabs(e1) > 0.001) || (fabs(e2) > 0.001) || (fabs(e3) > 0.001)) {
		Debug.Print("----------\n");
		Debug.Print("dots", Vector3(d1,d2,d3));
		Debug.Print("err", Vector3(e1,e2,e3));
		return false;
	}
	return true;
}

inline void Verify_Normal2(Vector3 &Normal, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3)
{
	Vector3 v1 = loc2 - loc1;
	Vector3 v2 = loc3 - loc1;
	Vector3 normal = Vector3::Cross_Product(v1,v2);
	normal.Normalize();
	if(!Verify_Normal(Normal, loc1,loc2,loc3)) {
		Vector3 diff = Normal - normal;
		if(Verify_Normal(normal, loc1,loc2,loc3)) {
			Debug.Print("calculated worked.\n");

		}
	}
//	Normal = normal;
}

#endif

//// end of debug code to toss

/*
**	Determine the ray that corresponds to the specified screen coordinates with respect 
** to the camera location, direction and projection information.
*/

inline void IntersectionClass::Get_Screen_Ray(float screen_x, float screen_y, const LayerClass &Layer)
{

	// copy screen coords to member data
	ScreenX = screen_x;
	ScreenY = screen_y;

	// extract needed pointers from the world
	CameraClass *camera = Layer.Camera;

	// determine the ray corresponding to the camera and distance to projection plane
	Matrix3D camera_matrix = camera->Get_Transform();
	Vector3 camera_location  = camera->Get_Position();

	// the projected ray has the same origin as the camera
	*RayLocation = camera_location; 

// these 6 lines worked for SR 1.1
// build the projected screen vector
// float x_offset = width / (float)Scene->width; // render width in pixels divided by display width in pixels = ratio of displayed area
// float y_offset = height / (float)Scene->height;
//	float zmod = Scene->perspective;
//	float xmod = ((ScreenX / x_offset) * width  - xmid - Scene->xstart) * zmod  * 16384.0f/ (Scene->axratio * 128.0f);
//	float ymod = ((ScreenY / y_offset) * height - ymid - Scene->ystart) * zmod  * 16384.0f/ (Scene->ayratio * 128.0f);

	// determine the location of the screen coordinate in camera-model space
	const ViewportClass &viewport = camera->Get_Viewport();

//	float aspect = camera->Get_Aspect_Ratio();

	Vector2 min,max;
	camera->Get_View_Plane(min,max);
	float xscale = (max.X - min.X);
	float yscale = (max.Y - min.Y);

	float zmod = -1.0; // Scene->vpd; // Note: view plane distance is now always 1.0 from the camera
	float xmod = (-ScreenX + 0.5 + viewport.Min.X) * zmod * xscale;// / aspect;
	float ymod = (ScreenY - 0.5 - viewport.Min.Y) * zmod * yscale;// * aspect;

//	float xmod = (ScreenX - 0.5 - viewport.Min.X) * zmod / Scene->axratio;
//	float ymod = (ScreenY - 0.5 - viewport.Min.Y) * zmod / Scene->ayratio;

// sr1.2
//	float xmod = (ScreenX - 0.5 - Scene->xstart) * zmod / Scene->axratio;
//	float ymod = (ScreenY - 0.5 - Scene->ystart) * zmod / Scene->ayratio;
// sr1.1
//	float xmod = x_offset * zmod; //projection_width;
//	float ymod = y_offset * zmod; //projection_height;

	// transform the screen coordinates by the camera's matrix into world coordinates.
	float x = zmod * camera_matrix[0][2] + xmod * camera_matrix[0][0] + ymod * camera_matrix[0][1];
	float y = zmod * camera_matrix[1][2] + xmod * camera_matrix[1][0] + ymod * camera_matrix[1][1];
	float z = zmod * camera_matrix[2][2] + xmod * camera_matrix[2][0] + ymod * camera_matrix[2][1];

	RayDirection->Set(x,y,z);
	RayDirection->Normalize();

	// set the maximum intersection distance to the back clipping plane
	MaxDistance =  camera->Get_Depth();
	//Max_Distance =  Scene->zstop * Scene->depth;

}

/*
**	This is the Point_In_Polygon_Z low level function, optimized for use by _Intersect_Triangles_Z.
**	If it is inside, it will adjust the Z value of the point to be on the triangle plane.
*/
inline bool IntersectionClass::_Point_In_Polygon_Z(
	Vector3 &Point, 
	Vector3 &Corner1,
	Vector3 &Corner2,
	Vector3 &Corner3
) 
{
// these defines could be variables if support for other axis were neccessary
#define AXIS_1 0
#define AXIS_2 1
#define AXIS_3 2

	double u0 = Point[AXIS_1] - Corner1[AXIS_1]; 
	double v0 = Point[AXIS_2] - Corner1[AXIS_2];

	// determine the 2d vectors on the dominant plane from the first vertex to the other two
	double u1 = Corner2[AXIS_1] - Corner1[AXIS_1]; 
	double v1 = Corner2[AXIS_2] - Corner1[AXIS_2];
	double u2 = Corner3[AXIS_1] - Corner1[AXIS_1]; 
	double v2 = Corner3[AXIS_2] - Corner1[AXIS_2];

	double alpha, beta;
	bool intersect = false; 

	// calculate alpha and beta as normalized (0..1) percentages across the 2d projected triangle
	// and do bounds checking (sum <= 1)  to determine whether or not the triangle intersection occurs.

	if (u1 == 0.0f)    {
	  beta = u0 / u2;			// beta is the percentage down the edge Corner1->Corner3
	  if ((beta >= 0.0f) && (beta <= 1.0f)) {		// make sure it's within the edge segment
			alpha = (v0 - beta * v2) / v1;	 // alpha is the percentage down the edge Corner1->Corner2

			// if alpha is valid & the sum of alpha & beta is <= 1 then it's within the triangle
			// note:       0.00001 added after testing an intersection of a square in the middle indicated
			// an error of 0.0000001350, apparently due to roundoff.
			intersect = ((alpha >= 0.0) && ((alpha + beta) <= 1.0)); 
	  }
	} else {
	  beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
	  if ((beta >= 0.0) && (beta <= 1.0)) {
			alpha = (u0 - beta * u2) / u1;
			intersect = ((alpha >= 0.0) && ((alpha + beta) <= 1.0));
	  }
	}

	// if it is inside, adjust the Z value to sit upon the triangle plane.
	if(intersect) {
		float u3 = Corner2[AXIS_3] - Corner1[AXIS_3];
		float v3 = Corner3[AXIS_3] - Corner1[AXIS_3];

		Point[AXIS_3] = u3 * alpha + v3 * beta + Corner1[AXIS_3];
	}

	return intersect;
}

/*
**	Another way to access the Point_In_Polygon function
**
*/
inline bool IntersectionClass::_Point_In_Polygon_Z(
	Vector3 &Point, 
	Vector3 Corners[3]
) 
{
	return _Point_In_Polygon_Z(Point, Corners[0], Corners[1], Corners[2]);
}

/*
**	This is the general purpose Point_In_Polygon low level function. It can be called directly if you know
**	the dominant projection axes, such as in the case of 2d intersecion with heightfields.
*/
inline bool IntersectionClass::_Point_In_Polygon(
	Vector3 &Point, 
	Vector3 &loc1, 
	Vector3 &loc2, 
	Vector3 &loc3, 
	int axis_1, 
	int axis_2,
	float &Alpha,
	float &Beta) 
{

	double u0 = Point[axis_1] - loc1[axis_1]; 
	double v0 = Point[axis_2] - loc1[axis_2];

	// determine the 2d vectors on the dominant plane from the first vertex to the other two
	double u1 = loc2[axis_1] - loc1[axis_1]; 
	double v1 = loc2[axis_2] - loc1[axis_2];
	double u2 = loc3[axis_1] - loc1[axis_1]; 
	double v2 = loc3[axis_2] - loc1[axis_2];

	double alpha, beta;
	bool intersect = false; 

	// calculate alpha and beta as normalized (0..1) percentages across the 2d projected triangle
	// and do bounds checking (sum <= 1)  to determine whether or not the triangle intersection occurs.

#ifdef DEBUG_NORMALS
	bool debugmode = false;
	if(FinalResult->Alpha == 777) {
		debugmode = true;
	}
#endif

	if (u1 == 0.0f)    {
	  Beta = beta = u0 / u2;			// beta is the percentage down the edge loc1->loc3
	  if ((beta >= 0.0f) && (beta <= 1.0f)) {		// make sure it's within the edge segment
			Alpha = alpha = (v0 - beta * v2) / v1;	 // alpha is the percentage down the edge loc1->loc2

			// if alpha is valid & the sum of alpha & beta is <= 1 then it's within the triangle
			// note:       0.00001 added after testing an intersection of a square in the middle indicated
			// an error of 0.0000001350, apparently due to roundoff.
			intersect = ((alpha >= 0.0) && ((alpha + beta) <= 1.0)); 
	  }
	} else {
	  Beta = beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
	  if ((beta >= 0.0) && (beta <= 1.0)) {
			Alpha = alpha = (u0 - beta * u2) / u1;
			intersect = ((alpha >= 0.0) && ((alpha + beta) <= 1.0));
	  }
	}

#ifdef DEBUG_NORMALS
	if(debugmode) {
		Debug.Print("Intersect", intersect);
		Debug.Print("Normal ", Normal);		
		Debug.Print("Point 1", loc1);		
		Debug.Print("Point 2", loc2);		
		Debug.Print("Point 3", loc3);		
		Debug.Print("Inter  ", FinalResult->Intersection);
		Debug.Print("a/b", (float) alpha, (float) beta);
		Debug.Print("sum", (float) alpha + (float) beta);
		Debug.Print("diff", (float) (alpha - beta));
		float d1 = Vector3::Dot_Product(Normal, loc1);
		float d2 = Vector3::Dot_Product(Normal, loc2);
		float d3 = Vector3::Dot_Product(Normal, loc3);

		float e1 = d1 - d2;
		float e2 = d2 - d3;
		float e3 = d3 - d1;

		Debug.Print("dots", Vector3(d1,d2,d3));
		Debug.Print("err", Vector3(e1,e2,e3));
	}
#endif


	return intersect;
}

/*
**	This version calls the base form using member data from the FinalResult struct for 
**	some of it's arguments.
*/
inline bool IntersectionClass::_Point_In_Polygon(
	IntersectionResultClass *FinalResult, 
	Vector3 &loc1, 
	Vector3 &loc2, 
	Vector3 &loc3, 
	int axis_1, 
	int axis_2) 
{
	return (FinalResult->Intersects = _Point_In_Polygon(	FinalResult->Intersection, loc1, loc2, loc3, 
								axis_1, axis_2, FinalResult->Alpha, FinalResult->Beta));
}

/*
**	This version determines the dominant plane of the 3d triangle to be point-in-poly tested
**	and then calls the next form of _Point_In_Polygon
*/
inline bool IntersectionClass::_Point_In_Polygon(IntersectionResultClass *FinalResult, Vector3 &Normal, Vector3 &loc1, Vector3 &loc2, Vector3 &loc3) {

	// first, find the dominant axis and use the plane perpendicular to it as defined by axis_1, axis_2
	int axis_1, axis_2;
	_Find_Polygon_Dominant_Plane(Normal, axis_1, axis_2);

	return _Point_In_Polygon(FinalResult, loc1, loc2, loc3, axis_1, axis_2);
}


/*
**	Determine the Z distance to the specified polygon.
*/
inline float IntersectionClass::Plane_Z_Distance(Vector3 &PlaneNormal, Vector3 &PlanePoint) 
{
	// do a parallel check
	float divisor = (PlaneNormal[0] *(*RayDirection)[0] + PlaneNormal[1] *(*RayDirection)[1] + PlaneNormal[2] * (*RayDirection)[2]);
	if(divisor == 0) return false; // parallel

	// determine distance to plane
	double d = - (PlanePoint[0] * PlaneNormal[0] + PlanePoint[1] * PlaneNormal[1] + PlanePoint[2] * PlaneNormal[2]);

	float value = - (d + PlaneNormal[0] * (*RayLocation)[0] + PlaneNormal[1] *  (*RayLocation)[1] + PlaneNormal[2] * (*RayLocation)[2]) / divisor;

	return value;
}

/*
**	This function will find the z elevation for the passed Vector3 whose x/y components
**	are defined, using the specified vertex & surface normal to determine the correct value
*/
inline float IntersectionClass::_Get_Z_Elevation(
	Vector3 &Point, 
	Vector3 &PlanePoint, 
	Vector3 &PlaneNormal)
{
	
	// do a parallel check
	if(PlaneNormal[2] == 0) return false;

	// determine distance to plane
	double d = - (PlanePoint[0] * PlaneNormal[0] + PlanePoint[1] * PlaneNormal[1] + PlanePoint[2] * PlaneNormal[2]);

	float value = - (d + PlaneNormal[0] * Point[0] + PlaneNormal[1] *  Point[1] ) / PlaneNormal[2];

	return value;
}

/*
**	Optimized intersection test that only considers the x/y component of the intersection object
**	and will determine the intersection location down the Z axis.
*/
inline bool IntersectionClass::Intersect_Polygon_Z(IntersectionResultClass *Result, Vector3 &PolygonNormal, Vector3 &v1, Vector3 &v2, Vector3 &v3)
{

	Result->Range = Plane_Z_Distance(PolygonNormal, v1);
	(Result->Intersection)[0] = (*RayLocation)[0];
	(Result->Intersection)[1] = (*RayLocation)[1];
	(Result->Intersection)[2] = (*RayLocation)[2] - Result->Range;
	return _Point_In_Polygon(Result, PolygonNormal, v1, v2, v3);
}


/*
**	Scale the normalized direction ray to the distance of intersection
*/
void IntersectionClass::Calculate_Intersection(IntersectionResultClass *Result)
{
	(Result->Intersection)[0] = (*RayLocation)[0] + (*RayDirection)[0] * Result->Range;
	(Result->Intersection)[1] = (*RayLocation)[1] + (*RayDirection)[1] * Result->Range;
	(Result->Intersection)[2] = (*RayLocation)[2] + (*RayDirection)[2] * Result->Range;
}

/*
**	Plane intersection test that assumes a normalized RayDirection. Only determines if 
**	plane is parallel and if not, the range to it (which may be negative or beyond MaxRange).
**	It doesn't determine point of intersection either.
*/
inline bool IntersectionClass::Intersect_Plane_Quick(IntersectionResultClass *Result, Vector3 &PlaneNormal, Vector3 &PlanePoint) 
{
	// do a parallel check
	float divisor = (PlaneNormal[0] *(*RayDirection)[0] + PlaneNormal[1] *(*RayDirection)[1] + PlaneNormal[2] * (*RayDirection)[2]);
	if(divisor == 0) return false; // parallel

	// determine distance to plane
	float d = - (PlanePoint[0] * PlaneNormal[0] + PlanePoint[1] * PlaneNormal[1] + PlanePoint[2] * PlaneNormal[2]);
	Result->Range = - (d + PlaneNormal[0] * (*RayLocation)[0] + PlaneNormal[1] *  (*RayLocation)[1] + PlaneNormal[2] *  (*RayLocation)[2])  / divisor;

	return true;
}

/*
**	Determine if the specified ray will intersect the plane; returns false for planes 
**	parallel and behind ray origin.
**	Sets Range to the distance from the ray location to the intersection. 
**	Note: Range is undefined if an intersection didn't occur.
*/
inline bool IntersectionClass::Intersect_Plane(IntersectionResultClass *Result, Vector3 &PlaneNormal, Vector3 &PlanePoint) {
								 
	// normalize the ray direction
	RayDirection->Normalize();

	// call the quick test routine
	if(!Intersect_Plane_Quick(Result, PlaneNormal, PlanePoint)) return false;

	// check to make sure it's not behind the ray's origin
	if(Result->Range <= 0) return false; 

	// check to make sure it's not beyond max distance
	if(Result->Range > MaxDistance) return false;

	// determine point of intersection
	Calculate_Intersection(Result);

	return true;
}



/*
**	Return the index of the largest normal component 0..2
**	used by Find_Triangle_Dominant_Plane()
*/
inline int IntersectionClass::_Largest_Normal_Index(Vector3 &v)
{
	float x = fabsf(v[0]);
	float y = fabsf(v[1]);
	float z = fabsf(v[2]);
	if(x > y) {
		if(x > z) {
			return 0; // x > y && x > z --> x is the max
		}
		return 2; // x > y && !(x > z) --> z is the max
	}
	if(y > z) 
		return 1; //  x <= y && y > z --> y is the max
	return 2; // y > x &&  y > z --> z is the max
}

/*
** Use the Polygon's currently defined surface normal to determine it's dominant axis.
** Axis_1 and Axis_2 are set to the indices of the two axis that define the dominant plane.
*/
inline void IntersectionClass::_Find_Polygon_Dominant_Plane(Vector3 &Normal, int &Axis_1, int &Axis_2)
{
	switch (_Largest_Normal_Index(Normal)) 
	{
	case 0: 
		// Dominant is the X axis
		Axis_1 = 2;
		Axis_2 = 1;
		break;
	case 1: 
		// Dominant is the Y axis
		Axis_1 = 2;
		Axis_2 = 0;
		break;
	case 2: 
		// Dominant is the Z axis
		Axis_1 = 0;
		Axis_2 = 1;
		break;
	}
}

/*
**	Returns true if ray intersects polygon.
**	Changes passed Intersection argument to location of intersection if it occurs, 
**	and sets Range to the distance from the ray location to the intersection.
**	If Interpolated_Normal is specified it will interpolate the surface normal based 
**	on the vertex normals.
*/
inline bool IntersectionClass::Intersect_Polygon(IntersectionResultClass *Result, Vector3 &PolygonNormal, Vector3 &v1, Vector3 &v2, Vector3 &v3)
{
	// first check to see if it hits the plane; determine plane normal and find point on plane (from a vertex)
	
#ifdef DEBUG_NORMALS
	Verify_Normal2(PolygonNormal, v1,v2,v3);
#endif

	if(Intersect_Plane(Result, PolygonNormal, v1)) {
		// then check to see if it it actually intersects the polygon. 
		return _Point_In_Polygon(Result, PolygonNormal, v1, v2, v3);
	}
	// doesn't even hit the plane, return false.
	return false;
}

/*
**	This version will calc the normal for the polygon before calling
**	a lower form of Intersect_Polygon
*/
inline bool IntersectionClass::Intersect_Polygon(IntersectionResultClass *Result, Vector3 &v1, Vector3 &v2, Vector3 &v3)
{
	Vector3 vec1 = v2 - v1;
	Vector3 vec2 = v3 - v1;
#ifdef ALLOW_TEMPORARIES
	Vector3 normal = Vector3::Cross_Product(vec1, vec2);
#else
	Vector3 normal;
	Vector3::Cross_Product(vec1, vec2, &normal);
#endif

	return Intersect_Polygon(Result, normal, v1,v2,v3);
}


// called after Interpolate_Intersection_Normal.
// transform the intersection and the normal from model coords into world coords
inline void IntersectionClass::Transform_Model_To_World_Coords(IntersectionResultClass *FinalResult) 
{
#ifdef ALLOW_TEMPORARIES
	FinalResult->Intersection = FinalResult->ModelMatrix * FinalResult->Intersection + FinalResult->ModelLocation;
	if(IntersectionNormal != 0) 
	{
		Vector3 normal(*IntersectionNormal);
		*IntersectionNormal = FinalResult->ModelMatrix * normal; 
	}
#else
	FinalResult->ModelMatrix.mulVector3(FinalResult->Intersection);
	FinalResult->Intersection += FinalResult->ModelLocation;
	if(IntersectionNormal != 0) 
	{
		FinalResult->ModelMatrix.mulVector3(*IntersectionNormal);
	}
#endif
}



bool IntersectionClass::Intersect_Screen_Object(	IntersectionResultClass *Final_Result,
											Vector4 &Area, 
											RenderObjClass *obj)
{
	if(Final_Result->Intersects =  ((ScreenX >= Area[0]) && (ScreenX <= Area[2]) && (ScreenY >= Area[1]) && (ScreenY <= Area[3]))) {
		Final_Result->IntersectionType = IntersectionResultClass::GENERIC;
		Final_Result->IntersectedRenderObject = obj;
		Final_Result->Range = 0;
		return true;
	}
	return false;
}


/*
**	Determines the point of intersection, if any between the line segments AB and CD. 
**	If an intersection occurs, then the UV values are interpolated along AB.
**	Disregards the Z value and considers only the X/Y data except for determining
**	the Z value of the intersection.
**	This function could be easily modified to support other axes.
* /
void IntersectionClass::_Intersect_Lines_Z(	
	Vector3 &A, 
	Vector3 &B,
	Vector2 &UVStart,
	Vector2 &UVEnd,
	Vector3 &C, 
	Vector3 &D,
	Vector3 ClippedPoints[6], 
	Vector2 ClippedUV[6],
	int &DestIndex)
{
	/*
		Let A,B,C,D be 2-space position vectors.  Then the directed line segments AB & CD are given by:

				AB=A+r(B-A), r in [0,1]
				CD=C+s(D-C), s in [0,1]
		If AB & CD intersect, then

				A+r(B-A)=C+s(D-C), or

				Ax+r(Bx-Ax)=Cx+s(Dx-Cx)
				Ay+r(By-Ay)=Cy+s(Dy-Cy)  for some r,s in [0,1]
		Solving the above for r and s yields

					(Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
				r = -----------------------------  (eqn 1)
					(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
					(Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
				s = -----------------------------  (eqn 2)
					(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
		Let P be the position vector of the intersection point, then

				P=A+r(B-A) or

				Px=Ax+r(Bx-Ax)
				Py=Ay+r(By-Ay)
		By examining the values of r & s, you can also determine some other limiting conditions:

				If 0<=r<=1 & 0<=s<=1, intersection exists
					r<0 or r>1 or s<0 or s>1 line segments do not intersect
		If the denominator in eqn 1 is zero, AB & CD are parallel

		If the numerator in eqn 1 is also zero, AB & CD are coincident

		If the intersection point of the 2 lines are needed (lines in this context mean infinite lines) regardless whether the two line segments intersect, then

		If r>1, P is located on extension of AB
		If r<0, P is located on extension of BA
		If s>1, P is located on extension of CD
		If s<0, P is located on extension of DC
	* /

	// the numerator is required for all execution routes
	float numerator =   (A[AXIS_2] - C[AXIS_2]) * (D[AXIS_1] - C[AXIS_1]) - (A[AXIS_1] - C[AXIS_1]) * (D[AXIS_2] - C[AXIS_2]);

	// if the denominator is zero, then the segments are parallel.
	float denominator = (B[AXIS_1] - A[AXIS_1]) * (D[AXIS_2] - C[AXIS_2]) - (B[AXIS_2] - A[AXIS_2]) * (D[AXIS_1] - C[AXIS_1]);

	// r & s are percentages through the line segment.
	float r, s;

	// check to see if they are parallel
	if(denominator == 0) {

		// check to see if they are coincident
		// a numerator of zero with a denominator of zero indicates coincident lines.
		if (numerator != 0) {

			// parallel, not coincident lines (and segments) do not intersect.
			return;
		}

		// perform special case for parallel segments

		// determine relative position 0..1 of C and D on one of the 1d vectors of A-B
		float len = B[AXIS_1] - A[AXIS_1];
		float cpos,dpos;

		// if the length of the edge on the first axis is zero, use the other axis instead.
		if(len) {
			len = 1.0 / len;
			cpos = (C[AXIS_1] - A[AXIS_1]) * len;
			dpos = (D[AXIS_1] - A[AXIS_1]) * len;
		} else {
			len = B[AXIS_2] - A[AXIS_2];

			// degenerate triangle test
			if(len == 0)
				return;
			
			len = 1.0 / len;
			cpos = (C[AXIS_2] - A[AXIS_2]) * len;
			dpos = (D[AXIS_2] - A[AXIS_2]) * len;
		}

		// check to see if there's any overlap
		// one of the two pos values must be 0>pos>1 or there is no intersection.
		// this test will ensure that cpos & dpos will not both be outside the same end of the segment.
		if(((cpos < 0) && (dpos < 0)) || ((cpos > 1) && (dpos > 1))) 
			return;

		if(cpos < 0) {
			// C is outside, therefore D is inside or on other side.
			// use the original vertex.
			ClippedPoints[DestIndex] = A;
			ClippedUV[DestIndex++] = UVStart;
		} else if (cpos > 1) {
			// C is outside far side, therefore D is inside or on other side.
			// use the far vertex.
			ClippedPoints[DestIndex] = B;
			ClippedUV[DestIndex++] = UVEnd;
		} else {
			// C is inside.
			// Use C as the vertex, and interpolate the UV coords.
			ClippedPoints[DestIndex] = C;
			ClippedUV[DestIndex++] = (UVEnd - UVStart) * cpos + UVStart;
		}

		if(dpos < 0) {
			// D is outside near vertex, therefore C is inside or outside far vertex
			// use near vertex
			ClippedPoints[DestIndex] = A;
			ClippedUV[DestIndex++] = UVStart;
		} else if (dpos > 1) {
			// D is outside far vertex, therefore C is inside or outside the near vertex.
			// use the far vertex.
			ClippedPoints[DestIndex] = B;
			ClippedUV[DestIndex++] = UVEnd;
		} else {
			// D is inside.
			// Use D as the vertex, and interpolate the UV coords.
			ClippedPoints[DestIndex] = D;
			ClippedUV[DestIndex++] = (UVEnd - UVStart) * dpos + UVStart;
		}
		return;

	} 

	// determine the percentage into the line segments that the intersection occurs.
	// an intersection of segments will produce r & s values between 0 & 1.
	denominator = 1.0 / denominator;
	r = numerator * denominator;

	numerator = (A[AXIS_2] - C[AXIS_2]) * (B[AXIS_1] - A[AXIS_1]) - (A[AXIS_1] - C[AXIS_1]) * (B[AXIS_2] - A[AXIS_2]);
	s = numerator * denominator;

	// determine if the line intersect within the defined segments.
	if((0.0 <= r) && (r <= 1.0) && (0.0 <= s) && (s <= 1.0)) {

		// they intersect.
		// determine intersection point 
		Vector3 v = D - C;
//		float len = v.Length();
		ClippedPoints[DestIndex] = C + v * s;
		
		// interpolate UV values
		Vector2 uv = UVEnd - UVStart;
//		len = uv.Length();
		ClippedUV[DestIndex++] = UVStart + uv * r;
	}
}

/*
	A failed attempt to use a graphics gem vol 2 example


	// Compute a1, b1, c1, where line joining points 1 and 2
    // is "a1 x  +  b1 y  +  c1  =  0".
	float a1 = B[AXIS_2] - A[AXIS_2];
	float b1 = B[AXIS_1] - A[AXIS_1];
	float c1 = B[AXIS_2] * A[AXIS_1] - A[AXIS_1] * B[AXIS_2];

	// Compute r3 & r4, the sign values
    float r3 = a1 * C[AXIS_1] + b1 * C[AXIS_2] + c1;
    float r4 = a1 * D[AXIS_1] + b1 * D[AXIS_2] + c1;

	// Check signs of r3 and r4.  If both point 3 and point 4 lie on
    // same side of line 1, the line segments do not intersect.
    if ( r3 != 0 && r4 != 0 && (((r3 < 0) && (r4 < 0)) || ((r3 > 0) && (r4 > 0)))
        return; // ( DONT_INTERSECT );

    // Compute a2, b2, c2
    float a2 = D[AXIS_2] - C[AXIS_2];
    float b2 = C[AXIS_1] - D[AXIS_1];
    float c2 = D[AXIS_1] * C[AXIS_2] - C[AXIS_1] * D[AXIS_2];

    // Compute r1 and r2
    float r1 = a2 * A[AXIS_1] + b2 * A[AXIS_2] + c2;
    float r2 = a2 * B[AXIS_1] + b2 * B[AXIS_2] + c2;

    // Check signs of r1 and r2.  If both point 1 and point 2 lie
    // on same side of second line segment, the line segments do
    // not intersect.
    if ( r1 != 0 && r2 != 0 && (((r1 < 0) && (r2 < 0)) || ((r1 > 0) && (r2 > 0))))
        return; // ( DONT_INTERSECT );

    // Line segments intersect: compute intersection point. 
    float denom = a1 * b2 - a2 * b1;
    if ( denom == 0 )
        return; // ( COLLINEAR );

    float offset = denom < 0 ? - denom * 0.5f : denom * 0.5f;

    // The denom/2 is to get rounding instead of truncating.  It
    // is added or subtracted to the numerator, depending upon the
    // sign of the numerator.
 
    float num = b1 * c2 - b2 * c1;
    float x = ( num < 0 ? num - offset : num + offset ) / denom;

    num = a2 * c1 - a1 * c2;
    float y = ( num < 0 ? num - offset : num + offset ) / denom;

	ClippedPoints[DestIndex] = Vector3(x,y,0);
	ClippedUV[DestIndex++] = Vector3
    return; //( DO_INTERSECT );  // lines_intersect
*/

inline bool IntersectionClass::In_Front_Of_Line
(
	const Vector3 & p,			// point to test
	const Vector3 & e0,			// point on edge
	const Vector3 & de			// direction of edge
)
{
	Vector3 dp = p - e0;
	float val = de.X*dp.Y - de.Y*dp.X;
	if (val > 0.0f) {
		return true;
	}
	return false;
}

inline float IntersectionClass::Intersect_Lines
(
	const Vector3 & p0,			// start of line segment
	const Vector3 & p1,			// end of line segment
	const Vector3 & e0,			// point on clipping edge
	const Vector3 & de			// direction of clipping edge
)
{
	float dpx = p1.X - p0.X;
	float dpy = p1.Y - p0.Y;

	float den = de.Y * dpx - de.X * dpy;
	
	if (fabs(den) > WWMATH_EPSILON) {

		float num = p0.Y*de.X - p0.X*de.Y + e0.X*de.Y - e0.Y*de.X;
		float t = num/den;
		if ((t >= 0.0f) && (t <= 1.0f)) {
			return t;
		}
	}

	return 0.0f;
}


#define EMIT(p,uv) OutPoints[outnum] = p; OutUVs[outnum] = uv; outnum++;


inline int IntersectionClass::Clip_Triangle_To_LineXY(	
	int incount,
	Vector3 * InPoints,
	Vector2 * InUVs,
	Vector3 * OutPoints,
	Vector2 * OutUVs,
	const Vector3 & edge_point0,
	const Vector3 & edge_point1
)
{
	Vector3 e0 = edge_point0;
	Vector3 de = edge_point1 - edge_point0;
	
	// number of verts output.
	int outnum = 0;

	// start and end verts of the current edge
	int p0,p1;
	p0 = incount-1;
	
	// intersection temporaries.
	float intersection;
	Vector3 intersection_point;
	Vector2 intersection_uv;

	// loop over each edge in the input polygon
	for (p1=0; p1<incount; p1++) {

		if (In_Front_Of_Line(InPoints[p1],e0,de)) {
			if (In_Front_Of_Line(InPoints[p0],e0,de)) {

				// both inside, emit p1
				EMIT(InPoints[p1],InUVs[p1]);
				
			} else {

				// edge going out->in, emit intersection and endpoint
				intersection = Intersect_Lines(InPoints[p0], InPoints[p1], e0, de);
				intersection_point = (1.0f - intersection) * InPoints[p0] + intersection * InPoints[p1];
				intersection_uv = (1.0f - intersection) * InUVs[p0] + intersection * InUVs[p1];
				EMIT(intersection_point,intersection_uv);
				EMIT(InPoints[p1],InUVs[p1]);
			}
		} else {

			if (In_Front_Of_Line(InPoints[p0], e0, de)) {
				
				// edge going in->out, emit intersection
				intersection = Intersect_Lines(InPoints[p0],InPoints[p1], e0, de);
				intersection_point = (1.0f - intersection) * InPoints[p0] + intersection * InPoints[p1];
				intersection_uv = (1.0f - intersection) * InUVs[p0] + intersection * InUVs[p1];
				EMIT(intersection_point,intersection_uv);

			}

		}

		// move to next edge
		p0 = p1;
	}

	return outnum;
}

inline int IntersectionClass::_Intersect_Triangles_Z(	
	Vector3 ClipPoints[3], 
	Vector3 TrianglePoints[3], 
	Vector2 UV[3],
	Vector3 ClippedPoints[6], 
	Vector2 ClippedUV[6]
)
{
	int count;
	Vector3 tmp_points[6];
	Vector2 tmp_uv[6];
	
	count = Clip_Triangle_To_LineXY(3,TrianglePoints,UV,ClippedPoints,ClippedUV,ClipPoints[0],ClipPoints[1]);
	count = Clip_Triangle_To_LineXY(count,ClippedPoints,ClippedUV,tmp_points,tmp_uv,ClipPoints[1],ClipPoints[2]);
	count = Clip_Triangle_To_LineXY(count,tmp_points,tmp_uv,ClippedPoints,ClippedUV,ClipPoints[2],ClipPoints[0]);
	
	return count;
}

/*
**	This function will fill the passed array with the set of points & uv values that represent
**	the boolean operation of the anding of the ClipPoints with the TrianglePoints. The UV values	
**	provided for the TrianglePoints triangle are used to generate accurate UV values for any
**	new points created by this operation.
**	This function returns the number of vertices it required to define the intersection.
* /
int IntersectionClass::_Intersect_Triangles_Z(	
	Vector3 ClipPoints[3], 
	Vector3 TrianglePoints[3], 
	Vector2 UV[3],
	Vector3 ClippedPoints[6], 
	Vector2 ClippedUV[6]
)
{
	// first, check to see if all triangle points are inside clip area
	// the point in polygon will drop any inside points to the clip triangle plane.
	bool inside[3];

	bool noclip;
	noclip =  inside[0] = _Point_In_Polygon_Z(TrianglePoints[0], ClipPoints);
	noclip &= inside[1] = _Point_In_Polygon_Z(TrianglePoints[1], ClipPoints);
	noclip &= inside[2] = _Point_In_Polygon_Z(TrianglePoints[2], ClipPoints);

	// if all points are inside clip area, then copy the triangle points &
	// UV's to the destination & return 3 (the number of points in the clipped polygon).
	if(noclip) {
		ClippedPoints[0] = TrianglePoints[0];
		ClippedPoints[1] = TrianglePoints[1];
		ClippedPoints[2] = TrianglePoints[2];
		ClippedUV[0] = UV[0];
		ClippedUV[1] = UV[1];
		ClippedUV[2] = UV[2];

		return 3;
	}

	int points = 0;	// number of output polygon points

	//	not all uv triangle points are inside the clip triangle.
	//	Test to see if any clip points are inside the uv triangle
	float alpha, beta;
	if(_Point_In_Polygon(ClipPoints[0], TrianglePoints[0], TrianglePoints[1], TrianglePoints[2], 0, 1, alpha, beta)) {
		ClippedPoints[points] = ClipPoints[0];
		Vector2 uv1 = UV[1] - UV[0];
		Vector2 uv2 = UV[2] - UV[0];
		ClippedUV[points++] = UV[0] + alpha * uv1 + beta * uv2; 
	}

	if(_Point_In_Polygon(ClipPoints[1], TrianglePoints[0], TrianglePoints[1], TrianglePoints[2], 0, 1, alpha, beta)) {
		ClippedPoints[points] = ClipPoints[1];
		Vector2 uv1 = UV[1] - UV[0];
		Vector2 uv2 = UV[2] - UV[0];
		ClippedUV[points++] = UV[0] + alpha * uv1 + beta * uv2; 
	}

	if(_Point_In_Polygon(ClipPoints[2], TrianglePoints[0], TrianglePoints[1], TrianglePoints[2], 0, 1, alpha, beta)) {
		ClippedPoints[points] = ClipPoints[2];
		Vector2 uv1 = UV[1] - UV[0];
		Vector2 uv2 = UV[2] - UV[0];
		ClippedUV[points++] = UV[0] + alpha * uv1 + beta * uv2; 
	}

	// if all 3 clip points are inside the decal triangle then return
	if(points == 3)
		return;

	//	The clip triangle does not fully contain the uv triangle, and the uv triangle
	//	does not fully contain the clip triangle.
	//	Intersect any edge which has at least one outside point with all of the clip edges. 
	//	First, determine which edges to test. Those points that are already clipped (by being inside)
	//	are immediately copied to the clipped point & uv arrays.

	//	these bools indicate which edges of the triangle to be clipped are to be tested.
	bool test_01 = false;
	bool test_02 = false;
	bool test_12 = false;

	if( inside[0] )	{	ClippedPoints[points] = TrianglePoints[0]; ClippedUV[points++] = UV[0];	} 
	else {	test_01 = test_02 = true; } 
	
	if( inside[1] )	{	ClippedPoints[points] = TrianglePoints[1]; ClippedUV[points++] = UV[1];	}
	else {	test_01 = test_12 = true; }

	if( inside[2] )	{	ClippedPoints[points] = TrianglePoints[2]; ClippedUV[points++] = UV[2];}
	else {	test_02 = test_12 = true; }

	//	Now test each indicated segment.
	//	Intersect_2D_Lines will interpolate the clipped UV values if an intersection occurs, and it
	//	will also increment the points variable (passed as a reference).
	//	Any intersections are stored in the passed ClippedPoints array.
	if(test_01) {
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[1],
							UV[0], UV[1],
							ClipPoints[0], ClipPoints[1],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[1],
							UV[0], UV[1],
							ClipPoints[0], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[1],
							UV[0], UV[1],
							ClipPoints[1], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
	}
	if(test_02) {
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[2],
							UV[0], UV[2],
							ClipPoints[0], ClipPoints[1],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[2],
							UV[0], UV[2],
							ClipPoints[0], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[0], TrianglePoints[2],
							UV[0], UV[2],
							ClipPoints[1], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
	}
	if(test_12) {
		_Intersect_Lines_Z(	TrianglePoints[1], TrianglePoints[2],
							UV[1], UV[2],
							ClipPoints[0], ClipPoints[1],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[1], TrianglePoints[2],
							UV[1], UV[2],
							ClipPoints[0], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
		_Intersect_Lines_Z(	TrianglePoints[1], TrianglePoints[2],
							UV[1], UV[2],
							ClipPoints[1], ClipPoints[2],
							ClippedPoints, 
							ClippedUV,
							points);
	}

	//	If no intersections have occurred, then the triangle must be completely outside
	//	the clipping area.
/*
	//	if it is determined that no intersections have occurred, then copy the clip triangle points
	//	into the destination array and determine the correct UV values for the subset of the
	//	triangle that was clipped.
	if(points == 0) {
		ClippedPoints[0] = ClipPoints[0];
		ClippedPoints[1] = ClipPoints[1];
		ClippedPoints[2] = ClipPoints[2];
		ClippedUV[0] = UV[0];
		ClippedUV[1] = UV[1];
		ClippedUV[2] = UV[2];

		return 3;
	}
* /
	// points will be 0, 3, 4, 5 or 6 
	if(!((points == 0) || (points == 3) || (points == 4) || (points == 5) || (points == 6))) {
		Debug.Print("points", points);
		return 0; //_Intersect_Triangles_Z(	ClipPoints, TrianglePoints, UV, ClippedPoints, ClippedUV);
	}

	return points;

}
*/


#endif 
