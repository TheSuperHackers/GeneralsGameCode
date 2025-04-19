/*
**	Command & Conquer Renegade(tm)
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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/ViewerScene.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/15/01 3:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "ViewerScene.H"
#include "Camera.H"
#include "WW3D.H"

#include "RendObj.H"
#include "AssetMgr.H"
#include "rinfo.h"
#include "lightenvironment.h"

/*
** ViewerSceneIterator
** This iterator is used by the ViewerSceneClass to allow
** the user to iterate through its render objects. 
*/
class ViewerSceneIterator : public SceneIterator
{
public:
	virtual void					First(void);
	virtual void					Next(void);
	virtual bool					Is_Done(void);
	virtual RenderObjClass *	Current_Item(void);

protected:

	ViewerSceneIterator(RefRenderObjListClass * renderlist);

	RefRenderObjListIterator	RobjIterator;	

	friend class ViewerSceneClass;
};

ViewerSceneIterator::ViewerSceneIterator(RefRenderObjListClass *list)
:	RobjIterator(list)
{
}

void ViewerSceneIterator::First(void)
{
	RobjIterator.First();
}

void ViewerSceneIterator::Next(void)
{
	RobjIterator.Next();
}

bool ViewerSceneIterator::Is_Done(void)
{
	return RobjIterator.Is_Done();
}

RenderObjClass * ViewerSceneIterator::Current_Item(void)
{
	return RobjIterator.Peek_Obj();
}




/*
**
** ViewerSceneClass Implementation!
**
*/

////////////////////////////////////////////////////////////////////////
//
//	Visibility_Check
//
//	Note: We overide this method to remove the LOD preparation.  We
// need to be able to specify an LOD and not have it switch on us.
//
////////////////////////////////////////////////////////////////////////
void
ViewerSceneClass::Visibility_Check (CameraClass *camera)
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

		int lod_level = robj->Get_LOD_Level ();
					
		// Prepare visible objects for LOD:
		if (robj->Is_Really_Visible()) {
			robj->Prepare_LOD(*camera);
		}

		if (m_AllowLODSwitching == false) {
			robj->Set_LOD_Level (lod_level);
		}
	}

   Visibility_Checked = true;

	//SimpleSceneClass::Visibility_Check (camera);
	return ;
}

void
ViewerSceneClass::Add_To_Lineup (RenderObjClass *obj)
{
	assert(obj);

	// If this is an insignificant object (ie. we don't need to
	// rearrange existing objects to accomodate it), don't bother
	// adding it to the lineup. Ex: Adding a light to the lineup
	// is pretty silly.
	if (!Can_Line_Up(obj))
		return;

	
	// We will add this object to the scene next to any
	// existing objects. It will be placed at (0, Y, 0)
	// where Y is a value to be calculated such that the
	// center of the scene (0,0,0) is in the middle of the
	// row of objects. The existing objects will need to
	// be moved along the -Y axis to make room.

	// Figure out how 'wide' the object is (width assuming it
	// it facing along the +X axis).
	AABoxClass obj_box;
	obj->Get_Obj_Space_Bounding_Box(obj_box);
	float obj_width = obj_box.Extent.Y * 2.0f;

	// Figure out the bounding box for the objects in the lineup.
	AABoxClass scene_box = Get_Line_Up_Bounding_Box();
	float scene_width = scene_box.Extent.Y * 2.0f;

	// How much do we have to move the existing objects by?
	float new_scene_width = scene_width + obj_width + obj_width/3;
	float delta = (new_scene_width - scene_width) / 2;

	// Move the existing objects along the -Y axis.
	int num_existing_objects = 0;
	SceneIterator *it = Create_Iterator();
	assert(it);
	for (; !it->Is_Done(); it->Next())
	{
		RenderObjClass *current_obj = it->Current_Item();
		assert(current_obj);
		if (Can_Line_Up(current_obj))
		{
			Vector3 pos = current_obj->Get_Position();
			pos.Y -= delta;
			current_obj->Set_Position(pos);

			++num_existing_objects;
		}
	}
	Destroy_Iterator(it);

	// Move the new object so that it will be in line
	// with the existing objects.
	if (num_existing_objects > 0)
		obj->Set_Position(Vector3(0,new_scene_width/2 - obj_box.Extent.Y, 0));
	else
		obj->Set_Position(Vector3(0,0,0));

	// Add the object to the scene.
	Add_Render_Object(obj);

	// Add the object to the list of objects in the lineup.
	LineUpList.Add(obj);
}

void
ViewerSceneClass::Clear_Lineup (void)
{
	// Remove every object in the lineup from the scene,
	// and remove each object from the line up list.
	RenderObjClass *obj = NULL;
	while (obj = LineUpList.Remove_Head())
		Remove_Render_Object(obj);
}

SphereClass
ViewerSceneClass::Get_Bounding_Sphere (void)
{
	// Iterate through every object in the scene, adding its
	// bounding sphere to the current bounding sphere. The sum of
	// the bounding spheres will be the scene's bounding sphere.
	SphereClass bounding_sphere(Vector3(0,0,0), 0.0f);
	SceneIterator *it = Create_Iterator();
	assert(it);
	for (; !it->Is_Done(); it->Next())
	{
		RenderObjClass *rend_obj = it->Current_Item();
		assert(rend_obj);
		// Omit lights in the bounding sphere calculations.
		if (rend_obj->Class_ID() != RenderObjClass::CLASSID_LIGHT)
			bounding_sphere.Add_Sphere(rend_obj->Get_Bounding_Sphere());
	}
	Destroy_Iterator(it);

	return bounding_sphere;
}

AABoxClass
ViewerSceneClass::Get_Line_Up_Bounding_Box (void)
{
	// Iterate through each object in the lineup, adding its
	// bounding box to the current bounding box. The sum
	// of the bounding boxes will be the lineup's bounding box.
	AABoxClass sum_of_boxes(Vector3(0,0,0), Vector3(0,0,0));
	SceneIterator *it = Create_Iterator();
	assert(it);
	for (; !it->Is_Done(); it->Next())
	{
		RenderObjClass *rend_obj = it->Current_Item();
		assert(rend_obj);
		if (Can_Line_Up(rend_obj))
			sum_of_boxes.Add_Box(rend_obj->Get_Bounding_Box());
	}
	Destroy_Iterator(it);

	return sum_of_boxes;
}

bool
ViewerSceneClass::Can_Line_Up (RenderObjClass * obj)
{
	assert(obj);
	return Can_Line_Up(obj->Class_ID());
}

bool
ViewerSceneClass::Can_Line_Up (int class_id)
{
	return (class_id == RenderObjClass::CLASSID_HMODEL) ||
			 (class_id == RenderObjClass::CLASSID_HLOD);
}

SceneIterator *
ViewerSceneClass::Create_Line_Up_Iterator (void)
{
	return new ViewerSceneIterator(&LineUpList);
}

void
ViewerSceneClass::Destroy_Line_Up_Iterator (SceneIterator *it)
{
	delete it;
}

void	ViewerSceneClass::Add_Render_Object(RenderObjClass * obj)
{
	SceneClass::Add_Render_Object(obj);
	if (obj->Class_ID()==RenderObjClass::CLASSID_LIGHT)
		LightList.Add(obj);
	else
		RenderList.Add(obj);

	// Recalculate the fogging distances.
	Recalculate_Fog_Planes();
}

void	ViewerSceneClass::Recalculate_Fog_Planes (void)
{
	const float FOG_OPAQUE_MULTIPLE	= 8.0f;
	const float FOG_MINIMUM_DEPTH	= 200.0f;

	// Adjust the fog far clipping plane based on the size of the
	// scene's bounding box depth (X value). We'll have the fog be
	// completely opaque at FOG_OPAQUE_MULTIPLE times the depth of
	// the scene's bounding box.
	float fog_near=0, fog_far=0;
	Get_Fog_Range(&fog_near, &fog_far);
	SphereClass sphere = Get_Bounding_Sphere();

	// Calculate the fog far plane. If it is too close to the
	// near plane, use the camera's far clip plane setting.
	fog_far = sphere.Radius * FOG_OPAQUE_MULTIPLE;
	if (fog_far < fog_near + FOG_MINIMUM_DEPTH)
		fog_far = fog_near + FOG_MINIMUM_DEPTH;
	Set_Fog_Range(fog_near, fog_far);
}

void	ViewerSceneClass::Customized_Render(RenderInfoClass & rinfo)
{
#ifdef WW3D_DX8 // just use simplescene for now...
	// If visibility has not been checked for this scene since the last
   // Render() call, check it (set/clear the visibility bit in all render
   // objects in the scene).
   if (!Visibility_Checked) {
      // set the visibility bit in all render objects in all layers.
	   Visibility_Check(&rinfo.Camera);
   }
   Visibility_Checked = false;	

	// Install the vertex processors.  Derived scenes may want to use some
	// form of spatial subdivision to only insert the needed vps...
	RefRenderObjListIterator it(&LightList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Vertex_Processor_Push(rinfo);
	}

	if (FogEnabled) Fog->Vertex_Processor_Push(rinfo);

	// make a light environmemt class

	LightEnvironmentClass lenv;

	lenv.Reset(Vector3(0,0,0),AmbientLight);
	RefRenderObjListIterator it(&LightList);

	for (it.First(&LightList); !it.Is_Done(); it.Next()) {
		lenv.Add_Light(*(LightClass*)it.Peek_Obj());
	}	
	lenv.Pre_Render_Update(rinfo.Camera.Get_Transform());

	rinfo.light_environment=&lenv;

	// allow all objects in the update list to do their "every frame" processing
	for (it.First(&UpdateList); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->On_Frame_Update();
	}

	// loop through all render objects in the list:
	for (it.First(&RenderList); !it.Is_Done(); it.Next()) {

		// get the render object
		RenderObjClass * robj = it.Peek_Obj();

		if (robj->Is_Really_Visible()) {

			// Do "visible" processing and add to the surrender scene
			robj->Render(rinfo);
		}
	}

	if (FogEnabled) Fog->Vertex_Processor_Pop(rinfo);

	// Now loop through the objects, removing their vertex processors.  See
	// note above regarding more efficient methods of managing vertex processors.
	for (it.First(&LightList); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Vertex_Processor_Pop(rinfo);
	}

#else
	SimpleSceneClass::Customized_Render(rinfo);
#endif //WW3D_DX8
}