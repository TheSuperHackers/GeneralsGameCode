/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file contains macros and functions to help with path handling.

#pragma once

#include "BaseType.h"
#include <string.h>

// Returns true for a separator the host platform uses to open files.
inline bool isNativePathSeparator(char ch)
{
#ifdef _WIN32
	return ch == '\\' || ch == '/';
#else
	return ch == '/';
#endif
}

// Returns true for either separator. Game data paths carry '\\' on every platform.
inline bool isPathSeparator(char ch)
{
	return ch == '/' || ch == '\\';
}

inline bool isAbsolutePath(const char* path)
{
	if (path == nullptr)
	{
		return false;
	}

	if (isNativePathSeparator(path[0]))
	{
		return true;
	}

#ifdef _WIN32
	const bool hasDriveLetter = (path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z');
	if (hasDriveLetter && path[1] == ':' && isNativePathSeparator(path[2]))
	{
		return true;
	}
#endif

	return false;
}

inline char getNativePathSeparator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

inline const char* getLastPathSeparator(const char* path)
{
	return path ? maxPtr(strrchr(path, '/'), strrchr(path, '\\')) : nullptr;
}

inline const wchar_t* getLastPathSeparator(const wchar_t* path)
{
	return path ? maxPtr(wcsrchr(path, L'/'), wcsrchr(path, L'\\')) : nullptr;
}

// Returns the whole path when it contains no separator
inline const char* getFileName(const char* path)
{
	const char* lastSeparator = getLastPathSeparator(path);
	return lastSeparator ? lastSeparator + 1 : path;
}

inline const char* getExtension(const char* path)
{
	const char* lastDot = strrchr(path, '.');

	if (!lastDot)
	{
		return nullptr;
	}

	const char* lastSeparator = getLastPathSeparator(path);

	// Check if the dot is contained in the filename
	if (lastSeparator && lastDot < lastSeparator)
	{
		return nullptr;
	}

	return lastDot;
}

inline const wchar_t* getExtension(const wchar_t* path)
{
	const wchar_t* lastDot = wcsrchr(path, L'.');

	if (!lastDot)
	{
		return nullptr;
	}

	const wchar_t* lastSeparator = getLastPathSeparator(path);

	// Check if the dot is contained in the filename
	if (lastSeparator && lastDot < lastSeparator)
	{
		return nullptr;
	}

	return lastDot;
}
