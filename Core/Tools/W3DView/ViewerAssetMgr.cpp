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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/ViewerAssetMgr.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/27/01 1:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "ViewerAssetMgr.h"
#include "texture.h"
#include "ww3d.h"
#include "Utils.h"


////////////////////////////////////////////////////////////////////////
//
//	Load_3D_Assets
//
////////////////////////////////////////////////////////////////////////
bool
ViewerAssetMgrClass::Load_3D_Assets (FileClass &w3dfile)
{
	//
	//	Allow the base class to process
	//
	bool retval = WW3DAssetManager::Load_3D_Assets (w3dfile);
	if (retval) {
		
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_Texture
//
////////////////////////////////////////////////////////////////////////
TextureClass *
ViewerAssetMgrClass::Get_Texture (const char * tga_filename, MipCountType mip_level_count)
{
	//
	// See if the texture has already been loaded.
	//	

	StringClass lower_case_name(tga_filename,true);
	_strlwr(lower_case_name.Peek_Buffer());
	TextureClass* tex = TextureHash.Get(lower_case_name);	

	//
	//	Check to see if this texture is "missing"
	//
	if (!tex) {
		Find_Missing_Textures  (m_MissingTextureList, tga_filename);
	}

	//
	//	Create the texture
	//
	TextureClass * texture = WW3DAssetManager::Get_Texture(tga_filename, mip_level_count);
	return texture;
}


////////////////////////////////////////////////////////////////////////
//
//	Open_Texture_File_Cache
//
////////////////////////////////////////////////////////////////////////
void
ViewerAssetMgrClass::Open_Texture_File_Cache (const char * /*prefix*/)
{
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Close_Texture_File_Cache
//
////////////////////////////////////////////////////////////////////////
void
ViewerAssetMgrClass::Close_Texture_File_Cache (void)
{
	return ;
}
