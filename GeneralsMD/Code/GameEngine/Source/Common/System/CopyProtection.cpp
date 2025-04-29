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

// FILE: CopyProtection.cpp //////////////////////////////////////////////////
// Author: Matthew D. Campbell
// Taken From: Denzil Long's code in Tiberian Sun, by way of Yuri's Revenge
//////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/CopyProtection.h"

#ifndef DO_COPY_PROTECTION
//#pragma MESSAGE("*********************************************************")
//#pragma MESSAGE("************* COPY PROTECTION IS DISABLED ***************")
//#pragma MESSAGE("*********************************************************")
#else

LPVOID CopyProtect::s_protectedData = NULL;
static const char* const LAUNCHER_GUID =
	"150C6462-4E49-4ccf-B073-57579569D994"; // Generals Multiplayer Test Launcher GUID
static const char* const protectGUID =
	"6096561D-8A70-48ed-9FF8-18552419E50D"; // Generals Multiplayer Test Protect GUID

/*
static const char* const LAUNCHER_GUID =
	"FB327081-64F2-43d8-9B72-503C3B765134"; // Generals Launcher GUID
static const char* const protectGUID =
	"7BEB9006-CC19-4aca-913A-C870A88DE01A"; // Generals Protect GUID
*/

#if defined(RTS_DEBUG) || defined(_INTERNAL)
Bool skipProtection(void)
{
	//return FALSE;
	if (FindWindow("Afx:400000:8:10011:0:fe8e0125", NULL) != NULL) // DevStudio
	{
		DEBUG_LOG(("DevStudio is running - skipping notifyLauncher()\n"));
		return TRUE;
	}
	//return FALSE;
	return TRUE;
}
#endif

// ---------------------------------------------------------------------------
Bool CopyProtect::isLauncherRunning(void)
{
	DEBUG_LOG(("COPYPROTECTION - Checking if launcher is running\n"));

	HANDLE launcherMutex = CreateMutex(NULL, FALSE, LAUNCHER_GUID);

	Bool isRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

	if (launcherMutex != NULL)
	{
		CloseHandle(launcherMutex);
	}
	DEBUG_LOG(("result was %d\n", (int)isRunning));

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	if (skipProtection())
	{
		DEBUG_LOG(("DevStudio is running - forcing to TRUE\n"));
		return TRUE;
	}
#endif

	return isRunning;
}

// ---------------------------------------------------------------------------
Bool CopyProtect::notifyLauncher(void)
{
	DEBUG_LOG(("COPYPROTECTION - Notify launcher\n"));

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	if (skipProtection())
	{
		DEBUG_LOG(("DevStudio is running - skipping notifyLauncher()\n"));
		return TRUE;
	}
#endif

	// Force system to create message queue
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// Signal launcher to send the beef
	unsigned long eventTime = (timeGetTime() + 60000);

	HANDLE event = NULL;

	while (timeGetTime() < eventTime)
	{
		event = OpenEvent(EVENT_MODIFY_STATE, TRUE, protectGUID);

		if (event)
		{
			break;
		}

		Sleep(0);
	}

	if (event)
	{
		SetEvent(event);
		CloseHandle(event);
		DEBUG_LOG(("Launcher notified.\n"));
		DEBUG_LOG(("Waiting for message from launcher.\n"));
		
		unsigned long endTime = (timeGetTime() + 10000);

		while (timeGetTime() <= endTime)
		{
			if (PeekMessage(&msg, NULL, 0xBEEF, 0xBEEF, PM_REMOVE))
			{
				// 0xBEEF is in the WM_APP - 0xBFFF range
				if (msg.message == 0xBEEF)
				{
					DEBUG_LOG(("COPYPROTECTION - Received message from launcher (Elapsed time %ld).\n",
						(10000 - (endTime - timeGetTime()))));

					HANDLE mappedFile = (HANDLE)msg.lParam;
					s_protectedData = MapViewOfFileEx(mappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

					if (s_protectedData == NULL)
					{
						DEBUG_LOG(("***** MapViewOfFileEx() Failed!\n"));
						break;
					}

					DEBUG_LOG(("The message says: %s\n", s_protectedData));

					return TRUE;
				}
			}

			Sleep(0);
		}

		DEBUG_LOG(("***** Failed to notify launcher!\n"));
		return FALSE;
		
	}
	else
	{
		DEBUG_LOG(("***** Failed to notify launcher!\n"));
	}

	return FALSE;
}


// ---------------------------------------------------------------------------
void CopyProtect::checkForMessage(UINT message, LPARAM lParam)
{
	// 0xBEEF is in the WM_APP - 0xBFFF range
	if (message == 0xBEEF)
	{
		DEBUG_LOG(("COPYPROTECTION - Received message from launcher.\n"));

		HANDLE mappedFile = (HANDLE)lParam;
		s_protectedData = MapViewOfFileEx(mappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

		if (s_protectedData == NULL)
		{
			DEBUG_LOG(("***** MapViewOfFileEx() Failed!"));
			return;
		}

		DEBUG_LOG(("The message says: %s\n", s_protectedData));
	}
}

// ---------------------------------------------------------------------------
Bool CopyProtect::validate(void)
{
	DEBUG_LOG(("COPYPROTECTION - Validating\n"));
	DEBUG_LOG(("s_protectedData = %d (%s)\n", s_protectedData, (s_protectedData)?s_protectedData:"EEK!"));

	if (s_protectedData != NULL)
	{
		return (strcmp((const char*)s_protectedData,
			"Play the \"Command & Conquer: Generals\" Multiplayer Test.") == 0);
	}

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	if (skipProtection())
	{
		DEBUG_LOG(("DevStudio is running - forcing to TRUE\n"));
		return TRUE;
	}
#endif

	return FALSE;
}

// ---------------------------------------------------------------------------
void CopyProtect::shutdown(void)
{
	DEBUG_LOG(("COPYPROTECTION - Shutdown\n"));

	if (s_protectedData != NULL)
	{
		UnmapViewOfFile(s_protectedData);
		s_protectedData = NULL;
	}
}

#endif // DO_COPY_PROTECTION
