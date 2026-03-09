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
	virtual ~GameSpyInfo() override;
	virtual void reset() override;
	virtual void clearGroupRoomList() override { m_groupRooms.clear(); m_gotGroupRoomList = false; }
	virtual GroupRoomMap* getGroupRoomList() override { return &m_groupRooms; }
	virtual void addGroupRoom( GameSpyGroupRoom room ) override;
	virtual Bool gotGroupRoomList() override { return m_gotGroupRoomList; }
	virtual void joinGroupRoom( Int groupID ) override;
	virtual void leaveGroupRoom() override;
	virtual void joinBestGroupRoom() override;
	virtual void setCurrentGroupRoom( Int groupID ) override { m_currentGroupRoomID = groupID; m_playerInfoMap.clear(); }
	virtual Int  getCurrentGroupRoom() override { return m_currentGroupRoomID; }
	virtual void updatePlayerInfo( PlayerInfo pi, AsciiString oldNick = AsciiString::TheEmptyString ) override;
	virtual void playerLeftGroupRoom( AsciiString nick ) override;
	virtual PlayerInfoMap* getPlayerInfoMap() override { return &m_playerInfoMap; }

	virtual void setLocalName( AsciiString name ) override { m_localName = name; }
	virtual AsciiString getLocalName() override { return m_localName; }
	virtual void setLocalProfileID( Int profileID ) override { m_localProfileID = profileID; }
	virtual Int getLocalProfileID() override { return m_localProfileID; }
	virtual AsciiString getLocalEmail() override { return m_localEmail; }
	virtual void setLocalEmail( AsciiString email ) override { m_localEmail = email;	}
	virtual AsciiString getLocalPassword() override { return m_localPasswd;	}
	virtual void setLocalPassword( AsciiString passwd ) override { m_localPasswd = passwd;	}
	virtual void setLocalBaseName( AsciiString name ) override { m_localBaseName = name; }
	virtual AsciiString getLocalBaseName() override { return m_localBaseName; }
	virtual void setCachedLocalPlayerStats( PSPlayerStats stats ) override {m_cachedLocalPlayerStats = stats;	}
	virtual PSPlayerStats getCachedLocalPlayerStats() override { return m_cachedLocalPlayerStats;	}

	virtual BuddyInfoMap* getBuddyMap() override { return &m_buddyMap; }
	virtual BuddyInfoMap* getBuddyRequestMap() override { return &m_buddyRequestMap; }
	virtual BuddyMessageList* getBuddyMessages() override { return &m_buddyMessages; }
	virtual Bool isBuddy( Int id ) override;

	virtual void clearStagingRoomList() override;
	virtual StagingRoomMap* getStagingRoomList() override { return &m_stagingRooms; }
	virtual GameSpyStagingRoom* findStagingRoomByID( Int id ) override;
	virtual void addStagingRoom( GameSpyStagingRoom room ) override;
	virtual void updateStagingRoom( GameSpyStagingRoom room ) override;
	virtual void removeStagingRoom( GameSpyStagingRoom room ) override;
	virtual Bool hasStagingRoomListChanged() override;
	virtual void leaveStagingRoom() override;
	virtual void markAsStagingRoomHost() override;
	virtual void markAsStagingRoomJoiner( Int game ) override;
	virtual Int getCurrentStagingRoomID() override { return m_localStagingRoomID; }

	virtual void sawFullGameList() override { m_sawFullGameList = TRUE; }

	virtual void setDisallowAsianText( Bool val ) override;
	virtual void setDisallowNonAsianText( Bool val ) override;
	virtual Bool getDisallowAsianText() override;
	virtual Bool getDisallowNonAsianText() override;
	// chat
	virtual void registerTextWindow( GameWindow *win ) override;
	virtual void unregisterTextWindow( GameWindow *win ) override;
	virtual Int addText( UnicodeString message, Color c, GameWindow *win ) override;
	virtual void addChat( PlayerInfo p, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win ) override;
	virtual void addChat( AsciiString nick, Int profileID, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win ) override;
	virtual Bool sendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox ) override;

	virtual void setMOTD( const AsciiString& motd ) override;
	virtual const AsciiString& getMOTD() override;
	virtual void setConfig( const AsciiString& config ) override;
	virtual const AsciiString& getConfig() override;

	virtual void setPingString( const AsciiString& ping ) override { m_pingString = ping; }
	virtual const AsciiString& getPingString() override { return m_pingString; }
	virtual Int getPingValue( const AsciiString& otherPing ) override;

	virtual Bool amIHost() override;
	virtual GameSpyStagingRoom* getCurrentStagingRoom() override;
	virtual void setGameOptions() override;

	virtual void addToIgnoreList( AsciiString nick ) override;
	virtual void removeFromIgnoreList( AsciiString nick ) override;
	virtual Bool isIgnored( AsciiString nick ) override;
	virtual IgnoreList returnIgnoreList() override;

	virtual void loadSavedIgnoreList() override;
	virtual SavedIgnoreMap returnSavedIgnoreList() override;
	virtual void addToSavedIgnoreList( Int profileID, AsciiString nick) override;
	virtual void removeFromSavedIgnoreList( Int profileID ) override;
	virtual Bool isSavedIgnored( Int profileID ) override;
	virtual void setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP) override;
	virtual UnsignedInt getInternalIP() override { return m_internalIP; }
	virtual UnsignedInt getExternalIP() override { return m_externalIP; }

	virtual Bool isDisconnectedAfterGameStart(Int *reason) const override { if (reason) *reason = m_disconReason; return m_isDisconAfterGameStart; }
	virtual void markAsDisconnectedAfterGameStart(Int reason) override { m_isDisconAfterGameStart = TRUE; m_disconReason = reason; }

	virtual Bool didPlayerPreorder( Int profileID ) const override;
	virtual void markPlayerAsPreorder( Int profileID ) override;

	virtual void setMaxMessagesPerUpdate( Int num ) override;
	virtual Int getMaxMessagesPerUpdate() override;

	virtual Int getAdditionalDisconnects() override;
	virtual void clearAdditionalDisconnects() override;
	virtual void readAdditionalDisconnects() override;
	virtual void updateAdditionalGameSpyDisconnections(Int count) override;
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
