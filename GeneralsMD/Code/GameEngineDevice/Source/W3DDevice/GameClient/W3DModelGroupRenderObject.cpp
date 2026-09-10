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
 *                     $Archive:: /Commando/Code/ww3d2/Composite.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/27/01 12:45a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   W3DModelGroupRenderObject::W3DModelGroupRenderObject -- Constructor                           *
 *   W3DModelGroupRenderObject::W3DModelGroupRenderObject -- copy constructor                      *
 *   W3DModelGroupRenderObject::~W3DModelGroupRenderObject -- Destructor                           *
 *   W3DModelGroupRenderObject::operator -- assignment operator                                  *
 *   W3DModelGroupRenderObject::Restart -- Recursively call Restart on all sub-objects           *
 *   W3DModelGroupRenderObject::Get_Name -- returns the name of this render object               *
 *   W3DModelGroupRenderObject::Set_Name -- sets the name of this render object                  *
 *   W3DModelGroupRenderObject::Set_Base_Model_Name -- sets the "base-model-name"                *
 *   W3DModelGroupRenderObject::Get_Num_Polys -- returns the number of polys                     *
 *   W3DModelGroupRenderObject::Notify_Added -- notify all sub-objects that they were added      *
 *   W3DModelGroupRenderObject::Notify_Removed -- notifies all subobjs they were removed from th *
 *   W3DModelGroupRenderObject::Cast_Ray -- cast a ray against this object                       *
 *   W3DModelGroupRenderObject::Cast_AABox -- cast a swept AABox against this object             *
 *   W3DModelGroupRenderObject::Cast_OBBox -- cast a swept OBBox against this object             *
 *   W3DModelGroupRenderObject::Intersect_AABox -- intersect this object with an AABox           *
 *   W3DModelGroupRenderObject::Intersect_OBBox -- intersect this object with an OBBox           *
 *   W3DModelGroupRenderObject::Update_Obj_Space_Bounding_Volumes -- updates the object-space BV *
 *   W3DModelGroupRenderObject::Set_User_Data -- set the userdata pointer                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "W3DDevice/GameClient/W3DModelGroupRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWDebug/wwdebug.h"
#include <stdlib.h>


/***********************************************************************************************
 * W3DModelGroupRenderObject::W3DModelGroupRenderObject -- Constructor                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
W3DModelGroupRenderObject::W3DModelGroupRenderObject()
{
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::W3DModelGroupRenderObject -- copy constructor                        *
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
W3DModelGroupRenderObject::W3DModelGroupRenderObject(const W3DModelGroupRenderObject & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::~W3DModelGroupRenderObject -- Destructor                             *
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
W3DModelGroupRenderObject::~W3DModelGroupRenderObject()
{
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::operator -- assignment operator                                    *
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
W3DModelGroupRenderObject & W3DModelGroupRenderObject::operator = (const W3DModelGroupRenderObject & that)
{
	Set_Name(that.Get_Name());
	Set_Base_Model_Name(that.Get_Base_Model_Name());
	return *this;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Restart -- Recursively call Restart on all sub-objects             *
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
void W3DModelGroupRenderObject::Restart()
{
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		W3DRenderObject * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Restart();
		robj->Release_Ref();
	}
}

/***********************************************************************************************
 * W3DModelGroupRenderObject::Get_Name -- returns the name of this render object                 *
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
const char * W3DModelGroupRenderObject::Get_Name() const
{
	return Name;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Set_Name -- sets the name of this render object                    *
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
void W3DModelGroupRenderObject::Set_Name(const char * name)
{
	Name=name;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Set_Base_Model_Name -- sets the "base-model-name"                  *
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
void W3DModelGroupRenderObject::Set_Base_Model_Name(const char *name)
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


/***********************************************************************************************
 * W3DModelGroupRenderObject::Get_Num_Polys -- returns the number of polys                       *
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
int W3DModelGroupRenderObject::Get_Num_Polys() const
{
	int count = 0;
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		W3DRenderObject * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		count += robj->Get_Num_Polys();
		robj->Release_Ref();
	}
	return count;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Notify_Added -- notify all sub-objects that they were added        *
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
void W3DModelGroupRenderObject::Notify_Added(W3DScene * scene)
{
	W3DRenderObject::Notify_Added(scene);
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		W3DRenderObject * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Notify_Added(scene);
		robj->Release_Ref();
	}
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Notify_Removed -- notifies all subobjs they were removed from the  *
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
void W3DModelGroupRenderObject::Notify_Removed(W3DScene * scene)
{
	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		W3DRenderObject * robj = Get_Sub_Object(ni);
		WWASSERT(robj);
		robj->Notify_Removed(scene);
		robj->Release_Ref();
	}
	W3DRenderObject::Notify_Removed(scene);
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Cast_Ray -- cast a ray against this object                         *
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
bool W3DModelGroupRenderObject::Cast_Ray(W3DRayCastQuery & raytest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		W3DRenderObject * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_Ray(raytest);
		robj->Release_Ref();
	}
	return res;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Cast_AABox -- cast a swept AABox against this object               *
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
bool W3DModelGroupRenderObject::Cast_AABox(W3DBoxCastQuery & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		W3DRenderObject * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_AABox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Cast_OBBox -- cast a swept OBBox against this object               *
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
bool W3DModelGroupRenderObject::Cast_OBBox(W3DOrientedBoxCastQuery & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		W3DRenderObject * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Cast_OBBox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Intersect_AABox -- intersect this object with an AABox             *
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
bool W3DModelGroupRenderObject::Intersect_AABox(W3DBoxIntersectionQuery & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		W3DRenderObject * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Intersect_AABox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Intersect_OBBox -- intersect this object with an OBBox             *
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
bool W3DModelGroupRenderObject::Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest)
{
	bool res = false;
	for (int i=0; i<Get_Num_Sub_Objects(); i++) {
		W3DRenderObject * robj = Get_Sub_Object(i);
		WWASSERT(robj);
		res |= robj->Intersect_OBBox(boxtest);
		robj->Release_Ref();
	}
	return res;
}


/***********************************************************************************************
 * W3DModelGroupRenderObject::Update_Obj_Space_Bounding_Volumes -- updates the object-space BVs  *
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
void W3DModelGroupRenderObject::Update_Obj_Space_Bounding_Volumes()
{
	int i;
	W3DRenderObject * robj = nullptr;

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
   W3DRenderObject *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}

/***********************************************************************************************
 * W3DModelGroupRenderObject::Set_User_Data -- set the userdata                                  *
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
void W3DModelGroupRenderObject::Set_User_Data(void *value, bool recursive)
{
	W3DRenderObject::Set_User_Data(value);
	if (recursive) {
		for (int i=0; i<Get_Num_Sub_Objects(); i++) {
			W3DRenderObject * robj = Get_Sub_Object(i);
			WWASSERT(robj);
			robj->Set_User_Data(value,recursive);
			robj->Release_Ref();
		}
	}
}

const char * W3DModelGroupRenderObject::Get_Base_Model_Name () const
{
	if (BaseModelName.Is_Empty()) {
		return nullptr;
	}

	return BaseModelName;
}

