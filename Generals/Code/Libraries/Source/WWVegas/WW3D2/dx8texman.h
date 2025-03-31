/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/ww3d2/dx8texman.h                            $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/26/01 5:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX8TEXTUREMANAGER_H
#define DX8TEXTUREMANAGER_H

#include "always.h"
#include "texture.h"
#include "dx8wrapper.h"
#include "ww3dformat.h"
#include "dx8list.h"
#include "ww3dformat.h"
#include "multilist.h"

class DX8TextureManagerClass;

class DX8TextureTrackerClass : public MultiListObjectClass
{
friend DX8TextureManagerClass;
public:
	DX8TextureTrackerClass(unsigned int w, unsigned int h, WW3DFormat format,
		MipCountType count,bool rt,
		TextureClass *tex):
	Width(w),
	Height(h),
	Format(format),
	Mip_level_count(count),
	RenderTarget(rt),
	Texture(tex)	
	{
	}
private:
	unsigned int Width;
	unsigned int Height;
	WW3DFormat Format;
	MipCountType Mip_level_count;
	bool RenderTarget;
	TextureClass *Texture;
};

class DX8TextureManagerClass
{
public:
	static void Shutdown();
	static void Add(DX8TextureTrackerClass *track);
	static void Remove(TextureClass *tex);
	static void Release_Textures();
	static void Recreate_Textures();
private:
	static DX8TextureTrackerList Managed_Textures;
};

#endif // ifdef TEXTUREMANAGER