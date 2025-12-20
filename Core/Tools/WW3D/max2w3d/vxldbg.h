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

/* $Header: /Commando/Code/Tools/max2w3d/vxldbg.h 3     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Engine                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vxldbg.h                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/14/97 3:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ALWAYS_H
#include "always.h"
#endif

#include <max.h>

#ifndef SIMPDIB_H
#include "simpdib.h"
#endif

#ifndef VXL_H
#include "vxl.h"
#endif


class VoxelDebugWindowClass
{
public:

	VoxelDebugWindowClass(VoxelClass * vxl);
	~VoxelDebugWindowClass(void);

	void	Display_Window(void);
	bool	Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);

private:

	int						CurLayer;

	SimpleDIBClass *		Bitmap;
	VoxelClass *			Voxel;
	HWND						WindowHWND;
	HWND						ViewportHWND;
	ISpinnerControl *		LayerSpin;

	void update_display(void);
};
