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

// FILE: Debug.cpp
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Debug.cpp
//
// Created:   Steven Johnson, August 2001
//
// Desc:      Debug logging and other debug utilities
//
// ----------------------------------------------------------------------------

// SYSTEM INCLUDES
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Utility/stdio_adapter.h>
#include <Utility/stringex.h>

// USER INCLUDES
#define DEBUG_THREADSAFE
#include "Lib/Debug.h"


// ----------------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------------

#ifdef DEBUG_LOGGING

#if defined(RTS_DEBUG)
	#define DEBUG_FILE_NAME				"DebugLogFileD"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevD"
#else
	#define DEBUG_FILE_NAME				"DebugLogFile"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrev"
#endif

#endif

// ----------------------------------------------------------------------------
// PRIVATE TYPES
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// PRIVATE DATA
// ----------------------------------------------------------------------------
// TheSuperHackers @info Must not use static RAII types when set in DebugInit,
// because DebugInit can be called during static module initialization before the main function is called.
#ifdef DEBUG_LOGGING
static FILE *theLogFile = nullptr;
static char theLogFileName[ _MAX_PATH ];
static char theLogFileNamePrev[ _MAX_PATH ];
#endif
#define LARGE_BUFFER	8192
static char theBuffer[ LARGE_BUFFER ];	// make it big to avoid weird overflow bugs in debug mode
static int theDebugFlags = 0;
static DWORD theMainThreadID = 0;
static DebugCrashHandler theCrashHandler = nullptr;
#ifdef ALLOW_DEBUG_UTILS
static DebugIgnoreAssertsQuery theIgnoreAssertsQuery = nullptr;
static DebugMainWindowQuery theMainWindowQuery = nullptr;
static DebugStackDumpHandler theStackDumpHandler = nullptr;
static DebugCrashIgnoredHandler theCrashIgnoredHandler = nullptr;
static DebugLogHandler theLogHandler = nullptr;
#endif
#if defined(DEBUG_LOGGING) && defined(DEBUG_THREADSAFE)
// Created in DebugInit and never deleted, because other threads can be inside a log function
// when DebugShutdown is called.
static CRITICAL_SECTION theLogCriticalSection;
static bool theLogCriticalSectionInitialized = false;
#endif
// ----------------------------------------------------------------------------
// PUBLIC DATA
// ----------------------------------------------------------------------------

char* TheCurrentIgnoreCrashPtr = nullptr;
#ifdef DEBUG_LOGGING
UnsignedInt DebugLevelMask = 0;
const char *TheDebugLevels[DEBUG_LEVEL_MAX] = {
	"NET"
};
#endif

// ----------------------------------------------------------------------------
// PRIVATE PROTOTYPES
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString();
static const char *getCurrentTickString();
static void prepBuffer(char *buffer);
#ifdef DEBUG_LOGGING
static void doLogOutput(const char *buffer);
static void doLogOutput(const char *buffer, const char *endline);
#endif
#ifdef DEBUG_CRASHING
static int doCrashBox(const char *buffer, Bool logResult);
#endif
static void whackFunnyCharacters(char *buf);
#ifdef DEBUG_STACKTRACE
static void doStackDump();
#endif

// ----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------------

#ifdef ALLOW_DEBUG_UTILS
// ----------------------------------------------------------------------------
inline Bool ignoringAsserts()
{
	return theIgnoreAssertsQuery != nullptr && theIgnoreAssertsQuery();
}

// ----------------------------------------------------------------------------
inline HWND getThreadHWND()
{
	if (theMainThreadID == GetCurrentThreadId() && theMainWindowQuery != nullptr)
		return (HWND)theMainWindowQuery();

	return nullptr;
}

// ----------------------------------------------------------------------------

static int MessageBoxWrapper( LPCSTR lpText, LPCSTR lpCaption, UINT uType )
{
	HWND threadHWND = getThreadHWND();
	return ::MessageBox(threadHWND, lpText, lpCaption, uType);
}
#endif

#if defined(DEBUG_LOGGING) && defined(DEBUG_THREADSAFE)
// ----------------------------------------------------------------------------
// Serializes the log functions. Does nothing before DebugInit.
class ScopedLogLock
{
public:
	ScopedLogLock() : m_isLocked(theLogCriticalSectionInitialized)
	{
		if (m_isLocked)
			::EnterCriticalSection(&theLogCriticalSection);
	}

	~ScopedLogLock()
	{
		if (m_isLocked)
			::LeaveCriticalSection(&theLogCriticalSection);
	}

private:
	const bool m_isLocked;
};
#endif

// ----------------------------------------------------------------------------
// getCurrentTimeString
/**
	Return the current time in string form
*/
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString()
{
	time_t aclock;
	time(&aclock);
	struct tm *newtime = localtime(&aclock);
	return asctime(newtime);
}

// ----------------------------------------------------------------------------
// getCurrentTickString
/**
	Return the current TickCount in string form
*/
// ----------------------------------------------------------------------------
static const char *getCurrentTickString()
{
	static char TheTickString[32];
	snprintf(TheTickString, ARRAY_SIZE(TheTickString), "(T=%08lx)", ::GetTickCount());
	return TheTickString;
}

// ----------------------------------------------------------------------------
// prepBuffer
// zap the buffer and optionally prepend the tick time.
// ----------------------------------------------------------------------------
/**
	Empty the buffer passed in, then optionally prepend the current TickCount
	value in string form, depending on the setting of theDebugFlags.
*/
static void prepBuffer(char *buffer)
{
	buffer[0] = 0;
#ifdef ALLOW_DEBUG_UTILS
	if (theDebugFlags & DEBUG_FLAG_PREPEND_TIME)
	{
		strcpy(buffer, getCurrentTickString());
		strcat(buffer, " ");
	}
#endif
}

// ----------------------------------------------------------------------------
// doLogOutput
/**
	send a string directly to the log file and/or console without further processing.
*/
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
static void doLogOutput(const char *buffer)
{
		doLogOutput(buffer, "\n");
}

static void doLogOutput(const char *buffer, const char *endline)
{
	// log message to file
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_FILE)
	{
		if (theLogFile)
		{
			fprintf(theLogFile, "%s%s", buffer, endline);
			fflush(theLogFile);
		}
	}

	// log message to dev studio output window
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_CONSOLE)
	{
		::OutputDebugString(buffer);
		::OutputDebugString(endline);
	}

	if (theLogHandler != nullptr)
	{
		theLogHandler(buffer, endline);
	}
}
#endif // DEBUG_LOGGING

// ----------------------------------------------------------------------------
// doCrashBox
/*
	present a messagebox with the given message. Depending on user selection,
	we exit the app, break into debugger, or continue execution.
*/
// ----------------------------------------------------------------------------
#ifdef DEBUG_CRASHING
static int doCrashBox(const char *buffer, Bool logResult)
{
	int result;

	if (!ignoringAsserts()) {
		result = MessageBoxWrapper(buffer, "Assertion Failure", MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_ICONWARNING|MB_DEFBUTTON3);
		//result = MessageBoxWrapper(buffer, "Assertion Failure", MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_ICONWARNING);
	}	else {
		result = IDIGNORE;
	}

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
#endif

#ifdef DEBUG_STACKTRACE
// ----------------------------------------------------------------------------
/**
	Dumps a stack trace (from the current PC) to logfile and/or console.
*/
static void doStackDump()
{
	if (theStackDumpHandler != nullptr)
	{
		theStackDumpHandler(doLogOutput);
	}
}
#endif

// ----------------------------------------------------------------------------
// whackFunnyCharacters
/**
	Eliminates any undesirable nonprinting characters, aside from newline,
	replacing them with spaces.
*/
// ----------------------------------------------------------------------------
static void whackFunnyCharacters(char *buf)
{
	for (char *p = buf + strlen(buf) - 1; p >= buf; --p)
	{
		// ok, these are naughty magic numbers, but I'm guessing you know ASCII....
		if (*p >= 0 && *p < 32 && *p != 10 && *p != 13)
			*p = 32;
	}
}

// ----------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// DebugInit
// ----------------------------------------------------------------------------
#ifdef ALLOW_DEBUG_UTILS
/**
	Initialize the debug utilities. This should be called once, as near to the
	start of the app as possible, before anything else (since other code will
	probably want to make use of it).
*/
void DebugInit(int flags)
{
//	if (theDebugFlags != 0)
//		::MessageBox(nullptr, "Debug already inited", "", MB_OK|MB_APPLMODAL);

	// just quietly allow multiple calls to this, so that static ctors can call it.
	if (theDebugFlags == 0)
	{
		theDebugFlags = flags;

		theMainThreadID = GetCurrentThreadId();

	#if defined(DEBUG_LOGGING) && defined(DEBUG_THREADSAFE)
		if (!theLogCriticalSectionInitialized)
		{
			::InitializeCriticalSection(&theLogCriticalSection);
			theLogCriticalSectionInitialized = true;
		}
	#endif
	}

}
#endif

// ----------------------------------------------------------------------------
// DebugOpenLogFile
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
/**
	Open the log file in the folder of the executable. The prefix and suffix are
	put around the name of the log file, so that applications and their instances
	can write to different files. The log file of the previous run is kept under
	another name. Until this is called, messages are logged to the console only.
*/
void DebugOpenLogFile(const char *prefix, const char *suffix)
{
	char dirbuf[ _MAX_PATH ];
	::GetModuleFileName( nullptr, dirbuf, sizeof( dirbuf ) );
	if (char *pEnd = strrchr(dirbuf, '\\'))
	{
		*(pEnd + 1) = 0;
	}

	static_assert(ARRAY_SIZE(theLogFileNamePrev) >= ARRAY_SIZE(dirbuf), "Incorrect array size");
	strcpy(theLogFileNamePrev, dirbuf);
	strlcat(theLogFileNamePrev, prefix, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileNamePrev, DEBUG_FILE_NAME_PREV, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileNamePrev, suffix, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileNamePrev, ".txt", ARRAY_SIZE(theLogFileNamePrev));

	static_assert(ARRAY_SIZE(theLogFileName) >= ARRAY_SIZE(dirbuf), "Incorrect array size");
	strcpy(theLogFileName, dirbuf);
	strlcat(theLogFileName, prefix, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileName, DEBUG_FILE_NAME, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileName, suffix, ARRAY_SIZE(theLogFileNamePrev));
	strlcat(theLogFileName, ".txt", ARRAY_SIZE(theLogFileNamePrev));

	remove(theLogFileNamePrev);
	if (rename(theLogFileName, theLogFileNamePrev) != 0)
	{
		DebugLog("Warning: Could not rename buffer file '%s' to '%s'. Will remove instead", theLogFileName, theLogFileNamePrev);
		if (remove(theLogFileName) != 0)
		{
			DebugLog("Warning: Failed to remove file '%s'", theLogFileName);
		}
	}

	theLogFile = fopen(theLogFileName, "w");
	if (theLogFile != nullptr)
	{
		DebugLog("Log %s opened: %s", theLogFileName, getCurrentTimeString());
	}
}
#endif

// ----------------------------------------------------------------------------
// DebugLog
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
/**
	Print a string to the log file and/or console.
*/
void DebugLog(const char *format, ...)
{
#ifdef DEBUG_THREADSAFE
	ScopedLogLock scopedLogLock;
#endif

	if (theDebugFlags == 0)
		MessageBoxWrapper("DebugLog - Debug not inited properly", "", MB_OK|MB_TASKMODAL);

	prepBuffer(theBuffer);

	va_list args;
	va_start(args, format);
	size_t offset = strlen(theBuffer);
	vsnprintf(theBuffer + offset, ARRAY_SIZE(theBuffer) - offset, format, args);
	va_end(args);

	if (strlen(theBuffer) >= sizeof(theBuffer))
		MessageBoxWrapper("String too long for debug buffer", "", MB_OK|MB_TASKMODAL);

	whackFunnyCharacters(theBuffer);
	doLogOutput(theBuffer);
}

/**
	Print a string with no modifications to the log file and/or console.
*/
void DebugLogRaw(const char *format, ...)
{
#ifdef DEBUG_THREADSAFE
	ScopedLogLock scopedLogLock;
#endif

	if (theDebugFlags == 0)
		MessageBoxWrapper("DebugLogRaw - Debug not inited properly", "", MB_OK|MB_TASKMODAL);

	theBuffer[0] = 0;

	va_list args;
	va_start(args, format);
	vsnprintf(theBuffer, ARRAY_SIZE(theBuffer), format, args);
	va_end(args);

	if (strlen(theBuffer) >= sizeof(theBuffer))
		MessageBoxWrapper("String too long for debug buffer", "", MB_OK|MB_TASKMODAL);

	doLogOutput(theBuffer, "");
}

const char* DebugGetLogFileName()
{
	return theLogFileName;
}

const char* DebugGetLogFileNamePrev()
{
	return theLogFileNamePrev;
}

#endif

// ----------------------------------------------------------------------------
// DebugCrash
// ----------------------------------------------------------------------------
#ifdef DEBUG_CRASHING
/**
	Print a character string to the log file and/or console, then halt execution
	while presenting the user with an exit/debug/ignore dialog containing the same
	text message.

	TheSuperHackers @tweak Now shows a message box without any logging when debug was not yet initialized.
*/
void DebugCrash(const char *format, ...)
{
	// Note: You might want to make this thread safe, but we cannot. The reason is that
	// there is an implicit requirement on other threads that the message loop be running.

	// make it not static so that it'll be thread-safe.
	// make it big to avoid weird overflow bugs in debug mode
	char theCrashBuffer[ LARGE_BUFFER ];

	prepBuffer(theCrashBuffer);
	strlcat(theCrashBuffer, "ASSERTION FAILURE: ", ARRAY_SIZE(theCrashBuffer));

	va_list arg;
	va_start(arg, format);
	size_t offset =  strlen(theCrashBuffer);
	vsnprintf(theCrashBuffer + offset, ARRAY_SIZE(theCrashBuffer) - offset, format, arg);
	va_end(arg);

	whackFunnyCharacters(theCrashBuffer);

	const bool useLogging = theDebugFlags != 0;

	if (useLogging)
	{
#ifdef DEBUG_LOGGING
		if (ignoringAsserts())
		{
			doLogOutput("**** CRASH IN FULL SCREEN - Auto-ignored, CHECK THIS LOG!");
		}
		doLogOutput(theCrashBuffer);
#endif
#ifdef DEBUG_STACKTRACE
		doStackDump();
#endif
	}

	if (theCrashHandler != nullptr)
	{
		// Every crash reaches the handler, because the ignore option of the crash box does not apply.
		theCrashHandler(theCrashBuffer);
		return;
	}

	strlcat(theCrashBuffer, "\n\nAbort->exception; Retry->debugger; Ignore->continue", ARRAY_SIZE(theCrashBuffer));

	const int result = doCrashBox(theCrashBuffer, useLogging);

	if (result == IDIGNORE && TheCurrentIgnoreCrashPtr != nullptr)
	{
		int yn;
		if (!ignoringAsserts())
		{
			yn = MessageBoxWrapper("Ignore this crash from now on?", "", MB_YESNO|MB_TASKMODAL);
		}
		else
		{
			yn = IDYES;
		}
		if (yn == IDYES)
			*TheCurrentIgnoreCrashPtr = 1;
		if (theCrashIgnoredHandler != nullptr)
		{
			theCrashIgnoredHandler();
		}
	}

}
#endif

// ----------------------------------------------------------------------------
// DebugShutdown
// ----------------------------------------------------------------------------
#ifdef ALLOW_DEBUG_UTILS
/**
	Shut down the debug utilities. This should be called once, as near to the
	end of the app as possible, after everything else (since other code will
	probably want to make use of it).
*/
void DebugShutdown()
{
#ifdef DEBUG_LOGGING
	if (theLogFile)
	{
		DebugLog("Log closed: %s", getCurrentTimeString());
		fclose(theLogFile);
	}
	theLogFile = nullptr;
#endif
	theDebugFlags = 0;
}

// ----------------------------------------------------------------------------
// DebugGetFlags
// ----------------------------------------------------------------------------
/**
	Get the current values for the flags passed to DebugInit. Most code will never
	need to use this; the most common usage would be to temporarily enable or disable
	the DEBUG_FLAG_PREPEND_TIME bit for complex logfile messages.
*/
int DebugGetFlags()
{
	return theDebugFlags;
}

// ----------------------------------------------------------------------------
// DebugSetFlags
// ----------------------------------------------------------------------------
/**
	Set the current values for the flags passed to DebugInit. Most code will never
	need to use this; the most common usage would be to temporarily enable or disable
	the DEBUG_FLAG_PREPEND_TIME bit for complex logfile messages.
*/
void DebugSetFlags(int flags)
{
	theDebugFlags = flags;
}

// ----------------------------------------------------------------------------
// Host callbacks
// ----------------------------------------------------------------------------
void DebugSetIgnoreAssertsQuery(DebugIgnoreAssertsQuery query)
{
	theIgnoreAssertsQuery = query;
}

void DebugSetMainWindowQuery(DebugMainWindowQuery query)
{
	theMainWindowQuery = query;
}

void DebugSetStackDumpHandler(DebugStackDumpHandler handler)
{
	theStackDumpHandler = handler;
}

void DebugSetCrashIgnoredHandler(DebugCrashIgnoredHandler handler)
{
	theCrashIgnoredHandler = handler;
}

void DebugSetLogHandler(DebugLogHandler handler)
{
	theLogHandler = handler;
}

#endif	// ALLOW_DEBUG_UTILS

// ----------------------------------------------------------------------------
// DebugSetCrashHandler
// ----------------------------------------------------------------------------
/**
	Set the handler that receives debug and release crashes instead of the crash box.
	Pass nullptr to show the crash box again.
*/
void DebugSetCrashHandler(DebugCrashHandler handler)
{
	theCrashHandler = handler;
}

DebugCrashHandler DebugGetCrashHandler()
{
	return theCrashHandler;
}
