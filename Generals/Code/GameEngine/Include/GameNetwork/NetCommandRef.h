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

#pragma once

#include "GameNetwork/NetCommandMsg.h"
#include "Common/GameMemory.h"

#if defined(RTS_DEBUG)
//	#define DEBUG_NETCOMMANDREF
#endif

#ifdef DEBUG_NETCOMMANDREF
#define NEW_NETCOMMANDREF(msg) newInstance(NetCommandRef)(msg, __FILE__, __LINE__)
#else
#define NEW_NETCOMMANDREF(msg) newInstance(NetCommandRef)(msg)
#endif


class NetCommandRef : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandRef, "NetCommandRef")
public:
#ifdef DEBUG_NETCOMMANDREF
	NetCommandRef(NetCommandMsg *msg, char *filename, int line);
#else
	NetCommandRef(NetCommandMsg *msg);
#endif
	//~NetCommandRef();

	NetCommandMsg *getCommand();
	NetCommandRef *getNext();
	NetCommandRef *getPrev();
	void setNext(NetCommandRef *next);
	void setPrev(NetCommandRef *prev);

	void setRelay(UnsignedByte relay);
	UnsignedByte getRelay() const;

	time_t getTimeLastSent() const;
	void setTimeLastSent(time_t timeLastSent);

protected:
	NetCommandMsg *m_msg;
	NetCommandRef *m_next;
	NetCommandRef *m_prev;
	UnsignedByte m_relay; ///< Need this in the command reference since the relay value will be different depending on where this particular reference is being sent.
	time_t m_timeLastSent;

#ifdef DEBUG_NETCOMMANDREF
	UnsignedInt m_id;
#endif
};

/**
 * Return the command message.
 */
inline NetCommandMsg * NetCommandRef::getCommand()
{
	return m_msg;
}

/**
 * Return the next command ref in the list.
 */
inline NetCommandRef * NetCommandRef::getNext()
{
	return m_next;
}

/**
 * Return the previous command ref in the list.
 */
inline NetCommandRef * NetCommandRef::getPrev()
{
	return m_prev;
}

/**
 * Set the next command ref in the list.
 */
inline void NetCommandRef::setNext(NetCommandRef *next)
{
	m_next = next;
}

/**
 * Set the previous command ref in the list.
 */
inline void NetCommandRef::setPrev(NetCommandRef *prev)
{
	m_prev = prev;
}

/**
 * Return the time for the last time this command was sent from this reference.
 */
inline time_t NetCommandRef::getTimeLastSent() const
{
	return m_timeLastSent;
}

/**
 * Set the time for the last time this command was sent from this reference.
 */
inline void NetCommandRef::setTimeLastSent(time_t timeLastSent)
{
	m_timeLastSent = timeLastSent;
}

/**
 * Set the send relay for this reference of the command.
 */
inline void NetCommandRef::setRelay(UnsignedByte relay)
{
	m_relay = relay;
}

/**
 * Return the send relay for this refreence of the command.
 */
inline UnsignedByte NetCommandRef::getRelay() const
{
	return m_relay;
}
