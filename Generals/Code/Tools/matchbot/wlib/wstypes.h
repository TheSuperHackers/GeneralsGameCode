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

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name: Carpenter  (The RedAlert ladder creator)
File Name   : wstypes.h
Author      : Neal Kettler
Start Date  : June 3, 1997
Last Update : June 17, 1997  

Standard type definitions for the sake of portability and readability.
\***************************************************************************/

#ifndef WSTYPES_HEADER
#define WSTYPES_HEADER

#ifdef _REENTRANT   // reentrant = threaded
// Headers with non threadsafe libs need to come before my hacky
//  threadsafe.h otherwise they won't compile

#include <time.h>
#ifndef _WINDOWS
#define _POSIX_C_SOURCE 199506L
#define _POSIX_PTHREAD_SEMANTICS
#define __EXTENSIONS__
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WINDOWS
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#else
#include <time.h>
#include <sys/timeb.h>
#endif
#include "threadsafe.h"   // enforce threadsafe-only calls
#endif

#define adelete(X) (delete[](X))

#ifndef TRUE
#define TRUE               1
#endif

#ifndef FALSE
#define FALSE              0
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef NULL
#define NULL 0
#endif

//These are used for readability purposes mostly, when a method takes a
//  pointer or reference these help specify what will happen to the data
//  that is sent in.
#ifdef IN
#undef IN
#endif
#define IN const
#define OUT
#define INOUT
#define _IN_ const

// Used to declare a function or method as const or Read Only
#define RO const

typedef char               bit8;
typedef char               sint8;
typedef unsigned char      uint8;
typedef signed short int   sint16;
typedef unsigned short int uint16;
typedef signed int         sint32;
typedef unsigned int       uint32;

typedef float              float32;
typedef double             float64;

#define MAX_BIT8    0x1
#define MAX_UINT32  0xFFFFFFFF
#define MAX_UINT16  0xFFFF
#define MAX_UINT8   0xFF
#define MAX_SINT32  0x7FFFFFFF
#define MAX_SINT16  0x7FFF
#define MAX_SINT8   0x7F   

#ifdef _WINDOWS
#define strncasecmp _strnicmp
#define strcasecmp  _stricmp
#endif  

#endif
