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

// FILE: GameSpyGameInfo.h //////////////////////////////////////////////////////
// Generals GameSpy game setup information
// Author: Matthew D. Campbell, February 2002

#pragma once

#error this file is obsolete
#include "gamespy/peer/peer.h"

#include "GameNetwork/GameInfo.h"

class Transport;
class NAT;

class GameSpyGameSlot : public GameSlot
{
public:
	GameSpyGameSlot();
	Int getProfileID( void ) { return m_profileID; }
	void setProfileID( Int id ) { m_profileID = id; }
	AsciiString getLoginName( void ) { return m_gameSpyLogin; }
	void setLoginName( AsciiString name ) { m_gameSpyLogin = name; }
	AsciiString getLocale( void ) { return m_gameSpyLocale; }
	void setLocale( AsciiString name ) { m_gameSpyLocale = name; }
protected:
	Int m_profileID;
	AsciiString m_gameSpyLogin;
	AsciiString m_gameSpyLocale;
};

/**
  * GameSpyGameInfo class - maintains information about the GameSpy game and
	* the contents of its slot list throughout the game.
	*/
class GameSpyGameInfo : public GameInfo
{
private:
	GameSpyGameSlot m_GameSpySlot[MAX_SLOTS];											///< The GameSpy Games Slot List
	SBServer m_server;
	Bool m_hasBeenQueried;
	Transport *m_transport;
	Bool m_isQM;

public:
	GameSpyGameInfo();

	inline void setServer(SBServer server) { m_server = server; }
	inline SBServer getServer( void ) { return m_server; }

	AsciiString generateGameResultsPacket( void );

	virtual void init(void);
	virtual void resetAccepted(void);															///< Reset the accepted flag on all players

	void markGameAsQM( void ) { m_isQM = TRUE; }
	virtual void startGame(Int gameID);														///< Mark our game as started and record the game ID.
	virtual Int getLocalSlotNum( void ) const;				///< Get the local slot number, or -1 if we're not present

	void gotGOACall( void );																			///< Mark the game info as having been queried
};

extern GameSpyGameInfo *TheGameSpyGame;

void WOLDisplayGameOptions( void );
void WOLDisplaySlotList( void );
void GameSpyStartGame( void );
void GameSpyLaunchGame( void );
Bool GetLocalChatConnectionAddress(AsciiString serverName, UnsignedShort serverPort, UnsignedInt& localIP);
