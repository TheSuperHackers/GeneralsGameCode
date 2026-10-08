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

#pragma once

#include "Lib/Debug.h"

class AsciiString;

#ifdef ALLOW_DEBUG_UTILS

	// Initializes the debug utilities for an application that uses the game engine.
	DEBUG_EXTERN_C void GameDebugInit(int flags);

	#define GAME_DEBUG_INIT(f)			do { GameDebugInit(f); } while (0)

#else

	#define GAME_DEBUG_INIT(f)			((void)0)

#endif

DEBUG_EXTERN_C void ReleaseCrash(const char* reason);
DEBUG_EXTERN_C void ReleaseCrashLocalized(const AsciiString& p, const AsciiString& m);

#define RELEASE_CRASH(m)				do { ReleaseCrash(m); } while (0)
#define RELEASE_CRASHLOCALIZED(p, m)		do { ReleaseCrashLocalized(p, m); } while (0)
