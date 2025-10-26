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

//
// Platform independent thread creation (Win32 & POSIX)
//

#pragma once

#ifdef _WIN32
  #include <process.h>
#endif
#include "wstypes.h"
#include <stdlib.h>

#ifdef _WIN32
  #include <wtypes.h>
#else // UNIX
  #include <pthread.h>
#endif

// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const

#include "critsec.h"



#ifdef THREADFAC_CODE
  // This is the fake thread entry point for functions
  #ifdef _WIN32
    static unsigned __stdcall threadFuncLauncher(void *temp);
  #else  // UNIX
    static void *threadFuncLauncher(void *temp);
  #endif

  // Fake entry point for classes
  #ifdef _WIN32
    static unsigned __stdcall threadClassLauncher(void *temp);
  #else  // UNIX
    static void *threadClassLauncher(void *temp);
  #endif
#endif





// Forward definition of base class for threaded classes
class Runnable;

//
// Call the static method startThread to begin a new thread.
//
class ThreadFactory
{
 public:
  static bit8    startThread(void (*start_func)(void *), void *data);
  static bit8    startThread(Runnable &runable, void *data, bit8 destroy=FALSE);
};



//
// Base class for when you want a thread to execute inside a class
//  instead of a function.
//
class Runnable
{
 public:
                Runnable();
   virtual     ~Runnable();


   // ThreadFactory needs to be able to access the private
   // IsRunning_ field.
   friend class ThreadFactory;

   // So do the threadClassLaunchers
   #ifdef _WIN32
     friend unsigned __stdcall threadClassLauncher(void *temp);
   #else  // UNIX
     friend void *threadClassLauncher(void *temp);
   #endif

   virtual void run(void *data)=0;       // Thread entry point

           void startThread(void *data,bit8 destroy=FALSE)  // nice way to start a thread
           {
             ThreadFactory::startThread(*this,data,destroy);
           };

           // Is there a thread running in this class?
           static bit8 isRunning(void);

           // Get the count of threads running inside this class
           static int    getThreadCount();


  private:
   static int       ThreadCount_;
   static CritSec   CritSec_;           // to protect ThreadCount_
};
