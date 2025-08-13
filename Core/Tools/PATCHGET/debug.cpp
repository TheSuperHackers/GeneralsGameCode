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
#include <cstdarg>
#include <cstdio>

namespace patchget
{

char* TheCurrentIgnoreCrashPtr;

#define LARGE_BUFFER	8192
static char theBuffer[ LARGE_BUFFER ];	// make it big to avoid weird overflow bugs in debug mode

static int doCrashBox(const char *buffer, bool logResult)
{
	int result;

	result = ::MessageBox(NULL, buffer, "Assertion Failure", MB_ABORTRETRYIGNORE|MB_APPLMODAL|MB_ICONWARNING);

	switch(result)
	{
		case IDABORT:
#ifdef DEBUG_LOGGING
			if (logResult)
				DebugLog("[Abort]");
#endif
			_exit(1);
			break;
		case IDRETRY:
#ifdef DEBUG_LOGGING
			if (logResult)
				DebugLog("[Retry]");
#endif
			::DebugBreak();
			break;
		case IDIGNORE:
#ifdef DEBUG_LOGGING
			// do nothing, just keep going
			if (logResult)
				DebugLog("[Ignore]");
#endif
			break;
	}
	return result;
}

#if defined(DEBUG) || defined(DEBUG_LOGGING)

void DebugLog(const char *fmt, ...)
{
	va_list va;
	va_start( va, fmt );
	vsnprintf(theBuffer, LARGE_BUFFER, fmt, va );
	theBuffer[LARGE_BUFFER-1] = 0;
	va_end( va );

	OutputDebugString(theBuffer);
	OutputDebugString("\n");
	printf( "%s\n", theBuffer );
}

#endif // defined(DEBUG) || defined(DEBUG_LOGGING)

#ifdef DEBUG_CRASHING

void DebugCrash(const char *format, ...)
{
	theBuffer[0] = 0;
	strcat(theBuffer, "ASSERTION FAILURE: ");

	va_list arg;
  va_start(arg, format);
  vsprintf(theBuffer + strlen(theBuffer), format, arg);
  va_end(arg);

	if (strlen(theBuffer) >= sizeof(theBuffer))
		::MessageBox(NULL, "String too long for debug buffers", "", MB_OK|MB_APPLMODAL);

	OutputDebugString(theBuffer);
	OutputDebugString("\n");
	printf( "%s\n", theBuffer );

	strcat(theBuffer, "\n\nAbort->exception; Retry->debugger; Ignore->continue");

	int result = doCrashBox(theBuffer, true);

	if (result == IDIGNORE && TheCurrentIgnoreCrashPtr != NULL)
	{
		int yn;
		yn = ::MessageBox(NULL, "Ignore this crash from now on?", "", MB_YESNO|MB_APPLMODAL);
		if (yn == IDYES)
			*TheCurrentIgnoreCrashPtr = 1;
	}
}

#endif

} // namespace patchget
