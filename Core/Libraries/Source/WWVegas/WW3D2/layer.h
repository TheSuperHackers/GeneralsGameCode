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
 *                     $Archive:: /VSS_Sync/ww3d2/layer.h                                     $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "LISTNODE.H"
#include "vector3.h"

class SceneClass;
class CameraClass;

class LayerClass;
typedef Node<LayerClass *> LayerNodeClass;

class LayerClass : public LayerNodeClass
{

public:

	LayerClass(void);
	LayerClass(SceneClass * s,CameraClass * c,bool clear = false,bool clearz = false,const Vector3 & color = Vector3(0,0,0));
	LayerClass(const LayerClass & src);
	~LayerClass(void);


	/*
	** The following functions will handle the references of the Scene and Camera
	** objects properly.
	*/
	void						Set_Scene(SceneClass * scene);
	SceneClass *			Get_Scene(void) const;
	SceneClass *			Peek_Scene(void) const;
	void						Set_Camera(CameraClass * cam);
	CameraClass *			Get_Camera(void) const;
	CameraClass *			Peek_Camera(void) const;


	// [SKB: Aug 14 2001 @ 1:53pm] :
	// Add a method to copy one layer to another - I would like to create an assignment
	// operator but it could break old code.
	void						Set(const LayerClass & layer);

	/*
	** LayerClass members are public since this is a "lightweight" class
	*/
	bool						Clear;
	bool						ClearZ;
	Vector3					ClearColor;

	SceneClass *			Scene;
	CameraClass *			Camera;

};

typedef List<LayerClass *> LayerListClass;
