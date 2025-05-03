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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/win.h                                  $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 6/26/01 1:59p                                               $*
 *                                                                                             * 
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef WIN_H
#define WIN_H

/*
**	This header file includes the Windows headers. If there are any special pragmas that need
**	to occur around this process, they are performed here. Typically, certain warnings will need
**	to be disabled since the Windows headers are repleat with illegal and dangerous constructs.
**
**	Within the windows headers themselves, Microsoft has disabled the warnings 4290, 4514, 
**	4069, 4200, 4237, 4103, 4001, 4035, 4164. Makes you wonder, eh?
*/

// When including windows, lets just bump the warning level back to 3...
#if (_MSC_VER >= 1200)
#pragma warning(push, 3)
#endif

// this define should also be in the DSP just in case someone includes windows stuff directly
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include	<windows.h>
//#include <mmsystem.h>
//#include	<windowsx.h>
//#include	<winnt.h>
//#include	<winuser.h>

#if (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

#ifdef _WINDOWS
extern HINSTANCE	ProgramInstance;
extern HWND			MainWindow;
extern bool GameInFocus;

#ifdef RTS_DEBUG

void __cdecl Print_Win32Error(unsigned long win32Error);

#else // RTS_DEBUG

#define Print_Win32Error

#endif // RTS_DEBUG

#else // _WINDOWS
#include <unistd.h>
#endif // _WINDOWS

#endif // WIN_H
