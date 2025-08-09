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

// FILE: LANGameInfo.h //////////////////////////////////////////////////////
// Generals LAN game setup information
// Author: Matthew D. Campbell, December 2001

#pragma once

#ifndef __LANGAMEINFO_H__
#define __LANGAMEINFO_H__

#include "GameNetwork/GameInfo.h"
#include "GameNetwork/LANPlayer.h"

class GameWindow;

/**
  * LANGameSlot class - maintains information about the contents of a
	* game slot.  This persists throughout the game.
	*/
class LANGameSlot : public GameSlot
{
public:
	LANGameSlot();

	LANPlayer *getUser( void );																	///< Get the User structure associated with the slot (NULL for non-humans)

	// Various tests
	Bool isUser( LANPlayer *user );															///< Does this slot contain the given user?  Based off user->name
	Bool isUser( UnicodeString userName );									///< Does this slot contain the given user?
	Bool isLocalPlayer( void ) const;															///< Is this slot me?
	inline void setLogin( UnicodeString name ) { m_user.setLogin(name); }
	inline void setLogin( AsciiString name ) { m_user.setLogin(name); }
	inline void setHost( UnicodeString name ) { m_user.setHost(name); }
	inline void setHost( AsciiString name ) { m_user.setHost(name); }
	inline void setSerial( AsciiString serial ) { m_serial = serial; }
	inline AsciiString getSerial( void ) { return m_serial; }

	inline void setLastHeard( UnsignedInt t ) { m_lastHeard = t; }
	inline UnsignedInt getLastHeard( void ) { return m_lastHeard; }

	//LANGameSlot& operator=(const LANGameSlot& src);

private:

	LANPlayer m_user;	///< filled in for each getUser() call
	AsciiString m_serial;

	UnsignedInt m_lastHeard;
};

/**
  * LANGameInfo class - maintains information about the LAN game and
	* the contents of its slot list hroughout the game.
	*/
class LANGameInfo : public GameInfo
{
private:
	LANGameSlot m_LANSlot[MAX_SLOTS];											///< The Lan Games Slot List

public:
	LANGameInfo();
	void setSlot( Int slotNum, LANGameSlot slotInfo );	///< Set the slot state (human, open, AI, etc)
	LANGameSlot* getLANSlot( Int slotNum );							///< Get the slot
	const LANGameSlot* getConstLANSlot( Int slotNum ) const;							///< Get the slot
	virtual Int getLocalSlotNum( void ) const;												///< Get the local slot number, or -1 if we're not present
	Int getSlotNum( UnicodeString userName );						///< Get the slot number corresponding to a specific user, or -1 if he's not present

	inline UnsignedInt getLastHeard( void ) { return m_lastHeard; }
	inline void setLastHeard( UnsignedInt lastHeard ) { m_lastHeard = lastHeard; }
	inline LANGameInfo *getNext( void ) { return m_next; }
	inline void setNext( LANGameInfo *next ) { m_next = next; }

	// Game options
	void setMap( AsciiString mapName );									///< Set the map to play on
	void setSeed( Int seed );														///< Set the random seed for the game

	inline void setName( UnicodeString name ) { m_gameName = name; }		///< Set the Name of the Game
	inline UnicodeString getName( void ) { return m_gameName; }					///< Get the Name of the Game

	// Convinience functions that interface with the LANPlayer held in the slot list
	virtual void resetAccepted(void);														///< Reset the accepted flag on all players
	Bool amIHost( void );																///< Convenience function - is the local player the game host?

	/// Get the IP of selected player or return 0
	inline UnsignedInt getIP( int who )
	{
		return m_LANSlot[who].getIP();
	}

	/// Set the IP of the Selected Player
	inline void setIP( int who, UnsignedInt IP )
	{
		m_LANSlot[who].setIP(IP);
	}

	/// set whether or not this is a direct connect game or not.
	inline void setIsDirectConnect(Bool isDirectConnect)
	{
		m_isDirectConnect = isDirectConnect;
	}

	/// returns whether or not this is a direct connect game or not.
	inline Bool getIsDirectConnect()
	{
		return m_isDirectConnect;
	}

	/// Set the Player Name
	inline void setPlayerName( int who, UnicodeString name )
	{
		m_LANSlot[who].setName(name);
	}

	/// Return the Player name or TheEmptyString
	inline UnicodeString getPlayerName(int who)
	{
		return m_LANSlot[who].getName();
	}

	/// Return the time the player was heard from last, or 0
	inline UnsignedInt getPlayerLastHeard( int who )
	{
		if (m_LANSlot[who].isHuman())
			return m_LANSlot[who].getLastHeard();
		return 0;
	}

	/// Set the last time we heard from the player
	inline void setPlayerLastHeard( int who, UnsignedInt lastHeard )
	{
		DEBUG_LOG(("LANGameInfo::setPlayerLastHeard - changing player %d last heard from %d to %d", who, getPlayerLastHeard(who), lastHeard));
		if (m_LANSlot[who].isHuman())
			m_LANSlot[who].setLastHeard(lastHeard);
	}

	/// Return the hosts IP or 0
	UnsignedInt getHostIP(void)
	{
		if (m_LANSlot[0].isHuman())
			return m_LANSlot[0].getIP();
		return 0;
	}


private:
	LANGameInfo *m_next;																///< Pointer for linked list
	UnsignedInt m_lastHeard;														///< The last time we heard from this game (for timeout purposes)
	UnicodeString m_gameName;														///< Game name.  @todo: are game names based off of host player names?
	Bool m_isDirectConnect;															///< Is this game a direct connect game, or a LAN game?
};

void LANDisplayGameList( GameWindow *gameListbox, LANGameInfo *gameList );	///< Displays the list of games in a listbox, preserving selections
void LANEnableStartButton(Bool enabled);

void LANDisplaySlotList( void );		///< Displays the slot list according to TheLANGameInfo
void LANDisplayGameOptions( void );	///< Displays the game options according to TheLANGameInfo

AsciiString GenerateGameOptionsString( void );
Bool ParseGameOptionsString(LANGameInfo *game, AsciiString options);


#endif // __LANGAMEINFO_H__
