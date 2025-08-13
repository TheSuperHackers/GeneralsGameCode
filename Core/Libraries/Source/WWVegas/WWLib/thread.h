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

#ifndef THREAD_H
#define THREAD_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "always.h"
#include "Vector.H"

struct _EXCEPTION_POINTERS;


// ****************************************************************************
//
// To create a new thread just derive a new class from this and define
// Thread_Function. Creating the TheadClass object doesn't start the
// thread. To start the thread you must call Execute().
//
// In your own thread remember to check for "running" flag of the base class.
// If the flag is false you must exit the asap. Stop() is the function that
// will clear the flag and expect you to exit from the thread. If you are
// not exiting in certain time (defined as a parameter to Stop()) it will
// force-kill the thread to prevent the program from halting.
//
// ****************************************************************************

class ThreadClass
{
public:
	typedef int (*ExceptionHandlerType)(int exception_code, struct _EXCEPTION_POINTERS *e_info);

	ThreadClass(const char *name = NULL, ExceptionHandlerType exception_handler = NULL);
	virtual ~ThreadClass();

	// Execute Thread_Function(). Note that only one instance can be executed at a time.
	void Execute();

	// Thread priority 0 is normal, positive numbers are higher and normal and negative are lower.
	void Set_Priority(int priority);

	// Stop thread execution. Kill after ms milliseconds if not responding.
	void Stop(unsigned ms=3000);

	// Put current thread sleep for ms milliseconds (can be called from any thread, ThreadClass or other)
	static void Sleep_Ms(unsigned ms=0);

	// Put current thread in sleep and switch to next one (Useful for balansing the thread switches with game update)
	static void Switch_Thread();

	// Return calling thread's unique thread id
	static unsigned _Get_Current_Thread_ID();

	// Returns true if the thread is running.
	bool Is_Running();

	// Gets the name of the thread.
	const char *Get_Name(void) {return(ThreadName);};

	// Get info about a registered thread by it's index.
	static int Get_Thread_By_Index(int index, char *name_ptr = NULL);

protected:

	// User defined thread function. The thread function should check for "running" flag every now and then
	// and exit the thread if running is false.
	virtual void Thread_Function() = 0;
	volatile bool running;

	// Name of thread.
	char ThreadName[64];

	// ID of thread.
	unsigned ThreadID;

	// Exception handler for this thread.
	ExceptionHandlerType ExceptionHandler;

private:
	static void __cdecl Internal_Thread_Function(void*);
	volatile unsigned long handle;
	int thread_priority;
};

#endif
