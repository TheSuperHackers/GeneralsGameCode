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

#ifndef MUTEX_H
#define MUTEX_H

#pragma once

#include "always.h"
#include "thread.h"

#if !(defined(_MSC_VER) && _MSC_VER < 1300)
#include <atomic>
#endif

// Always use mutex or critical section when accessing the same data from multiple threads!

// ----------------------------------------------------------------------------
//
// Mutex class is an expensive way of synchronization! Use critical sections
// (below) for all synchronization. Use mutexes for inter-process locking.
//
// ----------------------------------------------------------------------------

class MutexClass
{
	void* handle;
	unsigned locked;

	// Lock and unlock are private so that you can't use them directly. Use LockClass as a sentry instead!
	// Lock returns true if lock was succesful, false otherwise
	bool Lock(int time);
	void Unlock();

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
	MutexClass(const char* name = NULL);
	~MutexClass();

	enum {
		WAIT_INFINITE=-1
	};

	class LockClass
	{
		MutexClass& mutex;
		bool failed;
	public:

		// In order to lock a mutex create a local instance of LockClass with mutex as a parameter.
		// Time is in milliseconds, INFINITE means infinite wait.
		LockClass(MutexClass& m, int time=MutexClass::WAIT_INFINITE);
		~LockClass();

		// Returns true if the lock failed
		bool Failed() { return failed; }
	private:
		LockClass &operator=(const LockClass&) { return(*this); }
	};
	friend class LockClass;
};

// ----------------------------------------------------------------------------
//
// Critical sections are faster than mutex classes and they should be used
// for all synchronization.
//
// ----------------------------------------------------------------------------

class CriticalSectionClass
{
	void* handle;
	unsigned locked;

	// Lock and unlock are private so that you can't use them directly. Use LockClass as a sentry instead!
	void Lock();
	void Unlock();

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
	CriticalSectionClass();
	~CriticalSectionClass();

	class LockClass
	{
		CriticalSectionClass& CriticalSection;
	public:
		// In order to lock a mutex create a local instance of LockClass with mutex as a parameter.
		LockClass(CriticalSectionClass& c);
		~LockClass();
	private:
		LockClass &operator=(const LockClass&) { return(*this); }
	};
	friend class LockClass;
};

// ----------------------------------------------------------------------------
//
// Fast critical section is really fast version of CriticalSection. The downside
// of it is that it can't be locked multiple times from the same thread.
//
// ----------------------------------------------------------------------------

class FastCriticalSectionClass
{
#if defined(_MSC_VER) && _MSC_VER < 1300
	// TheSuperHackers @info Mauller 30/03/2025 Added volatile to prevent reordering of critical sections if inlined
	volatile unsigned Flag;
#else
	std::atomic_flag Flag{};
#endif

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
	FastCriticalSectionClass()
#if defined(_MSC_VER) && _MSC_VER < 1300
		: Flag(0)
#endif
	{}

	class LockClass
	{
		FastCriticalSectionClass& cs;
	public:
		__forceinline LockClass(FastCriticalSectionClass& critical_section) : cs(critical_section)
		{
			lock();
		}

		~LockClass()
		{
			unlock();
		}

	private:

    void lock() {
#if defined(_MSC_VER) && _MSC_VER < 1300
		  volatile unsigned& nFlag=cs.Flag;

		  #define ts_lock _emit 0xF0
		  assert(((unsigned)&nFlag % 4) == 0);

      // I'm terribly sorry for these emits in here but
      // VC won't inline any functions that have labels in them...

      // Had to remove the emits back to normal
      // ASM statements because sometimes the jump
      // would be 1 byte off....

		  __asm mov ebx, [nFlag]
		  __asm ts_lock
		  __asm bts dword ptr [ebx], 0
		  __asm jnc BitSet
      //__asm _emit 0x73
      //__asm _emit 0x0f

		  The_Bit_Was_Previously_Set_So_Try_Again:
		    ThreadClass::Switch_Thread();
		  __asm mov ebx, [nFlag]
		  __asm ts_lock
		  __asm bts dword ptr [ebx], 0
		  __asm jc  The_Bit_Was_Previously_Set_So_Try_Again
      //_asm _emit 0x72
      //_asm _emit 0xf1

      BitSet:
        ;
#else
        while (cs.Flag.test_and_set(std::memory_order_acq_rel)) {
            cs.Flag.wait(true, std::memory_order_relaxed);
        }
#endif
    }

    void unlock() {
#if defined(_MSC_VER) && _MSC_VER < 1300
      cs.Flag=0;
#else
      cs.Flag.clear(std::memory_order_release);
      cs.Flag.notify_one();
#endif
    }

		LockClass &operator=(const LockClass&);
    LockClass(const LockClass&);
	};

  friend class LockClass;
};

#endif
