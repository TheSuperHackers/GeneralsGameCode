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

inline char getNativePathSeparator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

inline Bool isPathSeparator(char c)
{
	return c == '/' || c == '\\';
}

inline const char* getLastPathSeparator(const char* path)
{
	return maxPtr(strrchr(path, '/'), strrchr(path, '\\'));
}

inline const wchar_t* getLastPathSeparator(const wchar_t* path)
{
	return maxPtr(wcsrchr(path, L'/'), wcsrchr(path, L'\\'));
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
