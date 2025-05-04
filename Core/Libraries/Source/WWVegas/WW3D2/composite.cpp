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
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CompositeRenderObjClass::CompositeRenderObjClass -- Constructor                           *
 *   CompositeRenderObjClass::CompositeRenderObjClass -- copy constructor                      *
 *   CompositeRenderObjClass::~CompositeRenderObjClass -- Destructor                           *
 *   CompositeRenderObjClass::operator -- assignment operator                                  *
 *   CompositeRenderObjClass::Restart -- Recursively call Restart on all sub-objects           *
 *   CompositeRenderObjClass::Get_Name -- returns the name of this render object               *
 *   CompositeRenderObjClass::Set_Name -- sets the name of this render object                  *
 *   CompositeRenderObjClass::Set_Base_Model_Name -- sets the "base-model-name"                *
 *   CompositeRenderObjClass::Get_Num_Polys -- returns the number of polys                     *
 *   CompositeRenderObjClass::Notify_Added -- notify all sub-objects that they were added      *
 *   CompositeRenderObjClass::Notify_Removed -- notifies all subobjs they were removed from th *
 *   CompositeRenderObjClass::Cast_Ray -- cast a ray against this object                       *
 *   CompositeRenderObjClass::Cast_AABox -- cast a swept AABox against this object             *
 *   CompositeRenderObjClass::Cast_OBBox -- cast a swept OBBox against this object             *
 *   CompositeRenderObjClass::Intersect_AABox -- intersect this object with an AABox           *
 *   CompositeRenderObjClass::Intersect_OBBox -- intersect this object with an OBBox           *
 *   CompositeRenderObjClass::Create_Decal -- create a decal on this object                    *
 *   CompositeRenderObjClass::Delete_Decal -- remove a logical decal from this object          *
 *   CompositeRenderObjClass::Update_Obj_Space_Bounding_Volumes -- updates the object-space BV *
 *   CompositeRenderObjClass::Set_User_Data -- set the userdata pointer                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "composite.h"
#include "wwdebug.h"
#include <stdlib.h>
#include <string.h>


/***********************************************************************************************
 * CompositeRenderObjClass::CompositeRenderObjClass -- Constructor                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
CompositeRenderObjClass::CompositeRenderObjClass(void)
{
}


/***********************************************************************************************
 * CompositeRenderObjClass::CompositeRenderObjClass -- copy constructor                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
CompositeRenderObjClass::CompositeRenderObjClass(const CompositeRenderObjClass & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
}


/***********************************************************************************************
 * CompositeRenderObjClass::~CompositeRenderObjClass -- Destructor                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
CompositeRenderObjClass::~CompositeRenderObjClass(void)
{
}


/***********************************************************************************************
 * CompositeRenderObjClass::operator -- assignment operator                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
CompositeRenderObjClass & CompositeRenderObjClass::operator = (const CompositeRenderObjClass & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
	return *this;
}


/***********************************************************************************************
 * CompositeRenderObjClass::Restart -- Recursively call Restart on all sub-objects             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/30/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Restart(void)
{
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Restart();
		robj->Release_Ref();
	}
}

/***********************************************************************************************
 * CompositeRenderObjClass::Get_Name -- returns the name of this render object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
const char * CompositeRenderObjClass::Get_Name(void) const															
{ 
	return Name; 
}


/***********************************************************************************************
 * CompositeRenderObjClass::Set_Name -- sets the name of this render object                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Set_Name(const char * name)												
{ 
	Name=name;
}


/***********************************************************************************************
 * CompositeRenderObjClass::Set_Base_Model_Name -- sets the "base-model-name"                  *
 *                                                                                             *
 *    The base-model-name was needed by the aggregate code.  Ask Patrick Smith about it :-)    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Set_Base_Model_Name(const char *name)
{
	// NULL is a legal value for BaseModelName. Unfortunately,
	// StringClass::operator= does not modify the string when
	// assigning NULL, so we explicitly handle that case here.
	if (name != 0) {
		BaseModelName = name;
	} else {
		BaseModelName = "";
	}
}


/***********************************************************************************************
 * CompositeRenderObjClass::Get_Num_Polys -- returns the number of polys                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
int CompositeRenderObjClass::Get_Num_Polys(void) const
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


/***********************************************************************************************
 * CompositeRenderObjClass::Notify_Added -- notify all sub-objects that they were added        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Notify_Removed -- notifies all subobjs they were removed from the  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Cast_Ray -- cast a ray against this object                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Cast_AABox -- cast a swept AABox against this object               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Cast_OBBox -- cast a swept OBBox against this object               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Intersect_AABox -- intersect this object with an AABox             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Intersect_OBBox -- intersect this object with an OBBox             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * CompositeRenderObjClass::Create_Decal -- create a decal on this object                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Create_Decal(DecalGeneratorClass * generator)
{
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		robj->Create_Decal(generator);
		robj->Release_Ref();
	}
}


/***********************************************************************************************
 * CompositeRenderObjClass::Delete_Decal -- remove a logical decal from this object            *
 *                                                                                             *
 * This internally removes all decals with the given ID.  The ID comes from the generator      *
 * which was used to create the decals.                                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Delete_Decal(uint32 decal_id)
{
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		robj->Delete_Decal(decal_id);
		robj->Release_Ref();
	}
}


/***********************************************************************************************
 * CompositeRenderObjClass::Update_Obj_Space_Bounding_Volumes -- updates the object-space BVs  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void CompositeRenderObjClass::Update_Obj_Space_Bounding_Volumes(void)
{
	int i;
	RenderObjClass * robj = NULL;

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

/***********************************************************************************************
 * CompositeRenderObjClass::Set_User_Data -- set the userdata                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
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

const char * CompositeRenderObjClass::Get_Base_Model_Name (void) const
{
	if (BaseModelName.Is_Empty()) {
		return NULL;
	}
	
	return BaseModelName;
}