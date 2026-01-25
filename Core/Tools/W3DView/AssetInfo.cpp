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
 *                     $Archive:: /Commando/Code/Tools/W3DView/AssetInfo.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/00 4:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "AssetInfo.h"
//#include "HModel.h"
#include "assetmgr.h"
#include "htree.h"

/////////////////////////////////////////////////////////////////
//
//	Initialize
//
void
AssetInfoClass::Initialize (void)
{
	// If this isn't a material, then try to get its hierarchy name (if there is one)
	if (m_AssetType != TypeMaterial) {

		// Assume we are wrapping an instance as apposed to an asset 'name'.
		RenderObjClass *prender_obj = m_pRenderObj;
		if (prender_obj)
			prender_obj->Add_Ref();

		// If we are wrapping an asset name, then create an instance of it.
		if (prender_obj == nullptr) {
			prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (m_Name);
		}

		if (prender_obj != nullptr) {

			// Get the hierarchy tree for this object (if one exists)
			const HTreeClass *phtree = prender_obj->Get_HTree ();
			if (phtree) {

				// Get the name of the hierarchy tree
				m_HierarchyName = phtree->Get_Name ();
			}
		}

		// Release our hold on the temporary object
		REF_PTR_RELEASE (prender_obj);
	}

	return ;
}


