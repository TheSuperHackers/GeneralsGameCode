/*
**	Command & Conquer Generals(tm)
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

// FILE: WOLinit.cpp //////////////////////////////////////////////////////
// Westwood Online DLL/COM/ initialization/teardown
// Author: Matthew D. Campbell, December 2001

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <snmp.h>
#include <winreg.h>

#include <atlbase.h>
extern CComModule _Module;  // Required for COM - must be between atlbase.h and atlcom.h.  Funky, no?
#include <atlcom.h>

#include <stdio.h>
#include <stdarg.h>

#include "wolSetup.h"
#include "WOLAPI/wolapi.h"

unsigned long g_wolapiRegistryVersion = 0;
unsigned long g_wolapiRealVersion = 0;
bool g_wolapiInstalled = false;
char g_wolapiRegFilename[MAX_PATH];
char g_wolapiRealFilename[MAX_PATH];
char g_generalsFilename[MAX_PATH];
char g_generalsSerial[1024];

#define GENERALS_REG_KEY_TOP "HKEY_LOCAL_MACHINE"														///< Registry base
#define GENERALS_REG_KEY_PATH "SOFTWARE\\Westwood\\Generals"								///< Generals registry key
#define GENERALS_REG_KEY_BOTTOM GENERALS_REG_KEY_PATH "\\"									///< Generals registry key with trailing backslashes
#define GENERALS_REG_KEY_VERSION "Version"																	///< Version registry key
#define GENERALS_REG_KEY_SKU "SKU"																					///< SKU registry key
#define GENERALS_REG_KEY_NAME "Name"																				///< Product name registry key
#define GENERALS_REG_KEY_INSTALLPATH "InstallPath"													///< Install path registry key
#define GENERALS_REG_KEY_SERIAL "Serial"																		///< Serial # registry key
#define GENERALS_REG_KEY GENERALS_REG_KEY_TOP "\\" GENERALS_REG_KEY_BOTTOM	///< Full Generals registry path

#define WOLAPI_REG_KEY_TOP "HKEY_LOCAL_MACHINE"															///< Registry base
#define WOLAPI_REG_KEY_PATH "SOFTWARE\\Westwood\\WOLAPI"										///< WOLAPI registry key
#define WOLAPI_REG_KEY_BOTTOM WOLAPI_REG_KEY_PATH "\\"											///< WOLAPI registry key with trailing backslashes
#define WOLAPI_REG_KEY_VERSION "Version"																		///< Version registry key
#define WOLAPI_REG_KEY_INSTALLPATH "InstallPath"														///< Install path registry key
#define WOLAPI_REG_KEY WOLAPI_REG_KEY_TOP "\\" WOLAPI_REG_KEY_BOTTOM				///< Full WOLAPI registry path

#define DLL_REG_KEY_TOP "HKEY_CLASSES_ROOT"															///< Registry base
#define DLL_REG_KEY_PATH "CLSID\\{18FD6763-F5EA-4fa5-B2A9-668554152FAE}\\InprocServer32"										///< WOLAPI registry key
#define DLL_REG_KEY_BOTTOM DLL_REG_KEY_PATH "\\"											///< WOLAPI registry key with trailing backslashes
#define DLL_REG_KEY_LOCATION ""																		///< Version registry key

void getPathsFromRegistry( void )
{
	HKEY handle;
	unsigned long type;
	unsigned long size;
	int returnValue;

	size = sizeof(g_generalsFilename);
	strcpy(g_generalsFilename, "No install path in registry");

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, GENERALS_REG_KEY_PATH, 0, KEY_ALL_ACCESS, &handle ) == ERROR_SUCCESS) {

		returnValue = RegQueryValueEx(handle, GENERALS_REG_KEY_INSTALLPATH, NULL, &type, (unsigned char *) &g_generalsFilename, &size);

		if (returnValue != ERROR_SUCCESS)
		{
			strcpy(g_generalsFilename, "No install path in registry");
		}

		RegCloseKey( handle );
	}

	size = sizeof(g_generalsSerial);
	strcpy(g_generalsSerial, "0");

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, GENERALS_REG_KEY_PATH, 0, KEY_ALL_ACCESS, &handle ) == ERROR_SUCCESS) {

		returnValue = RegQueryValueEx(handle, GENERALS_REG_KEY_SERIAL, NULL, &type, (unsigned char *) &g_generalsSerial, &size);

		if (returnValue != ERROR_SUCCESS)
		{
			strcpy(g_generalsSerial, "0");
		}

		RegCloseKey( handle );
	}

	size = sizeof(g_wolapiRegFilename);
	strcpy(g_wolapiRegFilename, "No install path in registry");
	g_wolapiInstalled = true;

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, WOLAPI_REG_KEY_PATH, 0, KEY_ALL_ACCESS, &handle ) == ERROR_SUCCESS) {

		returnValue = RegQueryValueEx(handle, WOLAPI_REG_KEY_INSTALLPATH, NULL, &type, (unsigned char *) &g_wolapiRegFilename, &size);

		if (returnValue != ERROR_SUCCESS)
		{
			strcpy(g_wolapiRegFilename, "No install path in registry");
			g_wolapiInstalled = false;
		}

		RegCloseKey( handle );
	}

	size = sizeof(g_wolapiRealFilename);
	strcpy(g_wolapiRealFilename, "No wolapi.dll installed");

	if (RegOpenKeyEx( HKEY_CLASSES_ROOT, DLL_REG_KEY_PATH, 0, KEY_ALL_ACCESS, &handle ) == ERROR_SUCCESS) {

		returnValue = RegQueryValueEx(handle, DLL_REG_KEY_LOCATION, NULL, &type, (unsigned char *) &g_wolapiRealFilename, &size);

		if (returnValue != ERROR_SUCCESS)
		{
			strcpy(g_wolapiRealFilename, "No wolapi.dll installed");
			g_wolapiInstalled = false;
		}

		RegCloseKey( handle );
	}
}

void setupGenerals( const char *genPath, const char *genSerial )
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if (RegCreateKeyEx( HKEY_LOCAL_MACHINE, GENERALS_REG_KEY_PATH, 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &handle, NULL ) == ERROR_SUCCESS) {

		type = REG_SZ;
		size = strlen(genPath)+1;
		returnValue = RegSetValueEx(handle, GENERALS_REG_KEY_INSTALLPATH, 0, type, (unsigned char *) genPath, size);

		size = strlen(genSerial)+1;
		returnValue = RegSetValueEx(handle, GENERALS_REG_KEY_SERIAL, 0, type, (unsigned char *) genSerial, size);

		size = strlen("Generals")+1;
		returnValue = RegSetValueEx(handle, GENERALS_REG_KEY_NAME, 0, type, (unsigned char *) "Generals", size);

		type = REG_DWORD;
		size = sizeof(DWORD);
		unsigned long value = 65536;
		returnValue = RegSetValueEx(handle, GENERALS_REG_KEY_VERSION, 0, type, (unsigned char *) &value, size);
		value = 12544;
		returnValue = RegSetValueEx(handle, GENERALS_REG_KEY_SKU, 0, type, (unsigned char *) &value, size);

		RegCloseKey( handle );
	}

}


/**
	* OLEInitializer class - Init and shutdown OLE & COM as a global
	* object.  Scary, nasty stuff, COM.  /me shivers.
	*/
class OLEInitializer
{
 public:
          OLEInitializer() { OleInitialize(NULL); }
         ~OLEInitializer() { OleUninitialize(); }
};
OLEInitializer g_OLEInitializer;
CComModule _Module;

IChat *g_pChat = NULL;

/**
	* checkInstalledWolapiVersion inits WOLAPI if possible and gets its version
	* number.  It also saves off its install path from the registry.
	*/
void checkInstalledWolapiVersion( void )
{
	// Initialize this instance
	_Module.Init(NULL, g_hInst);

	// Create the WOLAPI instance
	CoCreateInstance(CLSID_Chat, NULL, CLSCTX_INPROC_SERVER, \
					 IID_IChat, (void**)&g_pChat);

	if (g_pChat)
	{
		// Grab versions
		g_pChat->GetVersion(&g_wolapiRealVersion);

		// Release everything
		g_pChat->Release();

		g_wolapiInstalled = true;
	}

	_Module.Term();

	// Grab path info from registry
	getPathsFromRegistry();

	return;
}

