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

// FILE: PeerDefs.h //////////////////////////////////////////////////////
// Generals GameSpy Peer (chat) definitions
// Author: Matthew D. Campbell, June 2002

#pragma once

#include "gamespy/peer/peer.h"
#include "gamespy/gp/gp.h"

#include "GameClient/Color.h"
#include "Common/STLTypedefs.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"

class GameWindow;
class PSPlayerStats;

typedef std::set<AsciiString> IgnoreList;
typedef std::map<Int, AsciiString> SavedIgnoreMap;

enum RCItemType CPP_11(: Int)
{
	ITEM_BUDDY,
	ITEM_REQUEST,
	ITEM_NONBUDDY,
	ITEM_NONE,
};

class GameSpyRCMenuData
{
public:
	AsciiString m_nick;
	GPProfile m_id;
	RCItemType m_itemType;
};

class BuddyInfo
{
public:
	GPProfile m_id;
	AsciiString m_name;
	AsciiString m_email;
	AsciiString m_countryCode;
	GPEnum m_status;
	UnicodeString m_statusString;
	UnicodeString m_locationString;
};
typedef std::map<GPProfile, BuddyInfo> BuddyInfoMap;

class BuddyMessage
{
public:
	UnsignedInt m_timestamp;
	GPProfile m_senderID;
	AsciiString m_senderNick;
	GPProfile m_recipientID;
	AsciiString m_recipientNick;
	UnicodeString m_message;
};
typedef std::list<BuddyMessage> BuddyMessageList;

class GameSpyGroupRoom
{
public:
	GameSpyGroupRoom() { m_name = AsciiString::TheEmptyString; m_translatedName = UnicodeString::TheEmptyString; m_groupID = m_numWaiting = m_maxWaiting = m_numGames = m_numPlaying = 0; }
	AsciiString m_name;
	UnicodeString m_translatedName;
	Int m_groupID;
	Int m_numWaiting;
	Int m_maxWaiting;
	Int m_numGames;
	Int m_numPlaying;
};
typedef std::map<Int, GameSpyGroupRoom> GroupRoomMap;

class Transport;
class NAT;

typedef std::map<Int, GameSpyStagingRoom *> StagingRoomMap;

class PlayerInfo
{
public:
	PlayerInfo() { m_name = m_locale = AsciiString::TheEmptyString; m_wins = m_losses = m_rankPoints = m_side = m_preorder = m_profileID = m_flags = 0; }
	AsciiString m_name;
	AsciiString m_locale;
	Int m_wins;
	Int m_losses;
	Int m_profileID;
	Int m_flags;
	Int m_rankPoints;
	Int m_side;
	Int m_preorder;
	Bool isIgnored( void );
};
struct AsciiComparator
{
	bool operator()(AsciiString s1, AsciiString s2) const;
};


typedef std::map<AsciiString, PlayerInfo, AsciiComparator> PlayerInfoMap;

enum GameSpyColors CPP_11(: Int) {
	GSCOLOR_DEFAULT = 0,
	GSCOLOR_CURRENTROOM,
	GSCOLOR_ROOM,
	GSCOLOR_GAME,
	GSCOLOR_GAME_FULL,
	GSCOLOR_GAME_CRCMISMATCH,
	GSCOLOR_PLAYER_NORMAL,
	GSCOLOR_PLAYER_OWNER,
	GSCOLOR_PLAYER_BUDDY,
	GSCOLOR_PLAYER_SELF,
	GSCOLOR_PLAYER_IGNORED,
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
	GSCOLOR_MAP_SELECTED,
	GSCOLOR_MAP_UNSELECTED,
	GSCOLOR_MOTD,
	GSCOLOR_MOTD_HEADING,
	GSCOLOR_MAX
};

extern Color GameSpyColor[GSCOLOR_MAX];

enum GameSpyBuddyStatus CPP_11(: Int) {
	BUDDY_OFFLINE,
	BUDDY_ONLINE,
	BUDDY_LOBBY,
	BUDDY_STAGING,
	BUDDY_LOADING,
	BUDDY_PLAYING,
	BUDDY_MATCHING,
};

// ---------------------------------------------------
// this class holds info used in the main thread
class GameSpyInfoInterface
{
public:
	virtual ~GameSpyInfoInterface() {};
	virtual void reset( void ) {};
	virtual void clearGroupRoomList( void ) = 0;
	virtual GroupRoomMap* getGroupRoomList( void ) = 0;
	virtual void addGroupRoom( GameSpyGroupRoom room ) = 0;
	virtual Bool gotGroupRoomList( void ) = 0;
	virtual void joinGroupRoom( Int groupID ) = 0;
	virtual void leaveGroupRoom( void ) = 0;
	virtual void joinBestGroupRoom( void ) = 0;
	virtual void setCurrentGroupRoom( Int groupID ) = 0;
	virtual Int  getCurrentGroupRoom( void ) = 0;
	virtual void updatePlayerInfo( PlayerInfo pi, AsciiString oldNick = AsciiString::TheEmptyString ) = 0;
	virtual void playerLeftGroupRoom( AsciiString nick ) = 0;
	virtual PlayerInfoMap* getPlayerInfoMap( void ) = 0;

	virtual BuddyInfoMap* getBuddyMap( void ) = 0;
	virtual BuddyInfoMap* getBuddyRequestMap( void ) = 0;
	virtual BuddyMessageList* getBuddyMessages( void ) = 0;
	virtual Bool isBuddy( Int id ) = 0;

	virtual void setLocalName( AsciiString name ) = 0;
	virtual AsciiString getLocalName( void ) = 0;
	virtual void setLocalProfileID( Int profileID ) = 0;
	virtual Int getLocalProfileID( void ) = 0;
	virtual AsciiString getLocalEmail( void ) = 0;
	virtual void setLocalEmail( AsciiString email ) = 0;
	virtual AsciiString getLocalPassword( void ) = 0;
	virtual void setLocalPassword( AsciiString passwd ) = 0;
	virtual void setLocalBaseName( AsciiString name ) = 0;
	virtual AsciiString getLocalBaseName( void ) = 0;

	virtual void setCachedLocalPlayerStats( PSPlayerStats stats ) = 0;
	virtual PSPlayerStats getCachedLocalPlayerStats( void ) = 0;

	virtual void clearStagingRoomList( void ) = 0;
	virtual StagingRoomMap* getStagingRoomList( void ) = 0;
	virtual GameSpyStagingRoom* findStagingRoomByID( Int id ) = 0;
	virtual void addStagingRoom( GameSpyStagingRoom room ) = 0;
	virtual void updateStagingRoom( GameSpyStagingRoom room ) = 0;
	virtual void removeStagingRoom( GameSpyStagingRoom room ) = 0;
	virtual Bool hasStagingRoomListChanged( void ) = 0;
	virtual void leaveStagingRoom( void ) = 0;
	virtual void markAsStagingRoomHost( void ) = 0;
	virtual void markAsStagingRoomJoiner( Int game ) = 0;
	virtual void sawFullGameList( void ) = 0;

	virtual Bool amIHost( void ) = 0;
	virtual GameSpyStagingRoom* getCurrentStagingRoom( void ) = 0;
	virtual void setGameOptions( void ) = 0;
	virtual Int getCurrentStagingRoomID( void ) = 0;

	virtual void setDisallowAsianText( Bool val ) = 0;
	virtual void setDisallowNonAsianText( Bool val ) = 0;
	virtual Bool getDisallowAsianText( void ) = 0;
	virtual Bool getDisallowNonAsianText(void ) = 0;

	// chat
	virtual void registerTextWindow( GameWindow *win ) = 0;
	virtual void unregisterTextWindow( GameWindow *win ) = 0;
	virtual Int addText( UnicodeString message, Color c, GameWindow *win ) = 0;
	virtual void addChat( PlayerInfo p, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win ) = 0;
	virtual void addChat( AsciiString nick, Int profileID, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win ) = 0;
	virtual Bool sendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox ) = 0;

	virtual void setMOTD( const AsciiString& motd ) = 0;
	virtual const AsciiString& getMOTD( void ) = 0;

	virtual void setConfig( const AsciiString& config ) = 0;
	virtual const AsciiString& getConfig( void ) = 0;

	virtual void setPingString( const AsciiString& ping ) = 0;
	virtual const AsciiString& getPingString( void ) = 0;
	virtual Int getPingValue( const AsciiString& otherPing ) = 0;

	static GameSpyInfoInterface* createNewGameSpyInfoInterface( void );

	virtual void addToSavedIgnoreList( Int profileID, AsciiString nick ) = 0;
	virtual void removeFromSavedIgnoreList( Int profileID ) = 0;
	virtual Bool isSavedIgnored( Int profileID ) = 0;
	virtual SavedIgnoreMap returnSavedIgnoreList( void ) = 0;
	virtual void loadSavedIgnoreList( void ) = 0;

	virtual IgnoreList returnIgnoreList( void ) = 0;
	virtual void addToIgnoreList( AsciiString nick ) = 0;
	virtual void removeFromIgnoreList( AsciiString nick ) = 0;
	virtual Bool isIgnored( AsciiString nick ) = 0;

	virtual void setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP) = 0;
	virtual UnsignedInt getInternalIP(void) = 0;
	virtual UnsignedInt getExternalIP(void) = 0;

	virtual Bool isDisconnectedAfterGameStart(Int *reason) const = 0;
	virtual void markAsDisconnectedAfterGameStart(Int reason) = 0;

	virtual Bool didPlayerPreorder( Int profileID ) const = 0;
	virtual void markPlayerAsPreorder( Int profileID ) = 0;

	virtual void setMaxMessagesPerUpdate( Int num ) = 0;
	virtual Int getMaxMessagesPerUpdate( void ) = 0;

	virtual Int getAdditionalDisconnects( void ) = 0;
	virtual void clearAdditionalDisconnects( void ) = 0;
	virtual void readAdditionalDisconnects( void ) = 0;
	virtual void updateAdditionalGameSpyDisconnections(Int count) = 0;
};

extern GameSpyInfoInterface *TheGameSpyInfo;

void WOLDisplayGameOptions( void );
void WOLDisplaySlotList( void );
Bool GetLocalChatConnectionAddress(AsciiString serverName, UnsignedShort serverPort, UnsignedInt& localIP);
void SetLobbyAttemptHostJoin(Bool start);
void SendStatsToOtherPlayers(const GameInfo *game);

class PSPlayerStats;
void GetAdditionalDisconnectsFromUserFile(PSPlayerStats *stats);
extern Int GetAdditionalDisconnectsFromUserFile(Int playerID);

//-------------------------------------------------------------------------
// These functions set up the globals and threads neccessary for our GameSpy impl.

void SetUpGameSpy( const char *motdBuffer, const char *configBuffer );
void TearDownGameSpy( void );
