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
 *                     $Archive:: /VSS_Sync/ww3d2/scene.h                                     $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "refcount.h"
#include "vector3.h"
#include "robjlist.h"
#include "wwdebug.h"

class	RenderObjClass;
class	RenderInfoClass;
class CameraClass;
class ChunkLoadClass;
class ChunkSaveClass;

/*
** SceneIterator
** This object is used to iterate through the render objects
** in a scene.
*/
class SceneIterator
{
public:

	virtual							~SceneIterator(void) { };
	virtual void					First(void) = 0;
	virtual void					Next(void) = 0;
	virtual bool					Is_Done(void) = 0;
	virtual RenderObjClass *	Current_Item(void) = 0;

protected:

	SceneIterator(void) { };
};


/**
** SceneClass
** This is a bunch of render objects that define a 3D scene.
** The requirements of a SceneClass are:
** - The ablility to pass its renderobject's internal surrender representation
**   to surrender when asked in the Render method.
** - The ability to add and remove render objects from the scene
** - The ability to create an iterator for the user which uses the
**   SceneIterator interface and allows the user to iterate through
**   all render objects or visible render objects in the scene.
**
** The "registration" interface is used by certain render objects to enable
** extra processing.  For example, particle emmitters/buffers need to be updated
** each frame regardless of whether they are visible so they are registered
** for "ON_FRAME_UPDATE" processing.
*/
class SceneClass : public RefCountClass
{
public:
	SceneClass(void);
	virtual ~SceneClass(void);

	///////////////////////////////////////////////////////////////////////////////////
	// RTTI information.
	///////////////////////////////////////////////////////////////////////////////////
	enum {
		SCENE_ID_UNKOWN = 0xFFFFFFFF,
		SCENE_ID_SCENE = 0,
		SCENE_ID_SIMPLE,

		SCENE_ID_LAST = 0x0000FFFF,
	};
	virtual int					Get_Scene_ID(void) const	{	return SCENE_ID_SCENE;	}


	virtual void				Add_Render_Object(RenderObjClass * obj);
	virtual void				Remove_Render_Object(RenderObjClass * obj);

	virtual SceneIterator *	Create_Iterator(bool onlyvisible = false)				= 0;
	virtual void				Destroy_Iterator(SceneIterator * it)					= 0;

	virtual void				Set_Ambient_Light(const Vector3 & color)				{ AmbientLight = color; }
	virtual const Vector3 &	Get_Ambient_Light(void)										{ return AmbientLight; }

	///////////////////////////////////////////////////////////////////////////////////
	//	Fog methods
	///////////////////////////////////////////////////////////////////////////////////
	virtual void				Set_Fog_Enable(bool set)									{ FogEnabled = set; }
	virtual bool				Get_Fog_Enable(void)											{ return FogEnabled; }

	virtual void				Set_Fog_Color(const Vector3 & color)					{ FogColor = color; }
	virtual const Vector3 &	Get_Fog_Color(void)											{ return FogColor; }

	virtual void				Set_Fog_Range( float start, float end )				{ FogStart = start; FogEnd = end; }
	virtual void				Get_Fog_Range( float * start, float * end )			{ *start = FogStart; *end = FogEnd; }

	///////////////////////////////////////////////////////////////////////////////////
	//	Render Modes
	///////////////////////////////////////////////////////////////////////////////////
	enum PolyRenderType
	{
		POINT,
		LINE,
		FILL
	};

	void							Set_Polygon_Mode(PolyRenderType mode)					{ PolyRenderMode = mode; }
	PolyRenderType				Get_Polygon_Mode(void)										{ return PolyRenderMode; }

	///////////////////////////////////////////////////////////////////////////////////
	//	Second pass render mode is a debug feature which renders the whole scene twice.
	///////////////////////////////////////////////////////////////////////////////////
	enum ExtraPassPolyRenderType
	{
		EXTRA_PASS_DISABLE,
		EXTRA_PASS_LINE,
		EXTRA_PASS_CLEAR_LINE
	};

	void							Set_Extra_Pass_Polygon_Mode(ExtraPassPolyRenderType mode)		{ ExtraPassPolyRenderMode = mode; }
	ExtraPassPolyRenderType Get_Extra_Pass_Polygon_Mode(void)										{ return ExtraPassPolyRenderMode; }

	///////////////////////////////////////////////////////////////////////////////////
	//	Object processing registration
	///////////////////////////////////////////////////////////////////////////////////
	enum RegType
	{
		ON_FRAME_UPDATE = 0,
		LIGHT,
		RELEASE,
	};

	virtual void				Register(RenderObjClass * obj,RegType for_what)		= 0;
	virtual void				Unregister(RenderObjClass * obj,RegType for_what)	= 0;

	///////////////////////////////////////////////////////////////////////////////////
	//	Point visibility - used by DazzleRenderObj when no custom handler is installed
	///////////////////////////////////////////////////////////////////////////////////
	virtual float				Compute_Point_Visibility(	RenderInfoClass & rinfo,
																		const Vector3 & point)	{ return 1.0f; }


	///////////////////////////////////////////////////////////////////////////////////
	//	Save-Load, records the fog, depth cue, etc settings into a chunk
	///////////////////////////////////////////////////////////////////////////////////
	virtual void				Save(ChunkSaveClass & csave);
	virtual void				Load(ChunkLoadClass & cload);

protected:
	virtual void	Render(RenderInfoClass & rinfo);	//Made virtual so we can override -MW

	Vector3						AmbientLight;
	PolyRenderType				PolyRenderMode;
	ExtraPassPolyRenderType	ExtraPassPolyRenderMode;

	bool							FogEnabled;
	Vector3						FogColor;
	float							FogStart;
	float							FogEnd;

	friend class WW3D;

	/*
	** Not implemented!
	*/
	SceneClass (const SceneClass &);
	SceneClass & operator == (const SceneClass &);

private:
	virtual void				Customized_Render(RenderInfoClass & rinfo)=0;
	virtual void				Pre_Render_Processing(RenderInfoClass & rinfo) {}
	virtual void				Post_Render_Processing(RenderInfoClass & rinfo) {}

};

/**
** SimpleSceneClass
** This is an example of a simple way to implement a scene class which
** can be rendered by WW3D.  Basically it stores a list of render objects
** When the Render function is called, it checks each object against the
** view frustum an marks it as visible or invisible.  Then it performs
** some LOD calculations on all of the visible objects, then has each
** render object add its internal (surrender) representation to the
** internal (surrender) representation of the scene.
*/
class SimpleSceneClass : public SceneClass
{
public:

	SimpleSceneClass(void);
	virtual ~SimpleSceneClass(void);

	virtual int	Get_Scene_ID(void)	{	return SCENE_ID_SIMPLE;	}

	virtual void Add_Render_Object(RenderObjClass * obj);
	virtual void Remove_Render_Object(RenderObjClass * obj);

	virtual void Remove_All_Render_Objects(void);

	virtual void Register(RenderObjClass * obj,RegType for_what);
	virtual void Unregister(RenderObjClass * obj,RegType for_what);

	virtual SceneIterator *		Create_Iterator(bool onlyvisible = false);
	virtual void					Destroy_Iterator(SceneIterator * it);

	// Set visibility status for my render objects. If not called explicitly
	// beforehand, will be called inside Render().
	virtual void Visibility_Check(CameraClass * camera);

	///////////////////////////////////////////////////////////////////////////////////
	//	Point visibility - used by DazzleRenderObj when no custom handler is installed
	///////////////////////////////////////////////////////////////////////////////////
	virtual float				Compute_Point_Visibility(	RenderInfoClass & rinfo,
																		const Vector3 & point);

protected:

   // Has a visibility check been performed since scene was last rendered?
   bool Visibility_Checked;

	RefRenderObjListClass	RenderList;
	RefRenderObjListClass	UpdateList;
	RefRenderObjListClass	LightList;
	RefRenderObjListClass	ReleaseList;

	friend class SimpleSceneIterator;

	virtual void Customized_Render(RenderInfoClass & rinfo);
	virtual void Post_Render_Processing(RenderInfoClass& rinfo);
};
