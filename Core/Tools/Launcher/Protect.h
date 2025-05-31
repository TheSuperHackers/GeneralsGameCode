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

/******************************************************************************
*
* FILE
*     $Archive: /APILauncher/Protect.h $
*
* DESCRIPTION
*     Copy protection
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Mcampbell $
*
* VERSION INFO
*     $Modtime: 6/18/01 8:24p $
*     $Revision: 4 $
*
******************************************************************************/

#ifndef __PROTECT_H__
#define __PROTECT_H__

#ifdef COPY_PROTECT

#include <windows.h>

//#define OLDWAY

#ifdef OLDWAY
template<typename T> class RefPtr;
class UString;


class Protect
	{
	public:
		Protect();
		~Protect();

		void SendMappedFileHandle(HANDLE process, DWORD threadID) const;

	private:
		// Prevent copy & assignment
		Protect(const Protect&);
		const Protect& operator=(const Protect&);

		// Retrieve machine unique key
		RefPtr<UString> GetPassKey(void) const;

		HANDLE mLauncherMutex;
		HANDLE mMappedFile;
	};

#else

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl InitializeProtect(void);
void __cdecl ShutdownProtect(void);
void __cdecl SendProtectMessage(HANDLE process, DWORD threadID);

#ifdef __cplusplus
};
#endif

#endif

#endif // COPY_PROTECT

#endif // PROTECT_H
