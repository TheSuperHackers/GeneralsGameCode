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

// FILE: PeerDefs.cpp //////////////////////////////////////////////////////
// Generals GameSpy Peer (chat) definitions
// Author: Matthew D. Campbell, June 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include <set>

#include "Common/GameState.h"
#include "Common/RandomValue.h"
#include "Common/IgnorePreferences.h"
#include "Common/CustomMatchPreferences.h"
#include "Common/GameSpyMiscPreferences.h"
#include "Common/Recorder.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "GameClient/MapUtil.h"
#include "GameClient/ShellHooks.h"
#include "GameClient/GameText.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#include "GameNetwork/GameSpy/PeerDefsImplementation.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PingThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/RankPointValue.h"
#include "GameLogic/GameLogic.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

GameSpyInfoInterface *TheGameSpyInfo = NULL;
extern GameSpyStagingRoom *TheGameSpyGame = NULL;
void deleteNotificationBox( void );

bool AsciiComparator::operator()(AsciiString s1, AsciiString s2) const
{
	return stricmp(s1.str(), s2.str()) < 0;
}

GameSpyInfo::GameSpyInfo()
{
	reset();
	TheGameSpyGame = &m_localStagingRoom;
	m_isDisconAfterGameStart = FALSE;
}

GameSpyInfo::~GameSpyInfo()
{
	TheGameSpyGame = NULL;
	reset();
}

void GameSpyInfo::reset( void )
{
	m_sawFullGameList = FALSE;
	m_isDisconAfterGameStart = FALSE;
	m_currentGroupRoomID = 0;
	clearGroupRoomList();
	clearStagingRoomList();
	m_localStagingRoomID = 0;
	m_buddyRequestMap.clear();
	m_buddyMap.clear();
	m_buddyMessages.clear();
	m_joinedStagingRoom = 0;
	m_isHosting = false;
	m_localStagingRoomID = 0;
	m_localStagingRoom.reset();
	m_gotGroupRoomList = false;
	m_localName = "";
	m_localProfileID = 0;
	m_maxMessagesPerUpdate = 100;
	
	// Added By Sadullah Nader
	// Initialization missing and needed
	m_disallowAsainText = FALSE;
	m_disallowNonAsianText = FALSE;
	m_disconReason = 0;
	m_localBaseName.clear();
	m_localEmail.clear();
	m_localPasswd.clear();
	m_pingString.clear();
	m_rawConfig.clear();
	m_rawMotd.clear();
	//

	m_internalIP = m_externalIP = 0;

	m_savedIgnoreMap.clear();
	m_preorderPlayers.clear();

	m_cachedLocalPlayerStats.reset();
	
	m_additionalDisconnects = -1;
}

Bool GameSpyInfo::didPlayerPreorder( Int profileID ) const
{
	std::set<Int>::const_iterator it = m_preorderPlayers.find(profileID);
	return (it != m_preorderPlayers.end());
}

void GameSpyInfo::markPlayerAsPreorder( Int profileID )
{
	m_preorderPlayers.insert(profileID);
}

void GameSpyInfo::setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP)
{
	m_internalIP = internalIP;
	m_externalIP = externalIP;
}

void GameSpyInfo::readAdditionalDisconnects( void )
{
	m_additionalDisconnects = GetAdditionalDisconnectsFromUserFile(m_localProfileID);
	DEBUG_LOG(("GameSpyInfo::readAdditionalDisconnects() found %d disconnects.\n", m_additionalDisconnects));
}

Int GameSpyInfo::getAdditionalDisconnects( void )
{
	DEBUG_LOG(("GameSpyInfo::getAdditionalDisconnects() would have returned %d.  Returning 0 instead.\n", m_additionalDisconnects));
	return 0;
}

void GameSpyInfo::clearAdditionalDisconnects( void )
{
	m_additionalDisconnects = 0;
}

GameSpyInfoInterface* GameSpyInfoInterface::createNewGameSpyInfoInterface( void )
{
	return NEW GameSpyInfo;
}

Bool GameSpyInfo::amIHost( void )
{
	return m_isHosting;
}

GameSpyStagingRoom* GameSpyInfo::getCurrentStagingRoom( void )
{
	if (m_isHosting || m_joinedStagingRoom)
		return &m_localStagingRoom;

	StagingRoomMap::iterator it = m_stagingRooms.find(m_joinedStagingRoom);
	if (it != m_stagingRooms.end())
		return it->second;

	return NULL;
}

void GameSpyInfo::setGameOptions( void )
{
	if (!m_isHosting)
		return;

	// set options for game lists, and UTM players in-game
	PeerRequest req;
	req.peerRequestType = PeerRequest::PEERREQUEST_SETGAMEOPTIONS;
	req.options = GameInfoToAsciiString(&m_localStagingRoom).str();

	Int i;
	AsciiString mapName = TheGameState->realMapPathToPortableMapPath(m_localStagingRoom.getMap());
	AsciiString newMapName;
	for (i=0; i<mapName.getLength(); ++i)
	{
		char c = mapName.getCharAt(i);
		if (c != '\\')
			newMapName.concat(c);
		else
			newMapName.concat('/');
	}
	req.gameOptsMapName = newMapName.str();

	req.gameOptions.numPlayers = 0;
	req.gameOptions.numObservers = 0;
	Int numOpenSlots = 0;
	AsciiString playerInfo = "";
	for (i=0; i<MAX_SLOTS; ++i)
	{
		Int wins = 0, losses = 0, profileID = 0;
		GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(i);
		req.gameOptsPlayerNames[i] = "";
		if (!slot->isOccupied())
		{
			if (slot->isOpen())
				++numOpenSlots;
		}
		else
		{
			AsciiString playerName;
			if (slot->isHuman())
			{
				playerName.translate(slot->getName());
				req.gameOptsPlayerNames[i] = playerName.str();
				PlayerInfoMap::iterator it = m_playerInfoMap.find(playerName);
				if (it != m_playerInfoMap.end())
				{
					wins = it->second.m_wins;
					losses = it->second.m_losses;
					profileID = it->second.m_profileID;
				}
				req.gameOptions.wins[req.gameOptions.numObservers+req.gameOptions.numPlayers] = wins;
				req.gameOptions.losses[req.gameOptions.numObservers+req.gameOptions.numPlayers] = losses;
				req.gameOptions.profileID[req.gameOptions.numObservers+req.gameOptions.numPlayers] = profileID;
				req.gameOptions.faction[req.gameOptions.numObservers+req.gameOptions.numPlayers] = slot->getPlayerTemplate();
				req.gameOptions.color[req.gameOptions.numObservers+req.gameOptions.numPlayers] = slot->getColor();
				if (slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
				{
					++req.gameOptions.numObservers;
				}
				else
				{
					++req.gameOptions.numPlayers;
				}
			}
			else if (slot->isAI())
			{
				// add in AI players
				switch (slot->getState())
				{
				case SLOT_EASY_AI:
					playerName = "CE";
					break;
				case SLOT_MED_AI:
					playerName = "CM";
					break;
				case SLOT_BRUTAL_AI:
					playerName = "CH";
					break;
				}
				req.gameOptsPlayerNames[i] = playerName.str(); // name is unused - we go off of the profileID
				req.gameOptions.wins[req.gameOptions.numObservers+req.gameOptions.numPlayers] = 0;
				req.gameOptions.losses[req.gameOptions.numObservers+req.gameOptions.numPlayers] = 0;
				req.gameOptions.profileID[req.gameOptions.numObservers+req.gameOptions.numPlayers] = slot->getState();
				req.gameOptions.faction[req.gameOptions.numObservers+req.gameOptions.numPlayers] = slot->getPlayerTemplate();
				req.gameOptions.color[req.gameOptions.numObservers+req.gameOptions.numPlayers] = slot->getColor();
				++req.gameOptions.numPlayers;
			}
		}
	}
	req.gameOptions.maxPlayers = numOpenSlots + req.gameOptions.numPlayers + req.gameOptions.numObservers;
	TheGameSpyPeerMessageQueue->addRequest(req);

	req.peerRequestType = PeerRequest::PEERREQUEST_UTMROOM;
	req.UTM.isStagingRoom = TRUE;
	req.id = "Pings/";
	AsciiString pings;
	for (i=0; i<MAX_SLOTS; ++i)
	{
		if (i!=0)
			pings.concat(",");

		GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(i);
		if (slot && slot->isHuman())
		{
			pings.concat(slot->getPingString());
		}
		else
		{
			pings.concat("0");
		}
	}
	req.options = pings.str();
	TheGameSpyPeerMessageQueue->addRequest(req);
}

Bool GameSpyInfo::isBuddy( Int id )
{
	return m_buddyMap.find(id) != m_buddyMap.end();
}

void GameSpyInfo::addGroupRoom( GameSpyGroupRoom room )
{
	if (room.m_groupID == 0)
	{
		m_gotGroupRoomList = TRUE;

		GroupRoomMap::iterator iter;

		// figure out how many good strings we've got
		std::vector<UnicodeString> names;
		Int numRooms = 0;
		for (iter = getGroupRoomList()->begin(); iter != getGroupRoomList()->end(); ++iter)
		{
			GameSpyGroupRoom room = iter->second;
			if (room.m_groupID != TheGameSpyConfig->getQMChannel())
			{
				++numRooms;

				AsciiString groupLabel;
				groupLabel.format("GUI:%s", room.m_name.str());

				Bool exists = FALSE;
				UnicodeString groupName = TheGameText->fetch(groupLabel, &exists);
				if (exists)
				{
					names.push_back(groupName);
				}
			}
		}

		if (!names.empty() && names.size() != numRooms)
		{
			// didn't get all names.  fix up
			Int nameIndex = 0;
			Int timesThrough = 1; // start with USA Lobby 1
			for (iter = TheGameSpyInfo->getGroupRoomList()->begin(); iter != TheGameSpyInfo->getGroupRoomList()->end(); ++iter)
			{
				GameSpyGroupRoom room = iter->second;
				if (room.m_groupID != TheGameSpyConfig->getQMChannel())
				{
					room.m_translatedName.format(L"%ls %d", names[nameIndex].str(), timesThrough);
					nameIndex = (nameIndex+1)%names.size();
					m_groupRooms[room.m_groupID] = room;
					if (!nameIndex)
					{
						// we've looped through the name list already.  increment the timesThrough counter
						++timesThrough;
					}
				}
			}
		}
	}
	else
	{
		DEBUG_LOG(("Adding group room %d (%s)\n", room.m_groupID, room.m_name.str()));
		AsciiString groupLabel;
		groupLabel.format("GUI:%s", room.m_name.str());
		room.m_translatedName = TheGameText->fetch(groupLabel);
		m_groupRooms[room.m_groupID] = room;
		if ( !stricmp("quickmatch", room.m_name.str()) )
		{
			DEBUG_LOG(("Group room %d (%s) is the QuickMatch room\n", room.m_groupID, room.m_name.str()));
			TheGameSpyConfig->setQMChannel(room.m_groupID);
		}
	}
}

void GameSpyInfo::joinGroupRoom( Int groupID )
{
	if (groupID > 0)
	{
		PeerRequest req;
		req.peerRequestType = PeerRequest::PEERREQUEST_JOINGROUPROOM;
		req.groupRoom.id = groupID;
		TheGameSpyPeerMessageQueue->addRequest(req);
		m_playerInfoMap.clear();
	}
}

void GameSpyInfo::leaveGroupRoom( void )
{
	PeerRequest req;
	req.peerRequestType = PeerRequest::PEERREQUEST_LEAVEGROUPROOM;
	TheGameSpyPeerMessageQueue->addRequest(req);
	setCurrentGroupRoom(0);
	m_playerInfoMap.clear();
}

void GameSpyInfo::joinBestGroupRoom( void )
{
	if (m_currentGroupRoomID)
	{
		DEBUG_LOG(("Bailing from GameSpyInfo::joinBestGroupRoom() - we were already in a room\n"));
		m_currentGroupRoomID = 0;
		return;
	}

	if (m_groupRooms.size())
	{
		int minID = -1;
		int minPlayers = 1000;
		GroupRoomMap::iterator iter = m_groupRooms.begin();
		while (iter != m_groupRooms.end())
		{
			GameSpyGroupRoom room = iter->second;
			DEBUG_LOG(("Group room %d: %s (%d, %d, %d, %d)\n", room.m_groupID, room.m_name.str(), room.m_numWaiting, room.m_maxWaiting,
				room.m_numGames, room.m_numPlaying));

			if (TheGameSpyConfig->getQMChannel() != room.m_groupID && minPlayers > 25 && room.m_numWaiting < minPlayers)
			{
				minID = room.m_groupID;
				minPlayers = room.m_numWaiting;
			}

			++iter;
		}

		if (minID > 0)
		{
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_JOINGROUPROOM;
			req.groupRoom.id = minID;
			TheGameSpyPeerMessageQueue->addRequest(req);
			m_playerInfoMap.clear();
		}
		else
		{
			GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSGroupRoomJoinFail"), NULL);
		}
	}
	else
	{
		GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSGroupRoomJoinFail"), NULL);
	}
}

void GameSpyInfo::updatePlayerInfo( PlayerInfo pi, AsciiString oldNick )
{
	if (!oldNick.isEmpty())
		playerLeftGroupRoom(oldNick);

	m_playerInfoMap[pi.m_name] = pi;

	if (pi.m_preorder != 0)
		markPlayerAsPreorder(pi.m_profileID);
}

void GameSpyInfo::playerLeftGroupRoom( AsciiString nick )
{
	PlayerInfoMap::iterator it = m_playerInfoMap.find(nick);
	if (it != m_playerInfoMap.end())
	{
		m_playerInfoMap.erase(it);
	}
}

void GameSpyInfo::clearStagingRoomList( void )
{
	Int numRoomsRemoved = 0;
	m_sawFullGameList = FALSE;
	m_stagingRoomsDirty = FALSE;
	
	StagingRoomMap::iterator it = m_stagingRooms.begin();
	while (it != m_stagingRooms.end())
	{
		++numRoomsRemoved;

		delete it->second;
		m_stagingRooms.erase(it);
		it = m_stagingRooms.begin();
	}
	if (numRoomsRemoved > 0)
	{
		//m_stagingRoomsDirty = true; // only consider ourselves dirty if we actually removed some games.
	}
}

void GameSpyInfo::addStagingRoom( GameSpyStagingRoom room )
{
	removeStagingRoom(room);
	GameSpyStagingRoom *newRoom = NEW GameSpyStagingRoom;
	*newRoom = room;
	newRoom->cleanUpSlotPointers();
	m_stagingRooms[room.getID()] = newRoom;
	m_stagingRoomsDirty = m_sawFullGameList;
}

void GameSpyInfo::updateStagingRoom( GameSpyStagingRoom room )
{
	addStagingRoom(room);
}

void GameSpyInfo::removeStagingRoom( GameSpyStagingRoom room )
{
	StagingRoomMap::iterator it = m_stagingRooms.find(room.getID());
	if (it != m_stagingRooms.end())
	{
		delete it->second;
		m_stagingRooms.erase(it);

		m_stagingRoomsDirty = m_sawFullGameList;
	}
}

Bool GameSpyInfo::hasStagingRoomListChanged( void )
{
	Bool val = m_stagingRoomsDirty;
	m_stagingRoomsDirty = false;
	return val;
}

GameSpyStagingRoom* GameSpyInfo::findStagingRoomByID( Int id )
{
	StagingRoomMap::iterator it = m_stagingRooms.find(id);
	if (it != m_stagingRooms.end())
		return it->second;

	return NULL;
}

void GameSpyInfo::leaveStagingRoom( void )
{
	m_localStagingRoomID = 0;
	PeerRequest req;
	req.peerRequestType = PeerRequest::PEERREQUEST_LEAVESTAGINGROOM;
	TheGameSpyPeerMessageQueue->addRequest(req);
	m_playerInfoMap.clear();
	m_joinedStagingRoom = FALSE;
	m_isHosting = FALSE;
}

void GameSpyInfo::markAsStagingRoomHost( void )
{
	m_localStagingRoomID = 0;
	m_joinedStagingRoom = FALSE; m_isHosting = TRUE;
  
  // There are a few options we don't want to reset when we are hosting (they carry over
  // from the the create game dialog).
  // Interesting fact: oldFactionsOnly will be carried over correctly if I remove these
  // lines. UseStats won't be. I have no idea why.
  Int useStats = m_localStagingRoom.getUseStats();
  Bool oldFactionsOnly = m_localStagingRoom.oldFactionsOnly();

  m_localStagingRoom.reset();
	m_localStagingRoom.enterGame();
	m_localStagingRoom.setSeed(GetTickCount());
  
  m_localStagingRoom.setUseStats( useStats );
  m_localStagingRoom.setOldFactionsOnly( oldFactionsOnly );

	GameSlot newSlot;
	UnicodeString uName;
	uName.translate(m_localName);
	newSlot.setState(SLOT_PLAYER, uName);

	m_localStagingRoom.setLocalIP(m_externalIP);
	newSlot.setIP(m_externalIP);

	m_localStagingRoom.setSlot(0,newSlot);
	m_localStagingRoom.setLocalName(m_localName);

	TheMapCache->updateCache();
	m_localStagingRoom.setMap(getDefaultMap(TRUE));
	m_localStagingRoom.adjustSlotsForMap(); // close slots that the map can't hold. BGC
}

void GameSpyInfo::markAsStagingRoomJoiner( Int game )
{
	m_localStagingRoomID = game;
	m_joinedStagingRoom = TRUE; m_isHosting = FALSE;
	m_localStagingRoom.reset();
	m_localStagingRoom.enterGame();
	StagingRoomMap::iterator it = m_stagingRooms.find(game);
	if (it != m_stagingRooms.end())
	{
		GameSpyStagingRoom *info = it->second;
		info->cleanUpSlotPointers();
		AsciiString options = GameInfoToAsciiString(info);
#ifdef DEBUG_CRASHING
		Bool res =
#endif
		ParseAsciiStringToGameInfo(&m_localStagingRoom, options);
		DEBUG_ASSERTCRASH(res, ("Could not parse game info \"%s\"", options.str()));
		m_localStagingRoom.setInGame();
		m_localStagingRoom.setLocalName(m_localName);
		m_localStagingRoom.setExeCRC(info->getExeCRC());
		m_localStagingRoom.setIniCRC(info->getIniCRC());
		m_localStagingRoom.setAllowObservers(info->getAllowObservers());
		m_localStagingRoom.setHasPassword(info->getHasPassword());
		m_localStagingRoom.setGameName(info->getGameName());
		DEBUG_LOG(("Joining game: host is %ls\n", m_localStagingRoom.getConstSlot(0)->getName().str()));
	}
}

void GameSpyInfo::setMOTD( const AsciiString& motd )
{
	m_rawMotd = motd;
}

const AsciiString& GameSpyInfo::getMOTD( void )
{
	return m_rawMotd;
}

void GameSpyInfo::setConfig( const AsciiString& config )
{
	m_rawConfig = config;
}

const AsciiString& GameSpyInfo::getConfig( void )
{
	return m_rawConfig;
}

// --------------------------------------------------------------
void SetUpGameSpy( const char *motdBuffer, const char *configBuffer )
{
	if (!motdBuffer)
		motdBuffer = "";
	if (!configBuffer)
		configBuffer = "";
	TearDownGameSpy();

	AsciiString dir = TheGlobalData->getPath_UserData();
	CreateDirectory(dir.str(), NULL);
	dir.format("%sGeneralsOnline", TheGlobalData->getPath_UserData().str());
	CreateDirectory(dir.str(), NULL);
	dir.format("%sGeneralsOnline\\Ladders", TheGlobalData->getPath_UserData().str());
	CreateDirectory(dir.str(), NULL);

	TheGameSpyBuddyMessageQueue = GameSpyBuddyMessageQueueInterface::createNewMessageQueue();
	TheGameSpyBuddyMessageQueue->startThread();

	TheGameSpyPeerMessageQueue = GameSpyPeerMessageQueueInterface::createNewMessageQueue();
	TheGameSpyPeerMessageQueue->startThread();
	
	TheGameSpyPSMessageQueue = GameSpyPSMessageQueueInterface::createNewMessageQueue();
	TheGameSpyPSMessageQueue->startThread();

	/*
	TheGameSpyGame = NEW GameSpyStagingRoom;
	*/

	TheGameSpyInfo = GameSpyInfoInterface::createNewGameSpyInfoInterface();
	TheGameSpyInfo->setMOTD(motdBuffer);
	TheGameSpyInfo->setConfig(configBuffer);

	CustomMatchPreferences pref;
	TheGameSpyInfo->setDisallowAsianText(pref.getDisallowAsianText());
	TheGameSpyInfo->setDisallowNonAsianText( pref.getDisallowNonAsianText());
	

	TheGameSpyConfig = GameSpyConfigInterface::create(configBuffer);

	TheLadderList = NEW LadderList;

	ThePinger = PingerInterface::createNewPingerInterface();
	ThePinger->startThreads();

	TheRankPointValues = NEW RankPoints;
}

void TearDownGameSpy( void )
{
	// save off cached stats
	if (TheGameSpyInfo && TheGameSpyInfo->getLocalProfileID())
	{
//		/* This was done on the score screen, so there is no need to do it now.
//		 *
		PSPlayerStats localPSStats = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
		if (localPSStats.id != 0)
		{
			GameSpyMiscPreferences mPref;
			mPref.setCachedStats(GameSpyPSMessageQueueInterface::formatPlayerKVPairs(localPSStats).c_str());
			mPref.write();
		}
//		*/
	}

	// End our threads before we kill off the singletons they reference.  No crashy-crash for you!
	if (TheGameSpyPSMessageQueue)
		TheGameSpyPSMessageQueue->endThread();
	if (TheGameSpyBuddyMessageQueue)
		TheGameSpyBuddyMessageQueue->endThread();
	if (TheGameSpyPeerMessageQueue)
		TheGameSpyPeerMessageQueue->endThread();
	if (ThePinger)
		ThePinger->endThreads();

	if(TheRankPointValues)
	{
		delete TheRankPointValues;
		TheRankPointValues = NULL;
	}
	if (TheGameSpyPSMessageQueue)
	{
		delete TheGameSpyPSMessageQueue;
		TheGameSpyPSMessageQueue = NULL;
	}

	if (TheGameSpyBuddyMessageQueue)
	{
		delete TheGameSpyBuddyMessageQueue;
		TheGameSpyBuddyMessageQueue = NULL;
	}

	if (TheGameSpyPeerMessageQueue)
	{
		delete TheGameSpyPeerMessageQueue;
		TheGameSpyPeerMessageQueue = NULL;
	}

	if (TheGameSpyInfo)
	{
		if (TheGameSpyInfo->getInternalIP())
		{
			// we've logged in before.  mark us as logging out.
			SignalUIInteraction(SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGOUT);
		}
		delete TheGameSpyInfo;
		TheGameSpyInfo = NULL;
	}

	if (ThePinger)
	{
		delete ThePinger;
		ThePinger = NULL;
	}

	if (TheLadderList)
	{
		delete TheLadderList;
		TheLadderList = NULL;
	}

	if (TheGameSpyConfig)
	{
		delete TheGameSpyConfig;
		TheGameSpyConfig = NULL;
	}

	// make sure the notification box doesn't exist
	deleteNotificationBox();
}


void GameSpyInfo::addToIgnoreList( AsciiString nick )
{
	m_ignoreList.insert(nick);
}

void GameSpyInfo::removeFromIgnoreList( AsciiString nick )
{
	m_ignoreList.erase(nick);
}

Bool GameSpyInfo::isIgnored( AsciiString nick )
{
	return m_ignoreList.find(nick) != m_ignoreList.end();
}

IgnoreList GameSpyInfo::returnIgnoreList( void )
{
 return m_ignoreList;
}

void GameSpyInfo::addToSavedIgnoreList( Int profileID, AsciiString nick)
{
	m_savedIgnoreMap[profileID] = nick;
	IgnorePreferences pref;
	pref.setIgnore(nick, profileID, true);
	pref.write();
}

void GameSpyInfo::removeFromSavedIgnoreList( Int profileID )
{
	m_savedIgnoreMap.erase(profileID);
	IgnorePreferences pref;
	pref.setIgnore(AsciiString::TheEmptyString, profileID, false);
	pref.write();
}

Bool GameSpyInfo::isSavedIgnored( Int profileID )
{
	return m_savedIgnoreMap.find(profileID) != m_savedIgnoreMap.end();
}

SavedIgnoreMap	GameSpyInfo::returnSavedIgnoreList( void )
{
	return m_savedIgnoreMap;
}

static Int grabHexInt(const char *s)
{
	char tmp[5] = "0xff";
	tmp[2] = s[0];
	tmp[3] = s[1];
	Int b = strtol(tmp, NULL, 16);
	return b;
}

Int GameSpyInfo::getPingValue( const AsciiString& otherPing )
{
	if (m_pingString.getLength() != otherPing.getLength())
	{
		return TheGameSpyConfig->getPingTimeoutInMs();
	}

	if (m_pingString.getLength() % 2 != 0)
	{
		return TheGameSpyConfig->getPingTimeoutInMs();
	}

	Int best = 255+255;
	const char *myStr = m_pingString.str();
	const char *otherStr = otherPing.str();

	while (*myStr)
	{
		Int myVal = grabHexInt(myStr);
		Int otherVal = grabHexInt(otherStr);
		Int val = myVal + otherVal;
		best = (val < best) ? val : best;
		myStr += 2;
		otherStr += 2;
	}

	return best * TheGameSpyConfig->getPingTimeoutInMs() / (255+255);
}

Bool PlayerInfo::isIgnored( void )
{
	return (m_profileID)?TheGameSpyInfo->isSavedIgnored(m_profileID):TheGameSpyInfo->isIgnored(m_name);
}

void GameSpyInfo::loadSavedIgnoreList( void )
{
	m_savedIgnoreMap.clear();
	IgnorePreferences prefs;
	m_savedIgnoreMap = prefs.getIgnores();	
}

void GameSpyInfo::setDisallowAsianText( Bool val )
{
	m_disallowAsainText = val;
}

void GameSpyInfo::setDisallowNonAsianText( Bool val )
{
	m_disallowNonAsianText = val;
}

Bool GameSpyInfo::getDisallowAsianText( void )
{
	return m_disallowAsainText;
}
Bool GameSpyInfo::getDisallowNonAsianText(void )
{
	return m_disallowNonAsianText;
}

void GameSpyInfo::setMaxMessagesPerUpdate( Int num )
{
	m_maxMessagesPerUpdate = num;
}

Int GameSpyInfo::getMaxMessagesPerUpdate( void )
{
	return m_maxMessagesPerUpdate;
}

/**This function is used to force an update of player's gamespy stats with an additional
disconnection.  This is used upon starting a new game so that if user disconnects prior
to finishing game, the disconnection stays on the server.  If he completes the game, we
remove this extra disconnection inside of populatePlayerInfo() on the ScoreScreen.  This
seems like the only secure way to handle this issue since users can abort the process
before we can detect/log disconnections.*/
void GameSpyInfo::updateAdditionalGameSpyDisconnections(Int count)
{
	if (TheRecorder->isMultiplayer() && TheGameLogic->isInInternetGame() && TheGameSpyGame && TheGameSpyGame->getUseStats())
	{	
		Int localID = TheGameSpyInfo->getLocalProfileID();
		PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(localID);
		
		Player *player=ThePlayerList->getLocalPlayer();

		Int ptIdx;
		const PlayerTemplate *myTemplate = player->getPlayerTemplate();
		DEBUG_LOG(("myTemplate = %X(%s)\n", myTemplate, myTemplate->getName().str()));
		for (ptIdx = 0; ptIdx < ThePlayerTemplateStore->getPlayerTemplateCount(); ++ptIdx)
		{
			const PlayerTemplate *nthTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(ptIdx);
			DEBUG_LOG(("nthTemplate = %X(%s)\n", nthTemplate, nthTemplate->getName().str()));
			if (nthTemplate == myTemplate)
			{
					break;
			}
		}

		Bool anyAI = FALSE;
		for (Int i=0; i<MAX_SLOTS; ++i)
		{
			const GameSlot *slot = TheGameInfo->getConstSlot(i);

			if (slot->isAI())
			{
				anyAI = TRUE;
			}
		}

		//Check for cases where we're not tracking stats.
		if (anyAI || stats.id == 0 || myTemplate->isObserver() || player->getPlayerType() != PLAYER_HUMAN || player->isPlayerObserver())
			return;

		Int disCons=stats.discons[ptIdx];
		disCons += count;
		if (disCons < 0)
		{	DEBUG_LOG(("updateAdditionalGameSpyDisconnections() - disconnection count below zero\n"));
			return;	//something is wrong here
		}
		stats.discons[ptIdx] = disCons;	//add an additional disconnection to their stats.

		//Add an additional disconnection to player stats.
		PSRequest req;

		req.requestType = PSRequest::PSREQUEST_UPDATEPLAYERSTATS;
		req.email = TheGameSpyInfo->getLocalEmail().str();
		req.nick = TheGameSpyInfo->getLocalBaseName().str();
		req.password = TheGameSpyInfo->getLocalPassword().str();
		req.player = stats;
		req.addDesync = FALSE;
		req.addDiscon = FALSE;
		req.lastHouse = ptIdx;

		TheGameSpyPSMessageQueue->addRequest(req);
		TheGameSpyPSMessageQueue->trackPlayerStats(stats);

		// force an update of our shtuff
		PSResponse newResp;
		newResp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
		newResp.player = stats;
		TheGameSpyPSMessageQueue->addResponse(newResp);

		// cache our stuff for easy reading next time
   		GameSpyMiscPreferences mPref;
   		mPref.setCachedStats(GameSpyPSMessageQueueInterface::formatPlayerKVPairs(stats).c_str());
   		mPref.write();
	}
}
