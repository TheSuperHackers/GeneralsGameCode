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

// FILE: Debug.h
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Debug.h
//
// Created:    Steven Johnson, August 2001
//
// Desc:       Debug Utilities
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define NO_RELEASE_DEBUG_LOGGING

#ifdef RELEASE_DEBUG_LOGGING  ///< Creates a DebugLogFile.txt (No I or D) with all the debug log goodness.  Good for startup problems.
	#define ALLOW_DEBUG_UTILS 1
	#define DEBUG_LOGGING 1
	#define DISABLE_DEBUG_CRASHING 1
	#define DISABLE_DEBUG_STACKTRACE 1
#endif

// These are stolen from the WW3D Debug file. REALLY useful. :-)
#define STRING_IT(a) #a
#define TOKEN_IT(a) STRING_IT(,##a)
#define MESSAGE(a) message (__FILE__ "(" TOKEN_IT(__LINE__) ") : " a)

// by default, turn on ALLOW_DEBUG_UTILS if RTS_DEBUG is turned on.
#if defined(RTS_DEBUG) && !defined(ALLOW_DEBUG_UTILS) && !defined(DISABLE_ALLOW_DEBUG_UTILS)
	#define ALLOW_DEBUG_UTILS 1
#elif defined(DEBUG_LOGGING) || defined(DEBUG_CRASHING) || defined(DEBUG_STACKTRACE)
	// TheSuperHackers @tweak also turn on when any of the above options is already set.
	#define ALLOW_DEBUG_UTILS 1
#endif

// these are predicated on ALLOW_DEBUG_UTILS, not RTS_DEBUG, and allow you to selectively disable
// bits of the debug stuff for special builds.
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_LOGGING) && !defined(DISABLE_DEBUG_LOGGING)
	#define DEBUG_LOGGING 1
#endif
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_CRASHING) && !defined(DISABLE_DEBUG_CRASHING)
	#define DEBUG_CRASHING 1
#endif
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_STACKTRACE) && !defined(DISABLE_DEBUG_STACKTRACE)
	#define DEBUG_STACKTRACE 1
	#ifndef DEBUG_LOGGING
		#define DEBUG_LOGGING 1 // TheSuperHackers @build Stack trace requires logging.
	#endif
#endif

#ifdef __cplusplus
	#define DEBUG_EXTERN_C extern "C"
#else
	#define DEBUG_EXTERN_C extern
#endif


// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

/// @todo: the standard line-to-string trick isn't working correctly in vc6; figure out why
#define DEBUG_STRING_IT(b)	#b
#define DEBUG_TOKEN_IT(a)		DEBUG_STRING_IT(a)
#define DEBUG_FILENLINE			__FILE__ ":" DEBUG_TOKEN_IT(__LINE__)

#ifdef ALLOW_DEBUG_UTILS

	enum
	{
		DEBUG_FLAG_LOG_TO_FILE = 0x01,
		DEBUG_FLAG_LOG_TO_CONSOLE = 0x02,
		DEBUG_FLAG_PREPEND_TIME = 0x04,
		DEBUG_FLAGS_DEFAULT = (DEBUG_FLAG_LOG_TO_FILE | DEBUG_FLAG_LOG_TO_CONSOLE),
	};

	DEBUG_EXTERN_C void DebugInit(int flags);
	DEBUG_EXTERN_C void DebugShutdown();

	DEBUG_EXTERN_C int DebugGetFlags();
	DEBUG_EXTERN_C void DebugSetFlags(int flags);

	#define DEBUG_INIT(f)						do { DebugInit(f); } while (0)
	#define DEBUG_SHUTDOWN()				do { DebugShutdown(); } while (0)

	// TheSuperHackers @info The application that hosts the debug utilities connects them
	// to its own state with these callbacks. All of them are optional.

	// Returns true while debug crashes must be ignored without showing the crash box.
	typedef bool (*DebugIgnoreAssertsQuery)();
	// Returns the HWND of the window that owns the boxes shown from the main thread.
	typedef void *(*DebugMainWindowQuery)();
	// Writes the stack trace of a debug crash line by line to the given output.
	typedef void (*DebugStackDumpHandler)(void (*output)(const char *line));
	// Called after a debug crash was ignored.
	typedef void (*DebugCrashIgnoredHandler)();
	// Receives every log message in addition to the log file and the console.
	typedef void (*DebugLogHandler)(const char *buffer, const char *endline);

	DEBUG_EXTERN_C void DebugSetIgnoreAssertsQuery(DebugIgnoreAssertsQuery query);
	DEBUG_EXTERN_C void DebugSetMainWindowQuery(DebugMainWindowQuery query);
	DEBUG_EXTERN_C void DebugSetStackDumpHandler(DebugStackDumpHandler handler);
	DEBUG_EXTERN_C void DebugSetCrashIgnoredHandler(DebugCrashIgnoredHandler handler);
	DEBUG_EXTERN_C void DebugSetLogHandler(DebugLogHandler handler);

#else

	#define DEBUG_INIT(f)						((void)0)
	#define DEBUG_SHUTDOWN()				((void)0)

#endif

#ifdef DEBUG_LOGGING

	DEBUG_EXTERN_C void DebugLog(const char *format, ...);
	DEBUG_EXTERN_C void DebugLogRaw(const char *format, ...);
	DEBUG_EXTERN_C void DebugOpenLogFile(const char *prefix, const char *suffix);
	DEBUG_EXTERN_C const char* DebugGetLogFileName();
	DEBUG_EXTERN_C const char* DebugGetLogFileNamePrev();

	// This defines a bitmask of log types that we care about, to allow some flexability
	// in what gets logged.  This should be extended to asserts, too, but the assert box
	// is waiting to be rewritten. -MDC 3/19/2003
	extern unsigned int DebugLevelMask;
	enum
	{
		DEBUG_LEVEL_NET = 0,           // in-game network
		DEBUG_LEVEL_MAX
	};
	extern const char *TheDebugLevels[DEBUG_LEVEL_MAX];

	#define DEBUG_LOG(m)						do { { DebugLog m ; } } while (0) // Log message with trailing new line character (LF)
	#define DEBUG_LOG_RAW(m)				do { { DebugLogRaw m ; } } while (0) // Log message without trailing new line character (LF)
	#define DEBUG_LOG_LEVEL(l, m)		do { if (l & DebugLevelMask) { DebugLog m ; } } while (0)
	#define DEBUG_LOG_LEVEL_RAW(l, m)	do { if (l & DebugLevelMask) { DebugLogRaw m ; } } while (0)
	#define DEBUG_ASSERTLOG(c, m)		do { { if (!(c)) DebugLog m ; } } while (0)

#else

	#define DEBUG_LOG(m)						((void)0)
	#define DEBUG_LOG_RAW(m)				((void)0)
	#define DEBUG_LOG_LEVEL(l, m)		((void)0)
	#define DEBUG_LOG_LEVEL_RAW(l, m)	((void)0)
	#define DEBUG_ASSERTLOG(c, m)		((void)0)

#endif

#ifdef DEBUG_CRASHING

	DEBUG_EXTERN_C void DebugCrash(const char *format, ...);

	/*
		Yeah, it's a sleazy global, since we can't reasonably add
		any args to DebugCrash due to the varargs nature of it.
		We'll just let it slide in this case...
	*/
	DEBUG_EXTERN_C char* TheCurrentIgnoreCrashPtr;

	#define DEBUG_CRASH(m)	\
		do { \
			{ \
				static char ignoreCrash = 0; \
				if (!ignoreCrash) { \
					TheCurrentIgnoreCrashPtr = &ignoreCrash; \
					DebugCrash m ; \
					TheCurrentIgnoreCrashPtr = nullptr; \
				} \
			} \
		} while (0)

	#define DEBUG_ASSERTCRASH(c, m)		do { { if (!(c)) DEBUG_CRASH(m); } } while (0)

#else

	#define DEBUG_CRASH(m)					((void)0)
	#define DEBUG_ASSERTCRASH(c, m)	((void)0)

#endif

// Debug and release crashes go to the crash handler instead of a crash box,
// for example to fail a unit test. A debug crash then continues, a release crash still exits.
typedef void (*DebugCrashHandler)(const char *message);
DEBUG_EXTERN_C void DebugSetCrashHandler(DebugCrashHandler handler);
DEBUG_EXTERN_C DebugCrashHandler DebugGetCrashHandler();

// MACROS //////////////////////////////////////////////////////////////////
