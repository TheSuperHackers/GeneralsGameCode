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

#pragma once

#include "Common/GameMemory.h"
#include "GameNetwork/NetCommandRef.h"

/**
 * The NetCommandList is a ordered linked list of NetCommandRef objects.
 * The list is ordered based on the command id, player id, and command type.
 * It is ordered in this way to aid in constructing the packets efficiently.
 * The list keeps track of the last message inserted in order to accommodate
 * adding commands in order more efficiently since that is whats going to be
 * done most of the time.  If the new message doesn't go after the last message
 * inserted, then the list will be traversed linearly until the proper spot is
 * found.  We can get away with this inefficient method since these occurances
 * will be rare.  Also, the list is not expected to ever have more than 30 or so
 * commands on it at a time.  Five commands would probably be a normal amount.
 */

class NetCommandList : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandList, "NetCommandList")
public:
	NetCommandList();
	//virtual ~NetCommandList();

	void init();									///< Initialize the list
	void reset();									///< Reset the list to the initial state.
	NetCommandRef * addMessage(NetCommandMsg *cmdMsg);	///< Add message to the list in its properly ordered place.
	Bool isEqualCommandMsg(NetCommandMsg *msg1, NetCommandMsg *msg2);
	NetCommandRef * getFirstMessage();				///< Get the first message on the list.
	NetCommandRef * findMessage(NetCommandMsg *msg);	///< Find and return a reference to the given message if one exists.
	NetCommandRef * findMessage(UnsignedShort commandID, UnsignedByte playerID);	///< Find and return a reference to the
																								///< message given the player id and the command id.
																								///< This will only check against messages of types that require
																								///< a command id.
	void removeMessage(NetCommandRef *msg);			///< Remove the given message from the list.
	void appendList(NetCommandList *list);			///< Append the given list to the end of this list.
	Int length();									///< Returns the number of nodes in this list.  This is inefficient and is meant to be a debug tool.

protected:
	NetCommandRef *m_first;							///< Head of the list.
	NetCommandRef *m_last;							///< Tail of the list.
	NetCommandRef *m_lastMessageInserted;			///< The last message that was inserted to this list.
};
