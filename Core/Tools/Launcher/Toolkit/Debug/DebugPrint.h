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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     Debug printing mechanism
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#ifndef _DEBUGPRINT_H_
#define _DEBUGPRINT_H_

#ifdef _DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

//! Ouput debug print messages to the debugger and log file.
void __cdecl DebugPrint(const char* string, ...);
void __cdecl PrintWin32Error(const char* string, ...);

extern char debugLogName[];


#ifdef __cplusplus
}
#endif

#else // _DEBUG

#define DebugPrint
#define PrintWin32Error

#endif // _DEBUG

#endif // _DEBUGPRINT_H_
