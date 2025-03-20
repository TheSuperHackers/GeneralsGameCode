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
*     $Archive: /Renegade Setup/Autorun/ViewHTML.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Maria_l $
*
* VERSION INFO
*     $Modtime: 2/16/01 11:32a $
*     $Revision: 3 $
*
******************************************************************************/

#pragma warning(disable : 4201 4310)
#include	<windows.h>

#include "ViewHTML.h"
//#include "..\win.h"
#include <stdio.h>
//#include "debugprint.h"
#include "Wnd_File.h"


/******************************************************************************
*
* NAME
*     ViewHTML
*
* DESCRIPTION
*     Launch the default browser to view the specified URL
*
* INPUTS
*     URL      - Website address
*     Wait     - Wait for user to close browser (default = false)
*     Callback - User callback to invoke during wait (default = NULL callback)
*
* RESULT
*     Success - True if successful; otherwise false
*
******************************************************************************/

bool ViewHTML(const char* url, bool wait, CallbackHook& callback)
	{
//	DebugPrint("ViewHTML()\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ));
	Msg( __LINE__, TEXT(__FILE__), TEXT("ViewHTML()" ));
	Msg( __LINE__, TEXT(__FILE__), TEXT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ));

	//--------------------------------------------------------------------------
	// Just return if no URL specified
	//--------------------------------------------------------------------------
	if ((url == NULL) || (strlen(url) == 0))
		{
//		DebugPrint("***** No URL specified.\n");
		Msg( __LINE__, TEXT(__FILE__), TEXT("***** No URL specified." ));
		return false;
		}

	//--------------------------------------------------------------------------
	// Create unique temporary HTML filename
	// JFS: Fixed so that it would go to the temp folder which was crashing
	// on limited users.
	//--------------------------------------------------------------------------
	char tempPath[MAX_PATH];
	char filename1[MAX_PATH];
	char filename2[MAX_PATH];
	
	// Expand the TMP environment variable. 
	{ 
		DWORD dwResult;
		dwResult = ExpandEnvironmentStrings( "%TEMP%", tempPath,  MAX_PATH);
		if(dwResult == 0)
			return false;
	}

	GetTempFileName(tempPath, "WS", 0, filename1);

	strcpy( filename2, filename1 );
	char* extPtr = strrchr(filename2, '.');
	strcpy(extPtr, ".html");


//	DebugPrint(filename);
	Msg( __LINE__, TEXT(__FILE__), TEXT("filename = %s"), filename2 );

	//--------------------------------------------------------------------------
	// Create file
	//--------------------------------------------------------------------------
	HANDLE file = CreateFile(
					filename2, 
					GENERIC_WRITE, 
					0, 
					NULL, 
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL, 
					NULL);

	if (file == INVALID_HANDLE_VALUE)
		{
//		DebugPrint("***** Unable to create temporary HTML file '%s'\n", filename);
		Msg( __LINE__, TEXT(__FILE__), TEXT("***** Unable to create temporary HTML file '%s"), filename2 );
		return false;
		}

	// Write generic contents
	const char* contents = "<title>ViewHTML</title>";
	DWORD written;
	WriteFile(file, contents, strlen(contents), &written, NULL);
	CloseHandle(file);

	// Find the executable that can launch this file
	char exeName[MAX_PATH];
	HINSTANCE hInst = FindExecutable(filename2, NULL, exeName);

	// Delete temporary file
	DeleteFile(filename2);
	DeleteFile(filename1);

	if ((int)hInst <= 32)
		{
//		DebugPrint("***** Unable to find executable that will display HTML files.\n");
		Msg( __LINE__, TEXT(__FILE__), TEXT("***** Unable to find executable that will display HTML files."));
		return false;
		}

	// Launch browser with specified URL
	char commandLine[MAX_PATH];
	sprintf(commandLine, "[open] %s", url);

	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
  
	PROCESS_INFORMATION processInfo;

	BOOL createSuccess = CreateProcess(
			exeName, 
			commandLine, 
			NULL, 
			NULL, 
			FALSE,
			0, 
			NULL, 
			NULL, 
			&startupInfo, 
			&processInfo);

	if (createSuccess == FALSE)
		{
//		DebugPrint("\t**** Failed to CreateProcess(%s, %s)\n", exeName, commandLine);
		Msg( __LINE__, TEXT(__FILE__), TEXT("\t**** Failed to CreateProcess(%s, %s)"), exeName, commandLine );
		return false;
		}

	if (wait == true)
		{
		WaitForInputIdle(processInfo.hProcess, 5000);

		bool waiting = true;

		while (waiting == true)
			{
			if (callback.DoCallback() == true)
				{
				break;
				}
			
			Sleep(100);

			DWORD exitCode;
			GetExitCodeProcess(processInfo.hProcess, &exitCode);

			if (exitCode != STILL_ACTIVE)
				{
				waiting = false;
				}
			}
		}

	return true;
	}
