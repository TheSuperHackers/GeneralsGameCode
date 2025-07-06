/*
**	Command & Conquer Generals(tm)
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

// FILE: PersistentStorageDefs.h //////////////////////////////////////////////////////
// Generals GameSpy Persistent Storage definitions
// Author: Matthew D. Campbell, July 2002

#pragma once

#ifndef __PERSISTENTSTORAGEDEFS_H__
#define __PERSISTENTSTORAGEDEFS_H__

enum LocaleType CPP_11(: Int)
{
    LOC_UNKNOWN = 0,
    LOC_MIN = 1,
    LOC_MAX = 37
};

void HandlePersistentStorageResponses(void);
void UpdateLocalPlayerStats(void);

void SetLookAtPlayer( Int id, AsciiString nick );
void PopulatePlayerInfoWindows( AsciiString parentWindowName );

#endif // __PERSISTENTSTORAGEDEFS_H__
