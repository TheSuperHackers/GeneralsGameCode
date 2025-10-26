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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// LatchRestore.h /////////////////////////////////////////////////////////////////////////////////
// Author: John K. McDonald, Jr.
// 09/19/2002
// DO NOT DISTRIBUTE
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/*
	The purpose of the LatchRestore class is to allow you to override member variables for the scope
	of a function. Here's the code that this saves:

	void Foo::func(Team *overrideTeam)
	{
		Team *saveTeam = m_saveTeam;
		m_saveTeam = overrideTeam;

		// ... stuff	...

		if (fu)
		{
			// early return
			m_saveTeam = saveTeam;
			return true;
		}

		if (bar)
		{
			// early return
			m_saveTeam = saveTeam;
			return true;
		}

		if (munkees)
		{
			// early return
			m_saveTeam = saveTeam;
			return true;
		}


		m_saveTeam = saveTeam;
		return false;
	}

	Instead, the code would simply be:

	void Foo::func(Team *overrideTeam)
	{
		LatchRestore<Team *> latch(m_saveTeam, overrideTeam);

		// ... stuff	...

		if (fu)
			return true;

		if (bar)
			return true;

		if (munkees)
			return true;

		return false;
	}

*/

template <typename T>
class LatchRestore
{
	protected:
		T valueToRestore;
		T& whereToRestore;

	public:
		LatchRestore(T& dest, const T& src) : whereToRestore(dest)
		{
			valueToRestore = dest;
			dest = src;
		}

		virtual ~LatchRestore()
		{
			whereToRestore = valueToRestore;
		}
};
