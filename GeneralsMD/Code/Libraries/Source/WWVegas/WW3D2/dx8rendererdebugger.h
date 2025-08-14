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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8rendererdebugger.h                  $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/06/01 6:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX8_RENDERER_DEBUGGER_H
#define DX8_RENDERER_DEBUGGER_H

#include "always.h"

class StringClass;
class MeshClass;

// Note! For the debugger to be usable, the application must call DX8RendererDebugger::Update() once
// each frame.

class DX8RendererDebugger
{
	static bool Enabled;
public:
	static void Enable(bool enable);
	WWINLINE static bool Is_Enabled() { return Enabled; }
	static void Get_String(StringClass& s);
	static void Update();
#ifdef WWDEBUG
	static void Add_Mesh(MeshClass* mesh);
#else
	static void Add_Mesh(MeshClass* mesh) {}
#endif

	static void Disable_Mesh(unsigned id);
	static void Enable_Mesh(unsigned id);
	static void Disable_All();
	static void Enable_All();
};

#endif
