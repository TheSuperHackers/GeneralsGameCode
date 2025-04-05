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

// FILE: debug.cpp ////////////////////////////////////////////////////////
// Minmal debug info
// Author: Matthew D. Campbell, Sept 2002

#include <windows.h>
#include "debug.h"
#include <cstdio>

#ifdef DEBUG

void DebugLog(const char *fmt, ...)
{
	static char buffer[1024];
	va_list va;
	va_start( va, fmt );
	vsnprintf(buffer, 1024, fmt, va );
	buffer[1023] = 0;
	va_end( va );

	printf( "%s", buffer );
	OutputDebugString( buffer );
}

#endif // DEBUG

