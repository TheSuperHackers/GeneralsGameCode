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
 *                     $Archive:: /Commando/Code/ww3d2/composite.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/27/01 12:45a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "composite.h"
#include "WWDebug/wwdebug.h"
#include <stdlib.h>


CompositeRenderObjClass::CompositeRenderObjClass()
{
}


CompositeRenderObjClass::CompositeRenderObjClass(const CompositeRenderObjClass & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
}


CompositeRenderObjClass::~CompositeRenderObjClass()
{
}


CompositeRenderObjClass & CompositeRenderObjClass::operator = (const CompositeRenderObjClass & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
	return *this;
}


void CompositeRenderObjClass::Restart()
{
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Restart();
		robj->Release_Ref();
	}
}

const char * CompositeRenderObjClass::Get_Name() const
{
	return Name;
}


void CompositeRenderObjClass::Set_Name(const char * name)
{
	Name=name;
}


// The base-model-name was needed by the aggregate code. Ask Patrick Smith about it :-).
void CompositeRenderObjClass::Set_Base_Model_Name(const char *name)
{
	// null is a legal value for BaseModelName. Unfortunately,
	// StringClass::operator= does not modify the string when
	// assigning null, so we explicitly handle that case here.
	if (name != nullptr) {
		BaseModelName = name;
	} else {
		BaseModelName = "";
	}
}


int CompositeRenderObjClass::Get_Num_Polys() const
{
	int count = 0;
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		count += robj->Get_Num_Polys();
		robj->Release_Ref();
	}
	return count;
}


void CompositeRenderObjClass::Notify_Added(SceneClass * scene)
{
	RenderObjClass::Notify_Added(scene);
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Notify_Added(scene);
		robj->Release_Ref();
	}
}


void CompositeRenderObjClass::Notify_Removed(SceneClass * scene)
{
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Notify_Removed(scene);
		robj->Release_Ref();
	}
	RenderObjClass::Notify_Removed(scene);
}


bool CompositeRenderObjClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_Ray(raytest);
		robj->Release_Ref();
	}
	return res;
}


bool CompositeRenderObjClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_AABox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


bool CompositeRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_OBBox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


bool CompositeRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Intersect_AABox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


bool CompositeRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Intersect_OBBox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


void CompositeRenderObjClass::Create_Decal(DecalGeneratorClass * generator)
{
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		robj->Create_Decal(generator);
		robj->Release_Ref();
	}
}


// Removes all decals with the given ID. The ID comes from the generator which was used to create
// the decals.
void CompositeRenderObjClass::Delete_Decal(uint32 decal_id)
{
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		robj->Delete_Decal(decal_id);
		robj->Release_Ref();
	}
}


void CompositeRenderObjClass::Update_Obj_Space_Bounding_Volumes()
{
	int i;
	RenderObjClass * robj = nullptr;

	// if we don't have any sub objects, just set default bounds
	if (Get_Num_Sub_Objects() <= 0) {
		ObjSphere.Init(Vector3(0,0,0),0);
		ObjBox.Center.Set(0,0,0);
		ObjBox.Extent.Set(0,0,0);
		return;
	}


	AABoxClass obj_aabox;
	MinMaxAABoxClass box;
	SphereClass sphere;

	// loop through all sub-objects, combining their object-space bounding spheres and boxes.
	robj = Get_Sub_Object(0);
	WWASSERT(robj);
	robj->Get_Obj_Space_Bounding_Sphere(ObjSphere);
	robj->Get_Obj_Space_Bounding_Box(obj_aabox);
	robj->Release_Ref();
	box.Init(obj_aabox);

	for (i=1; i<Get_Num_Sub_Objects(); i++) {

		robj = Get_Sub_Object(i);
		WWASSERT(robj);

		robj->Get_Obj_Space_Bounding_Sphere(sphere);
		robj->Get_Obj_Space_Bounding_Box(obj_aabox);

		ObjSphere.Add_Sphere(sphere);
		box.Add_Box(obj_aabox);

		robj->Release_Ref();
	}

	ObjBox.Init(box);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}

void CompositeRenderObjClass::Set_User_Data(void *value, bool recursive)
{
	RenderObjClass::Set_User_Data(value);
	if (recursive) {
		for (int i=0; i<Get_Num_Sub_Objects(); i++) {
			RenderObjClass * robj = Get_Sub_Object(i);
			WWASSERT(robj);
			robj->Set_User_Data(value,recursive);
			robj->Release_Ref();
		}
	}
}

const char * CompositeRenderObjClass::Get_Base_Model_Name () const
{
	if (BaseModelName.Is_Empty()) {
		return nullptr;
	}

	return BaseModelName;
}
