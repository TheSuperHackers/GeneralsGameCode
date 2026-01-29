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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// CriticalSection.h ///////////////////////////////////////////////////////
// Utility class to use critical sections in areas of code.
// Author: JohnM And MattC, August 13, 2002

#pragma once

#include "Common/PerfTimer.h"

#ifdef PERF_TIMERS
extern PerfGather TheCritSecPerfGather;
#endif

class CriticalSection
{
	CRITICAL_SECTION m_windowsCriticalSection;

	public:
		CriticalSection()
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			InitializeCriticalSection( &m_windowsCriticalSection );
		}

		virtual ~CriticalSection()
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			DeleteCriticalSection( &m_windowsCriticalSection );
		}

	public:	// Use these when entering/exiting a critical section.
		void enter( void )
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			EnterCriticalSection( &m_windowsCriticalSection );
		}

		void exit( void )
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			LeaveCriticalSection( &m_windowsCriticalSection );
		}
};

class ScopedCriticalSection
{
	private:
		CriticalSection *m_cs;

	public:
		ScopedCriticalSection( CriticalSection *cs ) : m_cs(cs)
		{
			if (m_cs)
				m_cs->enter();
		}

		virtual ~ScopedCriticalSection( )
		{
			if (m_cs)
				m_cs->exit();
		}
};

// These should be null on creation then non-null in WinMain or equivalent.
// This allows us to be silently non-threadsafe for WB and other single-threaded apps.
extern CriticalSection *TheAsciiStringCriticalSection;
extern CriticalSection *TheUnicodeStringCriticalSection;
extern CriticalSection *TheDmaCriticalSection;
extern CriticalSection *TheMemoryPoolCriticalSection;
extern CriticalSection *TheDebugLogCriticalSection;
