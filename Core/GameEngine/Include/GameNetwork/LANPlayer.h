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

// LANPlayer.h ///////////////////////////////////////////////////////////////
// LAN Player Class used for both the LANAPI and LANGameInfo
// Author: Matthew D. Campbell, October 2001 (Pulled out of LANAPI.h by CLH on 12/21/01

#pragma once

/**
 * LAN player class.  This is for players sitting in the lobby.  Players are
 * uniquely identified by their IP addresses.
 */
class LANPlayer
{
public:
	LANPlayer() { m_name = m_login = m_host = L""; m_lastHeard = 0; m_next = NULL; m_IP = 0; }

	// Access functions
	UnicodeString getName( void ) { return m_name; }
	void setName( UnicodeString name ) { m_name = name; }
	UnicodeString getLogin( void ) { return m_login; }
	void setLogin( UnicodeString name ) { m_login = name; }
	void setLogin( AsciiString name ) { m_login.translate(name); }
	UnicodeString getHost( void ) { return m_host; }
	void setHost( UnicodeString name ) { m_host = name; }
	void setHost( AsciiString name ) { m_host.translate(name); }
	UnsignedInt getLastHeard( void ) { return m_lastHeard; }
	void setLastHeard( UnsignedInt lastHeard ) { m_lastHeard = lastHeard; }
	LANPlayer *getNext( void ) { return m_next; }
	void setNext( LANPlayer *next ) { m_next = next; }
	UnsignedInt getIP( void ) { return m_IP; }
	void setIP( UnsignedInt IP ) { m_IP = IP; }

protected:
	UnicodeString m_name;			///< Player name
	UnicodeString m_login;		///< login name
	UnicodeString m_host;			///< machine name
	UnsignedInt m_lastHeard;	///< The last time we heard from this player (for timeout purposes)
	LANPlayer *m_next;				///< Linked list pointer
	UnsignedInt m_IP;					///< Player's IP
};
