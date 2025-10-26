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

// IPEnumeration.h ///////////////////////////////////////////////////////////////
// Class for enumerating IP addresses
// Author: Matthew D. Campbell, October 2001

#pragma once

#include "GameNetwork/Transport.h"

/**
 * IP wrapper class.
 */
class EnumeratedIP : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(EnumeratedIP, "EnumeratedIP")
public:
	EnumeratedIP() { m_IPstring = ""; m_next = NULL; m_IP = 0; }

	// Access functions
	inline AsciiString getIPstring( void ) { return m_IPstring; }
	inline void setIPstring( AsciiString name ) { m_IPstring = name; }
	inline UnsignedInt getIP( void ) { return m_IP; }
	inline void setIP( UnsignedInt IP ) { m_IP = IP; }
	inline EnumeratedIP *getNext( void ) { return m_next; }
	inline void setNext( EnumeratedIP *next ) { m_next = next; }

protected:
	AsciiString m_IPstring;
	UnsignedInt m_IP;
	EnumeratedIP *m_next;
};
EMPTY_DTOR(EnumeratedIP)


/**
 * The IPEnumeration class is used to obtain a list of IP addresses on the
 * local machine.
 */
class IPEnumeration
{
public:

	IPEnumeration();
	~IPEnumeration();

	EnumeratedIP * getAddresses( void );		///< Return a linked list of local IP addresses
	AsciiString getMachineName( void );			///< Return the Network Neighborhood machine name

protected:
	void addNewIP( UnsignedByte a, UnsignedByte b, UnsignedByte c, UnsignedByte d );

	EnumeratedIP *m_IPlist;
	Bool m_isWinsockInitialized;
};
