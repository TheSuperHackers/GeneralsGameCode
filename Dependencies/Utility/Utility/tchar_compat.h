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

// This file defines TCHAR and related macros for compatibility with non-windows platforms.
#pragma once

// TCHAR
#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#define _TCHAR_DEFINED
#endif

#ifndef _LPCTSTR_DEFINED
typedef const TCHAR* LPCTSTR;
#define _LPCTSTR_DEFINED
#endif

#ifndef _LPTSTR_DEFINED
typedef TCHAR* LPTSTR;
#define _LPTSTR_DEFINED
#endif

#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcsclen strlen
#define _tcscpy strcpy

