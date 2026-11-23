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

/* $Header: /Commando/Code/Tools/max2w3d/w3ddesc.cpp 3     3/04/99 1:58p Naty_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3ddesc.cpp                    $*
 *                                                                                             *
 *                      $Author:: Naty_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/03/99 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "w3ddesc.h"
#include "w3dexp.h"
#include "dllmain.h"

// MAX calls this method when it needs a pointer to a new instance of the plug-in class.
void * W3dClassDesc::Create(BOOL)
{
	return new W3dExportClass;
}

// Controls if the plug-in shows up in lists from the user to choose from.
int W3dClassDesc::IsPublic()
{
	return 1;
}

// This method returns the name of the class. This name appears in the button for the plug-in in the
// MAX user interface.
const TCHAR * W3dClassDesc::ClassName()
{
	return Get_String(IDS_W3D_SHORT_DESCRIPTION);
}

SClass_ID W3dClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}

// This method must return the unique ID for the object. If two ClassIDs conflict, the system will
// only load the first one it finds. The ClassID consists of two unsigned 32-bit quantities.
Class_ID	W3dClassDesc::ClassID()
{
	return W3D_EXPORTER_CLASS_ID;
}

// Returns a string describing the category a plug-in fits into.
const TCHAR * W3dClassDesc::Category()
{
	return Get_String(IDS_SCENE_EXPORT);
}

