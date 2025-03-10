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

#ifndef __DEBUG_H_
#define __DEBUG_H_

class AsciiString;

#if defined(_DEBUG) && defined(_INTERNAL)
	#error "Only one at a time of these should ever be defined"
#endif

#define NO_RELEASE_DEBUG_LOGGING

#ifdef RELEASE_DEBUG_LOGGING  ///< Creates a DebugLogFile.txt (No I or D) with all the debug log goodness.  Good for startup problems.
	#define ALLOW_DEBUG_UTILS 1
	#define DEBUG_LOGGING 1
	#define DISABLE_DEBUG_CRASHING 1
	#define DISABLE_DEBUG_STACKTRACE 1
	#define DISABLE_DEBUG_PROFILE 1
#endif

// These are stolen from the WW3D Debug file. REALLY useful. :-)
#define STRING_IT(a) #a																				  
#define TOKEN_IT(a) STRING_IT(,##a)
#define MESSAGE(a) message (__FILE__ "(" TOKEN_IT(__LINE__) ") : " a)

// by default, turn on ALLOW_DEBUG_UTILS if _DEBUG is turned on.
#if (defined(_DEBUG) || defined(_INTERNAL)) && !defined(ALLOW_DEBUG_UTILS) && !defined(DISABLE_ALLOW_DEBUG_UTILS)
	#define ALLOW_DEBUG_UTILS 1
#endif

// these are predicated on ALLOW_DEBUG_UTILS, not _DEBUG, and allow you to selectively disable
// bits of the debug stuff for special builds.
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_LOGGING) && !defined(DISABLE_DEBUG_LOGGING)
	#define DEBUG_LOGGING 1
#endif
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_CRASHING) && !defined(DISABLE_DEBUG_CRASHING)
	#define DEBUG_CRASHING 1
#endif
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_STACKTRACE) && !defined(DISABLE_DEBUG_STACKTRACE)
	#define DEBUG_STACKTRACE 1
#endif
#if defined(ALLOW_DEBUG_UTILS) && !defined(DEBUG_PROFILE) && !defined(DISABLE_DEBUG_PROFILE)
	#define DEBUG_PROFILE 1
#endif

#ifdef __cplusplus
	#define DEBUG_EXTERN_C extern "C"
#else
	#define DEBUG_EXTERN_C extern
#endif

// The following define controls whether fake ips are enabled
// If it's enabled, it changes the behavior of the game in some ways:
// - The game can be started multiple times on the same machine.
// - It can be called with the commandline -fakeip 1 (or some other number),
//   assigning it a fake ip.
// - Multiple instances with fake ips can communicate with each other on the
//   same machine, as if they are communicating in a LAN.
// - The replay is saved to a separate file for each instance (00000000_42.rep for ip 42)
// - The logfile (if one is created) is also saved with the ip name in the filename.
// So, if you want to use this to debug network functionality on a single computer
// you can enable this define and start the game multiple times, each time passing a different
// fake ip.
// WARNING: If this define is set (even if no fake ip is supplied):
// - all network communication works internally via broadcasts (not just in the lobby room).
//   so you might wanna disconnect from your network while using this.
// - the network protocol becomes incompatible with the original.
//
// Note that this define is currently in Debug.h. That's because the logfile is created
// before the commandline is parsed, and we need to create the log file with the correct name.
#define ENABLE_FAKE_IP 0

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

#if ENABLE_FAKE_IP
	DEBUG_EXTERN_C int getFakeIPNo();
#endif

#ifdef ALLOW_DEBUG_UTILS

	enum 
	{
		DEBUG_FLAG_LOG_TO_FILE = 0x01,	
		DEBUG_FLAG_LOG_TO_CONSOLE = 0x02,
		DEBUG_FLAG_PREPEND_TIME = 0x04,
#ifdef _INTERNAL
		// by default, _INTERNAL builds log to file, but not to console, in the interest
		// of speed. want console output? just change this line:
		DEBUG_FLAGS_DEFAULT = (DEBUG_FLAG_LOG_TO_FILE)
#else
		DEBUG_FLAGS_DEFAULT = (DEBUG_FLAG_LOG_TO_FILE | DEBUG_FLAG_LOG_TO_CONSOLE)
#endif
	};

	DEBUG_EXTERN_C void DebugInit(int flags);
	DEBUG_EXTERN_C void DebugShutdown();

	DEBUG_EXTERN_C int DebugGetFlags();
	DEBUG_EXTERN_C void DebugSetFlags(int flags);

	#define DEBUG_INIT(f)						do { DebugInit(f); } while (0)
	#define DEBUG_SHUTDOWN()				do { DebugShutdown(); } while (0)

#else

	#define DEBUG_INIT(f)						((void)0)
	#define DEBUG_SHUTDOWN()				((void)0)

#endif

#ifdef DEBUG_LOGGING

	DEBUG_EXTERN_C void DebugLog(const char *format, ...);

	#define DEBUG_LOG(m)						do { { DebugLog m ; } } while (0)
	#define DEBUG_ASSERTLOG(c, m)		do { { if (!(c)) DebugLog m ; } } while (0)

#else

	#define DEBUG_LOG(m)						((void)0)
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
					TheCurrentIgnoreCrashPtr = NULL; \
				} \
			} \
		} while (0)

	#define DEBUG_ASSERTCRASH(c, m)		do { { if (!(c)) DEBUG_CRASH(m); } } while (0)

	//Note: RELEASE_CRASH(m) is now always defined.
	//#define RELEASE_CRASH(m)					DEBUG_CRASH((m))

#else

	#define DEBUG_CRASH(m)					((void)0)
	#define DEBUG_ASSERTCRASH(c, m)	((void)0)

//	DEBUG_EXTERN_C void ReleaseCrash(const char* reason);

//	#define RELEASE_CRASH(m)				do { ReleaseCrash(m); } while (0)

#endif

DEBUG_EXTERN_C void ReleaseCrash(const char* reason);
DEBUG_EXTERN_C void ReleaseCrashLocalized(const AsciiString& p, const AsciiString& m);

#define RELEASE_CRASH(m)				do { ReleaseCrash(m); } while (0)
#define RELEASE_CRASHLOCALIZED(p, m)		do { ReleaseCrashLocalized(p, m); } while (0)


#ifdef DEBUG_PROFILE

class SimpleProfiler
{
private:
	__int64 m_freq;
	__int64 m_startThisSession;
	__int64 m_totalThisSession;
	__int64 m_totalAllSessions;
	int			m_numSessions;

public:

	SimpleProfiler();
	void start();
	void stop();
	void stopAndLog(const char *msg, int howOftenToLog, int howOftenToResetAvg);
	double getTime();				// of most recent session, in milliseconds
	int getNumSessions();
	double getTotalTime();	// total over all sessions, in milliseconds
	double getAverageTime();	// averaged over all sessions, in milliseconds

};

#define BEGIN_PROFILE(uniqueid) \
	static SimpleProfiler prof_##uniqueid; \
	prof_##uniqueid.start();

#define END_PROFILE(uniqueid, msg, howoftentolog, howoftentoreset) \
	prof_##uniqueid.stopAndLog(msg, howoftentolog, howoftentoreset);

#else
	
#define BEGIN_PROFILE(uniqueid)
#define END_PROFILE(uniqueid, msg, howoftentolog, howoftentoreset)

#endif

// MACROS //////////////////////////////////////////////////////////////////

#endif // __DEBUG_H_
