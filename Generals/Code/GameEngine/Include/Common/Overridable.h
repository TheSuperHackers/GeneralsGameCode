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

// Overridable.h //////////////////////////////////////////////////////////////////////////////////
// Electronic Arts Pacific
// Do Not Distribute

#pragma once

#include "Common/GameMemory.h"

/*
	In order for something to live in an OVERRIDE<> object, it must be derived from Overridable
	(publicly).

	This is useful for things like templates, where we want to override the template and make sure
	that all instances get the updated values (for instance, via map.ini)
*/

class Overridable : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Overridable, "Overridable"  )

	private:
		Overridable *m_nextOverride;
		Bool m_isOverride;

	public:
		Overridable() : m_nextOverride(nullptr), m_isOverride(false) {}

		// return a constant version of m_nextOverride, which can be null if there is no
		// override
		const Overridable *getNextOverride( void ) const
		{
			return m_nextOverride;
		}

		// recursively ask if there is a next override, and if not, return this.
		const Overridable *getFinalOverride( void ) const
		{
			if (m_nextOverride)
				return m_nextOverride->getFinalOverride();
			return this;
		}

		// set the next override on this object. This currently makes no attempt to prevent leaks.
		// it probably shouldn't because doing so could cause infinite loops
		void setNextOverride(Overridable *nextOverridable)
		{
			m_nextOverride = nextOverridable;
		}

		// useful for the LocomotorStore to cleanup overrides.
		Overridable *friend_getNextOverride( void )
		{
			return m_nextOverride;
		}

		// useful for the LocomotorStore to create an override dangling off the final override.
		Overridable *friend_getFinalOverride( void )
		{
			if (m_nextOverride)
				return m_nextOverride->friend_getFinalOverride();
			return this;
		}

		// useful for the LocomotorStore to create an override dangling off the final override.
		const Overridable *friend_getFinalOverride( void ) const
		{
			if (m_nextOverride)
				return m_nextOverride->friend_getFinalOverride();
			return this;
		}

		// used by ini-parsing functions to mark specific Overridables as overrides
		void markAsOverride( void )
		{
			m_isOverride = true;
		}

		// used in factory reset() calls at the end of a game to clean up overrides.  Can return nullptr
		// if the first Overridable is itself an override
		Overridable *deleteOverrides( void )
		{
			if ( m_isOverride )
			{
				deleteInstance(this);
				return nullptr;
			}
			else if ( m_nextOverride )
			{
				m_nextOverride = m_nextOverride->deleteOverrides();
			}
			return this;
		}
};

// cleans up and dangling overrides.
__inline Overridable::~Overridable()
{
	deleteInstance(m_nextOverride);
}
