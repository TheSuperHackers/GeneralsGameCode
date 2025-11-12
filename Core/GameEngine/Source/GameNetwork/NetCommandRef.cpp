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


#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/NetCommandRef.h"

#ifdef DEBUG_NETCOMMANDREF
static UnsignedInt refNum = 0;
#endif

/**
 * Constructor.  Attach to the given network command.
 */
#ifdef DEBUG_NETCOMMANDREF
NetCommandRef::NetCommandRef(NetCommandMsg *msg, char *filename, int line)
#else
NetCommandRef::NetCommandRef(NetCommandMsg *msg)
#endif
{
	m_msg = msg;
	m_next = NULL;
	m_prev = NULL;
	m_msg->attach();
	m_timeLastSent = -1;

#ifdef DEBUG_NETCOMMANDREF
	m_id = ++refNum;
	DEBUG_LOG(("NetCommandRef %d allocated in file %s line %d", m_id, filename, line));
#endif
}

/**
 * Destructor. Detach from the network command.
 */
NetCommandRef::~NetCommandRef()
{
	if (m_msg != NULL)
	{
		m_msg->detach();
	}
 	DEBUG_ASSERTCRASH(m_next == NULL, ("NetCommandRef::~NetCommandRef - m_next != NULL"));
	DEBUG_ASSERTCRASH(m_prev == NULL, ("NetCommandRef::~NetCommandRef - m_prev != NULL"));

#ifdef DEBUG_NETCOMMANDREF
	DEBUG_LOG(("NetCommandRef %d deleted", m_id));
#endif
}

