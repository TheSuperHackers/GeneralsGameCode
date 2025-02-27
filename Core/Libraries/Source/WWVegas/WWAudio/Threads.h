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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/Threads.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/17/99 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "windows.h"
#include "Vector.h"
#include "mutex.h"

// Forward declarations
class RefCountClass;


//////////////////////////////////////////////////////////////////////////
//
//	WWAudioThreadsClass
//
//	Simple class that provides a common namespace for tying thread
// information together.
//
//////////////////////////////////////////////////////////////////////////
class WWAudioThreadsClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		WWAudioThreadsClass (void);
		~WWAudioThreadsClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		//
		//	Delayed release mechanism
		//
		static HANDLE		Create_Delayed_Release_Thread (LPVOID param = NULL);
		static void			End_Delayed_Release_Thread (DWORD timeout = 20000);
		static void			Add_Delayed_Release_Object (RefCountClass *object, DWORD delay = 2000);
		static void			Flush_Delayed_Release_Objects (void);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private methods
		//////////////////////////////////////////////////////////////////////
		static void	__cdecl Delayed_Release_Thread_Proc (LPVOID param);

		//////////////////////////////////////////////////////////////////////
		//	Private data types
		//////////////////////////////////////////////////////////////////////
		typedef struct _DELAYED_RELEASE_INFO
		{
			RefCountClass *	object;
			DWORD					time;

			_DELAYED_RELEASE_INFO *next;

		} DELAYED_RELEASE_INFO;

		//typedef DynamicVectorClass<DELAYED_RELEASE_INFO *>	RELEASE_LIST;

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		static HANDLE						m_hDelayedReleaseThread;
		static HANDLE						m_hDelayedReleaseEvent;
		//static RELEASE_LIST		m_ReleaseList;
		static CriticalSectionClass	m_CriticalSection;
		static DELAYED_RELEASE_INFO *	m_ReleaseListHead;
		static CriticalSectionClass	m_ListMutex;
		static bool							m_IsShuttingDown;
};
