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

/* $Header: /G/wwlib/bittype.h 4     4/02/99 1:37p Eric_c $ */
/***************************************************************************
 ***                  Confidential - Westwood Studios                    ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Voxel Technology                         *
 *                                                                         *
 *                    File Name : BITTYPE.h                                *
 *                                                                         *
 *                   Programmer : Greg Hjelstrom                           *
 *                                                                         *
 *                   Start Date : 02/24/97                                 *
 *                                                                         *
 *                  Last Update : February 24, 1997 [GH]                   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifdef __APPLE__
#include <stdint.h>
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
// TheSuperHackers @build macOS - On macOS LP64, unsigned long is 8 bytes.
// Security framework defines uint32 as uint32_t (4 bytes). Use matching types.
#ifdef __APPLE__
typedef uint32_t uint32;
#else
typedef unsigned long uint32;
#endif
typedef unsigned int uint;

typedef signed char sint8;
typedef signed short sint16;
#ifdef __APPLE__
typedef int32_t sint32;
#else
typedef signed long sint32;
#endif
typedef signed int sint;

typedef float float32;
typedef double float64;

#ifndef __APPLE__ // On macOS, these types are provided by our windows.h shim
                  // with correct sizes
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef int BOOL;
typedef unsigned short USHORT;
typedef const char *LPCSTR;
typedef unsigned int UINT;
typedef unsigned long ULONG;
#endif

#if defined(_MSC_VER) && _MSC_VER < 1300
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif
