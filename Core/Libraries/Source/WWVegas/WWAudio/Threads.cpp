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
 *                     $Archive:: /Commando/Code/WWAudio/Threads.cpp                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/17/99 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "Threads.h"
#include "Utils.h"
#include <process.h>


///////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::DELAYED_RELEASE_INFO *	WWAudioThreadsClass::m_ReleaseListHead	= NULL;
CriticalSectionClass		WWAudioThreadsClass::m_ListMutex;
HANDLE						WWAudioThreadsClass::m_hDelayedReleaseThread	= (HANDLE)-1;
HANDLE						WWAudioThreadsClass::m_hDelayedReleaseEvent	= (HANDLE)-1;
CriticalSectionClass		WWAudioThreadsClass::m_CriticalSection;
bool							WWAudioThreadsClass::m_IsShuttingDown			= false;

///////////////////////////////////////////////////////////////////////////////////////////
//
//	WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::WWAudioThreadsClass (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	~WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::~WWAudioThreadsClass (void)
{
	return ;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
HANDLE
WWAudioThreadsClass::Create_Delayed_Release_Thread (LPVOID param)
{
	//
	//	If the thread isn't already running, then
	//
	if (m_hDelayedReleaseThread == (HANDLE)-1) {
		m_hDelayedReleaseEvent	= ::CreateEvent (NULL, FALSE, FALSE, NULL);
		m_hDelayedReleaseThread = (HANDLE)::_beginthread (Delayed_Release_Thread_Proc, 0, param);
	}

	return m_hDelayedReleaseThread;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	End_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::End_Delayed_Release_Thread (DWORD timeout)
{
	m_IsShuttingDown = true;

	//
	//	If the thread is running, then wait for it to finish
	//
	if (m_hDelayedReleaseThread != (HANDLE)-1) {
		::SetEvent (m_hDelayedReleaseEvent);
		::WaitForSingleObject (m_hDelayedReleaseThread, timeout);

		m_hDelayedReleaseEvent	= (HANDLE)-1;
		m_hDelayedReleaseThread	= (HANDLE)-1;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Delayed_Release_Object
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Add_Delayed_Release_Object
(
	RefCountClass *	object,
	DWORD					delay
)
{
	if (m_IsShuttingDown) {
		REF_PTR_RELEASE (object);
	} else {

		//
		//	Make sure we have a thread running that will handle
		// the operation for us.
		//
		if (m_hDelayedReleaseThread == (HANDLE)-1) {
			Create_Delayed_Release_Thread ();
		}

		//
		//	Wait for the release thread to finish using the
		// list pointer
		//
		{
			CriticalSectionClass::LockClass lock(m_ListMutex);

			//
			//	Create a new delay-information structure and
			//	add it to our list
			//
			DELAYED_RELEASE_INFO *info = W3DNEW DELAYED_RELEASE_INFO;
			info->object	= object;
			info->time		= ::GetTickCount () + delay;
			info->next		= m_ReleaseListHead;

			m_ReleaseListHead = info;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Delayed_Release_Objects
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Flush_Delayed_Release_Objects (void)
{
	CriticalSectionClass::LockClass lock(m_CriticalSection);

	//
	//	Loop through all the objects in our delay list, and
	// free them now.
	//
	DELAYED_RELEASE_INFO *info = NULL;
	DELAYED_RELEASE_INFO *next = NULL;
	for (info = m_ReleaseListHead; info != NULL; info = next) {
		next = info->next;

		//
		//	Free the object
		//
		REF_PTR_RELEASE (info->object);
		SAFE_DELETE (info);
	}

	m_ReleaseListHead = NULL;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Delayed_Release_Thread_Proc
//
///////////////////////////////////////////////////////////////////////////////////////////
void __cdecl
WWAudioThreadsClass::Delayed_Release_Thread_Proc (LPVOID /*param*/)
{
	const DWORD base_timeout = 2000;
	DWORD timeout = base_timeout + rand () % 1000;

	//
	//	Keep looping forever until we are singalled to quit (or an error occurs)
	//
	while (::WaitForSingleObject (m_hDelayedReleaseEvent, timeout) == WAIT_TIMEOUT) {

		{
			CriticalSectionClass::LockClass lock(m_ListMutex);

			//
			//	Loop through all the objects in our delay list, and
			// free any that have expired.
			//
			DWORD current_time			= ::GetTickCount ();
			DELAYED_RELEASE_INFO *curr = NULL;
			DELAYED_RELEASE_INFO *prev	= NULL;
			DELAYED_RELEASE_INFO *next	= NULL;
			for (curr = m_ReleaseListHead; curr != NULL; curr = next) {
				next = curr->next;

				//
				//	If the time has expired, free the object
				//
				if (current_time >= curr->time) {

					//
					//	Unlink the object
					//
					if (prev == NULL) {
						m_ReleaseListHead = next;
					} else {
						prev->next = next;
					}

					//
					//	Free the object
					//
					REF_PTR_RELEASE (curr->object);
					SAFE_DELETE (curr);

				} else {
					prev = curr;
				}
			}
		}

		//
		//	To avoid 'periodic' releases, randomize our timeout
		//
		timeout = base_timeout + rand () % 1000;
	}

	Flush_Delayed_Release_Objects ();
	return ;
}

/*
///////////////////////////////////////////////////////////////////////////////////////////
//
//	Begin_Modify_List
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioThreadsClass::Begin_Modify_List (void)
{
	bool retval = false;

	//
	//	Wait for up to one second to modify the list object
	//
	if (m_ListMutex != NULL) {
		retval = (::WaitForSingleObject (m_ListMutex, 1000) == WAIT_OBJECT_0);
		WWASSERT (retval);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	End_Modify_List
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::End_Modify_List (void)
{
	//
	//	Release this thread's hold on the mutex object.
	//
	if (m_ListMutex != NULL) {
		::ReleaseMutex (m_ListMutex);
	}

	return ;
}
*/

