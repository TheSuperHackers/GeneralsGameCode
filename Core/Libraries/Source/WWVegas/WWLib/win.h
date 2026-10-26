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
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/16/01 2:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifdef _WIN32

// When including windows, lets just bump the warning level back to 3...
#if (_MSC_VER >= 1200)
#pragma warning(push, 3)
#endif

#include <windows.h>
#include <mmsystem.h>    // for timeGetTime
// #include	<windowsx.h>
// #include	<winnt.h>
// #include	<winuser.h>

#if (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

extern HINSTANCE ProgramInstance;
extern HWND MainWindow;
extern bool GameInFocus;

#ifdef RTS_DEBUG

void __cdecl Print_Win32Error(unsigned long win32Error);

#else    // RTS_DEBUG

#define Print_Win32Error

#endif    // RTS_DEBUG

#else    // _WIN32
// #include <unistd.h>	// file does not exist
#endif    // _WIN32
