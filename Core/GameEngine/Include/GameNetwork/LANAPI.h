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

// LANAPI.h ///////////////////////////////////////////////////////////////
// LANAPI singleton class - defines interface to LAN broadcast communications
// Author: Matthew D. Campbell, October 2001

#pragma once

#include "GameNetwork/Transport.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/LANPlayer.h"
#include "GameNetwork/LANGameInfo.h"

//static const Int g_lanPlayerNameLength = 20;
static const Int g_lanPlayerNameLength = 12; // reduced length because of game option length
//static const Int g_lanLoginNameLength = 16;
//static const Int g_lanHostNameLength = 16;
static const Int g_lanLoginNameLength = 1;
static const Int g_lanHostNameLength = 1;
//static const Int g_lanGameNameLength = 32;
static const Int g_lanGameNameLength = 16; // reduced length because of game option length
static const Int g_lanGameNameReservedLength = 16; // save N wchars for ID info
static const Int g_lanMaxChatLength = 100;
static const Int m_lanMaxOptionsLength = MAX_PACKET_SIZE - ( 8 + (g_lanGameNameLength+1)*2 + 4 + (g_lanPlayerNameLength+1)*2
																														+ (g_lanLoginNameLength+1) + (g_lanHostNameLength+1) );
static const Int g_maxSerialLength = 23; // including the trailing '\0'

struct LANMessage;

/**
 * The LANAPI class is used to instantiate a singleton which
 * implements the interface to all LAN broadcast communications.
 */
class LANAPIInterface : public SubsystemInterface
{
public:

	virtual ~LANAPIInterface() { };

	virtual void init( void ) = 0;															///< Initialize or re-initialize the instance
	virtual void reset( void ) = 0;															///< reset the logic system
	virtual void update( void ) = 0;														///< update the world

	virtual void setIsActive(Bool isActive ) = 0;								///< Tell TheLAN whether or not the app is active.

	// Possible types of chat messages
	enum ChatType
	{
		LANCHAT_NORMAL = 0,
		LANCHAT_EMOTE,
		LANCHAT_SYSTEM,
	};

	// Request functions generate network traffic
	virtual void RequestLocations( void ) = 0;																				///< Request everybody to respond with where they are
	virtual void RequestGameJoin( LANGameInfo *game, UnsignedInt ip = 0 ) = 0;				///< Request to join a game
	virtual void RequestGameJoinDirectConnect( UnsignedInt ipaddress ) = 0;						///< Request to join a game at an IP address
	virtual void RequestGameLeave( void ) = 0;																				///< Tell everyone we're leaving
	virtual void RequestAccept( void ) = 0;																						///< Indicate we're OK with the game options
	virtual void RequestHasMap( void ) = 0;																						///< Send our map status
	virtual void RequestChat( UnicodeString message, ChatType format ) = 0;						///< Send a chat message
	virtual void RequestGameStart( void ) = 0;																				///< Tell everyone the game is starting
	virtual void RequestGameStartTimer( Int seconds ) = 0;
	virtual void RequestGameOptions( AsciiString gameOptions, Bool isPublic, UnsignedInt ip = 0 ) = 0;		///< Change the game options
	virtual void RequestGameCreate( UnicodeString gameName, Bool isDirectConnect ) = 0;	///< Try to host a game
	virtual void RequestGameAnnounce( void ) = 0;																			///< Sound out current game info if host
//	virtual void RequestSlotList( void ) = 0;																					///< Pump out the Slot info.
	virtual void RequestSetName( UnicodeString newName ) = 0;													///< Pick a new name
	virtual void RequestLobbyLeave( Bool forced ) = 0;																///< Announce that we're leaving the lobby
	virtual void ResetGameStartTimer( void ) = 0;

	// Possible result codes passed to On functions
	enum ReturnType
	{
		RET_OK,							// Any function
		RET_TIMEOUT,				// OnGameJoin/Leave/Start, etc
		RET_GAME_FULL,			// OnGameJoin
		RET_DUPLICATE_NAME,	// OnGameJoin
		RET_CRC_MISMATCH,		// OnGameJoin
		RET_SERIAL_DUPE,		// OnGameJoin
		RET_GAME_STARTED,		// OnGameJoin
		RET_GAME_EXISTS,		// OnGameCreate
		RET_GAME_GONE,			// OnGameJoin
		RET_BUSY,						// OnGameCreate/Join/etc if another action is in progress
		RET_UNKNOWN,				// Default message for oddity
	};
	UnicodeString getErrorStringFromReturnType( ReturnType ret );

	// On functions are (generally) the result of network traffic
	virtual void OnGameList( LANGameInfo *gameList ) = 0;																							///< List of games
	virtual void OnPlayerList( LANPlayer *playerList ) = 0;																				///< List of players in the Lobby
	virtual void OnGameJoin( ReturnType ret, LANGameInfo *theGame ) = 0;															///< Did we get in the game?
	virtual void OnPlayerJoin( Int slot, UnicodeString playerName ) = 0;													///< Someone else joined our game (host only; joiners get a slotlist)
	virtual void OnHostLeave( void ) = 0;																													///< Host left the game
	virtual void OnPlayerLeave( UnicodeString player ) = 0;																				///< Someone left the game
	virtual void OnAccept( UnsignedInt playerIP, Bool status ) = 0;																///< Someone's accept status changed
	virtual void OnHasMap( UnsignedInt playerIP, Bool status ) = 0;																///< Someone's map status changed
	virtual void OnChat( UnicodeString player, UnsignedInt ip,
											 UnicodeString message, ChatType format ) = 0;														///< Chat message from someone
	virtual void OnGameStart( void ) = 0;																													///< The game is starting
	virtual void OnGameStartTimer( Int seconds ) = 0;
	virtual void OnGameOptions( UnsignedInt playerIP, Int playerSlot, AsciiString options ) = 0;	///< Someone sent game options
	virtual void OnGameCreate( ReturnType ret ) = 0;																							///< Your game is created
//	virtual void OnSlotList( ReturnType ret, LANGameInfo *theGame ) = 0;															///< Slotlist for a game in setup
	virtual void OnNameChange( UnsignedInt IP, UnicodeString newName ) = 0;												///< Someone has morphed

	// Misc utility functions
	virtual LANGameInfo * LookupGame( UnicodeString gameName ) = 0;														///< return a pointer to a game we know about
	virtual LANGameInfo * LookupGameByListOffset( Int offset ) = 0;														///< return a pointer to a game we know about
	virtual Bool SetLocalIP( UnsignedInt localIP ) = 0;																		///< For multiple NIC machines
	virtual void SetLocalIP( AsciiString localIP ) = 0;																		///< For multiple NIC machines
	virtual Bool AmIHost( void ) = 0;																											///< Am I hosting a game?
	virtual inline UnicodeString GetMyName( void ) = 0;																		///< What's my name?
	virtual inline LANGameInfo *GetMyGame( void ) = 0;															          ///< What's my Game?
	virtual void fillInLANMessage( LANMessage *msg ) = 0;																	///< Fill in default params
	virtual void checkMOTD( void ) = 0;
};


/**
 * The LANAPI class is used to instantiate a singleton which
 * implements the interface to all LAN broadcast communications.
 */
class LANAPI : public LANAPIInterface
{
public:

	LANAPI();
	virtual ~LANAPI();

	virtual void init( void );															///< Initialize or re-initialize the instance
	virtual void reset( void );															///< reset the logic system
	virtual void update( void );														///< update the world

	virtual void setIsActive(Bool isActive);								///< tell TheLAN whether or not

	// Request functions generate network traffic
	virtual void RequestLocations( void );																				///< Request everybody to respond with where they are
	virtual void RequestGameJoin( LANGameInfo *game, UnsignedInt ip = 0 );				///< Request to join a game
	virtual void RequestGameJoinDirectConnect( UnsignedInt ipaddress );						///< Request to join a game at an IP address
	virtual void RequestGameLeave( void );																				///< Tell everyone we're leaving
	virtual void RequestAccept( void );																						///< Indicate we're OK with the game options
	virtual void RequestHasMap( void );																						///< Send our map status
	virtual void RequestChat( UnicodeString message, ChatType format );						///< Send a chat message
	virtual void RequestGameStart( void );																				///< Tell everyone the game is starting
	virtual void RequestGameStartTimer( Int seconds );
	virtual void RequestGameOptions( AsciiString gameOptions, Bool isPublic, UnsignedInt ip = 0 );		///< Change the game options
	virtual void RequestGameCreate( UnicodeString gameName, Bool isDirectConnect );	///< Try to host a game
	virtual void RequestGameAnnounce( void );																			///< Send out game info if host
	virtual void RequestSetName( UnicodeString newName );													///< Pick a new name
//	virtual void RequestSlotList( void );																					///< Pump out the Slot info.
	virtual void RequestLobbyLeave( Bool forced );																///< Announce that we're leaving the lobby
	virtual void ResetGameStartTimer( void );

	// On functions are (generally) the result of network traffic
	virtual void OnGameList( LANGameInfo *gameList );																							///< List of games
	virtual void OnPlayerList( LANPlayer *playerList );																				///< List of players in the Lobby
	virtual void OnGameJoin( ReturnType ret, LANGameInfo *theGame );															///< Did we get in the game?
	virtual void OnPlayerJoin( Int slot, UnicodeString playerName );													///< Someone else joined our game (host only; joiners get a slotlist)
	virtual void OnHostLeave( void );																													///< Host left the game
	virtual void OnPlayerLeave( UnicodeString player );																				///< Someone left the game
	virtual void OnAccept( UnsignedInt playerIP, Bool status );																///< Someone's accept status changed
	virtual void OnHasMap( UnsignedInt playerIP, Bool status );																///< Someone's map status changed
	virtual void OnChat( UnicodeString player, UnsignedInt ip,
											 UnicodeString message, ChatType format );														///< Chat message from someone
	virtual void OnGameStart( void );																													///< The game is starting
	virtual void OnGameStartTimer( Int seconds );
	virtual void OnGameOptions( UnsignedInt playerIP, Int playerSlot, AsciiString options );	///< Someone sent game options
	virtual void OnGameCreate( ReturnType ret );																							///< Your game is created
	//virtual void OnSlotList( ReturnType ret, LANGameInfo *theGame );															///< Slotlist for a game in setup
	virtual void OnNameChange( UnsignedInt IP, UnicodeString newName );												///< Someone has morphed
	virtual void OnInActive( UnsignedInt IP );																								///< Someone has alt-tabbed out.


	// Misc utility functions
	virtual LANGameInfo * LookupGame( UnicodeString gameName );														///< return a pointer to a game we know about
	virtual LANGameInfo * LookupGameByListOffset( Int offset );														///< return a pointer to a game we know about
	virtual LANPlayer * LookupPlayer( UnsignedInt playerIP );													///< return a pointer to a player we know about
	virtual Bool SetLocalIP( UnsignedInt localIP );																		///< For multiple NIC machines
	virtual void SetLocalIP( AsciiString localIP );																		///< For multiple NIC machines
	virtual Bool AmIHost( void );																											///< Am I hosting a game?
	virtual UnicodeString GetMyName( void ) { return m_name; }                 ///< What's my name?
	virtual LANGameInfo* GetMyGame( void ) { return m_currentGame; }					      ///< What's my Game?
	virtual UnsignedInt GetLocalIP( void ) { return m_localIP; }								///< What's my IP?
	virtual void fillInLANMessage( LANMessage *msg );																	///< Fill in default params
	virtual void checkMOTD( void );
protected:

	enum PendingActionType
	{
		ACT_NONE = 0,
		ACT_JOIN,
		ACT_JOINDIRECTCONNECT,
		ACT_LEAVE,
	};

	static const UnsignedInt s_resendDelta; // in ms

protected:
	LANPlayer *					m_lobbyPlayers;			///< List of players in the lobby
	LANGameInfo *				m_games;								///< List of games
	UnicodeString				m_name;							///< Who do we think we are?
	AsciiString					m_userName;						///< login name
	AsciiString					m_hostName;						///< machine name
	UnsignedInt					m_gameStartTime;
	Int									m_gameStartSeconds;

	PendingActionType		m_pendingAction;	///< What action are we performing?
	UnsignedInt					m_expiration;						///< When should we give up on our action?
	UnsignedInt					m_actionTimeout;
	UnsignedInt					m_directConnectRemoteIP;///< The IP address of the game we are direct connecting to.

	// Resend timer ---------------------------------------------------------------------------
	UnsignedInt					m_lastResendTime; // in ms

	Bool								m_isInLANMenu;		///< true while we are in a LAN menu (lobby, game options, direct connect)
	Bool								m_inLobby;											///< Are we in the lobby (not in a game)?
	LANGameInfo *				m_currentGame;							///< Pointer to game (setup screen) we are currently in (null for lobby)
	//LANGameInfo *m_currentGameInfo;			///< Pointer to game setup info we are currently in.

	UnsignedInt					m_localIP;
	Transport*					m_transport;

	UnsignedInt					m_broadcastAddr;

	UnsignedInt					m_lastUpdate;
	AsciiString					m_lastGameopt; /// @todo: hack for demo - remove this

	Bool								m_isActive;			///< is the game currently active?

protected:
	void sendMessage(LANMessage *msg, UnsignedInt ip = 0); // Convenience function
	void removePlayer(LANPlayer *player);
	void removeGame(LANGameInfo *game);
	void addPlayer(LANPlayer *player);
	void addGame(LANGameInfo *game);
	AsciiString createSlotString( void );

	// Functions to handle incoming messages -----------------------------------
	void handleRequestLocations( LANMessage *msg, UnsignedInt senderIP );
	void handleGameAnnounce( LANMessage *msg, UnsignedInt senderIP );
	void handleLobbyAnnounce( LANMessage *msg, UnsignedInt senderIP );
	void handleRequestGameInfo( LANMessage *msg, UnsignedInt senderIP );
	void handleRequestJoin( LANMessage *msg, UnsignedInt senderIP );
	void handleJoinAccept( LANMessage *msg, UnsignedInt senderIP );
	void handleJoinDeny( LANMessage *msg, UnsignedInt senderIP );
	void handleRequestGameLeave( LANMessage *msg, UnsignedInt senderIP );
	void handleRequestLobbyLeave( LANMessage *msg, UnsignedInt senderIP );
	void handleSetAccept( LANMessage *msg, UnsignedInt senderIP );
	void handleHasMap( LANMessage *msg, UnsignedInt senderIP );
	void handleChat( LANMessage *msg, UnsignedInt senderIP );
	void handleGameStart( LANMessage *msg, UnsignedInt senderIP );
	void handleGameStartTimer( LANMessage *msg, UnsignedInt senderIP );
	void handleGameOptions( LANMessage *msg, UnsignedInt senderIP );
	void handleInActive( LANMessage *msg, UnsignedInt senderIP );

};



/**
 * LAN message class
 */
#pragma pack(push, 1)
struct LANMessage
{
	enum Type				          ///< What kind of message are we?
	{
		// Locating everybody
		MSG_REQUEST_LOCATIONS,	///< Hey, where is everybody?
		MSG_GAME_ANNOUNCE,			///< Here I am, and here's my game info!
		MSG_LOBBY_ANNOUNCE,			///< Hey, I'm in the lobby!

		// Joining games
		MSG_REQUEST_JOIN,				///< Let me in!  Let me in!
		MSG_JOIN_ACCEPT,				///< Okay, you can join.
		MSG_JOIN_DENY,					///< Go away!  We don't want any!

		// Leaving games
		MSG_REQUEST_GAME_LEAVE,	///< I want to leave the game
		MSG_REQUEST_LOBBY_LEAVE,///< I'm leaving the lobby

		// Game options, chat, etc
		MSG_SET_ACCEPT,					///< I'm cool with everything as is.
		MSG_MAP_AVAILABILITY,		///< I do (not) have the map.
		MSG_CHAT,								///< Just spouting my mouth off.
		MSG_GAME_START,					///< Hold on; we're starting!
		MSG_GAME_START_TIMER,		///< The game will start in N seconds
		MSG_GAME_OPTIONS,				///< Here's some info about the game.
		MSG_INACTIVE,						///< I've alt-tabbed out.  Unaccept me cause I'm a poo-flinging monkey.

		MSG_REQUEST_GAME_INFO,	///< For direct connect, get the game info from a specific IP Address
	} messageType;

	WideChar name[g_lanPlayerNameLength+1]; ///< My name, for convenience
	char userName[g_lanLoginNameLength+1];	///< login name, for convenience
	char hostName[g_lanHostNameLength+1];		///< machine name, for convenience

	// No additional data is required for REQUEST_LOCATIONS, LOBBY_ANNOUNCE,
	// REQUEST_LOBBY_LEAVE, GAME_START.
	union
	{
		// StartTimer is sent with GAME_START_TIMER
		struct
		{
			Int seconds;
		} StartTimer;

		// GameJoined is sent with REQUEST_GAME_LEAVE
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
		} GameToLeave;

		// GameInfo if sent with GAME_ANNOUNCE
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			Bool inProgress;
			char options[m_lanMaxOptionsLength+1];
			Bool isDirectConnect;
		} GameInfo;

		// PlayerInfo is sent with REQUEST_GAME_INFO for direct connect games.
		struct
		{
			UnsignedInt ip;
			WideChar playerName[g_lanPlayerNameLength+1];
		} PlayerInfo;

		// GameToJoin is sent with REQUEST_JOIN
		struct
		{
			UnsignedInt gameIP;
			UnsignedInt exeCRC;
			UnsignedInt iniCRC;
			char serial[g_maxSerialLength];
		} GameToJoin;

		// GameJoined is sent with JOIN_ACCEPT
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			UnsignedInt gameIP;
			UnsignedInt playerIP;
			Int slotPosition;
		} GameJoined;

		// GameNotJoined is sent with JOIN_DENY
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			UnsignedInt gameIP;
			UnsignedInt playerIP;
			LANAPIInterface::ReturnType reason;
		} GameNotJoined;

		// Accept is sent with SET_ACCEPT
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			Bool isAccepted;
		} Accept;

		// Accept is sent with MAP_AVAILABILITY
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			UnsignedInt mapCRC;	// to make sure we're talking about the same map
			Bool hasMap;
		} MapStatus;

		// Chat is sent with CHAT
		struct
		{
			WideChar gameName[g_lanGameNameLength+1];
			LANAPIInterface::ChatType chatType;
			WideChar message[g_lanMaxChatLength+1];
		} Chat;

		// GameOptions is sent with GAME_OPTIONS
		struct
		{
			char options[m_lanMaxOptionsLength+1];
		} GameOptions;

	};
};
#pragma pack(pop)
