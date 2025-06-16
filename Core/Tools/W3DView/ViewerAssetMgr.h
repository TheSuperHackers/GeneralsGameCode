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
 *                     $Archive:: /Commando/Code/Tools/W3DView/ViewerAssetMgr.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/12/01 3:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VIEWER_ASSETMGR_H
#define __VIEWER_ASSETMGR_H

#include "assetmgr.h"


/////////////////////////////////////////////////////////////////////////////
//
// ViewerAssetMgrClass
//
/////////////////////////////////////////////////////////////////////////////
class ViewerAssetMgrClass : public WW3DAssetManager
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	ViewerAssetMgrClass (void) {}
	virtual ~ViewerAssetMgrClass (void) {}
	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	// Base class overrides
	//
	virtual bool						Load_3D_Assets (FileClass &w3dfile);
	virtual TextureClass *			Get_Texture(const char * filename, MipCountType mip_level_count=MIP_LEVELS_ALL);

	//
	//	Missing texture methods
	//
	void									Start_Tracking_Textures (void)	{ m_MissingTextureList.Delete_All (); }
	DynamicVectorClass<CString> &	Get_Missing_Texture_List (void)	{ return m_MissingTextureList; }

	//
	//	Texture caching overrides
	//
	virtual void						Open_Texture_File_Cache(const char * /*prefix*/);
	virtual void						Close_Texture_File_Cache();


private:

	///////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////
	DynamicVectorClass<CString>	m_MissingTextureList;
};


#endif //__VIEWER_ASSETMGR_H
