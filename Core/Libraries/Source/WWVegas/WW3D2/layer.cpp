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
 *                     $Archive:: /VSS_Sync/ww3d2/layer.cpp                                   $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LayerClass::LayerClass -- constructor                                                     *
 *   LayerClass::~LayerClass -- destructor                                                     *
 *   LayerClass::Set_Scene -- Set the scene used by this layer                                 *
 *   LayerClass::Get_Scene -- get the scene being rendered by this layer                       *
 *   LayerClass::Peek_Scene -- get the scene being rendered by this layer w/o a ref            *
 *   LayerClass::Set_Camera -- Set the camera being used by this layer                         *
 *   LayerClass::Get_Camera -- get the camera being used by this layer                         *
 *   LayerClass::LayerClass -- default constructor                                             *
 *   LC::Peek_Camera -- Get copy of camera.                                                    * 
 *   LC::Set -- Kinda like an assignment operator.                                             * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "layer.h"
#include "scene.h"
#include "camera.h"


/***********************************************************************************************
 * LayerClass::LayerClass -- default constructor                                               *
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
LayerClass::LayerClass(void) :
	Scene(NULL),
	Camera(NULL),
	Clear(false),
	ClearZ(true),
	ClearColor(0,0,0)
{
}	

LayerClass::LayerClass(const LayerClass &src) :
	Scene(src.Get_Scene()),
	Camera(src.Get_Camera()),
	Clear(src.Clear),
	ClearZ(src.ClearZ),
	ClearColor(src.ClearColor)
{
}	

/***********************************************************************************************
 * LayerClass::LayerClass -- constructor                                                       *
 *                                                                                             *
 *    simply constructs a layer class, initialized with the desired settings                   *
 *                                                                                             *
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
LayerClass::LayerClass
(
		SceneClass * scene,
		CameraClass * cam,
		bool clear,
		bool clearz,
		const Vector3 & color
) 
{ 
	if (scene) scene->Add_Ref();
	Scene = scene;

	if (cam) cam->Add_Ref();
	Camera = cam;

	Clear = clear; 
	ClearZ = clearz;
	ClearColor = color; 
}


/***********************************************************************************************
 * LayerClass::~LayerClass -- destructor                                                       *
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
LayerClass::~LayerClass(void) 
{
	if (Scene) {
		Scene->Release_Ref();
		Scene=0;
	}
	if (Camera) {
		Camera->Release_Ref();
		Camera=0;
	}
}


/***********************************************************************************************
 * LayerClass::Set_Scene -- Set the scene used by this layer                                   *
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
void LayerClass::Set_Scene(SceneClass * scene) 
{
	if (Scene) {
		Scene->Release_Ref();
	}
	Scene = scene;
	if (Scene) {
		Scene->Add_Ref();
	}
}


/***********************************************************************************************
 * LayerClass::Get_Scene -- get the scene being rendered by this layer                         *
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
SceneClass * LayerClass::Get_Scene(void) const
{
	if (Scene) {
		Scene->Add_Ref();
	}
	return Scene;
}


/***********************************************************************************************
 * LayerClass::Peek_Scene -- get the scene being rendered by this layer                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/8/99    NH : Created.                                                                   *
 *=============================================================================================*/
SceneClass * LayerClass::Peek_Scene(void) const
{
	return Scene;
}


/***********************************************************************************************
 * LayerClass::Set_Camera -- Set the camera being used by this layer                           *
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
void LayerClass::Set_Camera(CameraClass * cam) 
{
	if (Camera) {
		Camera->Release_Ref();
	}
	Camera = cam;
	if (Camera) {
		Camera->Add_Ref();
	}
}


/***********************************************************************************************
 * LayerClass::Get_Camera -- get the camera being used by this layer                           *
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
CameraClass * LayerClass::Get_Camera(void) const
{
	if (Camera) {
		Camera->Add_Ref();
	}
	return Camera;
}


/*********************************************************************************************** 
 * LC::Peek_Camera -- Get copy of camera.                                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/14/2001 SKB : Created.                                                                 * 
 *=============================================================================================*/
CameraClass * LayerClass::Peek_Camera(void) const
{
	return Camera;
}


/*********************************************************************************************** 
 * LC::Set -- Kinda like an assignment operator.                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/14/2001 SKB : Created.                                                                 * 
 *=============================================================================================*/
void LayerClass::Set(const LayerClass & layer)
{
	Set_Camera(layer.Peek_Camera());
	Set_Scene(layer.Peek_Scene());
	Clear = layer.Clear;
	ClearZ = layer.ClearZ;
	ClearColor = layer.ClearColor;
}
