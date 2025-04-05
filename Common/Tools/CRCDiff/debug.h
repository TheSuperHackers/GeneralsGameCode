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

// FILE: debug.h //////////////////////////////////////////////////////////////
// Minimal debug info
// Author: Matthew D. Campbell, Sept 2002

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG

#include <cstdarg>

void DebugLog( const char *fmt, ... );
#define DEBUG_LOG(x) DebugLog x

#else // DEBUG

#define DEBUG_LOG(x) {}

#endif // DEBUG

#endif // __DEBUG_H__

