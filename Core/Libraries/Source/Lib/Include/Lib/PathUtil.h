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

inline Char getNativePathSeparator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

// Returns true for a separator the host platform uses to open files.
template <typename T> Bool isFileSystemPathSeparator(T ch)
{
#ifdef _WIN32
	return ch == '\\' || ch == '/';
#else
	return ch == '/';
#endif
}

// Returns true for either separator. Game data paths carry '\\' on every platform.
template <typename T> Bool isPathSeparator(T ch)
{
	return ch == '/' || ch == '\\';
}

// Returns whether the given path is an absolute file path.
template <typename T> Bool isAbsolutePath(const T* path)
{
	if (path == nullptr)
	{
		return false;
	}

	if (isFileSystemPathSeparator(path[0]))
	{
		return true;
	}

#ifdef _WIN32
	const bool hasDriveLetter =
		(path[0] >= 'A' && path[0] <= 'Z') ||
		(path[0] >= 'a' && path[0] <= 'z');

	if (hasDriveLetter && path[1] == ':' && isFileSystemPathSeparator(path[2]))
	{
		return true;
	}
#endif

	return false;
}

// Returns a pointer to the first path separator of the given path.
template <typename T> T* getFirstPathSeparator(T* path)
{
	for (; *path; ++path)
	{
		if (isPathSeparator(*path))
			return path;
	}
	return nullptr;
}

// Returns a pointer to the last path separator of the given path.
template <typename T> T* getLastPathSeparator(T* path)
{
	T* last = nullptr;
	for (; *path; ++path)
	{
		if (isPathSeparator(*path))
			last = path;
	}
	return last;
}

// Returns the file name after the last path separator or the whole path as is if none was found.
template <typename T> T* getFileName(T* path)
{
	T* lastSeparator = getLastPathSeparator(path);
	return lastSeparator ? lastSeparator + 1 : path;
}

// Returns null when the file name has no extension, ignoring dots in directory names.
template <typename T> T* getExtension(T* path)
{
	T* lastDot = nullptr;
	for (; *path; ++path)
	{
		if (*path == '.')
			lastDot = path;
		else if (isPathSeparator(*path))
			lastDot = nullptr;
	}
	return lastDot;
}
