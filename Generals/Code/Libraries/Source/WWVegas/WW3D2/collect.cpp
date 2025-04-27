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
 *                     $Archive:: /Commando/Code/ww3d2/collect.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CollectionClass::CollectionClass -- default constructor for collection render object      *
 *   CollectionClass::CollectionClass -- constructor for collection render object              *
 *   CollectionClass::CollectionClass -- copy constructor                                      *
 *   CollectionClass::CollectionClass -- assignment operator                                   *
 *   CollectionClass::~CollectionClass -- destructor                                           *
 *   CollectionClass::Clone -- virtual copy constructor                                        *
 *   CollectionClass::Free -- releases all assets in use by this collection                    *
 *   CollectionClass::Class_ID -- returns class id for collection render objects               *
 *   CollectionClass::Get_Num_Polys -- returns the number of polygons in this collection       *
 *   CollectionClass::Render -- render this collection                                         *
 *   CollectionClass::Special_Render -- passes the special render call to all sub-objects      *
 *   CollectionClass::Set_Transform -- set the transform for this collection                   *
 *   CollectionClass::Set_Position -- set the position for this collection                     *
 *   CollectionClass::Get_Num_Sub_Objects -- returns the number of sub objects                 *
 *   CollectionClass::Get_Sub_Object -- returns a pointer to the desired sub object            *
 *   CollectionClass::Add_Sub_Object -- adds another object into this collection               *
 *   CollectionClass::Remove_Sub_Object -- removes a sub object from this collection           *
 *   CollectionClass::Cast_Ray -- passes the ray test to each sub object                       *
 *   CollectionClass::Cast_AABox -- passes the axis-aligned box test to each sub object        *
 *   CollectionClass::Cast_OBBox -- passes the oriented box test to each sub object            *
 *   CollectionClass::Intersect_AABox -- test for intersection with an AABox                   *
 *   CollectionClass::Intersect_OBBox -- test for intersection with an OBBox                   *
 *   CollectionClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding spher *
 *   CollectionClass::Get_Obj_Space_Bounding_Box -- returns the object-space bounding box      *
 *   CollectionClass::Snap_Point_Count -- returns the number of snap points in this collecion  *
 *   CollectionClass::Get_Snap_Point -- return the desired snap point                          *
 *   CollectionClass::Scale -- scale the objects in this collection                            *
 *   CollectionClass::Scale -- scale the objects in this collection                            *
 *   CollectionClass::Update_Obj_Space_Bounding_Volumes -- recomputes the object space boundin *
 *   CollectionClass::Update_Sub_Object_Transforms -- recomputes all sub object transforms     *
 *   CollectionLoaderClass::Load -- reads a collection from a w3d file                         *
 *   CollectionDefClass::CollectionDefClass -- constructor                                     *
 *   CollectionDefClass::~CollectionDefClass -- destructor for collection definition           *
 *   CollectionDefClass::Free -- releases assets in use by a collection definition             *
 *   CollectionDefClass::Get_Name -- returns name of the collection                            *
 *   CollectionDefClass::Load -- loads a collection definition from a w3d file                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "collect.h"
#include "chunkio.h"
#include "camera.h"
#include "wwdebug.h"
#include "snapPts.h"
#include "assetmgr.h"
#include "ww3d.h"
#include "w3derr.h"
//#include "sr.hpp"


CollectionLoaderClass _CollectionLoader;

/*
** CollectionDefClass.  This is the "blueprint" for a collection object
** The asset manager will store these until someone actually asks it to
** create an instance of the collection object
*/
class CollectionDefClass
{
public:

	CollectionDefClass(void);
	~CollectionDefClass(void);

	const char *	Get_Name(void) const;
	WW3DErrorType	Load(ChunkLoadClass & cload);

protected:

	void				Free(void);

	char								Name[W3D_NAME_LEN];
	DynamicVectorClass<char *> ObjectNames;
	SnapPointsClass *				SnapPoints;

	DynamicVectorClass <ProxyClass>	ProxyList;

	friend class CollectionClass;
};


/*
** CollectionPrototypeClass this is the render object prototype for 
** Collections.
*/
class CollectionPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(CollectionPrototypeClass)
public:
	CollectionPrototypeClass(CollectionDefClass * def)		{ ColDef = def; WWASSERT(ColDef); }

	virtual const char *			Get_Name(void) const			{ return ColDef->Get_Name(); }	
	virtual int								Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_COLLECTION; }
	virtual RenderObjClass *	Create(void)							{ return NEW_REF( CollectionClass, (*ColDef)); }	
	virtual void							DeleteSelf()							{ delete this; }

	CollectionDefClass *			ColDef;

protected:
	virtual ~CollectionPrototypeClass(void)					{ delete ColDef; }						 
};


/***********************************************************************************************
 * CollectionClass::CollectionClass -- default constructor for collection render object        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   23/8/00    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionClass::CollectionClass(void) :
	SnapPoints(NULL)
{
	Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * CollectionClass::CollectionClass -- constructor for collection render object                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionClass::CollectionClass(const CollectionDefClass & def) :
	SubObjects(def.ObjectNames.Count()),	
	SnapPoints(NULL)
{
	// Set our name
	Set_Name (def.Get_Name ());

	// create the sub objects
	SubObjects.Resize(def.ObjectNames.Count());
	for (int i=0; i<def.ObjectNames.Count(); i++) {
		WWASSERT(SubObjects.Count() == i);
		SubObjects.Add(WW3DAssetManager::Get_Instance()->Create_Render_Obj(def.ObjectNames[i]));
		SubObjects[i]->Set_Container(this);
	}

	// Copy the list of placeholder objects from the definition
	ProxyList = def.ProxyList;
		
	// grab ahold of the snap points.	
	SnapPoints = def.SnapPoints;
	if (SnapPoints) SnapPoints->Add_Ref();

	// set up our collision typeas the union of all of our sub-objects
	Update_Sub_Object_Bits();

	// update the object bounding volumes
	Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * CollectionClass::CollectionClass -- copy constructor                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionClass::CollectionClass(const CollectionClass & src) :
	CompositeRenderObjClass(src),
	SubObjects(src.SubObjects.Count()),
	SnapPoints(NULL)
{
	*this = src;
}


/***********************************************************************************************
 * CollectionClass::CollectionClass -- assignment operator                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionClass & CollectionClass::operator = (const CollectionClass & that)
{
	if (this != &that) {
		Free();
		CompositeRenderObjClass::operator = (that);

		SubObjects.Resize(that.SubObjects.Count());
		for (int i=0; i<that.SubObjects.Count(); i++) {
			WWASSERT(SubObjects.Count() == i);
			SubObjects.Add(that.SubObjects[i]->Clone());
			SubObjects[i]->Set_Container(this);
		}

		// Copy the list of placeholder objects from the definition
		ProxyList = that.ProxyList;
		
		SnapPoints = that.SnapPoints;
		if (SnapPoints) SnapPoints->Add_Ref();

		Update_Sub_Object_Bits();
		Update_Obj_Space_Bounding_Volumes();
	}
	return * this;
}


/***********************************************************************************************
 * CollectionClass::~CollectionClass -- destructor                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionClass::~CollectionClass(void)
{
	Free();
}


/***********************************************************************************************
 * CollectionClass::Clone -- virtual copy constructor                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * CollectionClass::Clone(void) const
{
	return NEW_REF( CollectionClass, (*this));	
}


/***********************************************************************************************
 * CollectionClass::Free -- releases all assets in use by this collection                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Free(void)
{
	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Set_Container(NULL);
		SubObjects[i]->Release_Ref();
		SubObjects[i] = NULL;
	}
	SubObjects.Delete_All();
	ProxyList.Delete_All ();

	REF_PTR_RELEASE(SnapPoints);	
}


/***********************************************************************************************
 * CollectionClass::Class_ID -- returns class id for collection render objects                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Class_ID(void)	const
{
	return RenderObjClass::CLASSID_COLLECTION;
}


/***********************************************************************************************
 * CollectionClass::Get_Num_Polys -- returns the number of polygons in this collection         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Get_Num_Polys(void) const
{
	int pcount = 0;
	for (int i=0; i<SubObjects.Count(); i++) {
		pcount += SubObjects[i]->Get_Num_Polys();
	}
	return pcount;
}


/***********************************************************************************************
 * CollectionClass::Render -- render this collection                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Render(RenderInfoClass & rinfo)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	if (Are_Sub_Object_Transforms_Dirty()) {
		Update_Sub_Object_Transforms();
	}

	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Render(rinfo);
	}
}


/***********************************************************************************************
 * CollectionClass::Special_Render -- passes the special render call to all sub-objects        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	if (Are_Sub_Object_Transforms_Dirty()) {
		Update_Sub_Object_Transforms();
	}

	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Special_Render(rinfo);
	}
}

/***********************************************************************************************
 * CollectionClass::Set_Transform -- set the transform for this collection                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * CollectionClass::Set_Position -- set the position for this collection                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Set_Position(const Vector3 &v)
{
	RenderObjClass::Set_Position(v);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * CollectionClass::Get_Num_Sub_Objects -- returns the number of sub objects                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Get_Num_Sub_Objects(void) const
{
	return SubObjects.Count();
}


/***********************************************************************************************
 * CollectionClass::Get_Sub_Object -- returns a pointer to the desired sub object              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * CollectionClass::Get_Sub_Object(int index) const
{
	if (SubObjects[index]) {
		SubObjects[index]->Add_Ref();
	}
	return SubObjects[index];
}


/***********************************************************************************************
 * CollectionClass::Add_Sub_Object -- adds another object into this collection                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Add_Sub_Object(RenderObjClass * subobj)
{
	WWASSERT(subobj);
	subobj->Add_Ref();
	subobj->Set_Container(this);
	subobj->Set_Transform(Transform);
	int res = SubObjects.Add(subobj);
	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
	if (Is_In_Scene()) {
		subobj->Notify_Added(Scene);
	}
	return res;
}


/***********************************************************************************************
 * CollectionClass::Remove_Sub_Object -- removes a sub object from this collection             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Remove_Sub_Object(RenderObjClass * robj)
{
	if (robj == NULL) return 0;
	
	int res = 0;

	Matrix3D tm = Get_Transform();

	for (int i=0; i<SubObjects.Count(); i++) {
		if (robj == SubObjects[i]) {
			
			if (Is_In_Scene()) {
				SubObjects[i]->Notify_Removed(Scene);
			}
			SubObjects[i]->Set_Container(NULL);
			SubObjects[i]->Set_Transform(tm);
			SubObjects[i]->Release_Ref();
			res = SubObjects.Delete(i);
			break;
		}
	}

	if (res != 0) {
		Update_Sub_Object_Bits();
		Update_Obj_Space_Bounding_Volumes();
	}

	return res;
}


/***********************************************************************************************
 * CollectionClass::Cast_Ray -- passes the ray test to each sub object                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	bool res = false;
	for (int i=0; i<SubObjects.Count(); i++) {
		res |= SubObjects[i]->Cast_Ray(raytest);
	}
	return res;
}


/***********************************************************************************************
 * CollectionClass::Cast_AABox -- passes the axis-aligned box test to each sub object          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<SubObjects.Count(); i++) {
		res |= SubObjects[i]->Cast_AABox(boxtest);
	}
	return res;
}


/***********************************************************************************************
 * CollectionClass::Cast_OBBox -- passes the oriented box test to each sub object              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<SubObjects.Count(); i++) {
		res |= SubObjects[i]->Cast_OBBox(boxtest);
	}
	return res;
}


/***********************************************************************************************
 * CollectionClass::Intersect_AABox -- test for intersection with an AABox                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<SubObjects.Count(); i++) {
		res |= SubObjects[i]->Intersect_AABox(boxtest);
	}
	return res;
}


/***********************************************************************************************
 * CollectionClass::Intersect_OBBox -- test for intersection with an OBBox                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	bool res = false;
	for (int i=0; i<SubObjects.Count(); i++) {
		res |= SubObjects[i]->Intersect_OBBox(boxtest);
	}
	return res;
}

/***********************************************************************************************
 * CollectionClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding sphere. *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere = BoundSphere;
}


/***********************************************************************************************
 * CollectionClass::Get_Obj_Space_Bounding_Box -- returns the object-space bounding box        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box = BoundBox;
}


/***********************************************************************************************
 * CollectionClass::Snap_Point_Count -- returns the number of snap points in this collecion    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Snap_Point_Count(void)
{
	if (SnapPoints) {
		return SnapPoints->Count();
	} else {
		return 0;
	}
}


/***********************************************************************************************
 * CollectionClass::Get_Snap_Point -- return the desired snap point                            *
 *                                                                                             *
 * This function will set the passed vector to be equal to the object space coordinates of     *
 * the desired snap point.                                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Get_Snap_Point(int index,Vector3 * set)
{
	WWASSERT(set != NULL);
	if (SnapPoints) {
		*set = (*SnapPoints)[index];
	} else {
		set->X = set->Y = set->Z = 0;
	}
}


/***********************************************************************************************
 * CollectionClass::Scale -- scale the objects in this collection                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Scale(float scale)
{
	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Scale(scale);
	}
}


/***********************************************************************************************
 * CollectionClass::Scale -- scale the objects in this collection                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Scale(float scalex, float scaley, float scalez)
{
	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Scale(scalex,scaley,scalez);
	}
}


/***********************************************************************************************
 * CollectionClass::Update_Obj_Space_Bounding_Volumes -- recomputes the object space bounding  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Update_Obj_Space_Bounding_Volumes(void)
{
	int i;
	if (SubObjects.Count() <= 0) {
		BoundSphere = SphereClass(Vector3(0,0,0),0);
		BoundBox.Center.Set(0,0,0);
		BoundBox.Extent.Set(0,0,0);
		return;
	}
	
	Matrix3D tm = Get_Transform();
	Set_Transform(Matrix3D(1));

	// loop through all sub-objects, combining their bounding spheres.
	BoundSphere = SubObjects[0]->Get_Bounding_Sphere();
	for (i=1; i < SubObjects.Count(); i++) {
		BoundSphere.Add_Sphere(SubObjects[i]->Get_Bounding_Sphere());				
	}

	// loop through the sub-objects, computing a box in the root coordinate 
	// system which bounds all of the meshes.  Note that we've set the
	// root coordinate system to identity for this.
	MinMaxAABoxClass box(Vector3(FLT_MAX,FLT_MAX,FLT_MAX),Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX));
	
	for (i=0; i < SubObjects.Count(); i++) {
		box.Add_Box(SubObjects[i]->Get_Bounding_Box());
	}

	BoundBox.Init(box);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();

	Set_Transform(tm);
}


/***********************************************************************************************
 * CollectionClass::Update_Sub_Object_Transforms -- recomputes all sub object transforms       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionClass::Update_Sub_Object_Transforms(void)
{
	RenderObjClass::Update_Sub_Object_Transforms();
	for (int i=0; i<SubObjects.Count(); i++) {
		SubObjects[i]->Set_Transform(Transform);
		SubObjects[i]->Update_Sub_Object_Transforms();
	}
	Set_Sub_Object_Transforms_Dirty(false);
}


/***********************************************************************************************
 * CollectionClass::Get_Placeholder -- Returns information about a placeholder object.
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/28/99    PDS : Created.                                                                 *
 *=============================================================================================*/
bool CollectionClass::Get_Proxy (int index, ProxyClass &proxy) const
{
	bool retval = false;

	if (index >= 0 && index < ProxyList.Count ()) {
		
		//
		// Return the proxy information to the caller
		//
		proxy		= ProxyList[index];
		retval	= true;
	}
	
	return retval;
}


/***********************************************************************************************
 * CollectionClass::Get_Proxy_Count -- Returns the count of proxy objects in the collection.
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/28/99    PDS : Created.                                                                 *
 *=============================================================================================*/
int CollectionClass::Get_Proxy_Count (void) const
{
	return ProxyList.Count ();
}


/***********************************************************************************************
 * CollectionDefClass::CollectionDefClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionDefClass::CollectionDefClass(void)
{
	SnapPoints = NULL;
}


/***********************************************************************************************
 * CollectionDefClass::~CollectionDefClass -- destructor for collection definition             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CollectionDefClass::~CollectionDefClass(void)
{
	Free();
}


/***********************************************************************************************
 * CollectionDefClass::Free -- releases assets in use by a collection definition               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CollectionDefClass::Free(void)
{
	for (int i=0; i<ObjectNames.Count(); i++) {
		delete[] ObjectNames[i];
	}

	ProxyList.Delete_All ();
}


/***********************************************************************************************
 * CollectionDefClass::Get_Name -- returns name of the collection                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
const char * CollectionDefClass::Get_Name(void) const
{
	return Name;
}


/***********************************************************************************************
 * CollectionDefClass::Load -- loads a collection definition from a w3d file                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType CollectionDefClass::Load(ChunkLoadClass & cload)
{
	Free();
	
	// open the header chunk and read it in
	W3dCollectionHeaderStruct header;
	if (!cload.Open_Chunk()) goto Error;
	if (cload.Cur_Chunk_ID() != W3D_CHUNK_COLLECTION_HEADER) goto Error;
	if (cload.Read(&header,sizeof(header)) != sizeof(header)) goto Error;
	if (!cload.Close_Chunk()) goto Error;

	strncpy(Name,header.Name,W3D_NAME_LEN);
	ObjectNames.Resize(header.RenderObjectCount);
	
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) 
		{
		case W3D_CHUNK_COLLECTION_OBJ_NAME:
			{
				WWASSERT(cload.Cur_Chunk_Length() > 0);
				char * name = W3DNEWARRAY char [cload.Cur_Chunk_Length()];
				cload.Read(name,cload.Cur_Chunk_Length());
				ObjectNames.Add(name);
				break;
			}

		case W3D_CHUNK_PLACEHOLDER:
			{
				// Read the placeholder information from the chunk
				WWASSERT(cload.Cur_Chunk_Length() > 0);
				W3dPlaceholderStruct info;
				cload.Read(&info, sizeof (info));

				// Read the placeholder name from the chunk
				char *name = W3DNEWARRAY char[info.name_len + 1];
				cload.Read(name, info.name_len);
				name[info.name_len] = 0;

				// Create a matrix from the data in the chunk
				Matrix3D transform (info.transform[0][0], info.transform[1][0], info.transform[2][0], info.transform[3][0],
										  info.transform[0][1], info.transform[1][1], info.transform[2][1], info.transform[3][1],
										  info.transform[0][2], info.transform[1][2], info.transform[2][2], info.transform[3][2]);

				// Add this placeholder to our list
				ProxyList.Add (ProxyClass (name, transform));

				// Free the name array
				delete [] name;
				break;
			}

		case W3D_CHUNK_POINTS:
			SnapPoints = NEW_REF(SnapPointsClass, ());
			SnapPoints->Load_W3D(cload);
			break;
		}

		cload.Close_Chunk();
	}
	
	return WW3D_ERROR_OK;

Error:
	
	return WW3D_ERROR_LOAD_FAILED;
}

/***********************************************************************************************
 * CollectionLoaderClass::Load -- reads a collection from a w3d file                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
PrototypeClass * CollectionLoaderClass::Load_W3D(ChunkLoadClass & cload)
{
	CollectionDefClass * def = W3DNEW CollectionDefClass;

	if (def == NULL) {
		return NULL;
	}

	if (def->Load(cload) != WW3D_ERROR_OK) {

		// load failed, delete the model and return an error
		delete def;
		return NULL;

	} else {
	
		// ok, accept this model! 
		CollectionPrototypeClass * proto = W3DNEW CollectionPrototypeClass(def);
		return proto;
	
	}
}



