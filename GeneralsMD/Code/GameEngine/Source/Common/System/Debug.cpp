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
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine


// USER INCLUDES 
#define DEBUG_THREADSAFE
#ifdef DEBUG_THREADSAFE
#include "Common/CriticalSection.h"
#endif
#include "Common/Debug.h"
#include "Common/SystemInfo.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#if defined(DEBUG_STACKTRACE) || defined(IG_DEBUG_STACKTRACE)
	#include "Common/StackDump.h"
#endif

// Horrible reference, but we really, really need to know if we are windowed.
extern bool DX8Wrapper_IsWindowed;
extern HWND ApplicationHWnd;

extern const char *gAppPrefix; /// So WB can have a different log file name.

#ifdef _INTERNAL
// this should ALWAYS be present
#pragma optimize("", off)
#endif

// ----------------------------------------------------------------------------
// DEFINES 
// ----------------------------------------------------------------------------

#ifdef DEBUG_LOGGING

#if defined(_INTERNAL)
	#define DEBUG_FILE_NAME				"DebugLogFileI.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevI.txt"
#elif defined(_DEBUG)
	#define DEBUG_FILE_NAME				"DebugLogFileD.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevD.txt"
#else
	#define DEBUG_FILE_NAME				"DebugLogFile.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrev.txt"
#endif

#endif

// ----------------------------------------------------------------------------
// PRIVATE TYPES 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// PRIVATE DATA 
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
static FILE *theLogFile = NULL;
#endif
#define LARGE_BUFFER	8192
static char theBuffer[ LARGE_BUFFER ];	// make it big to avoid weird overflow bugs in debug mode
static int theDebugFlags = 0;
static DWORD theMainThreadID = 0;
// ----------------------------------------------------------------------------
// PUBLIC DATA 
// ----------------------------------------------------------------------------

char* TheCurrentIgnoreCrashPtr = NULL;

// ----------------------------------------------------------------------------
// PRIVATE PROTOTYPES 
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString(void);
static const char *getCurrentTickString(void);
static const char *prepBuffer(const char* format, char *buffer);
#ifdef DEBUG_LOGGING
static void doLogOutput(const char *buffer);
#endif
static int doCrashBox(const char *buffer, Bool logResult);
static void whackFunnyCharacters(char *buf);
#ifdef DEBUG_STACKTRACE
static void doStackDump();
#endif

// ----------------------------------------------------------------------------
// PRIVATE FUNCTIONS 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
inline Bool ignoringAsserts()
{
#if defined(_DEBUG) || defined(_INTERNAL)
	return !DX8Wrapper_IsWindowed || (TheGlobalData&&TheGlobalData->m_debugIgnoreAsserts);
#else
	return !DX8Wrapper_IsWindowed;
#endif
}

// ----------------------------------------------------------------------------
inline HWND getThreadHWND()
{
	return (theMainThreadID == GetCurrentThreadId())?ApplicationHWnd:NULL;
}

// ----------------------------------------------------------------------------

int MessageBoxWrapper( LPCSTR lpText, LPCSTR lpCaption, UINT uType )
{
	HWND threadHWND = getThreadHWND();
	if (!threadHWND)
		return (uType & MB_ABORTRETRYIGNORE)?IDIGNORE:IDYES;

	return ::MessageBox(threadHWND, lpText, lpCaption, uType);
}

// ----------------------------------------------------------------------------
// getCurrentTimeString 
/** 
	Return the current time in string form
*/
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString(void)
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
static const char *getCurrentTickString(void)
{
	static char TheTickString[32];
	sprintf(TheTickString, "(T=%08lx)",::GetTickCount());
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
static const char *prepBuffer(const char* format, char *buffer)
{
	buffer[0] = 0;
#ifdef ALLOW_DEBUG_UTILS
	if (theDebugFlags & DEBUG_FLAG_PREPEND_TIME)
	{
		strcpy(buffer, getCurrentTickString());
		strcat(buffer, " ");
	}
#endif
	return format;
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
	// log message to file
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_FILE)
	{
		if (theLogFile)
		{
			fprintf(theLogFile, "%s", buffer);	// note, no \n (should be there already)
			fflush(theLogFile);
		}
	}

	// log message to dev studio output window
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_CONSOLE)
	{
		::OutputDebugString(buffer);
	}
}
#endif

// ----------------------------------------------------------------------------
// doCrashBox
/*
	present a messagebox with the given message. Depending on user selection,
	we exit the app, break into debugger, or continue execution. 
*/
// ----------------------------------------------------------------------------
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
				DebugLog("[Abort]\n");
#endif
			_exit(1);
			break;
		case IDRETRY:
#ifdef DEBUG_LOGGING
			if (logResult)
				DebugLog("[Retry]\n");
#endif
			::DebugBreak();
			break;
		case IDIGNORE:
#ifdef DEBUG_LOGGING
			// do nothing, just keep going
			if (logResult)
				DebugLog("[Ignore]\n");
#endif
			break;
	}
	return result;
}

#ifdef DEBUG_STACKTRACE
// ----------------------------------------------------------------------------
/**
	Dumps a stack trace (from the current PC) to logfile and/or console.
*/
static void doStackDump()
{
	const int STACKTRACE_SIZE	= 24;
	const int STACKTRACE_SKIP = 2;
	void* stacktrace[STACKTRACE_SIZE];

	doLogOutput("\nStack Dump:\n");
	::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
	::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, doLogOutput);
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
//		::MessageBox(NULL, "Debug already inited", "", MB_OK|MB_APPLMODAL);

	// just quietly allow multiple calls to this, so that static ctors can call it.
	if (theDebugFlags == 0) 
	{
		theDebugFlags = flags;

		theMainThreadID = GetCurrentThreadId();

	#ifdef DEBUG_LOGGING

		char dirbuf[ _MAX_PATH ];
		::GetModuleFileName( NULL, dirbuf, sizeof( dirbuf ) );
		char *pEnd = dirbuf + strlen( dirbuf );
		while( pEnd != dirbuf ) 
		{
			if( *pEnd == '\\' ) 
			{
				*(pEnd + 1) = 0;
				break;
			}
			pEnd--;
		}

		char prevbuf[ _MAX_PATH ];
		char curbuf[ _MAX_PATH ];

		strcpy(prevbuf, dirbuf);
		strcat(prevbuf, gAppPrefix);
		strcat(prevbuf, DEBUG_FILE_NAME_PREV);
		strcpy(curbuf, dirbuf);
		strcat(curbuf, gAppPrefix);
		strcat(curbuf, DEBUG_FILE_NAME);

 		remove(prevbuf);
		rename(curbuf, prevbuf);
		theLogFile = fopen(curbuf, "w");
		if (theLogFile != NULL)
		{
			DebugLog("Log %s opened: %s\n", curbuf, getCurrentTimeString());
		} 
	#endif
	}

}  
#endif

// ----------------------------------------------------------------------------
// DebugLog 
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
/**
	Print a character string to the logfile and/or console.
*/
void DebugLog(const char *format, ...)
{
#ifdef DEBUG_THREADSAFE
	ScopedCriticalSection scopedCriticalSection(TheDebugLogCriticalSection);
#endif

	if (theDebugFlags == 0)
		MessageBoxWrapper("DebugLog - Debug not inited properly", "", MB_OK|MB_TASKMODAL);

	format = prepBuffer(format, theBuffer);

	va_list arg;
  va_start(arg, format);
  vsprintf(theBuffer + strlen(theBuffer), format, arg);
  va_end(arg);

	if (strlen(theBuffer) >= sizeof(theBuffer))
		MessageBoxWrapper("String too long for debug buffer", "", MB_OK|MB_TASKMODAL);

	whackFunnyCharacters(theBuffer);
	doLogOutput(theBuffer);
} 
#endif

// ----------------------------------------------------------------------------
// DebugCrash
// ----------------------------------------------------------------------------
#ifdef DEBUG_CRASHING
/**
	Print a character string to the logfile and/or console, then halt execution
	while presenting the user with an exit/debug/ignore dialog containing the same
	text message.
*/
void DebugCrash(const char *format, ...)
{
	// Note: You might want to make this thread safe, but we cannot. The reason is that 
	// there is an implicit requirement on other threads that the message loop be running.

	// make it not static so that it'll be thread-safe.
	// make it big to avoid weird overflow bugs in debug mode
	char theCrashBuffer[ LARGE_BUFFER ];	
	if (theDebugFlags == 0)
	{
		if (!DX8Wrapper_IsWindowed) {
			if (ApplicationHWnd) {
				ShowWindow(ApplicationHWnd, SW_HIDE);
			}
		}
		MessageBoxWrapper("DebugCrash - Debug not inited properly", "", MB_OK|MB_TASKMODAL);
	}

	format = prepBuffer(format, theCrashBuffer);
	strcat(theCrashBuffer, "ASSERTION FAILURE: ");

	va_list arg;
  va_start(arg, format);
  vsprintf(theCrashBuffer + strlen(theCrashBuffer), format, arg);
  va_end(arg);

	if (strlen(theCrashBuffer) >= sizeof(theCrashBuffer))
	{
		if (!DX8Wrapper_IsWindowed) {
			if (ApplicationHWnd) {
				ShowWindow(ApplicationHWnd, SW_HIDE);
			}
		}
		MessageBoxWrapper("String too long for debug buffers", "", MB_OK|MB_TASKMODAL);
	}

#ifdef DEBUG_LOGGING
	if (ignoringAsserts()) 
	{
		doLogOutput("**** CRASH IN FULL SCREEN - Auto-ignored, CHECK THIS LOG!\n");
	}
	whackFunnyCharacters(theCrashBuffer);
	doLogOutput(theCrashBuffer);
#endif
#ifdef DEBUG_STACKTRACE
	if (!(TheGlobalData && TheGlobalData->m_debugIgnoreStackTrace))
	{
		doStackDump();
	}
#endif

	strcat(theCrashBuffer, "\n\nAbort->exception; Retry->debugger; Ignore->continue\n");

	int result = doCrashBox(theCrashBuffer, true);

	if (result == IDIGNORE && TheCurrentIgnoreCrashPtr != NULL) 
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
		if( TheKeyboard )
			TheKeyboard->resetKeys();
		if( TheMouse )
			TheMouse->reset();
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
		DebugLog("Log closed: %s\n", getCurrentTimeString());
		fclose(theLogFile);
	}
	theLogFile = NULL;
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

#endif	// ALLOW_DEBUG_UTILS

#ifdef DEBUG_PROFILE
// ----------------------------------------------------------------------------
SimpleProfiler::SimpleProfiler()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
	m_startThisSession = 0;
	m_totalThisSession = 0;
	m_totalAllSessions = 0;
	m_numSessions = 0;
}

// ----------------------------------------------------------------------------
void SimpleProfiler::start()
{
	DEBUG_ASSERTCRASH(m_startThisSession == 0, ("already started"));
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startThisSession);
}

// ----------------------------------------------------------------------------
void SimpleProfiler::stop()
{
	if (m_startThisSession != 0) 
	{
		__int64 stop;
		QueryPerformanceCounter((LARGE_INTEGER*)&stop);
		m_totalThisSession = stop - m_startThisSession;
		m_totalAllSessions += stop - m_startThisSession;
		m_startThisSession = 0;
		++m_numSessions;
	}
}

// ----------------------------------------------------------------------------
void SimpleProfiler::stopAndLog(const char *msg, int howOftenToLog, int howOftenToResetAvg)
{
	stop();
	// howOftenToResetAvg==0 means "never reset"
	if (howOftenToResetAvg > 0 && m_numSessions >= howOftenToResetAvg)
	{
		m_numSessions = 0;
		m_totalAllSessions = 0;
		DEBUG_LOG(("%s: reset averages\n",msg));
	}
	DEBUG_ASSERTLOG(m_numSessions % howOftenToLog != 0, ("%s: %f msec, total %f msec, avg %f msec\n",msg,getTime(),getTotalTime(),getAverageTime()));
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getTime()
{
	stop();
	return (double)m_totalThisSession / (double)m_freq * 1000.0;
}

// ----------------------------------------------------------------------------
int SimpleProfiler::getNumSessions()
{
	stop();
	return m_numSessions;
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getTotalTime()
{
	stop();
	if (!m_numSessions)
		return 0.0;

	return (double)m_totalAllSessions * 1000.0 / ((double)m_freq);
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getAverageTime()
{
	stop();
	if (!m_numSessions)
		return 0.0;

	return (double)m_totalAllSessions * 1000.0 / ((double)m_freq * (double)m_numSessions);
}

#endif	// ALLOW_DEBUG_UTILS

// ----------------------------------------------------------------------------
// ReleaseCrash
// ----------------------------------------------------------------------------
/**
	Halt the application, EVEN IN FINAL RELEASE BUILDS. This should be called
	only when a crash is guaranteed by continuing, and no meaningful continuation
	of processing is possible, even by throwing an exception.
*/

	#define RELEASECRASH_FILE_NAME				"ReleaseCrashInfo.txt"
	#define RELEASECRASH_FILE_NAME_PREV		"ReleaseCrashInfoPrev.txt"

	static FILE *theReleaseCrashLogFile = NULL;

	static void releaseCrashLogOutput(const char *buffer)
	{
		if (theReleaseCrashLogFile)
		{
			fprintf(theReleaseCrashLogFile, "%s", buffer);	// note, no \n (should be there already)
			fflush(theReleaseCrashLogFile);
		}
	}

void ReleaseCrash(const char *reason)
{
	/// do additional reporting on the crash, if possible

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}
//#if defined(_DEBUG) || defined(_INTERNAL)
//	/* static */ char buff[8192]; // not so static so we can be threadsafe
//	_snprintf(buff, 8192, "Sorry, a serious error occurred. (%s)", reason);/
//	buff[8191] = 0;
//	::MessageBox(NULL, buff, "Technical Difficulties...", MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
//#else
//	::MessageBox(NULL, "Sorry, a serious error occurred.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);
//#endif

	char prevbuf[ _MAX_PATH ];
	char curbuf[ _MAX_PATH ];

	if (TheGlobalData==NULL) {
		return; // We are shutting down, and TheGlobalData has been freed.  jba. [4/15/2003]
	}

	strcpy(prevbuf, TheGlobalData->getPath_UserData().str());
	strcat(prevbuf, RELEASECRASH_FILE_NAME_PREV);
	strcpy(curbuf, TheGlobalData->getPath_UserData().str());
	strcat(curbuf, RELEASECRASH_FILE_NAME);

 	remove(prevbuf);
	rename(curbuf, prevbuf);

	theReleaseCrashLogFile = fopen(curbuf, "w");
	if (theReleaseCrashLogFile)
	{
		fprintf(theReleaseCrashLogFile, "Release Crash at %s; Reason %s\n", getCurrentTimeString(), reason);
		fprintf(theReleaseCrashLogFile, "\nLast error:\n%s\n\nCurrent stack:\n", g_LastErrorDump.str());
		const int STACKTRACE_SIZE	= 12;
		const int STACKTRACE_SKIP = 6;
		void* stacktrace[STACKTRACE_SIZE];
		::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
		::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, releaseCrashLogOutput);

		fflush(theReleaseCrashLogFile);
		fclose(theReleaseCrashLogFile);
		theReleaseCrashLogFile = NULL;
	}

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}
#if defined(_DEBUG) || defined(_INTERNAL)
	/* static */ char buff[8192]; // not so static so we can be threadsafe
	_snprintf(buff, 8192, "Sorry, a serious error occurred. (%s)", reason);
	buff[8191] = 0;
	::MessageBox(NULL, buff, "Technical Difficulties...", MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
#else
// crash error messaged changed 3/6/03 BGC
//	::MessageBox(NULL, "Sorry, a serious error occurred.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);
//	::MessageBox(NULL, "You have encountered a serious error.  Serious errors can be caused by many things including viruses, overheated hardware and hardware that does not meet the minimum specifications for the game. Please visit the forums at www.generals.ea.com for suggested courses of action or consult your manual for Technical Support contact information.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);

// crash error message changed again 8/22/03 M Lorenzen... made this message box modal to the system so it will appear on top of any task-modal windows, splash-screen, etc.
  ::MessageBox(NULL, "You have encountered a serious error.  Serious errors can be caused by many things including viruses, overheated hardware and hardware that does not meet the minimum specifications for the game. Please visit the forums at www.generals.ea.com for suggested courses of action or consult your manual for Technical Support contact information.", 
   "Technical Difficulties...", 
   MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);


#endif

	_exit(1);
}  

void ReleaseCrashLocalized(const AsciiString& p, const AsciiString& m)
{
	if (!TheGameText) {
		ReleaseCrash(m.str());
		// This won't ever return
		return;
	}

	UnicodeString prompt = TheGameText->fetch(p);
	UnicodeString mesg = TheGameText->fetch(m);


	/// do additional reporting on the crash, if possible

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}

	if (TheSystemIsUnicode) 
	{
		::MessageBoxW(NULL, mesg.str(), prompt.str(), MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
	} 
	else 
	{
		// However, if we're using the default version of the message box, we need to 
		// translate the string into an AsciiString
		AsciiString promptA, mesgA;
		promptA.translate(prompt);
		mesgA.translate(mesg);
		//Make sure main window is not TOP_MOST
		::SetWindowPos(ApplicationHWnd, HWND_NOTOPMOST, 0, 0, 0, 0,SWP_NOSIZE |SWP_NOMOVE);
		::MessageBoxA(NULL, mesgA.str(), promptA.str(), MB_OK|MB_TASKMODAL|MB_ICONERROR);
	}

	char prevbuf[ _MAX_PATH ];
	char curbuf[ _MAX_PATH ];

	strcpy(prevbuf, TheGlobalData->getPath_UserData().str());
	strcat(prevbuf, RELEASECRASH_FILE_NAME_PREV);
	strcpy(curbuf, TheGlobalData->getPath_UserData().str());
	strcat(curbuf, RELEASECRASH_FILE_NAME);

 	remove(prevbuf);
	rename(curbuf, prevbuf);

	theReleaseCrashLogFile = fopen(curbuf, "w");
	if (theReleaseCrashLogFile)
	{
		fprintf(theReleaseCrashLogFile, "Release Crash at %s; Reason %s\n", getCurrentTimeString(), mesg.str());

		const int STACKTRACE_SIZE	= 12;
		const int STACKTRACE_SKIP = 6;
		void* stacktrace[STACKTRACE_SIZE];
		::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
		::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, releaseCrashLogOutput);

		fflush(theReleaseCrashLogFile);
		fclose(theReleaseCrashLogFile);
		theReleaseCrashLogFile = NULL;
	}

	_exit(1);
}
