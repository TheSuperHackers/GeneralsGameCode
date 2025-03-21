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

// DatGen.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "BFISH.H"
#include "SafeDisk/CdaPfn.h"
#include <Debug/DebugPrint.h>

void __cdecl doIt(void);

CDAPFN_DECLARE_GLOBAL(doIt, CDAPFN_OVERHEAD_L5, CDAPFN_CONSTRAINT_NONE);

static void doIt(void)
{
	// Generate passkey
	char passKey[128];
	passKey[0] = '\0';
	unsigned char installPath[MAX_PATH] = "";

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

	if (result != ERROR_SUCCESS)
	{
		return;
	}

	// Retrieve install path
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

	const char *plainText = "Play the \"Command & Conquer: Generals\" Multiplayer Test.";
	int textLen = strlen(plainText);
	char cypherText[128];

	DebugPrint("Retrieved PassKey: %s\n", passKey);

	// Decrypt protected data into the memory mapped file
	BlowfishEngine blowfish;
	int len = strlen(passKey);
	if (len > BlowfishEngine::MAX_KEY_LENGTH)
		len = BlowfishEngine::MAX_KEY_LENGTH;
	blowfish.Submit_Key(passKey, len);

	blowfish.Encrypt(plainText, textLen, cypherText);
	cypherText[textLen] = 0;
	DebugPrint("Encrypted data: %s\n", cypherText);

	DebugPrint("Install dir = '%s'\n", installPath);

	char *lastBackslash = strrchr((char *)installPath, '\\');
	if (lastBackslash)
		*lastBackslash = 0; // strip of \\game.exe from install path

	strcat((char *)installPath, "\\Generals.dat");

	DebugPrint("DAT file = '%s'\n", installPath);

	FILE *fp = fopen((char *)installPath, "wb");
	if (fp)
	{
		fwrite(cypherText, textLen, 1, fp);
		fclose(fp);
	}

	CDAPFN_ENDMARK(doIt);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	doIt();

	return 0;
}



