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

// Whether the character is a path separator
inline bool isPathSeparator(unsigned char ch)
{
	return ch == '/' || ch == '\\';
}

// Return a lowercase ascii character. Faster than tolower.
inline unsigned char toLowerAscii(unsigned char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		ch += 'a' - 'A';

	return ch;
}

// Compare two ascii paths and ignore case and different types of slashes. Useful for map lookups.
inline int comparePath(const char* a, const char* b)
{
	while (*a && *b)
	{
		unsigned char ca = static_cast<unsigned char>(*a);
		unsigned char cb = static_cast<unsigned char>(*b);

		if (isPathSeparator(ca) && isPathSeparator(cb))
		{
			++a;
			++b;
			continue;
		}

		ca = toLowerAscii(ca);
		cb = toLowerAscii(cb);

		if (ca != cb)
			return ca - cb;

		++a;
		++b;
	}

	// Ignore trailing separators
	while (isPathSeparator(*a))
		++a;
	while (isPathSeparator(*b))
		++b;

	return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

// Test if an ascii path starts with a given path and ignore case and different types of slashes.
inline bool startsWithPath(const char* path, const char* prefix)
{
	while (*path && *prefix)
	{
		unsigned char cp = static_cast<unsigned char>(*path);
		unsigned char cq = static_cast<unsigned char>(*prefix);

		if (isPathSeparator(cp) && isPathSeparator(cq))
		{
			++path;
			++prefix;
			continue;
		}

		cp = toLowerAscii(cp);
		cq = toLowerAscii(cq);

		if (cp != cq)
			return false;

		++path;
		++prefix;
	}

	// Ignore trailing separators in the prefix
	while (isPathSeparator(*prefix))
		++prefix;

	return *prefix == '\0';
}

// Hash an ascii path by normalizing case and different types of slashes. Useful for hash maps.
inline size_t hashPath(const char* path)
{
	constexpr const UnsignedInt64 FnvOffset = UnsignedInt64(14695981039346656037);
	constexpr const UnsignedInt64 FnvPrime  = UnsignedInt64(1099511628211);

	// FNV-1a 64-bit
	UnsignedInt64 hash = FnvOffset;

	// Find end
	const char* end = path;
	while (*end)
		++end;

	// Ignore trailing separators
	while (end > path && isPathSeparator(end[-1]))
		--end;

	// Hash normalized characters
	for (const char* p = path; p != end; ++p)
	{
		unsigned char ch = static_cast<unsigned char>(*p);

		if (isPathSeparator(ch))
			ch = '\\';
		else
			ch = toLowerAscii(ch);

		hash ^= ch;
		hash *= FnvPrime;
	}

	return (size_t)hash;
}

// Return file extension part from absolute or relative path when present. Null otherwise.
inline const char* getExtension(const char* path)
{
	const char* lastDot = strrchr(path, '.');

	if (!lastDot)
	{
		return nullptr;
	}

	const char* lastSeparator = maxPtr(strrchr(path, '/'), strrchr(path, '\\'));

	// Check if the dot is contained in the filename
	if (lastSeparator && lastDot < lastSeparator)
	{
		return nullptr;
	}

	return lastDot;
}

// Return file extension part from absolute or relative path when present. Null otherwise.
inline const wchar_t* getExtension(const wchar_t* path)
{
	const wchar_t* lastDot = wcsrchr(path, L'.');

	if (!lastDot)
	{
		return nullptr;
	}

	const wchar_t* lastSeparator = maxPtr(wcsrchr(path, L'/'), wcsrchr(path, L'\\'));

	// Check if the dot is contained in the filename
	if (lastSeparator && lastDot < lastSeparator)
	{
		return nullptr;
	}

	return lastDot;
}

// Returns whether the given path is assumed to be a file path or directory path.
// Requirements for file path are: Must contain a dot in the trailing name after the last slash.
inline bool isFilePath(const char* path)
{
	return getExtension(path) != nullptr;
}

// Returns whether the given path is assumed to be a file path or directory path.
// Requirements for file path are: Must contain a dot in the trailing name after the last slash.
inline bool isFilePath(const wchar_t* path)
{
	return getExtension(path) != nullptr;
}
