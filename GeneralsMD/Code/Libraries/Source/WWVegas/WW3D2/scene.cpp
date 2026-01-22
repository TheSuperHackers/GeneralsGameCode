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
 *                     $Archive:: /VSS_Sync/ww3d2/scene.cpp                                   $*
 *                                                                                             *
 *                   Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 07/01/02 12:55p                                              $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 * 06/27/02 KM Shader system light environment updates                                       *
 * 07/01/02 KM Coltype enum change to avoid MAX conflicts									   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SceneClass::SceneClass -- constructor                                                     *
 *   SceneClass::~SceneClass -- destructor                                                     *
 *   SceneClass::Render -- preps the scene for rendering, derived classes should override      *
 *   SceneClass::Add_Render_Object -- base add function                                        *
 *   SceneClass::Remove_Render_Object -- base remove function                                  *
 *   SceneClass::Set_Depth_Cue -- set the depth cue values                                     *
 *   SceneClass::Set_Fog_Range -- set the fog range                                            *
 *   SceneClass::Get_Fog_Range -- get the fog range                                            *
 *   SceneClass::Render -- preps the scene for rendering, derived classes should add functional*
 *   SimpleSceneClass -- Constructor                                                           *
 *   SimpleSceneClass::~SimpleSceneClass -- destructor                                         *
 *   SimpleSceneClass::Add_Render_Object -- add a render object to the scene                   *
 *   SimpleSceneClass::Remove_Render_Object -- remove a render object from this scene          *
 *   SimpleSceneClass::Visiblity_Check -- set the visiblity status of the render objects       *
 *   SimpleSceneClass::Render -- internal scene rendering function                             *
 *   SimpleSceneClass::Render -- Render this scene                                             *
 *   SimpleSceneClass::Create_Iterator -- create an iterator for this scene                    *
 *   SimpleSceneClass::Destroy_Iterator -- destroy an iterator of this scene                   *
 *   SceneClass::Save -- saves scene settings into a chunk                                     *
 *   SceneClass::Load -- loads scene settings from a chunk                                     *
 *   SimpleSceneClass::Compute_Point_Visibility -- returns visibility of a point               *
 *   SimpleSceneClass::Remove_All_Render_Objects -- Removes all render objects from the scene  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "scene.h"
#include "plane.h"
#include "camera.h"
#include "ww3d.h"
#include "rinfo.h"
#include "chunkio.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "sortingrenderer.h"
#include "coltest.h"


/*
** Chunk ID's used by SceneClass
*/
enum
{
	SCENECLASS_CHUNK_VARIABLES			= 0x00042300,

	SCENECLASS_VARIABLE_AMBIENTLIGHT	= 0x00,
	SCENECLASS_VARIABLE_POLYRENDERMODE,
	SCENECLASS_VARIABLE_FOGCOLOR,
	SCENECLASS_VARIABLE_FOGENABLED,
	SCENECLASS_VARIABLE_FOGSTART,
	SCENECLASS_VARIABLE_FOGEND,
};

/*
** SimpleSceneIterator
** This iterator is used by the SimpleSceneClass to allow
** the user to iterate through its render objects.
*/
class SimpleSceneIterator : public SceneIterator
{
public:
	virtual void					First(void);
	virtual void					Next(void);
	virtual bool					Is_Done(void);
	virtual RenderObjClass *	Current_Item(void);

protected:

	SimpleSceneIterator(RefRenderObjListClass * renderlist,bool onlyvis);

	RefRenderObjListIterator	RobjIterator;
	SimpleSceneClass *			Scene;
	bool								OnlyVis;

	friend class SimpleSceneClass;
};


/***********************************************************************************************
 * SceneClass::SceneClass -- constructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
SceneClass::SceneClass(void) :
	AmbientLight(0.5f,0.5f,0.5f),
	PolyRenderMode(FILL),
	ExtraPassPolyRenderMode(EXTRA_PASS_DISABLE),
	FogEnabled(false),
	FogColor(0,0,0),
	FogStart(0.0f),
	FogEnd(1000.0f)	// Arbitrary default value
{
}


/***********************************************************************************************
 * SceneClass::~SceneClass -- destructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
SceneClass::~SceneClass(void)
{
}

/***********************************************************************************************
 * SceneClass::Add_Render_Object -- base add function                                          *
 *                                                                                             *
 * This function only sets the render object's scene pointer and calls the notify method.      *
 * Derived classes must add the functionality of actually storing a pointer to the object      *
 * and doing something with it :)                                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj - pointer to the object to add                                                          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void SceneClass::Add_Render_Object(RenderObjClass * obj)
{
	obj->Notify_Added(this);
}


/***********************************************************************************************
 * SceneClass::Remove_Render_Object -- base remove function                                    *
 *                                                                                             *
 * Similar to the add function; concrete derived classes must override and actually remove     *
 * the object...                                                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj - pointer to the object to remove                                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void SceneClass::Remove_Render_Object(RenderObjClass * obj)
{
	obj->Notify_Removed(this);
}


/***********************************************************************************************
 * SceneClass::Render -- preps the scene for rendering, derived classes should add functionalit*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void SceneClass::Render(RenderInfoClass & rinfo)
{
	// Any stuff that needs to get done before anything else
	Pre_Render_Processing(rinfo);

	DX8Wrapper::Set_Fog(FogEnabled, FogColor, FogStart, FogEnd);

	if (Get_Extra_Pass_Polygon_Mode()==EXTRA_PASS_DISABLE) {
		Customized_Render(rinfo);
	}
	else {
		bool old_enable=WW3D::Is_Texturing_Enabled();

		DX8Wrapper::Set_DX8_Render_State (D3DRS_ZBIAS, 0);
		Customized_Render(rinfo);
		switch (Get_Extra_Pass_Polygon_Mode()) {
		case EXTRA_PASS_LINE:
			WW3D::Enable_Texturing(false);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			DX8Wrapper::Set_DX8_Render_State (D3DRS_ZBIAS, 7);
			Customized_Render(rinfo);
			break;
		case EXTRA_PASS_CLEAR_LINE:
			DX8Wrapper::Clear(true, false, Vector3(0.0f,0.0f,0.0f));	// Clear color but not z
			WW3D::Enable_Texturing(false);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			DX8Wrapper::Set_DX8_Render_State (D3DRS_ZBIAS, 7);
			Customized_Render(rinfo);
			break;
		}

		WW3D::Enable_Texturing(old_enable);
	}

	// Any stuff that needs to get done after anything else
	Post_Render_Processing(rinfo);
}

/***********************************************************************************************
 * SceneClass::Save -- saves scene settings into a chunk                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void SceneClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(SCENECLASS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_AMBIENTLIGHT,AmbientLight);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_POLYRENDERMODE,PolyRenderMode);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_FOGCOLOR,FogColor);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_FOGENABLED,FogEnabled);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_FOGSTART,FogStart);
	WRITE_MICRO_CHUNK(csave,SCENECLASS_VARIABLE_FOGEND,FogEnd);
	csave.End_Chunk();
}


/***********************************************************************************************
 * SceneClass::Load -- loads scene settings from a chunk                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void SceneClass::Load(ChunkLoadClass & cload)
{
	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() == SCENECLASS_CHUNK_VARIABLES) {

		while (cload.Open_Micro_Chunk()) {
			switch(cload.Cur_Micro_Chunk_ID()) {
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_AMBIENTLIGHT,AmbientLight);
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_POLYRENDERMODE,PolyRenderMode);
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_FOGCOLOR,FogColor);
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_FOGENABLED,FogEnabled);
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_FOGSTART,FogStart);
				READ_MICRO_CHUNK(cload,SCENECLASS_VARIABLE_FOGEND,FogEnd);
			}
			cload.Close_Micro_Chunk();
		}

	} else {
		WWDEBUG_SAY(("Unhandled Chunk: 0x%X in file: %s line: %d",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
	}
	cload.Close_Chunk();
}


/***********************************************************************************************
 * SimpleSceneClass -- Constructor                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *   9/10/99    GTH : Created.                                                                 *
 *=============================================================================================*/
SimpleSceneClass::SimpleSceneClass(void) :
   Visibility_Checked(false)
{

}

/***********************************************************************************************
 * SimpleSceneClass::~SimpleSceneClass -- destructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
SimpleSceneClass::~SimpleSceneClass(void)
{
	Remove_All_Render_Objects();
}

/***********************************************************************************************
 * SimpleSceneClass::Remove_All_Render_Objects -- Removes all render objects from the scene    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/27/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SimpleSceneClass::Remove_All_Render_Objects(void)
{
	RenderObjClass * obj;
	while ( ( obj = RenderList.Remove_Head() ) != nullptr ) {
		SceneClass::Remove_Render_Object(obj);
		obj->Release_Ref();							// remove head gets a ref
	}
}

/***********************************************************************************************
 * SimpleSceneClass::Add_Render_Object -- add a render object to the scene                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void SimpleSceneClass::Add_Render_Object(RenderObjClass * obj)
{
	SceneClass::Add_Render_Object(obj);
	RenderList.Add(obj);
}


/***********************************************************************************************
 * SimpleSceneClass::Remove_Render_Object -- remove a render object from this scene            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void SimpleSceneClass::Remove_Render_Object(RenderObjClass * obj)
{
	SceneClass::Remove_Render_Object(obj);
	// HY
	// this line must come last because otherwise it might cause
	// a premature release ref and cause a crash
	RenderList.Remove(obj);
}

void SimpleSceneClass::Register(RenderObjClass * obj,RegType for_what)
{
	switch (for_what) {
		case ON_FRAME_UPDATE:
			UpdateList.Add(obj);
			break;
		case LIGHT:
			LightList.Add_Tail(obj);
			break;
		case RELEASE:
			ReleaseList.Add(obj);
			break;
	};
}

void SimpleSceneClass::Unregister(RenderObjClass * obj,RegType for_what)
{
	switch (for_what) {
		case ON_FRAME_UPDATE:
			UpdateList.Remove(obj);
			break;
		case LIGHT:
			LightList.Remove(obj);
			break;
		case RELEASE:
			ReleaseList.Remove(obj);
			break;
	}
}



/***********************************************************************************************
 * SimpleSceneClass::Visiblity_Check -- set the visiblity status of the render objects         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *   4/13/98    NH : Added non-trivial checking (sphere vs. frustum).                          *
 *=============================================================================================*/
void SimpleSceneClass::Visibility_Check(CameraClass * camera)
{
	RefRenderObjListIterator it(&RenderList);

	// Loop over all top-level RenderObjects in this scene. If the bounding sphere is not in front
	// of all the frustum planes, it is invisible.
	for (it.First(); !it.Is_Done(); it.Next()) {

		RenderObjClass * robj = it.Peek_Obj();

		if (robj->Is_Force_Visible()) {
			robj->Set_Visible(true);
		} else {
			robj->Set_Visible(!camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
		}

		// Prepare visible objects for LOD:
		if(robj->Is_Really_Visible() && !robj->Is_Ignoring_LOD_Cost()) {
			robj->Prepare_LOD(*camera);
		}
	}

   Visibility_Checked = true;
}


/***********************************************************************************************
 * SimpleSceneClass::Compute_Point_Visibility -- returns visibility of a point                 *
 *                                                                                             *
 *    This function is used by the default dazzle behavior.  SimpleSceneClass's implementation *
 *    casts a ray against every object in the scene to see if the dazzle is occluded.          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/13/2001  gth : Created.                                                                 *
 *=============================================================================================*/
float SimpleSceneClass::Compute_Point_Visibility
(
	RenderInfoClass & rinfo,
	const Vector3 & point
)
{
	CastResultStruct res;
	LineSegClass ray(rinfo.Camera.Get_Position(),point);
	RayCollisionTestClass raytest(ray,&res,COLL_TYPE_PROJECTILE);

	RefRenderObjListIterator it(&RenderList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		RenderObjClass * robj = it.Peek_Obj();
		robj->Cast_Ray(raytest);
	}

	if (res.Fraction == 1.0f) {
		return 1.0f;
	} else {
		return 0.0f;
	}
}


/***********************************************************************************************
 * SimpleSceneClass::Render -- Render this scene                                               *
 *                                                                                             *
 * passes the gerd and camera to all render objects for rendering...                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98  GTH : Created.                                                                 *
 *   06/27/02	KM Shader system light environment updates                                       *
 *=============================================================================================*/
void SimpleSceneClass::Customized_Render(RenderInfoClass & rinfo)
{
//	SceneClass::Render(rinfo);

   // If visibility has not been checked for this scene since the last
   // Render() call, check it (set/clear the visibility bit in all render
   // objects in the scene).
   if (!Visibility_Checked) {
      // set the visibility bit in all render objects in all layers.
	   Visibility_Check(&rinfo.Camera);
   }
   Visibility_Checked = false;

	RefRenderObjListIterator it(&UpdateList);

	// allow all objects in the update list to do their "every frame" processing
	for (it.First(); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->On_Frame_Update();
	}

	// apply only the first four lights in the scene
	// derived classes should use light environment
	WWASSERT(rinfo.light_environment==nullptr);
	int count=0;
	// Turn off lights in case we have none
	DX8Wrapper::Set_Light(0,nullptr);
	DX8Wrapper::Set_Light(1,nullptr);
	DX8Wrapper::Set_Light(2,nullptr);
	DX8Wrapper::Set_Light(3,nullptr);

// (gth) WWShade only works with light environments.  We need to upgrade LightEnvironment to
// support real point lights, etc.  It will likely just evolve into "the n most important" lights
// rather than optimizing lights into directional lights...
#if 0
	for (it.First(&LightList); !it.Is_Done(); it.Next())
	{
		if (count<4)
		{
			DX8Wrapper::Set_Light(count,*(LightClass*)it.Peek_Obj());
		} else
		{
			// Simple scene only supports 4 global lights
			WWDEBUG_SAY(("Light %d ignored",count));
		}
		count++;
	}
#endif

	// adding light environment for new shader system
	if (!rinfo.light_environment)
	{
		static LightEnvironmentClass lenv;

		lenv.Reset(Vector3(0,0,0),AmbientLight);

		for (it.First(&LightList); !it.Is_Done(); it.Next())
		{
			lenv.Add_Light(*(LightClass*)it.Peek_Obj());
		}
		lenv.Pre_Render_Update(rinfo.Camera.Get_Transform());

		rinfo.light_environment=&lenv;
	}


	// loop through all render objects in the list:
	for (it.First(&RenderList); !it.Is_Done(); it.Next()) {

		// get the render object
		RenderObjClass * robj = it.Peek_Obj();

		if (robj->Is_Really_Visible()) {
			if (robj->Get_Render_Hook()) {
				if (robj->Get_Render_Hook()->Pre_Render(robj, rinfo)) {
					robj->Render(rinfo);
				}
				robj->Get_Render_Hook()->Post_Render(robj, rinfo);
			} else {
				robj->Render(rinfo);
			}
		}
	}
}

void SimpleSceneClass::Post_Render_Processing(RenderInfoClass& rinfo)
{
	// process the 'Release' list.  These are objects that have notified us that they
	// want to be released.  We have to walk this list twice, first un-linking the
	// object from the scene or its container.  And then removing them all from
	// the list.  (this last removal will destroy any auto-created objects)
	RefRenderObjListIterator it(&ReleaseList);
	for (it.First(&ReleaseList); !it.Is_Done(); it.Next()) {
		RenderObjClass * robj = it.Peek_Obj();
		if (robj->Get_Container()) {
			robj->Get_Container()->Remove_Sub_Object(robj);
		} else {
			robj->Remove();
		}
	}

	while(!ReleaseList.Is_Empty()) {
		ReleaseList.Release_Head();
	}
}

/***********************************************************************************************
 * SimpleSceneClass::Create_Iterator -- create an iterator for this scene                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/98    GTH : Created.                                                                 *
 *=============================================================================================*/
SceneIterator * SimpleSceneClass::Create_Iterator(bool onlyvisible)
{
	SimpleSceneIterator * it = W3DNEW SimpleSceneIterator(&RenderList,onlyvisible);
	return it;
}


/***********************************************************************************************
 * SimpleSceneClass::Destroy_Iterator -- destroy an iterator of this scene                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void SimpleSceneClass::Destroy_Iterator(SceneIterator * it)
{
	delete it;
}


SimpleSceneIterator::SimpleSceneIterator(RefRenderObjListClass * list,bool onlyvis) :
	RobjIterator(list)
{
	// TODO: make SimpleSceneIterator able to iterate through only the visible nodes.
	OnlyVis = onlyvis;
}

void SimpleSceneIterator::First(void)
{
	RobjIterator.First();
}

void SimpleSceneIterator::Next(void)
{
	RobjIterator.Next();
}

bool SimpleSceneIterator::Is_Done(void)
{
	return RobjIterator.Is_Done();
}

RenderObjClass * SimpleSceneIterator::Current_Item(void)
{
	return RobjIterator.Peek_Obj();
}
