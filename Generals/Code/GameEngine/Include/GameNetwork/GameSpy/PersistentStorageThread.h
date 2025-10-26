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

// FILE: PersistentStorageThread.h //////////////////////////////////////////////////////
// Generals GameSpy Persistent Storage thread class interface
// Author: Matthew D. Campbell, July 2002

#pragma once

#include "gamespy/gstats/gpersist.h"

#define MAX_BUDDY_CHAT_LEN 128

typedef std::map<Int, UnsignedInt> PerGeneralMap;
// this structure holds all info on a player that is stored online
class PSPlayerStats
{
public:
	PSPlayerStats( void );
	PSPlayerStats( const PSPlayerStats& other );
	void reset(void);

	Int id;
	PerGeneralMap wins;
	PerGeneralMap losses;
	PerGeneralMap games;
	PerGeneralMap duration;
	PerGeneralMap unitsKilled;
	PerGeneralMap unitsLost;
	PerGeneralMap unitsBuilt;
	PerGeneralMap buildingsKilled;
	PerGeneralMap buildingsLost;
	PerGeneralMap buildingsBuilt;
	PerGeneralMap earnings;
	PerGeneralMap techCaptured;
	PerGeneralMap discons;
	PerGeneralMap desyncs;
	PerGeneralMap surrenders;
	PerGeneralMap gamesOf2p;
	PerGeneralMap gamesOf3p;
	PerGeneralMap gamesOf4p;
	PerGeneralMap gamesOf5p;
	PerGeneralMap gamesOf6p;
	PerGeneralMap gamesOf7p;
	PerGeneralMap gamesOf8p;
	PerGeneralMap customGames;
	PerGeneralMap QMGames;
	Int locale;
	Int gamesAsRandom;
	std::string options;
	std::string systemSpec;
	Real lastFPS;
	Int lastGeneral;
	Int gamesInRowWithLastGeneral;
	Int challengeMedals;
	Int battleHonors;
	Int QMwinsInARow;
	Int maxQMwinsInARow;

	Int winsInARow;
	Int maxWinsInARow;
	Int lossesInARow;
	Int maxLossesInARow;
	Int disconsInARow;
	Int maxDisconsInARow;
	Int desyncsInARow;
	Int maxDesyncsInARow;

	Int builtParticleCannon;
	Int builtNuke;
	Int builtSCUD;

	Int lastLadderPort;
	std::string lastLadderHost;

	void incorporate( const PSPlayerStats& other );
};

// this class encapsulates a request for the thread
class PSRequest
{
public:
	PSRequest();
	enum
	{
		PSREQUEST_READPLAYERSTATS,			// read stats for a player
		PSREQUEST_UPDATEPLAYERSTATS,		// update stats on the server
		PSREQUEST_UPDATEPLAYERLOCALE,		// update locale on the server
		PSREQUEST_READCDKEYSTATS,				// read stats for a cdkey
		PSREQUEST_SENDGAMERESTOGAMESPY,	// report game results to GameSpy
		PSREQUEST_MAX
	} requestType;

	// player stats for the *PLAYERSTATS
	PSPlayerStats player;

	// cdkey for READCDKEYSTATS;
	std::string cdkey;

	// our info for UPDATEPLAYERSTATS
	std::string nick;
	std::string password;
	std::string email;
	Bool addDiscon;
	Bool addDesync;
	Int lastHouse;

	// for GameRes
	std::string results;
};

//-------------------------------------------------------------------------

// this class encapsulates a response from the thread
class PSResponse
{
public:
	enum
	{
		PSRESPONSE_PLAYERSTATS,
		PSRESPONSE_COULDNOTCONNECT,
		PSRESPONSE_PREORDER,
		PSRESPONSE_MAX
	} responseType;

	// player stats for the *PLAYERSTATS
	PSPlayerStats player;

	// preorder flag
	Bool preorder;
};

//-------------------------------------------------------------------------

// this is the actual message queue used to pass messages between threads
class GameSpyPSMessageQueueInterface
{
public:
	virtual ~GameSpyPSMessageQueueInterface() {}
	virtual void startThread( void ) = 0;
	virtual void endThread( void ) = 0;
	virtual Bool isThreadRunning( void ) = 0;

	virtual void addRequest( const PSRequest& req ) = 0;
	virtual Bool getRequest( PSRequest& req ) = 0;

	virtual void addResponse( const PSResponse& resp ) = 0;
	virtual Bool getResponse( PSResponse& resp ) = 0;

	// called from the main thread
	virtual void trackPlayerStats( PSPlayerStats stats ) = 0;
	virtual PSPlayerStats findPlayerStatsByID( Int id ) = 0;

	static GameSpyPSMessageQueueInterface* createNewMessageQueue( void );

	static std::string formatPlayerKVPairs( PSPlayerStats stats );
	static PSPlayerStats parsePlayerKVPairs( std::string kvPairs );
};

extern GameSpyPSMessageQueueInterface *TheGameSpyPSMessageQueue;
