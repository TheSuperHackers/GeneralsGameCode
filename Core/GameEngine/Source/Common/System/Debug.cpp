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

// SYSTEM INCLUDES
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine


// USER INCLUDES

// TheSuperHackers @feature helmutbuhler 04/10/2025
// Uncomment this to show normal logging stuff in the crc logging.
// This can be helpful for context, but can also clutter diffs because normal logs aren't necessarily
// deterministic or the same on all peers in multiplayer games.
//#define INCLUDE_DEBUG_LOG_IN_CRC_LOG

#include "Common/CommandLine.h"
#include "Common/Debug.h"
#include "Common/CRCDebug.h"
#include "Common/UnicodeString.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/GameText.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#if defined(DEBUG_STACKTRACE) || defined(IG_DEBUG_STACKTRACE)
	#include "Common/StackDump.h"
#endif
#ifdef RTS_ENABLE_CRASHDUMP
#include "Common/MiniDumper.h"
#endif


// Horrible reference, but we really, really need to know if we are windowed.
extern bool DX8Wrapper_IsWindowed;
extern HWND ApplicationHWnd;

extern const char *gAppPrefix; /// So WB can have a different log file name.

#ifdef ALLOW_DEBUG_UTILS
// ----------------------------------------------------------------------------
static bool queryIgnoreAsserts()
{
	if (!DX8Wrapper_IsWindowed)
		return true;
	if (TheGlobalData && TheGlobalData->m_headless)
		return true;
#ifdef DEBUG_CRASHING
	if (TheGlobalData && TheGlobalData->m_debugIgnoreAsserts)
		return true;
#endif

	return false;
}

// ----------------------------------------------------------------------------
static void *queryMainWindow()
{
	return ApplicationHWnd;
}

#ifdef DEBUG_STACKTRACE
// ----------------------------------------------------------------------------
static void handleStackDump(void (*output)(const char *line))
{
	if (TheGlobalData && TheGlobalData->m_debugIgnoreStackTrace)
		return;

	const int STACKTRACE_SIZE	= 24;
	// Skips FillStackAddresses, this function and the debug function that calls it, so that the trace begins at DebugCrash.
	const int STACKTRACE_SKIP = 3;
	void* stacktrace[STACKTRACE_SIZE];

	output("\nStack Dump:");
	::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
	::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, output);
}
#endif

// ----------------------------------------------------------------------------
static void handleCrashIgnored()
{
	if( TheKeyboard )
		TheKeyboard->resetKeys();
	if( TheMouse )
		TheMouse->reset();
}

#ifdef INCLUDE_DEBUG_LOG_IN_CRC_LOG
// ----------------------------------------------------------------------------
static void handleLog(const char *buffer, const char *endline)
{
	addCRCDebugLineNoCounter("%s%s", buffer, endline);
}
#endif

// ----------------------------------------------------------------------------
// GameDebugInit
// ----------------------------------------------------------------------------
/**
	Initialize the debug utilities for an application that uses the game engine.
	This connects them to the game engine and opens the log file of the client instance.
*/
void GameDebugInit(int flags)
{
	// just quietly allow multiple calls to this, so that static ctors can call it.
	if (DebugGetFlags() != 0)
		return;

	DebugSetIgnoreAssertsQuery(queryIgnoreAsserts);
	DebugSetMainWindowQuery(queryMainWindow);
#ifdef DEBUG_STACKTRACE
	DebugSetStackDumpHandler(handleStackDump);
#endif
	DebugSetCrashIgnoredHandler(handleCrashIgnored);
#ifdef INCLUDE_DEBUG_LOG_IN_CRC_LOG
	DebugSetLogHandler(handleLog);
#endif

	DebugInit(flags);

#ifdef DEBUG_LOGGING
	// TheSuperHackers @info Debug initialization can happen very early.
	// Determine the client instance id before creating the log file with an instance specific name.
	CommandLine::parseCommandLineForStartup();

	if (!rts::ClientInstance::initialize())
		return;

	char suffix[32];
	suffix[0] = 0;
	if (rts::ClientInstance::getInstanceId() > 1u)
	{
		snprintf(suffix, ARRAY_SIZE(suffix), "_Instance%.2u", rts::ClientInstance::getInstanceId());
	}

	DebugOpenLogFile(gAppPrefix, suffix);
#endif
}
#endif // ALLOW_DEBUG_UTILS

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

	static FILE *theReleaseCrashLogFile = nullptr;

	static void releaseCrashLogOutput(const char *buffer)
	{
		if (theReleaseCrashLogFile)
		{
			fprintf(theReleaseCrashLogFile, "%s\n", buffer);
			fflush(theReleaseCrashLogFile);
		}
	}


static const char *getCurrentTimeString()
{
	time_t aclock;
	time(&aclock);
	struct tm *newtime = localtime(&aclock);
	return asctime(newtime);
}


static void TriggerMiniDump()
{
#ifdef RTS_ENABLE_CRASHDUMP
	if (TheMiniDumper && TheMiniDumper->IsInitialized())
	{
		// Create both minimal and full memory dumps
		TheMiniDumper->TriggerMiniDump(DumpType_Minimal);
		TheMiniDumper->TriggerMiniDump(DumpType_Full);
	}

	MiniDumper::shutdownMiniDumper();
#endif
}


void ReleaseCrash(const char *reason)
{
	const DebugCrashHandler crashHandler = DebugGetCrashHandler();
	if (crashHandler != nullptr)
	{
		crashHandler(reason);
	}

	/// do additional reporting on the crash, if possible

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}

	TriggerMiniDump();

	char prevbuf[ _MAX_PATH ];
	char curbuf[ _MAX_PATH ];

	if (TheGlobalData==nullptr) {
		return; // We are shutting down, and TheGlobalData has been freed.  jba. [4/15/2003]
	}

	strlcpy(prevbuf, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(prevbuf));
	strlcat(prevbuf, RELEASECRASH_FILE_NAME_PREV, ARRAY_SIZE(prevbuf));
	strlcpy(curbuf, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(curbuf));
	strlcat(curbuf, RELEASECRASH_FILE_NAME, ARRAY_SIZE(curbuf));

 	remove(prevbuf);
	if (rename(curbuf, prevbuf) != 0)
	{
#ifdef DEBUG_LOGGING
		DebugLog("Warning: Could not rename buffer file '%s' to '%s'. Will remove instead", curbuf, prevbuf);
#endif
		if (remove(curbuf) != 0)
		{
#ifdef DEBUG_LOGGING
			DebugLog("Warning: Failed to remove file '%s'", curbuf);
#endif
		}
	}

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
		theReleaseCrashLogFile = nullptr;
	}

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}

#if defined(RTS_DEBUG)
	/* static */ char buff[8192]; // not so static so we can be threadsafe
	snprintf(buff, 8192, "Sorry, a serious error occurred. (%s)", reason);
	if (crashHandler == nullptr && !(TheGlobalData && TheGlobalData->m_headless))
	{
		::MessageBox(nullptr, buff, "Technical Difficulties...", MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
	}
#else
// crash error messaged changed 3/6/03 BGC
//	::MessageBox(nullptr, "Sorry, a serious error occurred.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);
//	::MessageBox(nullptr, "You have encountered a serious error.  Serious errors can be caused by many things including viruses, overheated hardware and hardware that does not meet the minimum specifications for the game. Please visit the forums at www.generals.ea.com for suggested courses of action or consult your manual for Technical Support contact information.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);

// crash error message changed again 8/22/03 M Lorenzen... made this message box modal to the system so it will appear on top of any task-modal windows, splash-screen, etc.
	if (crashHandler == nullptr && !(TheGlobalData && TheGlobalData->m_headless))
	{
		::MessageBox(nullptr, "You have encountered a serious error.  Serious errors can be caused by many things including viruses, overheated hardware and hardware that does not meet the minimum specifications for the game. Please visit the forums at www.generals.ea.com for suggested courses of action or consult your manual for Technical Support contact information.",
			"Technical Difficulties...",
			MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
	}


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

	const DebugCrashHandler crashHandler = DebugGetCrashHandler();
	if (crashHandler != nullptr)
	{
		crashHandler(m.str());
	}

	TriggerMiniDump();

	UnicodeString prompt = TheGameText->fetch(p);
	UnicodeString mesg = TheGameText->fetch(m);


	/// do additional reporting on the crash, if possible

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationHWnd) {
			ShowWindow(ApplicationHWnd, SW_HIDE);
		}
	}

	if (crashHandler == nullptr && !(TheGlobalData && TheGlobalData->m_headless))
	{
		::MessageBoxW(nullptr, mesg.str(), prompt.str(), MB_OK | MB_SYSTEMMODAL | MB_ICONERROR);
	}

	char prevbuf[ _MAX_PATH ];
	char curbuf[ _MAX_PATH ];

	strlcpy(prevbuf, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(prevbuf));
	strlcat(prevbuf, RELEASECRASH_FILE_NAME_PREV, ARRAY_SIZE(prevbuf));
	strlcpy(curbuf, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(curbuf));
	strlcat(curbuf, RELEASECRASH_FILE_NAME, ARRAY_SIZE(curbuf));

 	remove(prevbuf);
	if (rename(curbuf, prevbuf) != 0)
	{
#ifdef DEBUG_LOGGING
		DebugLog("Warning: Could not rename buffer file '%s' to '%s'. Will remove instead", curbuf, prevbuf);
#endif
		if (remove(curbuf) != 0)
		{
#ifdef DEBUG_LOGGING
			DebugLog("Warning: Failed to remove file '%s'", curbuf);
#endif
		}
	}

	theReleaseCrashLogFile = fopen(curbuf, "w");
	if (theReleaseCrashLogFile)
	{
		fprintf(theReleaseCrashLogFile, "Release Crash at %s; Reason %ls\n", getCurrentTimeString(), mesg.str());

		const int STACKTRACE_SIZE	= 12;
		const int STACKTRACE_SKIP = 6;
		void* stacktrace[STACKTRACE_SIZE];
		::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
		::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, releaseCrashLogOutput);

		fflush(theReleaseCrashLogFile);
		fclose(theReleaseCrashLogFile);
		theReleaseCrashLogFile = nullptr;
	}

	_exit(1);
}
