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

#include "critsec.h"
#include <assert.h>
#include "wlib/wdebug.h"



CritSec::CritSec()
{
#ifdef _UNIX
  pthread_mutex_init(&Mutex_, NULL);
  RefCount_ = 0;
#elif defined(_WIN32)
  InitializeCriticalSection(&CritSec_);
#endif
}

CritSec::~CritSec()
{
 #ifdef _UNIX
   pthread_mutex_destroy(&Mutex_);
 #elif defined(_WIN32)
   DeleteCriticalSection(&CritSec_);
 #endif
}

// The "lock" function blocks until the mutex is available.
// Returns 0 on success, error code on error.
//
// A thread that already has a lock will increment a reference count if it calls
//  lock again.  It must then call unlock() enough times to get the reference to 0.
//
// If refcount is not null you can get the current ref counter after the lock.
//
sint32 CritSec::lock(int *refcount) RO 
{
 #ifdef _UNIX
    sint32	status;

    // I TRY to get the lock. IF I succeed, OR if I fail because
    // I already have the lock, I  just increment the reference
    // count and return.
    if (((status = pthread_mutex_trylock(&Mutex_)) == 0) ||
	((status == EBUSY) && (ThreadId_ == pthread_self())))
    {
	ThreadId_ = pthread_self();
	RefCount_++;
        if (refcount)
          *refcount=RefCount_;
	return(0);
    }

    // Otherwise, I wait for the lock.
    if ((status = pthread_mutex_lock(&Mutex_)) == 0)
    {
	assert(RefCount_ == 0);
	ThreadId_ = pthread_self();
	RefCount_++;
    }
    else
    {
	ERRMSG("pthread_mutex_lock: " << strerror(errno));
    }

    if (refcount)
      *refcount=RefCount_;

    return(status);
 #elif defined(_WIN32)
   // TOFIX update the refcount
   EnterCriticalSection(&CritSec_);
   return(0);
  #else
    #error Must define either _WIN32 or _UNIX
 #endif
}

// The "unlock" function release the critical section.
sint32 CritSec::unlock(void) RO 
{
 #ifdef _UNIX
    sint32	status = 0;
    
    assert(RefCount_ >= 0);
    if (RefCount_ <= 0)
    {
	//ERRMSG("unlocking unlocked mutex!");
	return(-1);
    }

    ///assert(ThreadId_ == pthread_self());
    if (ThreadId_ != pthread_self())
    {
	WRNMSG("tried to unlock a mutex not owned by self");
        return(-1);
    }

    if (--RefCount_ == 0)
    {
	// Set thread id to zero -- we're going to release mutex
	ThreadId_ = (pthread_t)0;
	
	// Unlock the mutex.
	if ((status = pthread_mutex_unlock(&Mutex_)) != 0)
	    ERRMSG("pthread_mutex_lock: " << strerror(errno));
    }
    return status;
 #elif defined(_WIN32)
    LeaveCriticalSection(&CritSec_);
    return(0);
 #endif
}
