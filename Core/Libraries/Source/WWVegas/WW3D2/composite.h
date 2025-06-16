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
 *                     $Archive:: /Commando/Code/ww3d2/composite.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/25/01 12:25p                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "rendobj.h"
#include "wwstring.h"

/*
** CompositeRenderObjClass
** The sole purpose of this class is to encapsulate some of the chores that all
** "composite" (contain sub objects) render objects have to do.  Typically all
** of the functions are implemented through the existing sub-object interface
** so there is still no assumption on how you store/organize your sub-objects.
*/
class CompositeRenderObjClass : public RenderObjClass
{
public:

	CompositeRenderObjClass(void);
	CompositeRenderObjClass(const CompositeRenderObjClass & that);
	virtual ~CompositeRenderObjClass(void);
	CompositeRenderObjClass & operator = (const CompositeRenderObjClass & that);

	virtual void					Restart(void);

	virtual const char *			Get_Name(void) const;
	virtual void					Set_Name(const char * name);
	virtual const char *			Get_Base_Model_Name (void) const;
	virtual void					Set_Base_Model_Name (const char *name);
	virtual int						Get_Num_Polys(void) const;
	virtual void					Notify_Added(SceneClass * scene);
	virtual void					Notify_Removed(SceneClass * scene);

	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Create_Decal(DecalGeneratorClass * generator);
	virtual void					Delete_Decal(uint32 decal_id);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass	& sphere) const { sphere = ObjSphere; }
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const { box = ObjBox; }
	virtual void					Update_Obj_Space_Bounding_Volumes(void);

	virtual void					Set_User_Data(void *value, bool recursive = false);

protected:

	StringClass						Name;						// name of the render object
	StringClass						BaseModelName;			// name of the original render obj (before aggregation)
	SphereClass						ObjSphere;				// object-space bounding sphere
	AABoxClass						ObjBox;					// object-space bounding box
};



#endif