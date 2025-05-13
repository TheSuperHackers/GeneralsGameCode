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

#define THREADFAC_CODE

#include "threadfac.h"

int       Runnable::ThreadCount_ = 0;
CritSec   Runnable::CritSec_;           // to protect ThreadCount_

// MDC: Made all this dependent on _REENTRANT being defined so VC++ doesn't complain on
// single-threaded programs...


//
// Note: I chose the following type signature for thread functions
//   void function(void *data);
//
//
// Since Win32 & POSIX have different type signatures for the thread entry points
//  an intermediate system-dependent function in this file gets called first.
//  That function then calls the system independent version.  So the system dependent
//  version needs 2 Items  1) The address of the _real_ thread func 2) the data
//  to pass.  We only have 1 argument available, so you figure out the rest...
//
// This is for internal use only
//
struct ThreadInformation
{
  void     *startPoint;    // The address of the _real_ thread function, or class
  void     *data;          // data to pass to real thread function or class
  bit8      destroy;       // only applies to classes, should delete after execution?
};



//
// Start a thread inside a class
//
bit8 ThreadFactory::startThread(Runnable &runable, void *data, bit8 destroy)
{
#ifdef _REENTRANT
 
  {
	  Runnable::CritSec_.lock();
	  Runnable::ThreadCount_++;
	  Runnable::CritSec_.unlock();
  }
 

  ThreadInformation *tInfo=new ThreadInformation;
  tInfo->startPoint=(void *)&runable;
  tInfo->data=data;
  tInfo->destroy=destroy;

  #ifdef _WIN32
    // Under windows call _beginthreadex instead of CreateThread so you can
    //  use all the normal C library stuff. (IMPORTANT!!!)
    uint32 handle;
	uint32 stup1d;
    handle=_beginthreadex(NULL,0,  threadClassLauncher, tInfo, 0, &stup1d);
    if (handle!=NULL)
      return(TRUE);
    else
    {
      {
        runable.CritSec_.lock();
        runable.ThreadCount_--;   // Ok, so it didn't really start
        runable.CritSec_.unlock();
      }
      return(FALSE);
    }
  #else // UNIX
    // Setup thread attributes for client threads
    int retval;
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&threadAttr,PTHREAD_SCOPE_SYSTEM);
    retval=pthread_create(NULL,&threadAttr, threadClassLauncher, tInfo);
    if (retval==0)
      return(TRUE);
    else
    {
      {
        runable.CritSec_.lock();
        runable.ThreadCount_--;   // Ok, so it didn't really start
        runable.CritSec_.unlock();
      }
      return(FALSE);
    }
  #endif
#else
	return (FALSE);
#endif /* _REENTRANT */
}


//
// Start a thread inside a function
//
bit8 ThreadFactory::startThread(void (*start_func)(void *), void *data)
{
#ifdef _REENTRANT
  ThreadInformation *tInfo=new ThreadInformation;
  tInfo->startPoint=start_func;
  tInfo->data=data;

  #ifdef _WIN32
    // Under windows call _beginthreadex instead of CreateThread so you can
    //  use all the normal C library stuff. (IMPORTANT!!!)
    uint32 handle;
	unsigned temp;
    handle=_beginthreadex(NULL,0,  threadFuncLauncher, tInfo, 0, &temp);
    if (handle!=NULL)
      return(TRUE);
    return(FALSE);
  #else // UNIX
    // Setup thread attributes for client threads
    int retval;
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&threadAttr,PTHREAD_SCOPE_SYSTEM);
    retval=pthread_create(NULL,&threadAttr, threadFuncLauncher, tInfo);
    if (retval==0)
      return(TRUE);
    else
      return(FALSE);
  #endif
#else
	return(FALSE);
#endif /* REENTRANT */
}

#ifdef _WIN32
  unsigned __stdcall threadFuncLauncher(void *temp)
#else  // UNIX
  void *threadFuncLauncher(void *temp)
#endif
{
  ThreadInformation *tInfo=(ThreadInformation *)temp;

  void (*start_func)(void *);
  start_func=(void (*)(void *))tInfo->startPoint;

  void     *data=tInfo->data;
  delete(tInfo);

  start_func(data);
  return(0);
}

#ifdef _WIN32
  unsigned __stdcall threadClassLauncher(void *temp)
#else  // UNIX
  void *threadClassLauncher(void *temp)
#endif
{
  ThreadInformation *tInfo=(ThreadInformation *)temp;

  Runnable *thrClass=(Runnable *)tInfo->startPoint;
  void     *data=tInfo->data;
  bit8      destroy=tInfo->destroy;
  delete(tInfo);

  thrClass->run(data);

  if (destroy)     // May want to free memory after thread finishes
    delete(thrClass);

  {
	  Runnable::CritSec_.lock();
	  Runnable::ThreadCount_--;
	  Runnable::CritSec_.unlock();
  }

  #ifdef _WIN32
    ExitThread(0);    // is this really needed?
  #endif
  return(0);
}

Runnable::Runnable()
{ }

Runnable::~Runnable()
{ }

// Is there a thread running in this class
bit8 Runnable::isRunning(void)
{
  // Don't need to lock a simple assignment
  int temp=ThreadCount_;
  return((temp>0)?TRUE:FALSE);
}

// How many threads are running in this class
int Runnable::getThreadCount(void)
{
  // Don't need to lock a simple assignment
  int temp=ThreadCount_;
  return(temp);
}

#undef THREADFAC_CODE
