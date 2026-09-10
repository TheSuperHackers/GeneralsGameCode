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
 *                     $Archive:: /Commando/Code/ww3d2/Collect.h                              $*
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
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
import Graphics.Scene.Models.Factory;

#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
import Assets.ModelAssembly;

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DModelGroupRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWLib/ref_ptr.h"
import Graphics.Scene.Models.Children;
#include "WWLib/Vector.h"
#include "WWLib/wwstring.h"



/*
** W3DCollectionRenderObject
** This is a render object which contains a collection of render objects.
*/
class W3DCollectionRenderObject : public W3DModelGroupRenderObject
{
public:

	W3DCollectionRenderObject();
	W3DCollectionRenderObject(const Assets::ModelCollectionDesc & def);
	W3DCollectionRenderObject(const W3DCollectionRenderObject & src);
	W3DCollectionRenderObject & operator = (const W3DCollectionRenderObject &);
	virtual ~W3DCollectionRenderObject() override;
	virtual W3DRenderObject *	Clone() const override;

	virtual int						Class_ID()	const override;
	virtual int						Get_Num_Polys() const override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(W3DRenderContext & rinfo) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m) override;
	virtual void 					Set_Position(const Vector3 &v) override;
	virtual int						Get_Num_Sub_Objects() const override;
	virtual W3DRenderObject *	Get_Sub_Object(int index) const override;
	virtual int						Add_Sub_Object(W3DRenderObject * subobj) override;
	virtual int						Remove_Sub_Object(W3DRenderObject * robj) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection, Ray Tracing
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(W3DRayCastQuery & raytest) override;
	virtual bool					Cast_AABox(W3DBoxCastQuery & boxtest) override;
	virtual bool					Cast_OBBox(W3DOrientedBoxCastQuery & boxtest) override;
	virtual bool					Intersect_AABox(W3DBoxIntersectionQuery & boxtest) override;
	virtual bool					Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void		 			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const override;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const override;


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
	virtual int						Snap_Point_Count();
	virtual void					Get_Snap_Point(int index,Vector3 * set) override;
	virtual void					Scale(float scale) override;
	virtual void					Scale(float scalex, float scaley, float scalez) override;
   virtual void               Update_Obj_Space_Bounding_Volumes() override;

protected:

	void								Free();
	virtual void								Update_Sub_Object_Transforms() override;

	std::vector<Assets::ModelProxyDesc> ProxyList;
    using ChildOwner = RefCountPtr<W3DRenderObject>;
    using ChildAttachment = Graphics::ModelChildren<ChildOwner>::Attachment;
    Graphics::ModelChildren<ChildOwner> m_children;
	std::vector<Assets::Vector3f> SnapPoints;

	SphereClass										BoundSphere;
	AABoxClass										BoundBox;
};


/*
** CollectionLoaderClass
** Loader for collection objects
*/
Graphics::ModelFactory<W3DRenderObject>* Load_Collection_Factory(ChunkLoadClass& cload);
