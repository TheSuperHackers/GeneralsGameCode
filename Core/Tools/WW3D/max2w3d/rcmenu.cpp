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

/* $Header: /Commando/Code/Tools/max2w3d/rcmenu.cpp 6     4/19/00 12:24p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/rcmenu.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/18/00 8:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RCMenuClass::Init -- initialize the "right-click" menu                                    *
 *   RCMenuClass::Selected -- menu selection callback                                          *
 *   RCMenuClass::Toggle_Hierarchy -- toggle the "export hierarchy" option                     *
 *   RCMenuClass::Toggle_Geometry -- toggle the "export geometry" option                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rcmenu.h"
#include "w3dutil.h"
#include "util.h"


RCMenuClass TheRCMenu;

/***********************************************************************************************
 * RCMenuClass::Init -- initialize the "right-click" menu                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void RCMenuClass::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m)
{
	Installed=TRUE;

	SelNode = InterfacePtr->PickNode(hWnd,m);

	if (SelNode) {

		UINT menuflags;
		W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(SelNode);

		/*
		** Add the menu separator
		*/
		manager->AddMenu(this, MF_SEPARATOR, MENU_SEPARATOR, nullptr);

		/*
		** Add the Name of the object
		*/
		char string[64];
		sprintf(string,"%s:",SelNode->GetName());
		manager->AddMenu(this, MF_STRING | MF_DISABLED, MENU_NODE_NAME, string);

		/*
		** Add the pointer
		*/
//		sprintf(string,"0x%X",(unsigned long)SelNode);
//		manager->AddMenu(this, MF_STRING | MF_GRAYED, MENU_NODE_POINTER, string);

		/*
		** Add the hierarchy menu option
		*/
		if (wdata->Is_Bone()) {
			menuflags = MF_STRING | MF_CHECKED;
		} else {
			menuflags = MF_STRING;
		}
		manager->AddMenu(this, menuflags, MENU_TOGGLE_HIERARCHY, "W3D: Export Hierarchy");

		/*
		** Add the geometry menu option
		*/
		if (wdata->Is_Geometry()) {
			menuflags = MF_STRING | MF_CHECKED;
		} else {
			menuflags = MF_STRING;
		}
		manager->AddMenu(this, menuflags, MENU_TOGGLE_GEOMETRY, "W3D: Export Geometry");

	}
}

/***********************************************************************************************
 * RCMenuClass::Selected -- menu selection callback                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void RCMenuClass::Selected(UINT id)
{
	switch (id) {

		case MENU_TOGGLE_HIERARCHY:
			Toggle_Hierarchy(SelNode);
			break;
		case MENU_TOGGLE_GEOMETRY:
			Toggle_Geometry(SelNode);
			break;

	}
}

/***********************************************************************************************
 * RCMenuClass::Toggle_Hierarchy -- toggle the "export hierarchy" option                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void RCMenuClass::Toggle_Hierarchy(INode * node)
{

	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(SelNode);
	assert(wdata);

	if (wdata->Is_Bone()) {
		wdata->Enable_Export_Transform(false);
	} else {
		wdata->Enable_Export_Transform(true);
	}
}

/***********************************************************************************************
 * RCMenuClass::Toggle_Geometry -- toggle the "export geometry" option                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void RCMenuClass::Toggle_Geometry(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(SelNode);
	assert(wdata);

	if (wdata->Is_Geometry()) {
		wdata->Enable_Export_Geometry(false);
	} else {
		wdata->Enable_Export_Geometry(true);
	}
}


