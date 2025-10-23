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

// FILE: ScopedMutex.h ////////////////////////////////////////////////////////////////////////////
// Author: John McDonald, November 2002
// Desc:   A scoped mutex class to easily lock a scope with a pre-existing mutex object.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class ScopedMutex
{
	private:
		HANDLE m_mutex;

	public:
		ScopedMutex(HANDLE mutex) : m_mutex(mutex)
		{
			DWORD status = WaitForSingleObject(m_mutex, 500);
			if (status != WAIT_OBJECT_0) {
				DEBUG_LOG(("ScopedMutex WaitForSingleObject timed out - status %d", status));
			}
		}

		~ScopedMutex()
		{
			ReleaseMutex(m_mutex);
		}
};
