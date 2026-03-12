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

// Registry.cpp
// Simple interface for storing/retrieving registry values
// Author: Matthew D. Campbell, December 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Registry.h"

// GeneralsX @build felipebraz 11/02/2026 Phase 1.5 - Linux port
// Windows Registry types not available on Linux - define stub types
#ifdef _UNIX
#ifndef HKEY_LOCAL_MACHINE
typedef void* HKEY;  // Stub type for Linux (unused but needed for compilation)
#define HKEY_LOCAL_MACHINE  ((HKEY)(uintptr_t)0x80000002)
#define HKEY_CURRENT_USER   ((HKEY)(uintptr_t)0x80000001)
#endif
#include <ctype.h>   // toupper
#include <stdlib.h>  // getenv
#include <sys/stat.h> // stat
#include <unistd.h>  // getcwd
#endif

// GeneralsX @build felipebraz 11/02/2026 Phase 1.5 - Linux port
// Windows Registry API not available on Linux.
// Approach adapted from fighter19's DXVK port: map registry keys to
// environment variables (CNC_GENERALS_<KEY>, CNC_ZH_<KEY>), with a
// relative-path fallback for the standard project layout.
#ifdef _UNIX

// Low-level helpers - no Win32 registry on Linux, always return FALSE.
Bool  getStringFromRegistry(HKEY root, AsciiString path, AsciiString key, AsciiString& val)
{
	return FALSE;
}

Bool getUnsignedIntFromRegistry(HKEY root, AsciiString path, AsciiString key, UnsignedInt& val)
{
	return FALSE;
}

Bool setStringInRegistry( HKEY root, AsciiString path, AsciiString key, AsciiString val)
{
	return FALSE;
}

Bool setUnsignedIntInRegistry( HKEY root, AsciiString path, AsciiString key, UnsignedInt val)
{
	return FALSE;
}

// GeneralsX @feature felipebraz 25/02/2026 Environment variable registry for Linux
// Maps registry key lookups to environment variables so Linux users can
// configure install paths without a Windows registry.
// Pattern: CNC_GENERALS_<UPPERCASED_KEY> for base Generals,
//          CNC_ZH_<UPPERCASED_KEY> for Zero Hour.
// Example: export CNC_GENERALS_INSTALLPATH=/path/to/generals/
static Bool getEnvVar(const char *prefix, AsciiString key, AsciiString& val)
{
	char envName[256] = { 0 };
	const char* keyStr = key.str();
	int prefixLen = strlen(prefix);

	if (prefixLen + strlen(keyStr) + 1 > sizeof(envName))
		return FALSE;

	strcpy(envName, prefix);
	for (int i = 0; keyStr[i] != '\0'; ++i)
		envName[prefixLen + i] = toupper((unsigned char)keyStr[i]);
	envName[prefixLen + strlen(keyStr)] = '\0';

	const char* envValue = getenv(envName);
	if (envValue && envValue[0] != '\0')
	{
		val = envValue;
		DEBUG_LOG(("getEnvVar - found %s = %s", envName, envValue));
		return TRUE;
	}

	return FALSE;
}

// GeneralsX @feature felipebraz 18/02/2026 Auto-detect base Generals install path
// Steam ZH standalone puts base Generals assets in ./ZH_Generals/ (subdir).
// Dev split install puts them in ../Generals/ (sibling dir).
// Check both in priority order.
static Bool tryRelativeGeneralsPath(AsciiString& val)
{
	struct stat st;

	// Steam standalone ZH: base Generals assets live in ZH_Generals/ subdirectory
	const char *steamPath = "ZH_Generals/";
	if (stat(steamPath, &st) == 0 && S_ISDIR(st.st_mode))
	{
		val = steamPath;
		DEBUG_LOG(("tryRelativeGeneralsPath - found base Generals at %s (Steam standalone)", steamPath));
		return TRUE;
	}

	// Dev split install: Generals and ZH installed side-by-side under same parent
	const char *devPath = "../Generals/";
	if (stat(devPath, &st) == 0 && S_ISDIR(st.st_mode))
	{
		val = devPath;
		DEBUG_LOG(("tryRelativeGeneralsPath - found base Generals at %s (dev layout)", devPath));
		return TRUE;
	}

	return FALSE;
}

// GeneralsX @feature felipebraz 18/02/2026 Auto-detect game language from BIG files
// On Linux there is no Windows registry. Detect the language by checking which
// language-specific BIG file is present in the working directory.
// Precedence mirrors the Windows installer language selection.
static Bool tryAutoDetectLanguage(AsciiString& val)
{
	struct stat st;

	// Each entry: BIG file to probe → language string the game expects
	const struct { const char *bigFile; const char *language; } candidates[] = {
		{ "BrazilianZH.big", "brazilian" },
		{ "EnglishZH.big",   "english"   },
		{ "GermanZH.big",    "german"    },
		{ "FrenchZH.big",    "french"    },
		{ "SpanishZH.big",   "spanish"   },
		{ "ChineseZH.big",   "chinese"   },
		{ "KoreanZH.big",    "korean"    },
		{ "PolishZH.big",    "polish"    },
		{ nullptr,           nullptr     }
	};

	for (int i = 0; candidates[i].bigFile != nullptr; ++i)
	{
		if (stat(candidates[i].bigFile, &st) == 0)
		{
			val = candidates[i].language;
			DEBUG_LOG(("tryAutoDetectLanguage - detected language '%s' from %s",
					candidates[i].language, candidates[i].bigFile));
			return TRUE;
		}
	}

	return FALSE;
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	// Try environment variable first: CNC_GENERALS_INSTALLPATH, etc.
	if (getEnvVar("CNC_GENERALS_", key, val))
		return TRUE;

	// Fallback: auto-detect base Generals path for InstallPath key
	if (key == "InstallPath")
	{
		if (tryRelativeGeneralsPath(val))
			return TRUE;
	}

	return FALSE;
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	// Try environment variable: CNC_ZH_INSTALLPATH, CNC_ZH_LANGUAGE, etc.
	if (getEnvVar("CNC_ZH_", key, val))
		return TRUE;

	// Fallback: auto-detect language from available BIG files in CWD
	if (key == "Language")
	{
		if (tryAutoDetectLanguage(val))
			return TRUE;
	}

	return FALSE;
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
	return FALSE;
}

#else // Windows implementation

Bool  getStringFromRegistry(HKEY root, AsciiString path, AsciiString key, AsciiString& val)
{
	HKEY handle;
	unsigned char buffer[256];
	unsigned long size = 256;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = (char *)buffer;
		return TRUE;
	}

	return FALSE;
}

Bool getUnsignedIntFromRegistry(HKEY root, AsciiString path, AsciiString key, UnsignedInt& val)
{
	HKEY handle;
	unsigned char buffer[4];
	unsigned long size = 4;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = *(UnsignedInt *)buffer;
		return TRUE;
	}

	return FALSE;
}

Bool setStringInRegistry( HKEY root, AsciiString path, AsciiString key, AsciiString val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_SZ;
		size = val.getLength()+1;
		returnValue = RegSetValueEx(handle, key.str(), 0, type, (unsigned char *)val.str(), size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

Bool setUnsignedIntInRegistry( HKEY root, AsciiString path, AsciiString key, UnsignedInt val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_DWORD;
		size = 4;
		returnValue = RegSetValueEx(handle, key.str(), 0, type, (unsigned char *)&val, size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";

	fullPath.concat(path);
	DEBUG_LOG(("GetStringFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
#if RTS_GENERALS
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.concat(path);
	DEBUG_LOG(("GetStringFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
#if RTS_GENERALS
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.concat(path);
	DEBUG_LOG(("GetUnsignedIntFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

#endif // _UNIX

// Cross-platform functions - call registry/env-var functions above
AsciiString GetRegistryLanguage()
{
	static Bool cached = FALSE;
	// NOTE: static causes a memory leak, but we have to keep it because the value is cached.
	static AsciiString val = "english";
	if (cached) {
		return val;
	} else {
		cached = TRUE;
	}

	GetStringFromRegistry("", "Language", val);
	return val;
}

AsciiString GetRegistryGameName()
{
	AsciiString val = "GeneralsMPTest";
	GetStringFromRegistry("", "SKU", val);
	return val;
}

UnsignedInt GetRegistryVersion()
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "Version", val);
	return val;
}

UnsignedInt GetRegistryMapPackVersion()
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "MapPackVersion", val);
	return val;
}
