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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/intersec.cpp                           $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/06/01 5:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "intersec.h"
#include "camera.h"
#include "scene.h"
#include "intersec.inl"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// these statics are used for single-threaded use of the IntersectionClass ONLY
Vector3 IntersectionClass::_RayLocation(0,0,0);
Vector3 IntersectionClass::_RayDirection(0,0,0);
Vector3 IntersectionClass::_IntersectionNormal(0,0,0);


bool IntersectionClass::Intersect_Screen_Point_RenderObject(float screen_x, float screen_y, const LayerClass &Layer, RenderObjClass *RObj, IntersectionResultClass *FinalResult)
{
	Get_Screen_Ray(screen_x, screen_y, Layer);
	return Intersect_RenderObject(RObj, FinalResult);
}

bool IntersectionClass::Intersect_RenderObject(RenderObjClass *RObj, IntersectionResultClass *FinalResult)
{
	if(FinalResult == nullptr)
		FinalResult = &Result;

	return RObj->Intersect(this, FinalResult);
}

// iterate through the layers of a world, front to back, returning true if/when an intersection
// with an object occurs.
bool IntersectionClass::Intersect_Screen_Point_Layer_Range
(
	float screen_x,
	float screen_y,
	const LayerClass &TopLayer,
	const LayerClass &BackLayer
)
{

	// intersect from front layer to back layers. An intersection with an object
	// in any layer is assumed to be in front of any potential intersections in layers
	// below it.

	// find the last layer in the list
	const LayerClass *Layer = &TopLayer;

	// iterate through all layers in list
	while(Layer->Is_Valid()) {
		if(Intersect_Screen_Point_Layer(screen_x, screen_y, *Layer))
			return true;

		// if this is the back layer then that is all we need to test
		if(Layer == &BackLayer)
			return false;

		Layer = Layer->Next();
	}
	return false;
}


bool IntersectionClass::Intersect_Screen_Point_Layer(float screen_x, float screen_y, const LayerClass &Layer)
{
	// mark this object as not intersecting yet
	Result.Intersects = false;

	// first, do a test to make sure the screen coords are within the rendering area for this layer.
	const ViewportClass &v = Layer.Camera->Get_Viewport();

	if((screen_x < v.Min.X) ||
		(screen_x > v.Max.X)  ||
		(screen_y < v.Min.Y) ||
		(screen_y > v.Max.Y))
		return false;

	Result.Range = Layer.Camera->Get_Depth(); //scene->depth * scene->zstop;

	// get the ray for these screen coordinates
	Get_Screen_Ray(screen_x, screen_y, Layer);

	return Intersect_Layer(Layer, false);
}

bool IntersectionClass::Intersect_Layer(const LayerClass &Layer, bool Test_All)
{
	IntersectionResultClass FinalResult;

	Result.Intersects = false;

	SceneIterator *it = Layer.Scene->Create_Iterator(!Test_All);

	// select the first object
	it->First();

	// loop through all render objects in this layer:
	while(!it->Is_Done()) {

		// get the render object
		RenderObjClass * robj = it->Current_Item();
		it->Next();

		// only intersect if it was visible or if we must test all in layer
		// Added 'Generals' code to only detect intersection on matching Collision_Type. MW
		if( robj->Get_Collision_Type() & Result.CollisionType && (Test_All || robj->Is_Really_Visible()) && robj->Intersect(this, &FinalResult)) {
			if(FinalResult.Range < Result.Range) {
				Copy_Results(&FinalResult);
			}
		}
	}

	Layer.Scene->Destroy_Iterator(it);

	return Result.Intersects;
}


void IntersectionClass::Append_Object_Array(
	int MaxCount,
	int &CurrentCount,
	RenderObjClass **ObjectArray,
	RenderObjClass *Object)
{
	if(CurrentCount < MaxCount) {
		ObjectArray[CurrentCount] = Object;
		CurrentCount++;
		return;
	}
	WWDEBUG_SAY(("IntersectionClass::Append_Object_Array - Too many objects"));
}

// determines if specified plane-intersection point (co-planar with polygon) is within the the passed polygon.
// If Interpolated_Normal is specified, it will interpolate the normal for the intersection point
// note: Polygon normal MUST BE CORRECT

// this will return true if the ray intersects the specified box
// sets the point of intersection within the Request->Result.Intersection vector
bool IntersectionClass::Intersect_Box(Vector3 &Box_Min, Vector3 &Box_Max, IntersectionResultClass *FinalResult) {
	// Fast Ray-Box Intersection, modified from code written by Andrew Woo from "Graphics Gems", Academic Press, 1990

	enum {
		RIGHT = 0,
		LEFT,
		MIDDLE,
		PLANE_COUNT
	};

	bool inside = true;
	char quadrant[PLANE_COUNT];
	int counter;
	float distance[PLANE_COUNT];
	float candidate_plane[PLANE_COUNT];

	REGISTER Vector3 *intersection = &FinalResult->Intersection;

	// Find candidate planes and determine if the ray is outside the box
	for (counter = 0; counter < PLANE_COUNT; counter++) {
		if((*RayLocation)[counter] < Box_Min[counter]) {
			quadrant[counter] = LEFT;
			candidate_plane[counter] = Box_Min[counter];
			inside = false;
		} else {
			if ((*RayLocation)[counter] > Box_Max[counter]) {
				quadrant[counter] = RIGHT;
				candidate_plane[counter] = Box_Max[counter];
				inside = false;
			} else {
				quadrant[counter] = MIDDLE;
			}
		}
	}

	// check to see if the ray origin is inside bounding box
	if(inside)	{
		*intersection = *RayLocation;
		return FinalResult->Intersects = true;
	}

	// Calculate distances to candidate planes
	for (counter = 0; counter < PLANE_COUNT; counter++) {
		if ((quadrant[counter] != MIDDLE) && ((*RayDirection)[counter] != 0.0f))
			distance[counter] = (candidate_plane[counter] - (*RayLocation)[counter]) / (*RayDirection)[counter];
		else
			distance[counter] = -1.0f;
	}

	// get the largest of the distances for final choice of intersection
	int nearest_plane = 0;
	for (counter = 1; counter < PLANE_COUNT; counter++) {
		if (distance[nearest_plane] < distance[counter])
			nearest_plane = counter;
	}

	// Check to make sure the nearest plane is not behind the ray (inside box tested above)
	if (distance[nearest_plane] < 0.0f)
		return FinalResult->Intersects = false;

	for (counter = 0; counter < PLANE_COUNT; counter++) {
		if (nearest_plane != counter) {
			(*intersection)[counter] = (*RayLocation)[counter] + distance[nearest_plane] *(*RayDirection)[counter];
			if ((*intersection)[counter] < Box_Min[counter] || (*intersection)[counter] > Box_Max[counter])
				return FinalResult->Intersects = false;
		} else {
			(*intersection)[counter] = candidate_plane[counter];
		}
	}
	return FinalResult->Intersects = true; // ray hits box
}


// simply returns true if a ray hits the bounding sphere of any node in a hierarchy
// note: Result will only contain range, not the intersection point/normal.
bool IntersectionClass::Intersect_Hierarchy_Sphere_Quick(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult)
{

	int counter = Hierarchy->Get_Num_Sub_Objects();
	while(counter--) {
		RenderObjClass *obj = Hierarchy->Get_Sub_Object(counter);
		obj->Release_Ref(); // you already own a reference to this object indirectly..
		if(obj->Intersect_Sphere_Quick(this, FinalResult))
			return true;
	}
	return false;
}

// returns true if a ray hits the bounding sphere of any node in a hierarchy
// note: Result will contain range and the intersection point/normal.
bool IntersectionClass::Intersect_Hierarchy_Sphere(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult) {

	int counter = Hierarchy->Get_Num_Sub_Objects();
	while(counter--) {
		RenderObjClass *obj = Hierarchy->Get_Sub_Object(counter);
		obj->Release_Ref(); // you already own a reference to this object indirectly..
		if(obj->Intersect_Sphere(this, FinalResult))
			return true;
	}
	return false;
}

void IntersectionClass::Append_Hierarchy_Objects(
	int MaxCount,
	int &CurrentCount,
	RenderObjClass **ObjectArray,
	RenderObjClass *Hierarchy,
	bool Test_Bounding_Sphere,
	bool Convex)
{
	IntersectionResultClass result;

	// first check the bounding spheres for hits (if specified)
	int counter = Hierarchy->Get_Num_Sub_Objects();
	if(Test_Bounding_Sphere) {
		while(counter--) {
			RenderObjClass *obj = Hierarchy->Get_Sub_Object(counter);
			obj->Release_Ref();	// you already own a reference to the object indirectly
			if(obj->Intersect_Sphere_Quick(this, &result)) {
				Append_Object_Array(MaxCount, CurrentCount, ObjectArray, obj);
//				OutputDebugString("o"); // this shows one o per sphere intersection
			} else {
//				OutputDebugString("."); // this shows one . per sphere miss
			}
		}

	} else {
		// simply copy the pointers into the array
		while(counter--) {
			RenderObjClass *obj = Hierarchy->Get_Sub_Object(counter);
			Append_Object_Array(MaxCount, CurrentCount, ObjectArray, obj);
			obj->Release_Ref(); // you already own a reference to this object indirectly..
		}
	}
}

bool IntersectionClass::Intersect_Hierarchy(RenderObjClass *Hierarchy, IntersectionResultClass *FinalResult, bool Test_Bounding_Sphere, bool Convex ) {

//	OutputDebugString("\n");
//	return FinalResult->Intersects = false;

	RenderObjClass *candidate_objects[MAX_HIERARCHY_NODE_COUNT];
	int candidate_count =  0;

	Append_Hierarchy_Objects(MAX_HIERARCHY_NODE_COUNT, candidate_count, candidate_objects, Hierarchy, Test_Bounding_Sphere, Convex);

	// make sure there's at least one sphere hit before continuing to more expensive tests below..
	if(candidate_count == 0) {
	//			OutputDebugString("/"); // no sphere intersections
		return FinalResult->Intersects = false;
	}

	// note: Test_Bounding_Sphere argument is false because the Append_Hierarchy_Objects will have
	// already performed that test if indicated.
	if(Intersect_Object_Array(candidate_count, candidate_objects, FinalResult, false, Convex)) {
		return true;
	}
	return false;
}

RenderObjClass *IntersectionClass::Intersect_Sub_Object(float screenx, float screeny, LayerClass &layer, RenderObjClass *robj, IntersectionResultClass *result)
{
	if (robj->Get_Num_Sub_Objects()) {
		for (int lp = 0; lp < robj->Get_Num_Sub_Objects(); lp++) {
			RenderObjClass *sub = robj->Get_Sub_Object(lp);
			RenderObjClass *retval = Intersect_Sub_Object(screenx, screeny, layer, sub, result);
			sub->Release_Ref();
			if (retval) return retval;
		}
	}
	if (Intersect_Screen_Point_RenderObject(screenx, screeny, layer, robj, result)) {
		return robj;
	}
	return nullptr;
}

// finds the intersection of the nearest object in the array.
// This will usually be the last stage after determining potential intersections
// using Intersect_Sphere_Quick() and adding hits to the array for this
// more accurate test, as done in Intersect_Heirarchy().
bool IntersectionClass::Intersect_Object_Array(
	int Object_Count,
	RenderObjClass **ObjectArray,
	IntersectionResultClass *FinalResult,
	bool Test_Bounding_Sphere,
	bool Convex
	)
{
	IntersectionResultClass TemporaryResults[MAX_HIERARCHY_NODE_COUNT];
	assert(Object_Count <= MAX_HIERARCHY_NODE_COUNT);
	return Intersect_Object_Array(Object_Count, ObjectArray, FinalResult, TemporaryResults, Test_Bounding_Sphere, Convex);
}

bool IntersectionClass::Intersect_Object_Array(
	int Object_Count,
	RenderObjClass **ObjectArray,
	IntersectionResultClass *FinalResult,
	IntersectionResultClass *TemporaryResults,
	bool Test_Bounding_Sphere,
	bool Convex
	)
{
	//  Determine ranges for all intersections
	IntersectionClass temp(this);
	int counter = Object_Count;
	bool hit = false;

	// if it's a convex hierarchy (ie a control panel) then find the first hit otherwise use the more expensive exact intersection routine
	// for use with potentially concave hierarchies.
	int nearest_index = -1;
	if(ConvexTest || Convex) {
		if(Test_Bounding_Sphere) {
			while(counter--) {
				if(ObjectArray[counter]->Intersect_Sphere_Quick(this, &TemporaryResults[counter])) {
					hit = ObjectArray[counter]->Intersect(this, FinalResult);
				}
				if(hit) {
					nearest_index = counter;
					counter = 0;
				}
			}
		} else {
			while(counter--) {
				hit = ObjectArray[counter]->Intersect(this, FinalResult);
				if(hit) {
					nearest_index = counter;
					counter = 0;
				}
			}
		}
	} else {
		if(Test_Bounding_Sphere) {
			while(counter--) {
				if(ObjectArray[counter]->Intersect_Sphere_Quick(this, &TemporaryResults[counter])) {
					hit |= ObjectArray[counter]->Intersect(this, &TemporaryResults[counter]);
				}
			}
		} else {
			while(counter--) {
				hit |= ObjectArray[counter]->Intersect(this, &TemporaryResults[counter]);
			}
		}
	}
	// test to see if anything actually hit a mesh
	if( ! hit ) {
//		OutputDebugString("!"); // no mesh intersections
		return FinalResult->Intersects = false;
	}

	if(! (Convex || ConvexTest)) {
		// now find the nearest of the actual hits
		float nearest_range = (float) (2<<28);
		counter = Object_Count;
		while(counter--) {
			if(TemporaryResults[counter].Intersects && (nearest_range > TemporaryResults[counter].Range)) {
				nearest_index = counter;
				nearest_range = TemporaryResults[counter].Range;
			}
		}
		Copy_Results(FinalResult, &TemporaryResults[nearest_index]);
	}
//	OutputDebugString("+");
//	Debug.Print("Mesh ", Object_Array[nearest_index]);
//	Intersection_Node = candidate_indices[nearest_index];
	return true;
}

