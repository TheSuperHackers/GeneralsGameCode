/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

// This file contains macros to help compiling on non-windows platforms.
#pragma once

#ifndef _WIN32
// For size_t
#include <cstddef>
// For isdigit
#include <cctype>

// __forceinline
#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif

// _cdecl / __cdecl
#ifndef _cdecl
#define _cdecl
#endif
#ifndef __cdecl
#define __cdecl
#endif

// OutputDebugString
#ifndef OutputDebugString
#define OutputDebugString(str) printf("%s\n", str)
#endif

// _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT, _MAX_PATH
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#include "mem_compat.h"
#include "string_compat.h"
#include "tchar_compat.h"
#include "wchar_compat.h"
#include "time_compat.h"
#include "thread_compat.h"

#endif