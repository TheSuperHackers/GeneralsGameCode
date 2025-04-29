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

#pragma once

#ifndef __STACKDUMP_H_
#define __STACKDUMP_H_

#ifndef IG_DEGBUG_STACKTRACE
#define IG_DEBUG_STACKTRACE	1
#endif // Unsure about this one -ML 3/25/03
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL) || defined(IG_DEBUG_STACKTRACE)

// Writes a stackdump (provide a callback : gets called per line)
// If callback is NULL then will write using OuputDebugString
void StackDump(void (*callback)(const char*));

// Writes a stackdump (provide a callback : gets called per line)
// If callback is NULL then will write using OuputDebugString
void StackDumpFromContext(DWORD eip,DWORD esp,DWORD ebp, void (*callback)(const char*));

// Gets count* addresses from the current stack
void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip = 0);

// Do full stack dump using an address array
void StackDumpFromAddresses(void**addresses, unsigned int count, void (*callback)(const char*));

void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address);

// Dumps out the exception info and stack trace.
void DumpExceptionInfo( unsigned int u, EXCEPTION_POINTERS* e_info );

#else

__inline void StackDump(void (*callback)(const char*)) {};

// Gets count* addresses from the current stack
__inline void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip = 0) {}

// Do full stack dump using an address array
__inline void StackDumpFromAddresses(void**addresses, unsigned int count, void (*callback)(const char*)) {}

__inline void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address) {}

// Dumps out the exception info and stack trace.
__inline void DumpExceptionInfo( unsigned int u, EXCEPTION_POINTERS* e_info ) {};

#endif

extern AsciiString g_LastErrorDump;
#endif // __STACKDUMP_H_
