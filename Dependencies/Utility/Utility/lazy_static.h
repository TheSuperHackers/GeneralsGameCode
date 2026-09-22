/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file contains a holder for an object with static storage duration, which is constructed on
// the first call to get() and is never destroyed. Is Windows only.
//
// The holder must be declared at namespace scope or as a static member, without an initializer.
// It has no constructor, so it is zero-initialized before any code runs. That makes the object
// usable during the static initialization of other files, and because it is never destroyed, during
// the static destruction too. The construction is thread-safe on all compilers, unlike the
// initialization of function-local statics with VC6.
//
//   lazy_static<CriticalSection> Lock;
//   Lock.get().lock();

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <new>

#include "Utility/aligned_pod.h"
#include "Utility/interlocked_adapter.h"

template <typename T>
struct lazy_static
{
	T& get()
	{
		if (m_state != State_Constructed)
			construct();
		return *reinterpret_cast<T*>(m_storage.bytes);
	}

	// Is public only so that the type stays an aggregate without a constructor. Do not access directly.
	volatile long m_state;
	union
	{
		char bytes[sizeof(T)];
		aligned_pod<alignof(T)> alignment;
	} m_storage;

private:

	enum
	{
		State_Unconstructed,
		State_Constructing,
		State_Constructed
	};

	void construct()
	{
		if (::InterlockedCompareExchange(&m_state, State_Constructing, State_Unconstructed) == State_Unconstructed)
		{
			new (m_storage.bytes) T();
			::InterlockedCompareExchange(&m_state, State_Constructed, State_Constructing);
		}
		else
		{
			while (m_state != State_Constructed)
				::Sleep(0);
		}
	}
};
