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
 *                     $Archive:: /Commando/Code/ww3d2/collect.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "rendobj.h"
#include "composite.h"
#include "Vector.H"
#include "proto.h"
#include "w3d_file.h"
#include "wwstring.h"
#include "proxy.h"

class CollectionDefClass;
class SnapPointsClass;


/*
** CollectionClass
** This is a render object which contains a collection of render objects.
*/
class CollectionClass : public CompositeRenderObjClass
{
public:

	CollectionClass(void);
	CollectionClass(const CollectionDefClass & def);
	CollectionClass(const CollectionClass & src);
	CollectionClass & operator = (const CollectionClass &);
	virtual ~CollectionClass(void);
	virtual RenderObjClass *	Clone(void) const;

	virtual int						Class_ID(void)	const;
	virtual int						Get_Num_Polys(void) const;

	/////////////////////////////////////////////////////////////////////////////
	// Proxy interface
	/////////////////////////////////////////////////////////////////////////////
	virtual int						Get_Proxy_Count (void) const;
	virtual bool					Get_Proxy (int index, ProxyClass &proxy) const;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);
	virtual int						Get_Num_Sub_Objects(void) const;
	virtual RenderObjClass *	Get_Sub_Object(int index) const;
	virtual int						Add_Sub_Object(RenderObjClass * subobj);
	virtual int						Remove_Sub_Object(RenderObjClass * robj);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection, Ray Tracing
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void		 			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
	virtual int						Snap_Point_Count(void);
	virtual void					Get_Snap_Point(int index,Vector3 * set);
	virtual void					Scale(float scale);
	virtual void					Scale(float scalex, float scaley, float scalez);
   virtual void               Update_Obj_Space_Bounding_Volumes(void);

protected:

	void								Free(void);
	void								Update_Sub_Object_Transforms(void);

	DynamicVectorClass <ProxyClass>			ProxyList;
	DynamicVectorClass <RenderObjClass *>	SubObjects;
	SnapPointsClass *								SnapPoints;

	SphereClass										BoundSphere;
	AABoxClass										BoundBox;
};


/*
** CollectionLoaderClass
** Loader for collection objects
*/
class CollectionLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type(void) { return W3D_CHUNK_COLLECTION; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

extern CollectionLoaderClass _CollectionLoader;
