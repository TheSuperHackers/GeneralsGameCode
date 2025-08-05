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
 *                     $Archive:: /Commando/Code/ww3d2/boxrobj.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/11/01 2:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BOXROBJ_H
#define BOXROBJ_H

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "shader.h"
#include "proto.h"
#include "obbox.h"

class VertexMaterialClass;


/**
** BoxRenderObjClass: base class for AABox and OBBox collision boxes
**
** NOTE: these render objects were designed from the start to be used for
** collision boxes.  They are designed to normally never render unless you
** set the display mask and then, all boxes of that type will render.
** The display mask is 'AND'ed with the collision bits in the base render
** object class to determine if the box should be rendered.  WW3D provides
** an interface for setting this mask in your app.
**
** NOTE2: AABoxRenderObjClass is an axis-aligned box which will be positioned
** at a world-space offset (its local center) from the origin of the transform.
** This is done because AABoxes are used for rotationally invariant collision
** detection so we don't want the boxes to move around as the object rotates.
** For this reason, any AABoxes you use in a hierarchical model should be attached
** to the root and be constructed symmetrically...
**
** NOTE3: OBBoxRenderObjClass is an oriented box which is aligned with its transform
** but can have a center point that is offest from the transform's origin.
**
*/
class BoxRenderObjClass : public RenderObjClass
{

public:

	BoxRenderObjClass(void);
	BoxRenderObjClass(const W3dBoxStruct & def);
	BoxRenderObjClass(const BoxRenderObjClass & src);
	BoxRenderObjClass & operator = (const BoxRenderObjClass &);

	virtual int							Get_Num_Polys(void) const;
	virtual const char *				Get_Name(void) const;
	virtual void						Set_Name(const char * name);
	void									Set_Color(const Vector3 & color);
	void									Set_Opacity(float opacity) { Opacity = opacity; }

	static void							Init(void);
	static void							Shutdown(void);

	static void							Set_Box_Display_Mask(int mask);
	static int							Get_Box_Display_Mask(void);

	void									Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent);
	void									Set_Local_Min_Max(const Vector3 & min,const Vector3 & max);

	const Vector3 &					Get_Local_Center(void) { return ObjSpaceCenter; }
	const Vector3 &					Get_Local_Extent(void) { return ObjSpaceExtent; }

protected:

	virtual void						update_cached_box(void) = 0;
	void									render_box(RenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent);
	void									vis_render_box(SpecialRenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent);

	char									Name[2*W3D_NAME_LEN];
	Vector3								Color;
	Vector3								ObjSpaceCenter;
	Vector3								ObjSpaceExtent;
	float									Opacity;

	static bool							IsInitted;
	static int							DisplayMask;
};

inline void BoxRenderObjClass::Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent)
{
	ObjSpaceCenter = center;
	ObjSpaceExtent = extent;
	update_cached_box();
}

inline void BoxRenderObjClass::Set_Local_Min_Max(const Vector3 & min,const Vector3 & max)
{
	ObjSpaceCenter = (max + min) / 2.0f;
	ObjSpaceExtent = (max - min) / 2.0f;
	update_cached_box();
}


/*
** AABoxRenderObjClass -- RenderObject for axis-aligned collision boxes.
*/
class AABoxRenderObjClass : public W3DMPO, public BoxRenderObjClass
{
	W3DMPO_GLUE(AABoxRenderObjClass)
public:

	AABoxRenderObjClass(void);
	AABoxRenderObjClass(const W3dBoxStruct & def);
	AABoxRenderObjClass(const AABoxRenderObjClass & src);
	AABoxRenderObjClass(const AABoxClass & box);
	AABoxRenderObjClass & operator = (const AABoxRenderObjClass &);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);
   virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	/////////////////////////////////////////////////////////////////////////////
	// AABoxRenderObjClass Interface
	/////////////////////////////////////////////////////////////////////////////
	const AABoxClass &			Get_Box(void);

protected:

	virtual void					update_cached_box(void);

	AABoxClass						CachedBox;

};

inline const AABoxClass & AABoxRenderObjClass::Get_Box(void)
{
	Validate_Transform();
	update_cached_box();
	return CachedBox;
}

/*
** OBBoxRenderObjClass - render object for oriented collision boxes
*/
class OBBoxRenderObjClass : public W3DMPO, public BoxRenderObjClass
{
	W3DMPO_GLUE(OBBoxRenderObjClass)
public:

	OBBoxRenderObjClass(void);
	OBBoxRenderObjClass(const W3dBoxStruct & def);
	OBBoxRenderObjClass(const OBBoxRenderObjClass & src);
	OBBoxRenderObjClass(const OBBoxClass & box);
	OBBoxRenderObjClass & operator = (const OBBoxRenderObjClass &);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);
   virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	/////////////////////////////////////////////////////////////////////////////
	// OBBoxRenderObjClass Interface
	/////////////////////////////////////////////////////////////////////////////
	OBBoxClass &					Get_Box(void);

protected:

	virtual void					update_cached_box(void);

	OBBoxClass						CachedBox;

};


/*
** Loader for boxes
*/
class BoxLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type (void)  { return W3D_CHUNK_BOX; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};


// ----------------------------------------------------------------------------
/*
** Prototype for Box objects
*/
class BoxPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(BoxPrototypeClass)
public:
	BoxPrototypeClass(W3dBoxStruct box);
	virtual const char *				Get_Name(void) const;
	virtual int									Get_Class_ID(void) const;
	virtual RenderObjClass *		Create(void);
	virtual void								DeleteSelf() { delete this; }
private:
	W3dBoxStruct					Definition;
};

/*
** Instance of the loader which the asset manager installs
*/
extern BoxLoaderClass			_BoxLoader;




#endif

