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

#pragma once

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

	CompositeRenderObjClass();
	CompositeRenderObjClass(const CompositeRenderObjClass & that);
	virtual ~CompositeRenderObjClass() override;
	CompositeRenderObjClass & operator = (const CompositeRenderObjClass & that);

	virtual void					Restart() override;

	virtual const char *			Get_Name() const override;
	virtual void					Set_Name(const char * name) override;
	virtual const char *			Get_Base_Model_Name () const override;
	virtual void					Set_Base_Model_Name (const char *name) override;
	virtual int						Get_Num_Polys() const override;
	virtual void					Notify_Added(SceneClass * scene) override;
	virtual void					Notify_Removed(SceneClass * scene) override;

	virtual bool					Cast_Ray(RayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest) override;
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest) override;
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest) override;

	virtual void					Create_Decal(DecalGeneratorClass * generator) override;
	virtual void					Delete_Decal(uint32 decal_id) override;

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass	& sphere) const override { sphere = ObjSphere; }
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const override { box = ObjBox; }
	virtual void					Update_Obj_Space_Bounding_Volumes() override;

	virtual void					Set_User_Data(void *value, bool recursive = false) override;

protected:

	StringClass						Name;						// name of the render object
	StringClass						BaseModelName;			// name of the original render obj (before aggregation)
	SphereClass						ObjSphere;				// object-space bounding sphere
	AABoxClass						ObjBox;					// object-space bounding box
};
