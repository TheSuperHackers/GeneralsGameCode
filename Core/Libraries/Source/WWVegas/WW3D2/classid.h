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

/***************************************************************************
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Commando	                              *
 *                                                                         *
 *                     $Archive:: /Commando/Code/ww3d2/classid.h          $*
 *                                                                         *
 *                      $Author:: Jani_p                                  $*
 *                                                                         *
 *                     $Modtime:: 3/29/01 1:13a                           $*
 *                                                                         *
 *                    $Revision:: 2                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*/


#pragma once

#ifndef CLASSID_H
#define CLASSID_H

#include "always.h"

/*
** enum of all the WW3D class IDs.
*/
enum
{
	ID_INDIRECT_TEXTURE_CLASS = 0x10000,	// IndirectTextureClass					"texture.h"
	ID_VARIABLE_TEXTURE_CLASS,					// VariableTextureClass					"texture.h"
	ID_FILE_LIST_TEXTURE_CLASS,				// FileListTextureClass					"texture.h"
	ID_RESIZEABLE_TEXTURE_INSTANCE_CLASS,	// ResizeableTextureInstanceClass	"texture.h"
	ID_ANIM_TEXTURE_INSTANCE_CLASS,			// AnimTextureInstanceClass			"texture.h"
	ID_MANUAL_ANIM_TEXTURE_INSTANCE_CLASS,	// ManualAnimTextureInstanceClass	"texture.h"
	ID_TIME_ANIM_TEXTURE_INSTANCE_CLASS,	// TimeAnimTextureInstanceClass		"texture.h"
	ID_POINT_GROUP_CLASS,						// PointGroupClass						"pointgr.h"
	ID_MESH_MODEL_CLASS,							// MeshModelClass							"mesh.cpp"
	ID_CACHED_TEXTURE_FILE_CLASS,				// CachedTextureFileClass				"assetmgr.cpp"
	ID_STREAMING_TEXTURE_CLASS,				// StreamingTextureClass				"texture.h"
	ID_STREAMING_TEXTURE_INSTANCE_CLASS,	// StreamingTextureInstanceClass		"texture.h"
};


#endif
