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

/* $Header: /Commando/Code/Tools/max2w3d/w3ddesc.h 3     3/04/99 1:57p Naty_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3ddesc.h                      $*
 *                                                                                             *
 *                      $Author:: Naty_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/03/99 11:28a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef W3DDESC_H
#define W3DDESC_H

#include "always.h"
#include <max.h>

/*****************************************************************************
*
*  Class descriptors provide the system with information about the plug-in
*  classes in the DLL.
*
*****************************************************************************/
#define W3D_EXPORTER_CLASS_ID Class_ID(0x54d412df, 0x41466ae8)

class W3dClassDesc : public ClassDesc
{
public:
	void *			Create(BOOL);
	int				IsPublic();
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID			ClassID();
	const TCHAR *	Category();
};


#endif
