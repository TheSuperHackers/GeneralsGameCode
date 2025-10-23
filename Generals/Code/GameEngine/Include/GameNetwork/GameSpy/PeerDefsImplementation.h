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

// FILE: PeerDefsImplementation.h //////////////////////////////////////////////////////
// Generals GameSpy Peer (chat) implementation definitions
// Author: Matthew D. Campbell, Sept 2002

#pragma once

#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"


class GameSpyInfo : public GameSpyInfoInterface
{
public:
	GameSpyInfo();
	virtual ~GameSpyInfo();
	virtual void reset( void );
	virtual void clearGroupRoomList( void ) { m_groupRooms.clear(); m_gotGroupRoomList = false; }
	virtual GroupRoomMap* getGroupRoomList( void ) { return &m_groupRooms; }
	virtual void addGroupRoom( GameSpyGroupRoom room );
	virtual Bool gotGroupRoomList( void ) { return m_gotGroupRoomList; }
	virtual void joinGroupRoom( Int groupID );
	virtual void leaveGroupRoom( void );
	virtual void joinBestGroupRoom( void );
	virtual void setCurrentGroupRoom( Int groupID ) { m_currentGroupRoomID = groupID; m_playerInfoMap.clear(); }
	virtual Int  getCurrentGroupRoom( void ) { return m_currentGroupRoomID; }
	virtual void updatePlayerInfo( PlayerInfo pi, AsciiString oldNick = AsciiString::TheEmptyString );
	virtual void playerLeftGroupRoom( AsciiString nick );
	virtual PlayerInfoMap* getPlayerInfoMap( void ) { return &m_playerInfoMap; }

	virtual void setLocalName( AsciiString name ) { m_localName = name; }
	virtual AsciiString getLocalName( void ) { return m_localName; }
	virtual void setLocalProfileID( Int profileID ) { m_localProfileID = profileID; }
	virtual Int getLocalProfileID( void ) { return m_localProfileID; }
	virtual AsciiString getLocalEmail( void ) { return m_localEmail; }
	virtual void setLocalEmail( AsciiString email ) { m_localEmail = email;	}
	virtual AsciiString getLocalPassword( void ){ return m_localPasswd;	}
	virtual void setLocalPassword( AsciiString passwd ) { m_localPasswd = passwd;	}
	virtual void setLocalBaseName( AsciiString name ) { m_localBaseName = name; }
	virtual AsciiString getLocalBaseName( void ){ return m_localBaseName; }
	virtual void setCachedLocalPlayerStats( PSPlayerStats stats ) {m_cachedLocalPlayerStats = stats;	}
	virtual PSPlayerStats getCachedLocalPlayerStats( void ){ return m_cachedLocalPlayerStats;	}

	virtual BuddyInfoMap* getBuddyMap( void ) { return &m_buddyMap; }
	virtual BuddyInfoMap* getBuddyRequestMap( void ) { return &m_buddyRequestMap; }
	virtual BuddyMessageList* getBuddyMessages( void ) { return &m_buddyMessages; }
	virtual Bool isBuddy( Int id );

	virtual void clearStagingRoomList( void );
	virtual StagingRoomMap* getStagingRoomList( void ) { return &m_stagingRooms; }
	virtual GameSpyStagingRoom* findStagingRoomByID( Int id );
	virtual void addStagingRoom( GameSpyStagingRoom room );
	virtual void updateStagingRoom( GameSpyStagingRoom room );
	virtual void removeStagingRoom( GameSpyStagingRoom room );
	virtual Bool hasStagingRoomListChanged( void );
	virtual void leaveStagingRoom( void );
	virtual void markAsStagingRoomHost( void );
	virtual void markAsStagingRoomJoiner( Int game );
	virtual Int getCurrentStagingRoomID( void ) { return m_localStagingRoomID; }

	virtual void sawFullGameList( void ) { m_sawFullGameList = TRUE; }

	virtual void setDisallowAsianText( Bool val );
	virtual void setDisallowNonAsianText( Bool val );
	virtual Bool getDisallowAsianText( void );
	virtual Bool getDisallowNonAsianText(void );
	// chat
	virtual void registerTextWindow( GameWindow *win );
	virtual void unregisterTextWindow( GameWindow *win );
	virtual Int addText( UnicodeString message, Color c, GameWindow *win );
	virtual void addChat( PlayerInfo p, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win );
	virtual void addChat( AsciiString nick, Int profileID, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win );
	virtual Bool sendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox );

	virtual void setMOTD( const AsciiString& motd );
	virtual const AsciiString& getMOTD( void );
	virtual void setConfig( const AsciiString& config );
	virtual const AsciiString& getConfig( void );

	virtual void setPingString( const AsciiString& ping ) { m_pingString = ping; }
	virtual const AsciiString& getPingString( void ) { return m_pingString; }
	virtual Int getPingValue( const AsciiString& otherPing );

	virtual Bool amIHost( void );
	virtual GameSpyStagingRoom* getCurrentStagingRoom( void );
	virtual void setGameOptions( void );

	virtual void addToIgnoreList( AsciiString nick );
	virtual void removeFromIgnoreList( AsciiString nick );
	virtual Bool isIgnored( AsciiString nick );
	virtual IgnoreList returnIgnoreList( void );

	virtual void loadSavedIgnoreList( void );
	virtual SavedIgnoreMap returnSavedIgnoreList( void );
	virtual void addToSavedIgnoreList( Int profileID, AsciiString nick);
	virtual void removeFromSavedIgnoreList( Int profileID );
	virtual Bool isSavedIgnored( Int profileID );
	virtual void setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP);
	virtual UnsignedInt getInternalIP(void) { return m_internalIP; }
	virtual UnsignedInt getExternalIP(void) { return m_externalIP; }

	virtual Bool isDisconnectedAfterGameStart(Int *reason) const { if (reason) *reason = m_disconReason; return m_isDisconAfterGameStart; }
	virtual void markAsDisconnectedAfterGameStart(Int reason) { m_isDisconAfterGameStart = TRUE; m_disconReason = reason; }

	virtual Bool didPlayerPreorder( Int profileID ) const;
	virtual void markPlayerAsPreorder( Int profileID );

	virtual void setMaxMessagesPerUpdate( Int num );
	virtual Int getMaxMessagesPerUpdate( void );

	virtual Int getAdditionalDisconnects( void );
	virtual void clearAdditionalDisconnects( void );
	virtual void readAdditionalDisconnects( void );
	virtual void updateAdditionalGameSpyDisconnections(Int count);
private:
	Bool m_sawFullGameList;
	Bool m_isDisconAfterGameStart;
	Int m_disconReason;
	AsciiString m_rawMotd;
	AsciiString m_rawConfig;
	AsciiString m_pingString;
	GroupRoomMap m_groupRooms;
	StagingRoomMap m_stagingRooms;
	Bool m_stagingRoomsDirty;
	BuddyInfoMap m_buddyMap;
	BuddyInfoMap m_buddyRequestMap;
	PlayerInfoMap m_playerInfoMap;
	BuddyMessageList m_buddyMessages;
	Int m_currentGroupRoomID;
	Bool m_gotGroupRoomList;
	AsciiString m_localName;
	Int m_localProfileID;
	AsciiString m_localPasswd;
	AsciiString m_localEmail;
	AsciiString m_localBaseName;
	PSPlayerStats m_cachedLocalPlayerStats;
	Bool m_disallowAsainText;
	Bool m_disallowNonAsianText;
	UnsignedInt m_internalIP, m_externalIP;
	Int m_maxMessagesPerUpdate;

	Int m_joinedStagingRoom;								// if we join a staging room, this holds its ID (0 otherwise)
	Bool m_isHosting;												// if we host, this is true, and
	GameSpyStagingRoom m_localStagingRoom;	// this holds the GameInfo for it.
	Int m_localStagingRoomID;

	IgnoreList m_ignoreList;
	SavedIgnoreMap m_savedIgnoreMap;

	std::set<GameWindow *> m_textWindows;

	std::set<Int> m_preorderPlayers;
	Int m_additionalDisconnects;
};
