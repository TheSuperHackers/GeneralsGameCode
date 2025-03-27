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

// FILE: wolSetup.h //////////////////////////////////////////////////////
// Author: Matthew D. Campbell, December 2001

#ifndef __WOLSETUP_H__
#define __WOLSETUP_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void checkInstalledWolapiVersion( void );
void setupGenerals( const char *genPath, const char *genSerial );

extern HINSTANCE g_hInst;
extern unsigned long g_wolapiRegistryVersion;
extern unsigned long g_wolapiRealVersion;
extern bool g_wolapiInstalled;
extern char g_wolapiRegFilename[MAX_PATH];
extern char g_wolapiRealFilename[MAX_PATH];
extern char g_generalsFilename[MAX_PATH];
extern char g_generalsSerial[];

// TheSuperHackers @todo Check if this should be returning unsigned long.
static int MAJOR(unsigned long x) { return (((x) & 0xffff0000) >> 16); }
static int MINOR(unsigned long x) { return ((x) & 0xffff); }

#endif // __WOLSETUP_H__
