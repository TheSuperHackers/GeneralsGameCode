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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: CDCheck.h ////////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, January 2003
// Description: check for CD, popping up an in-game message box at game start.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __CDCHECK_H_
#define __CDCHECK_H_

typedef void (*gameStartCallback) (void);

Bool IsFirstCDPresent(void);
void CheckForCDAtGameStart( gameStartCallback callback );

#endif //__CDCHECK_H_
