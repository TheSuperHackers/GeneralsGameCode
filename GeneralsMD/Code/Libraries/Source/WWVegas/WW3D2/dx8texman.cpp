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
 *                 Project Name : DX8 Texture Manager                                          *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8texman.cpp                          $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 * 06/27/02 KM Texture class abstraction																			*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DX8TextureManagerClass::Shutdown -- Shuts down the texture manager                        *
 *   DX8TextureManagerClass::Add -- Adds a texture to be managed                               *
 *   DX8TextureManagerClass::Remove -- Removes a texture from being managed                    *
 *   DX8TextureManagerClass::Release_Textures -- Releases the internal d3d texture             *
 *   DX8TextureManagerClass::Recreate_Textures -- Reallocates lost textures                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// This class manages textures that are in the default pool
// ensuring that they are released on device loss
// and created on device reset

// Note: It does NOT addref to textures because it is called in the texture
// destructor

#include "dx8texman.h"

TextureTrackerList DX8TextureManagerClass::Managed_Textures;


/***********************************************************************************************
 * DX8TextureManagerClass::Shutdown -- Shuts down the texture manager                          *
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
 *   4/25/2001  hy : Created.                                                                  *
 *   5/16/2002  km : Added depth stencil texture tracking and abstraction                      *
 *=============================================================================================*/
void DX8TextureManagerClass::Shutdown()
{
	while (!Managed_Textures.Is_Empty())
	{
		TextureTrackerClass *track=Managed_Textures.Remove_Head();
		delete track;
		track=NULL;
	}
}

/***********************************************************************************************
 * DX8TextureManagerClass::Add -- Adds a texture to be managed                                 *
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
 *   4/25/2001  hy : Created.                                                                  *
 *   5/16/2002  km : Added depth stencil texture tracking and abstraction                      *
 *=============================================================================================*/
void DX8TextureManagerClass::Add(TextureTrackerClass *track)
{
	// this function should only be called by the texture constructor
	Managed_Textures.Add(track);
}


/***********************************************************************************************
 * DX8TextureManagerClass::Remove -- Removes a texture from being managed                      *
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
 *   4/25/2001  hy : Created.                                                                  *
 *   5/16/2002  km : Added depth stencil texture tracking and abstraction                      *
 *=============================================================================================*/
void DX8TextureManagerClass::Remove(TextureBaseClass *tex)
{
	// this function should only be called by the texture destructor
	TextureTrackerListIterator it(&Managed_Textures);

	while (!it.Is_Done())
	{
		TextureTrackerClass *track=it.Peek_Obj();
		if (track->Get_Texture()==tex)
		{
			it.Remove_Current_Object();
			delete track;
			break;
		}
		it.Next();
	}
}


/***********************************************************************************************
 * DX8TextureManagerClass::Release_Textures -- Releases the internal d3d texture               *
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
 *   4/25/2001  hy : Created.                                                                  *
 *   5/16/2002  km : Added depth stencil texture tracking and abstraction                      *
 *=============================================================================================*/
void DX8TextureManagerClass::Release_Textures()
{
	TextureTrackerListIterator it(&Managed_Textures);

	while (!it.Is_Done())
	{
		TextureTrackerClass *track=it.Peek_Obj();
		track->Release();
		it.Next();
	}
}


/***********************************************************************************************
 * DX8TextureManagerClass::Recreate_Textures -- Reallocates lost textures                      *
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
 *   4/25/2001  hy : Created.                                                                  *
 *   5/16/2002  km : Added depth stencil texture tracking and abstraction                      *
 *=============================================================================================*/
void DX8TextureManagerClass::Recreate_Textures()
{
	TextureTrackerListIterator it(&Managed_Textures);

	while (!it.Is_Done())
	{
		TextureTrackerClass *track=it.Peek_Obj();
		track->Recreate();
		track->Get_Texture()->Set_Dirty();
		it.Next();
	}
}

