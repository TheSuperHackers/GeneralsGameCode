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
 *                     $Archive:: /Commando/Code/ww3d2/ww3dids.h                              $*
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

#include "saveloadids.h"

/*
** These are the chunk-id's used by all persistent objects in WW3D.  The persistent object
** framework is defined in the WWSaveLoad library.
**
** Sept 23, 1999
** - Initial implementation of making the Commando engine persistent included making some
**   of WW3D persistent.  For this initial implementation, we're going to assume that we
**   can re-create all of our game objects from the asset manager and patch up any state
**   changes with custom game object code.  Therefore, the base class RenderObjClass has
**   a persist manager which simply saves the name of the render object and its transform
**   and re-creates that render object from the asset manager.
**
** - Currently lights are procedurally created rather than created from the asset manager.
**   For this reason, I'm implementing a persist manager and save-load functionality for
**   light class.  In the future lights may come from the asset manager in which case we
**   could remove this code.
**
** - It is also possible that later on when we make the save game stuff more robust we may
**   need to implement actual save load calls for more render objects but hopefully we can
**   avoid that since it will at least double the size of our files and just seems like a
**   lot of work to solve a few specific problems.
*/
enum
{
	WW3D_PERSIST_CHUNKID_RENDEROBJ		= CHUNKID_WW3D_BEGIN,
	WW3D_PERSIST_CHUNKID_LIGHT,
	WW3D_PERSIST_CHUNKID_DAZZLE,
};
