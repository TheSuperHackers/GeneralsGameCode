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

// FILE: GameSpy.h //////////////////////////////////////////////////////
// Generals GameSpy stuff
// Author: Matthew D. Campbell, February 2002

#pragma once

#include "gamespy/peer/peer.h"

#include "GameClient/Color.h"
#include "Common/STLTypedefs.h"

class GameSpyGroupRoom
{
public:
	GameSpyGroupRoom() { m_name = AsciiString::TheEmptyString; m_groupID = m_numWaiting = m_maxWaiting = m_numGames = m_numPlaying = 0; }
	AsciiString m_name;
	Int m_groupID;
	Int m_numWaiting;
	Int m_maxWaiting;
	Int m_numGames;
	Int m_numPlaying;
};
typedef std::map<Int, GameSpyGroupRoom> GroupRoomMap;

class GameSpyChatInterface : public SubsystemInterface
{
public:
	virtual ~GameSpyChatInterface() { };

	virtual void init( void ) = 0;
	virtual void reset( void ) = 0;
	virtual void update( void ) = 0;

	virtual Bool isConnected( void ) = 0;
	virtual void login(AsciiString loginName, AsciiString password = AsciiString::TheEmptyString, AsciiString email = AsciiString::TheEmptyString) = 0;
	virtual void reconnectProfile( void ) = 0;
	virtual void disconnectFromChat( void ) = 0;

	virtual void UTMRoom( RoomType roomType, const char *key, const char *val, Bool authenticate = FALSE ) = 0;
	virtual void UTMPlayer( const char *name, const char *key, const char *val, Bool authenticate = FALSE ) = 0;
	virtual void startGame( void ) = 0;
	virtual void leaveRoom( RoomType roomType ) = 0;
	virtual void setReady( Bool ready ) = 0;
	virtual void enumPlayers( RoomType roomType, peerEnumPlayersCallback callback, void *userData ) = 0;
	virtual void startListingGames( peerListingGamesCallback callback ) = 0;
	virtual void stopListingGames( void ) = 0;

	virtual void joinGroupRoom( Int ID ) = 0;
	virtual void joinStagingRoom( GServer server, AsciiString password ) = 0;
	virtual void createStagingRoom( AsciiString gameName, AsciiString password, Int maxPlayers ) = 0;
	virtual void joinBestGroupRoom( void ) = 0;

	inline PEER getPeer( void )									{ return m_peer; }
	inline AsciiString getLoginName( void )			{ return m_loginName; }
	inline AsciiString getPassword( void )			{ return m_password; }
	inline GroupRoomMap* getGroupRooms( void )	{ return &m_groupRooms; }
	inline Int getCurrentGroupRoomID( void )		{ return m_currentGroupRoomID; }
	inline Bool getUsingProfile( void )					{ return m_usingProfiles; }
	inline Int getProfileID( void )							{ return m_profileID; }

	inline void setCurrentGroupRoomID( Int ID )	{ m_currentGroupRoomID = ID; }
	void clearGroupRoomList(void);
	inline Int getNumGroupRooms( void )					{ return m_groupRooms.size(); }


protected:

	AsciiString m_loginName;
	AsciiString m_password;
	AsciiString m_email;
	Bool m_usingProfiles;
	Int m_profileID;
	PEER m_peer;

	GroupRoomMap m_groupRooms;
	Int m_currentGroupRoomID;
};

GameSpyChatInterface *createGameSpyChat( void );

extern GameSpyChatInterface *TheGameSpyChat;


void JoinRoomCallback(PEER peer, PEERBool success,
											PEERJoinResult result, RoomType roomType,
											void *param);																	///< Called when we (fail to) join a room.  param is address of Bool to store result

void ListGroupRoomsCallback(PEER peer, PEERBool success,
														int groupID, GServer server,
														const char * name, int numWaiting,
														int maxWaiting, int numGames,
														int numPlaying, void * param);					///< Called while listing group rooms

enum GameSpyColors CPP_11(: Int) {
	GSCOLOR_DEFAULT = 0,
	GSCOLOR_CURRENTROOM,
	GSCOLOR_ROOM,
	GSCOLOR_GAME,
	GSCOLOR_PLAYER_NORMAL,
	GSCOLOR_PLAYER_OWNER,
	GSCOLOR_PLAYER_BUDDY,
	GSCOLOR_PLAYER_SELF,
	GSCOLOR_CHAT_NORMAL,
	GSCOLOR_CHAT_EMOTE,
	GSCOLOR_CHAT_OWNER,
	GSCOLOR_CHAT_OWNER_EMOTE,
	GSCOLOR_CHAT_PRIVATE,
	GSCOLOR_CHAT_PRIVATE_EMOTE,
	GSCOLOR_CHAT_PRIVATE_OWNER,
	GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE,
	GSCOLOR_CHAT_BUDDY,
	GSCOLOR_CHAT_SELF,
	GSCOLOR_ACCEPT_TRUE,
	GSCOLOR_ACCEPT_FALSE,
	GSCOLOR_MAX
};

extern const Color GameSpyColor[GSCOLOR_MAX];
