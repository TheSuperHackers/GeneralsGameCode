/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

// TheSuperHackers @build feliwir 15/04/2025 Simple debug implementation useful for tools

#include "wwdebug.h"
#include <stdarg.h>
#include <stdlib.h>
#include <Utility/stdio_adapter.h>

char* TheCurrentIgnoreCrashPtr = nullptr;


#ifdef DEBUG_LOGGING

void DebugLog(const char *format, ...)
{
	// Print it to the console

	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
	printf("\n");
}

#endif

#ifdef DEBUG_CRASHING

void DebugCrash(const char *format, ...)
{
	// Print it to the console

	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
	printf("\n");

	// No exit in this stub
}

#endif
