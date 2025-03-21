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

/******************************************************************************
*
* FILE
*     $Archive: /APILauncher/Protect.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Mcampbell $
*
* VERSION INFO
*     $Modtime: 6/21/01 5:09p $
*     $Revision: 6 $
*
******************************************************************************/

#include "Protect.h"

#ifdef COPY_PROTECT

#include "BFISH.H"
#include <Support/UString.h>
#include <Support/RefPtr.h>
#include <Storage/File.h>
#include <windows.h>
#include <memory>
#include <assert.h>
#include <stdio.h>
#include <Debug/DebugPrint.h>

#include "SafeDisk/CdaPfn.h"

// This GUID should be unique for each product. (CHANGE IT WHEN DOING THE
// NEXT PRODUCT) Note that the game will need to agree on this GUID also, so
// the game will have to be modified also.
const char* const LAUNCHER_GUID =
	"150C6462-4E49-4ccf-B073-57579569D994"; // Generals Multiplayer Test Launcher GUID
const char* const protectGUID =
	"6096561D-8A70-48ed-9FF8-18552419E50D"; // Generals Multiplayer Test Protect GUID

/*
const char* const LAUNCHER_GUID =
	"FB327081-64F2-43d8-9B72-503C3B765134"; // Generals Launcher GUID
const char* const protectGUID =
	"7BEB9006-CC19-4aca-913A-C870A88DE01A"; // Generals Protect GUID
*/

HANDLE mLauncherMutex = NULL;
HANDLE mMappedFile = NULL;

void InitializeProtect(void)
{
	ShutdownProtect();

	DebugPrint("Initializing protection\n");

	mLauncherMutex = NULL;
	mMappedFile = NULL;

	// Secure launcher mutex
	mLauncherMutex = CreateMutex(NULL, FALSE, LAUNCHER_GUID);

	if ((mLauncherMutex == NULL) || (mLauncherMutex && (GetLastError() == ERROR_ALREADY_EXISTS)))
	{
		DebugPrint("***** Failed to create launcher mutex\n");
		return;
	}

	// Create memory mapped file to mirror protected file
	File file("Generals.dat", Rights_ReadOnly);

	if (!file.IsAvailable())
	{
		DebugPrint("***** Unable to find Generals.dat\n");
		return;
	}

	UInt32 fileSize = file.GetLength();

	SECURITY_ATTRIBUTES security;
	security.nLength = sizeof(security);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;

	mMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, &security, PAGE_READWRITE, 0, fileSize, NULL);

	if ((mMappedFile == NULL) || (mMappedFile && (GetLastError() == ERROR_ALREADY_EXISTS)))
	{
		PrintWin32Error("***** CreateFileMapping() Failed!");
		CloseHandle(mMappedFile);
		mMappedFile = NULL;
		return;
	}
}

CDAPFN_DECLARE_GLOBAL(SendProtectMessage, CDAPFN_OVERHEAD_L5, CDAPFN_CONSTRAINT_NONE);

void SendProtectMessage(HANDLE process, DWORD threadID)
{
	// Decrypt protected file contents to mapping file
	File file("Generals.dat", Rights_ReadOnly);

	if (!file.IsAvailable())
	{
		DebugPrint("***** Unable to find Generals.dat\n");
		return;
	}

	// Map file to programs address space
	LPVOID mapAddress = MapViewOfFileEx(mMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

	if (mapAddress == NULL)
	{
		PrintWin32Error("***** MapViewOfFileEx() Failed!");
		return;
	}

	void* buffer = NULL;
	UInt32 bufferSize = 0;
	file.Load(buffer, bufferSize);

	if (buffer && (bufferSize > 0))
	{
		DebugPrint("Generating PassKey\n");

		// Generate passkey
		char passKey[128];
		passKey[0] = '\0';

		// Get game information
		HKEY hKey;
		bool usesHKeycurrentUser = false;
		LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour", 0, KEY_READ, &hKey);
		if (result != ERROR_SUCCESS)
		{
			result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour", 0, KEY_READ, &hKey);
			usesHKeycurrentUser = true;
		}
		assert((result == ERROR_SUCCESS) && "Failed to open game registry key");

		if (result == ERROR_SUCCESS)
		{
			// Retrieve install path
			unsigned char installPath[MAX_PATH];
			DWORD type;
			DWORD sizeOfBuffer = sizeof(installPath);
			result = RegQueryValueEx(hKey, "InstallPath", NULL, &type, installPath, &sizeOfBuffer);

			assert((result == ERROR_SUCCESS) && "Failed to obtain game install path!");
			assert((strlen((const char*)installPath) > 0) && "Game install path invalid!");
			DebugPrint("Game install path: %s\n", installPath);

			// Retrieve Hard drive S/N
			char drive[8];
			_splitpath((const char*)installPath, drive, NULL, NULL, NULL);
			strcat(drive, "\\");

			DWORD volumeSerialNumber = 0;
			DWORD maxComponentLength;
			DWORD fileSystemFlags;
			BOOL volInfoSuccess = GetVolumeInformation((const char*)drive, NULL, 0,
														&volumeSerialNumber, &maxComponentLength, &fileSystemFlags, NULL, 0);

			if (volInfoSuccess == FALSE)
			{
				PrintWin32Error("***** GetVolumeInformation() Failed!");
			}

			DebugPrint("Drive Serial Number: %lx\n", volumeSerialNumber);

			// Add hard drive serial number portion
			char volumeSN[16];
			sprintf(volumeSN, "%lx-", volumeSerialNumber);
			strcat(passKey, volumeSN);

			// Retrieve game serial #
			unsigned char gameSerialNumber[64];
			gameSerialNumber[0] = '\0';
			sizeOfBuffer = sizeof(gameSerialNumber);
			if (usesHKeycurrentUser)
			{
				result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour\\ergc", 0, KEY_READ, &hKey);
			}
			else
			{
				result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour\\ergc", 0, KEY_READ, &hKey);
			}
			assert((result == ERROR_SUCCESS) && "Failed to open game serial registry key");

			if (result == ERROR_SUCCESS)
			{
				result = RegQueryValueEx(hKey, "", NULL, &type, gameSerialNumber, &sizeOfBuffer);
				assert((result == ERROR_SUCCESS) && "Failed to obtain game serial number!");
				assert((strlen((const char*)gameSerialNumber) > 0) && "Game serial number invalid!");
			}

			DebugPrint("Game serial number: %s\n", gameSerialNumber);

			RegCloseKey(hKey);

			// Add game serial number portion
			strcat(passKey, (char*)gameSerialNumber);
		}

		// Obtain windows product ID
		result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);
		assert((result == ERROR_SUCCESS) && "Failed to open windows registry key!");

		if (result == ERROR_SUCCESS)
		{
			// Retrieve Windows Product ID
			unsigned char winProductID[64];
			winProductID[0] = '\0';

			DWORD type;
			DWORD sizeOfBuffer = sizeof(winProductID);
			result = RegQueryValueEx(hKey, "ProductID", NULL, &type, winProductID, &sizeOfBuffer);

			assert((result == ERROR_SUCCESS) && "Failed to obtain windows product ID!");
			assert((strlen((const char*)winProductID) > 0) && "Invalid windows product ID");

			DebugPrint("Windows Product ID: %s\n", winProductID);

			RegCloseKey(hKey);

			// Add windows product ID portion
			strcat(passKey, "-");
			strcat(passKey, (char*)winProductID);
		}

		DebugPrint("Retrieved PassKey: %s\n", passKey);

		// Decrypt protected data into the memory mapped file
		BlowfishEngine blowfish;
		int len = strlen(passKey);
		if (len > BlowfishEngine::MAX_KEY_LENGTH)
			len = BlowfishEngine::MAX_KEY_LENGTH;
		blowfish.Submit_Key(passKey, len);
		blowfish.Decrypt(buffer, bufferSize, mapAddress);

		DebugPrint("Decrypted data: %s\n", mapAddress);

		free(buffer);
	}

	UnmapViewOfFile(mapAddress);

	//---------------------------------------------------------------------------
	// Send protection message
	//---------------------------------------------------------------------------
	DebugPrint("Sending protect message\n");

	DebugPrint("Creating running notification event.\n");
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, protectGUID);

	if ((event == NULL) || (event && (GetLastError() == ERROR_ALREADY_EXISTS)))
	{
		PrintWin32Error("***** CreateEvent() Failed!");
		return;
	}

	DebugPrint("Waiting for game (timeout in %.02f seconds)...\n", ((float)((5 * 60) * 1000) * 0.001));

#ifdef _DEBUG
	unsigned long start = timeGetTime();
#endif

	HANDLE handles[2];
	handles[0] = event;
	handles[1] = process;
	DWORD waitResult = WaitForMultipleObjects(2, &handles[0], FALSE, ((5 * 60) * 1000));

#ifdef _DEBUG
	unsigned long stop = timeGetTime();
#endif

	DebugPrint("WaitResult = %ld (WAIT_OBJECT_0 = %ld)\n", waitResult, WAIT_OBJECT_0);

	if (waitResult == WAIT_OBJECT_0)
	{
		if (mMappedFile != NULL)
		{
			DebugPrint("Sending game the beef. (%lx)\n", mMappedFile);
			BOOL sent = PostThreadMessage(threadID, 0xBEEF, 0, (LPARAM)mMappedFile);
			assert(sent == TRUE);
		}
	}
	else
	{
		DebugPrint("***** Timeout!\n");
	}

#ifdef _DEBUG
	DebugPrint("Waited %.02f seconds\n", ((float)(stop - start) * 0.001));
#endif

	CloseHandle(event);
	CDAPFN_ENDMARK(SendProtectMessage);
}


void ShutdownProtect(void)
{
	if (mMappedFile)
	{
		CloseHandle(mMappedFile);
		mMappedFile = NULL;
	}

	if (mLauncherMutex)
	{
		CloseHandle(mLauncherMutex);
		mLauncherMutex = NULL;
	}
}

#endif // COPY_PROTECT
