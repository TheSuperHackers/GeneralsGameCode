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

/* $Header: /Commando/Code/Tools/max2w3d/rcmenu.h 3     1/14/98 10:23a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/rcmenu.h                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/13/98 3:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "max.h"
#include "dllmain.h"
#include "resource.h"
#include "istdplug.h"

class W3DUtilityClass;

/**********************************************************************************************
**
** RCMenuClass - W3D Utility's right-click menu.
**
**********************************************************************************************/
class RCMenuClass : public RightClickMenu
{

public:

	RCMenuClass() {Installed=FALSE;}
	~RCMenuClass() {}

	void Bind(Interface * ipi, W3DUtilityClass * eni) { InterfacePtr = ipi; UtilityPtr = eni; }

	void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
	void Selected(UINT id);
	void Toggle_Hierarchy(INode * node);
	void Toggle_Geometry(INode * node);

public:

	BOOL Installed;

private:

	Interface *				InterfacePtr;
	W3DUtilityClass *		UtilityPtr;
	INode *					SelNode;

	enum {
		MENU_SEPARATOR = 0,
		MENU_TOGGLE_HIERARCHY,
		MENU_TOGGLE_GEOMETRY,
		MENU_NODE_NAME,
		MENU_NODE_POINTER
	};
};

extern RCMenuClass TheRCMenu;
