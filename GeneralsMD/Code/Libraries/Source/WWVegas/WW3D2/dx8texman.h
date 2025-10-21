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
 *                     $Archive:: /Commando/Code/ww3d2/dx8texman.h                            $*
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
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

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

class TextureTrackerClass : public MultiListObjectClass
{
public:
	TextureTrackerClass
	(
		unsigned int w,
		unsigned int h,
		MipCountType count,
		TextureBaseClass *tex
	)
	: Width(w),
	  Height(h),
	  Mip_level_count(count),
	  Texture(tex)
	{
	}

	virtual void Recreate() const =0;

	void Release()
	{
		Texture->Set_D3D_Base_Texture(NULL);
	}

	TextureBaseClass* Get_Texture() const { return Texture; }


protected:

	unsigned int Width;
	unsigned int Height;
	MipCountType Mip_level_count;
	TextureBaseClass *Texture;
};

class DX8TextureTrackerClass : public TextureTrackerClass
{
public:
	DX8TextureTrackerClass
	(
		unsigned int w,
		unsigned int h,
		WW3DFormat format,
		MipCountType count,
		TextureBaseClass *tex,
		bool rt
	)
	: TextureTrackerClass(w,h,count,tex), Format(format), RenderTarget(rt)
	{
	}

	virtual void Recreate() const
	{
		WWASSERT(Texture->Peek_D3D_Base_Texture()==NULL);
		Texture->Poke_Texture
		(
			DX8Wrapper::_Create_DX8_Texture
			(
				Width,
				Height,
				Format,
				Mip_level_count,
				D3DPOOL_DEFAULT,
				RenderTarget
			)
		);
	}

private:
	WW3DFormat Format;
	bool RenderTarget;
};

class DX8ZTextureTrackerClass : public TextureTrackerClass
{
public:
	DX8ZTextureTrackerClass
	(
		unsigned int w,
		unsigned int h,
		WW3DZFormat zformat,
		MipCountType count,
		TextureBaseClass* tex
	)
	: TextureTrackerClass(w,h,count,tex), ZFormat(zformat)
	{
	}

	virtual void Recreate() const
	{
		WWASSERT(Texture->Peek_D3D_Base_Texture()==NULL);
		Texture->Poke_Texture
		(
			DX8Wrapper::_Create_DX8_ZTexture
			(
				Width,
				Height,
				ZFormat,
				Mip_level_count,
				D3DPOOL_DEFAULT
			)
		);
	}


private:
	WW3DZFormat ZFormat;
};


class DX8TextureManagerClass
{
public:
	static void Shutdown();
	static void Add(TextureTrackerClass *track);
	static void Remove(TextureBaseClass *tex);
	static void Release_Textures();
	static void Recreate_Textures();
private:
	static TextureTrackerList Managed_Textures;
};

#endif // ifdef TEXTUREMANAGER
