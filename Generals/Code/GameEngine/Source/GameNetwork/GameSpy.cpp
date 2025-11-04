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

// FILE: GameSpy.cpp //////////////////////////////////////////////////////
// GameSpy callbacks, etc
// Author: Matthew D. Campbell, February 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "gamespy/gp/gp.h"
#include "gamespy/gstats/gpersist.h"

#include "GameNetwork/FirewallHelper.h"
#include "GameNetwork/GameSpy.h"
#include "GameNetwork/GameSpyChat.h"
#include "GameNetwork/GameSpyGameInfo.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpyGP.h"
#include "GameNetwork/GameSpyPersistentStorage.h"
#include "GameNetwork/GameSpyThread.h"
#include "GameNetwork/NAT.h"
#include "GameClient/Shell.h"
#include "GameClient/MessageBox.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "Common/version.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/GlobalData.h"
#include "Common/UserPreferences.h"
#include "GameLogic/ScriptEngine.h"

MutexClass TheGameSpyMutex;
static UnsignedInt mainThreadID = 0;
static Bool inThread = false;
#define ISMAINTHREAD	( ThreadClass::_Get_Current_Thread_ID() == mainThreadID )
GameSpyThreadClass *TheGameSpyThread = NULL;

/// polling/update thread function

void GameSpyThreadClass::Thread_Function()
{

	//poll network and update GameSpy object

	while (running)
	{
		inThread = true;
		if (m_doLogin)
		{
			m_doLogin = false;
			TheGameSpyChat->login(m_nick, m_pass, m_email);
		}
		if (m_readStats)
		{
			m_readStats = false;
			TheGameSpyPlayerInfo->threadReadFromServer();
		}
		if (m_updateLocale)
		{
			m_updateLocale = false;
			TheGameSpyPlayerInfo->threadSetLocale(m_locale);
		}
		if (m_updateWins)
		{
			m_updateWins = false;
			TheGameSpyPlayerInfo->threadSetLocale(m_wins);
		}
		if (m_updateLosses)
		{
			m_updateLosses = false;
			TheGameSpyPlayerInfo->threadSetLocale(m_losses);
		}
		inThread = false;
		Switch_Thread();
	}
}

AsciiString GameSpyThreadClass::getNextShellScreen( void )
{
	MutexClass::LockClass m(TheGameSpyMutex, 0);
	if (m.Failed())
		return AsciiString::TheEmptyString;
	return m_nextShellScreen;
}

Bool GameSpyThreadClass::showLocaleSelect( void )
{
	MutexClass::LockClass m(TheGameSpyMutex, 0);
	if (m.Failed())
		return false;
	return m_showLocaleSelect;
}

void GameSpyThreadClass::setNextShellScreen( AsciiString nextShellScreen )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	m_nextShellScreen = nextShellScreen;
}

void GameSpyThreadClass::setShowLocaleSelect( Bool val )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	m_showLocaleSelect = val;
}

void createRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param);


class GameSpyChat : public GameSpyChatInterface
{
public:
	GameSpyChat();
	virtual ~GameSpyChat();

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

	virtual Bool isConnected( void );
	virtual void login(AsciiString loginName, AsciiString password = AsciiString::TheEmptyString, AsciiString email = AsciiString::TheEmptyString);
	virtual void reconnectProfile( void );
	virtual void disconnectFromChat( void );

	virtual void UTMRoom( RoomType roomType, const char *key, const char *val, Bool authenticate = FALSE );
	virtual void UTMPlayer( const char *name, const char *key, const char *val, Bool authenticate = FALSE );
	virtual void startGame( void );
	virtual void leaveRoom( RoomType roomType );
	virtual void setReady( Bool ready );
	virtual void enumPlayers( RoomType roomType, peerEnumPlayersCallback callback, void *userData );
	virtual void startListingGames( peerListingGamesCallback callback );
	virtual void stopListingGames( void );

	virtual void joinGroupRoom( Int ID );
	virtual void joinStagingRoom( GServer server, AsciiString password );
	virtual void createStagingRoom( AsciiString gameName, AsciiString password, Int maxPlayers );
	virtual void joinBestGroupRoom( void );

	void loginQuick(AsciiString loginName);
	void loginProfile(AsciiString loginName, AsciiString password, AsciiString email);
	void setProfileID( Int ID ) { m_profileID = ID; }

	void _connectCallback(PEER peer, PEERBool success, void * param);
	void _nickErrorCallback(PEER peer, int type, const char * nick, void * param);
	void _GPConnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param);
	void _GPReconnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param);

	inline void finishJoiningGroupRoom( void ) { m_joiningGroupRoom = false; }
	inline void finishJoiningStagingRoom( void ) { m_joiningStagingRoom = false; }

private:
	UnsignedInt m_loginTimeoutPeriod; // in ms
	UnsignedInt m_loginTimeout;

	Bool m_joiningGroupRoom;
	Bool m_joiningStagingRoom;

	GameSpyThreadClass thread;
};

GameSpyChatInterface *TheGameSpyChat;
GameSpyChatInterface *createGameSpyChat( void )
{
	mainThreadID = ThreadClass::_Get_Current_Thread_ID();
	return NEW GameSpyChat;
}

void GameSpyChatInterface::clearGroupRoomList(void)
{
	m_groupRooms.clear();
}

GameSpyChat::GameSpyChat()
{
	m_peer = NULL;
}

GameSpyChat::~GameSpyChat()
{
	reset();
	if (thread.Is_Running())
		thread.Stop();
	TheGameSpyThread = NULL;
}

void GameSpyChat::init( void )
{
	reset();

	DEBUG_ASSERTCRASH(!thread.Is_Running(), ("Thread is running"));

	thread.Execute();
	thread.Set_Priority(0);
	TheGameSpyThread = &thread;
}

void GameSpyChat::reset( void )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	m_loginName.clear();
	m_password.clear();
	m_email.clear();
	m_usingProfiles = false;
	m_profileID = 0;
	if (m_peer)
		peerShutdown(m_peer);
	m_peer = NULL;
	m_groupRooms.clear();
	m_currentGroupRoomID = 0;

	m_loginTimeoutPeriod = 5000;
	m_loginTimeout = 0;

	m_joiningGroupRoom = false;
	m_joiningStagingRoom = false;

	//if (thread.Is_Running())
		//thread.Stop();
}

void GameSpyChat::update( void )
{
	MutexClass::LockClass m(TheGameSpyMutex, 0);
	if (!m.Failed() && m_peer)
	{
		if (!TheGameSpyThread->getNextShellScreen().isEmpty())
		{
			TheShell->pop();
			TheShell->push(TheGameSpyThread->getNextShellScreen());
			TheGameSpyThread->setNextShellScreen( AsciiString.TheEmptyString );
		}

		if (TheGameSpyThread->showLocaleSelect())
		{
			TheGameSpyThread->setShowLocaleSelect(false);
			WindowLayout *layout = NULL;
			layout = TheWindowManager->winCreateLayout( AsciiString( "Menus/PopupLocaleSelect.wnd" ) );
			layout->runInit();
			layout->hide( FALSE );
			layout->bringForward();
			TheWindowManager->winSetModal( layout->getFirstWindow() );
		}

		//if (!isConnected())
			//return;

		peerThink(m_peer);

		gpProcess(TheGPConnection);

		if (TheNAT != NULL) {
			NATStateType NATState = TheNAT->update();
			if (NATState == NATSTATE_DONE)
			{
				GameSpyLaunchGame();
			}
			else if (NATState == NATSTATE_FAILED)
			{
				GameSpyLaunchGame();
			}
		}

		if (TheFirewallHelper != NULL) {
			if (TheFirewallHelper->behaviorDetectionUpdate()) {
				TheGlobalData->m_firewallBehavior = TheFirewallHelper->getFirewallBehavior();
				OptionPreferences *pref = NEW OptionPreferences;
				char num[16];
				num[0] = 0;
				itoa(TheGlobalData->m_firewallBehavior, num, 10);
				AsciiString numstr;
				numstr = num;
				(*pref)["FirewallBehavior"] = numstr;
				pref->write();

				// we are now done with the firewall helper
				delete TheFirewallHelper;
				TheFirewallHelper = NULL;
			}
		}

		UnsignedInt now = timeGetTime();
		if (m_loginTimeout && now > m_loginTimeout)
		{
			// login timed out
			m_loginTimeout = 0;
			GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"Timed out connecting"), NULL);

			// Enable controls again
			//EnableLoginControls(TRUE);

			if (TheGameSpyGame)
				delete TheGameSpyGame;
			TheGameSpyGame = NULL;

			if (m_peer)
				peerShutdown(m_peer);
			m_peer = NULL;

			gpDisconnect(TheGPConnection);
			gpDestroy(TheGPConnection);
		}
	}
}

Bool GameSpyChat::isConnected( void )
{
	return m_peer && peerIsConnected(m_peer);
}

void GameSpyChat::UTMRoom( RoomType roomType, const char *key, const char *val, Bool authenticate )
{
	peerUTMRoom( m_peer, roomType, key, val, (authenticate)?PEERTrue:PEERFalse );
}

void GameSpyChat::UTMPlayer( const char *name, const char *key, const char *val, Bool authenticate )
{
	peerUTMPlayer( m_peer, name, key, val, (authenticate)?PEERTrue:PEERFalse );
}

void GameSpyChat::startGame( void )
{
	peerStartGame( m_peer, NULL, PEER_STOP_REPORTING );
}

void GameSpyChat::leaveRoom( RoomType roomType )
{
	peerLeaveRoom( m_peer, roomType, NULL );
}

void GameSpyChat::setReady( Bool ready )
{
	peerSetReady( m_peer, (ready)?PEERTrue:PEERFalse );
}

void GameSpyChat::enumPlayers( RoomType roomType, peerEnumPlayersCallback callback, void *userData )
{
	peerEnumPlayers( m_peer, roomType, callback, userData );
}

void GameSpyChat::startListingGames( peerListingGamesCallback callback )
{
	peerSetUpdatesRoomChannel( m_peer, "#gmtest_updates" );
	peerStartListingGames( m_peer, NULL, callback, NULL );
}

void GameSpyChat::stopListingGames( void )
{
	peerStopListingGames( m_peer );
}

void GameSpyChat::joinGroupRoom( Int ID )
{
	if (m_joiningGroupRoom || m_joiningStagingRoom)
		return;

	m_joiningGroupRoom = true;

	if (getCurrentGroupRoomID())
	{
		leaveRoom(GroupRoom);
		setCurrentGroupRoomID(0);
	}
	peerJoinGroupRoom(m_peer, ID, JoinRoomCallback, (void *)ID, PEERFalse);
}

void GameSpyChat::joinStagingRoom( GServer server, AsciiString password )
{
	if (m_joiningGroupRoom || m_joiningStagingRoom)
		return;

	m_joiningStagingRoom = true;

	peerJoinStagingRoom(m_peer, server, password.str(), JoinRoomCallback, server, PEERFalse);
}

void GameSpyChat::createStagingRoom( AsciiString gameName, AsciiString password, Int maxPlayers )
{
	if (m_joiningGroupRoom || m_joiningStagingRoom)
		return;

	m_joiningStagingRoom = true;

	Int oldGroupID = TheGameSpyChat->getCurrentGroupRoomID();

	TheGameSpyChat->leaveRoom(GroupRoom);
	TheGameSpyChat->setCurrentGroupRoomID(0);

	DEBUG_LOG(("GameSpyChat::createStagingRoom - creating staging room, name is %s", m_loginName.str()));

	peerCreateStagingRoom(m_peer, m_loginName.str(), maxPlayers, password.str(), createRoomCallback, (void *)oldGroupID, PEERFalse);
}

void GameSpyChat::joinBestGroupRoom( void )
{
	if (m_groupRooms.size())
	{
		int minID = -1;
		int minPlayers = 1000;
		GroupRoomMap::iterator iter = m_groupRooms.begin();
		while (iter != m_groupRooms.end())
		{
			GameSpyGroupRoom room = iter->second;
			DEBUG_LOG(("Group room %d: %s (%d, %d, %d, %d)", room.m_groupID, room.m_name.str(), room.m_numWaiting, room.m_maxWaiting,
				room.m_numGames, room.m_numPlaying));

			if (minPlayers > 25 && room.m_numWaiting < minPlayers)
			{
				minID = room.m_groupID;
				minPlayers = room.m_numWaiting;
			}

			++iter;
		}

		if (minID > 0)
		{
			joinGroupRoom(minID);
		}
		else
		{
			GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"No empty group rooms"), NULL);
		}
	}
	else
	{
		GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"No group rooms"), NULL);
	}
}

void GameSpyChat::disconnectFromChat( void )
{
	TheScriptEngine->signalUIInteract("GameSpyLogout");

	if (m_peer)
	{
		peerShutdown(m_peer);
		m_peer = NULL;
	}
	if (TheGameSpyGame)
		delete TheGameSpyGame;
	TheGameSpyGame = NULL;
}






//-----------------------------------------------------------------------

void DisconnectedCallback(PEER peer, const char * reason,
													void * param)
{
	TheScriptEngine->signalUIInteract("GameSpyLogout");

	if (TheGameSpyGame)
		delete TheGameSpyGame;
	TheGameSpyGame = NULL;

	GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSDisconnected"), NULL);

	WindowLayout * mainMenu = TheShell->findScreenByFilename("Menus/MainMenu.wnd");
	if (mainMenu)
	{
		while (TheShell->top() != mainMenu)
		{
			TheShell->pop();
		}
	}
}

void ReadyChangedCallback(PEER peer, const char * nick,
													PEERBool ready, void * param)
{
	if (TheGameSpyGame)
	{
		Int slotNum = TheGameSpyGame->getSlotNum(nick);
		if (slotNum >= 0)
		{
			GameSlot *slot = TheGameSpyGame->getSlot(slotNum);
			if (slot)
			{
				if (ready) {
					slot->setAccept();
				} else {
					slot->unAccept();
				}

				if (TheGameSpyGame->amIHost())
				{
					peerUTMRoom(TheGameSpyChat->getPeer(), StagingRoom, "SL/", GameInfoToAsciiString(TheGameSpyGame).str(), PEERFalse);
				}

				WOLDisplaySlotList();
			}
		}
	}
}

void GameStartedCallback(PEER peer, unsigned int IP,
												 const char * message, void * param)
{
	GameSpyStartGame();
}

void populateLobbyPlayerListbox(void);
void PlayerJoinedCallback(PEER peer, RoomType roomType,
													const char * nick, void * param)
{
	if (roomType == GroupRoom && TheGameSpyChat->getCurrentGroupRoomID())
	{
		populateLobbyPlayerListbox();
	}
	if (roomType == StagingRoom && TheGameSpyGame)
	{
		DEBUG_LOG(("StagingRoom, Game OK"));
		for (Int i=1; i<MAX_SLOTS; ++i)
		{
			GameSlot *slot = TheGameSpyGame->getSlot(i);
			if (slot && slot->getState() == SLOT_OPEN)
			{
				DEBUG_LOG(("Putting %s in slot %d", nick, i));
				UnicodeString uName;
				uName.translate(nick);
				slot->setState(SLOT_PLAYER, uName);
				slot->setColor(-1);
				slot->setPlayerTemplate(-1);
				slot->setStartPos(-1);
				slot->setTeamNumber(-1);
				TheGameSpyGame->resetAccepted();
				Int id;
				UnsignedInt ip;
				PEERBool success = peerGetPlayerInfoNoWait(TheGameSpyChat->getPeer(), nick, &ip, &id);
				DEBUG_LOG(("PlayerJoinedCallback - result from peerGetPlayerInfoNoWait, %d: ip=%d.%d.%d.%d, id=%d", success,
					(ip >> 24) & 0xff,
					(ip >> 16) & 0xff,
					(ip >> 8) & 0xff,
					ip & 0xff,
					id));
				success = PEERFalse;	// silence compiler warnings in release build
				slot->setIP(ip);
				if (TheGameSpyGame->amIHost())
				{
					peerUTMRoom(TheGameSpyChat->getPeer(), StagingRoom, "SL/", GameInfoToAsciiString(TheGameSpyGame).str(), PEERFalse);
				}

				WOLDisplaySlotList();
				break;
			}
		}
		if (i == MAX_SLOTS)
		{
			// we got all the way through without room for him - kick him
			if (TheGameSpyGame->amIHost())
			{
				peerUTMRoom(TheGameSpyChat->getPeer(), StagingRoom, "SL/", GameInfoToAsciiString(TheGameSpyGame).str(), PEERFalse);
			}
		}
	}
}

void PlayerLeftCallback(PEER peer, RoomType roomType,
												const char * nick, const char * reason,
												void * param)
{
	if (roomType == GroupRoom && TheGameSpyChat->getCurrentGroupRoomID())
	{
		populateLobbyPlayerListbox();
	}

	if (roomType == StagingRoom && TheGameSpyGame)
	{
		Int slotNum = TheGameSpyGame->getSlotNum(nick);
		if (slotNum >= 0)
		{
			GameSlot *slot = TheGameSpyGame->getSlot(slotNum);
			if (slot)
			{
				slot->setState(SLOT_OPEN);
				DEBUG_LOG(("Removing %s from slot %d", nick, slotNum));
				if (TheGameSpyGame->amIHost())
				{
					peerUTMRoom(TheGameSpyChat->getPeer(), StagingRoom, "SL/", GameInfoToAsciiString(TheGameSpyGame).str(), PEERFalse);
				}
				TheGameSpyGame->resetAccepted();

				WOLDisplaySlotList();
			}
		}
	}
}

void PlayerChangedNickCallback(PEER peer, RoomType roomType,
															 const char * oldNick,
															 const char * newNick, void * param)
{
	if (TheGameSpyChat->getCurrentGroupRoomID())
	{
		populateLobbyPlayerListbox();
	}
}

void PingCallback(PEER peer, const char * nick, int ping,
									void * param)
{
	DEBUG_LOG(("PingCallback"));
}

void CrossPingCallback(PEER peer, const char * nick1,
											 const char * nick2, int crossPing,
											 void * param)
{
	DEBUG_LOG(("CrossPingCallback"));
}

static void RoomUTMCallback(PEER peer, RoomType roomType, const char * nick,
														const char * command, const char * parameters,
														PEERBool authenticated, void * param)
{
	DEBUG_LOG(("RoomUTMCallback: %s says %s = [%s]", nick, command, parameters));
	if (roomType == StagingRoom && TheGameSpyGame)
	{
		Int slotNum = TheGameSpyGame->getSlotNum(nick);
		if (slotNum == 0 && !TheGameSpyGame->amIHost())
		{
			if (!strcmp(command, "SL"))
			{
				AsciiString options = parameters;
				options.trim();
				ParseAsciiStringToGameInfo(TheGameSpyGame, options.str());
				WOLDisplaySlotList();
			}
			else if (!strcmp(command, "HWS")) // HostWantsStart
			{
				Int slotNum = TheGameSpyGame->getLocalSlotNum();
				GameSlot *slot = TheGameSpyGame->getSlot(slotNum);
				if (slot->isAccepted() == false)
				{
					GameSpyAddText(TheGameText->fetch("GUI:HostWantsToStart"), GSCOLOR_DEFAULT);
				}
			}
		}
	}
}

static void PlayerUTMCallback(PEER peer, const char * nick,
															const char * command, const char * parameters,
															PEERBool authenticated, void * param)
{
	DEBUG_LOG(("PlayerUTMCallback: %s says %s = [%s]", nick, command, parameters));
	if (TheGameSpyGame)
	{
		Int slotNum = TheGameSpyGame->getSlotNum(nick);
		if (slotNum != 0 && TheGameSpyGame->amIHost())
		{
			if (!strcmp(command, "REQ"))
			{
				AsciiString options = parameters;
				options.trim();


				Bool change = false;
				Bool shouldUnaccept = false;
				AsciiString key;
				options.nextToken(&key, "=");
				Int val = atoi(options.str()+1);
				UnsignedInt uVal = atoi(options.str()+1);
				DEBUG_LOG(("GameOpt request: key=%s, val=%s from player %d", key.str(), options.str(), slotNum));

				GameSlot *slot = TheGameSpyGame->getSlot(slotNum);
				if (!slot)
					return;

				if (key == "Color")
				{
					if (val >= -1 && val < TheMultiplayerSettings->getNumColors() && val != slot->getColor())
					{
						Bool colorAvailable = TRUE;
						if(val != -1 )
						{
							for(Int i=0; i <MAX_SLOTS; i++)
							{
								GameSlot *checkSlot = TheGameSpyGame->getSlot(i);
								if(val == checkSlot->getColor() && slot != checkSlot)
								{
									colorAvailable = FALSE;
									break;
								}
							}
						}
						if(colorAvailable)
							slot->setColor(val);
						change = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid color %d", val));
					}
				}
				else if (key == "PlayerTemplate")
				{
					if (val >= PLAYERTEMPLATE_MIN && val < ThePlayerTemplateStore->getPlayerTemplateCount() && val != slot->getPlayerTemplate())
					{
						slot->setPlayerTemplate(val);
						change = true;
						shouldUnaccept = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid PlayerTemplate %d", val));
					}
				}
				else if (key == "StartPos")
				{
					if (val >= -1 && val < MAX_SLOTS && val != slot->getStartPos())
					{
						slot->setStartPos(val);
						change = true;
						shouldUnaccept = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid startPos %d", val));
					}
				}
				else if (key == "Team")
				{
					if (val >= -1 && val < MAX_SLOTS/2 && val != slot->getTeamNumber())
					{
						slot->setTeamNumber(val);
						change = true;
						shouldUnaccept = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid team %d", val));
					}
				}
				else if (key == "IP")
				{
					if (uVal != slot->getIP())
					{
						slot->setIP(uVal);
						change = true;
						shouldUnaccept = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid IP %d", uVal));
					}
				}
				else if (key == "NAT")
				{
					if ((val >= FirewallHelperClass::FIREWALL_TYPE_SIMPLE) &&
							(val <= FirewallHelperClass::FIREWALL_TYPE_DESTINATION_PORT_DELTA))
					{
						slot->setNATBehavior((FirewallHelperClass::FirewallBehaviorType)val);
						DEBUG_LOG(("Setting NAT behavior to %d for player %d", val, slotNum));
						change = true;
					}
					else
					{
						DEBUG_LOG(("Rejecting invalid NAT behavior %d from player %d", val, slotNum));
					}
				}

				if (change)
				{
					if (shouldUnaccept)
						TheGameSpyGame->resetAccepted();
					peerUTMRoom(TheGameSpyChat->getPeer(), StagingRoom, "SL/", GameInfoToAsciiString(TheGameSpyGame).str(), PEERFalse);
					WOLDisplaySlotList();
					DEBUG_LOG(("Slot value is color=%d, PlayerTemplate=%d, startPos=%d, team=%d, IP=0x%8.8X",
						slot->getColor(), slot->getPlayerTemplate(), slot->getStartPos(), slot->getTeamNumber(), slot->getIP()));
					DEBUG_LOG(("Slot list updated to %s", GameInfoToAsciiString(TheGameSpyGame).str()));
				}


			}
		}
	}
}

void GOABasicCallback(PEER peer, PEERBool playing, char * outbuf,
											int maxlen, void * param)
{
	snprintf(outbuf, maxlen, "\\gamename\\c&cgenerals\\gamever\\%s\\location\\%d",
		TheVersion->getAsciiVersion().str(), 0);
	DEBUG_LOG(("GOABasicCallback: [%s]", outbuf));
	TheGameSpyGame->gotGOACall();
}

void GOAInfoCallback(PEER peer, PEERBool playing, char * outbuf,
										 int maxlen, void * param)
{
	snprintf(outbuf, maxlen, "\\hostname\\%s\\hostport\\%d\\mapname\\%s\\gametype\\%s\\numplayers\\%d\\maxplayers\\%d\\gamemode\\%s",
		TheGameSpyChat->getLoginName().str(), 0, TheGameSpyGame->getMap().str(), "battle", TheGameSpyGame->getNumPlayers(), TheGameSpyGame->getMaxPlayers(), "wait");
	DEBUG_LOG(("GOAInfoCallback: [%s]", outbuf));
	TheGameSpyGame->gotGOACall();
}

void GOARulesCallback(PEER peer, PEERBool playing, char * outbuf,
											int maxlen, void * param)
{
	snprintf(outbuf, maxlen, "\\password\\0\\seed\\%d",
		TheGameSpyGame->getSeed());
	DEBUG_LOG(("GOARulesCallback: [%s]", outbuf));
	TheGameSpyGame->gotGOACall();
}

void GOAPlayersCallback(PEER peer, PEERBool playing, char * outbuf,
												int maxlen, void * param)
{
	AsciiString buf, tmp;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		DEBUG_LOG(("GOAPlayersCallback - about to call getSlot for player %d", i));
		GameSlot *slot = TheGameSpyGame->getSlot(i);
		AsciiString name;
		switch (slot->getState())
		{
		case SLOT_OPEN:
			name = "O";
			break;
		case SLOT_CLOSED:
			name = "X";
			break;
		case SLOT_EASY_AI:
			name = "CE";
			break;
		case SLOT_MED_AI:
			name = "CM";
			break;
		case SLOT_BRUTAL_AI:
			name = "CB";
			break;
		case SLOT_PLAYER:
			{
				AsciiString tmp;
				tmp.translate(slot->getName());
				name.format("H%s", tmp.str());
			}
			break;
		default:
			name = "?";
			break;
		}
		tmp.format("\\player_%d\\%s\\color_%d\\%d\\faction_%d\\%d\\team_%d\\%d\\pos_%d\\%d",
			i, name.str(),
			i, slot->getColor(),
			i, slot->getPlayerTemplate(),
			i, slot->getTeamNumber(),
			i, slot->getStartPos());
		buf.concat(tmp);
	}
	snprintf(outbuf, maxlen, buf.str());
	DEBUG_LOG(("GOAPlayersCallback: [%s]", outbuf));
	TheGameSpyGame->gotGOACall();
}

void JoinRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param)
{
	DEBUG_LOG(("JoinRoomCallback: success==%d, result==%d", success, result));
	switch (roomType)
	{
		case GroupRoom:
			{
				((GameSpyChat *)TheGameSpyChat)->finishJoiningGroupRoom();
				if (success)
				{
					// update our internal group room ID
					TheGameSpyChat->setCurrentGroupRoomID((Int)param);

					// see if we need to change screens
					WindowLayout *layout = TheShell->top();
					AsciiString windowFile = layout->getFilename();
					DEBUG_LOG(("Joined group room, active screen was %s", windowFile.str()));
					if (windowFile.compareNoCase("Menus/WOLCustomLobby.wnd") == 0)
					{
						// already on the right screen - just refresh it
						//GroupRoomJoinLobbyRefresh();
					}
					else
					{
						// change to the right screen
						TheShell->pop();
						TheShell->push("Menus/WOLCustomLobby.wnd");
					}
				}
				else
				{
					// failed to join lobby - bail to welcome screen
					WindowLayout *layout = TheShell->top();
					AsciiString windowFile = layout->getFilename();
					DEBUG_LOG(("Joined group room, active screen was %s", windowFile.str()));
					if (windowFile.compareNoCase("Menus/WOLWelcomeMenu.wnd") != 0)
					{
						TheShell->pop();
						TheShell->push("Menus/WOLWelcomeMenu.wnd");
					}
					GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"Unable to join group room"), NULL);
				}

				// Update buddy location
				if (TheGameSpyChat->getUsingProfile())
				{
					if (TheGameSpyChat->getCurrentGroupRoomID())
					{
						AsciiString s;
						s.format("c&cgenerals://0.0.0.0:0/%d", TheGameSpyChat->getCurrentGroupRoomID());
						gpSetStatus(TheGPConnection, GP_CHATTING, "Chatting", s.str());
					}
					else
					{
						gpSetStatus(TheGPConnection, GP_ONLINE, "Online", "");
					}
				}
			}
			break;
		case StagingRoom:
			{
				((GameSpyChat *)TheGameSpyChat)->finishJoiningStagingRoom();
				if (success)
				{
					DEBUG_LOG(("JoinRoomCallback - Joined staging room"));
					GServer server = (GServer)param;

					// leave any chat channels
					TheGameSpyChat->leaveRoom(GroupRoom);
					TheGameSpyChat->setCurrentGroupRoomID(0);

					// set up game info
					TheGameSpyGame->enterGame();
					TheGameSpyGame->setServer(server);
					GameSlot *slot = TheGameSpyGame->getSlot(0);
					AsciiString options, hostName;
					hostName = ServerGetPlayerStringValue(server, 0, "player", "<Empty>");
					UnicodeString uHostName;
					uHostName.translate(hostName.str() + 1); // go past the 'H'
					slot->setState(SLOT_PLAYER, uHostName);
					UnsignedInt localIP = peerGetLocalIP(TheGameSpyChat->getPeer());
					GetLocalChatConnectionAddress("peerchat.gamespy.com", 6667, localIP);
					localIP = ntohl(localIP); // The IP returned from GetLocalChatConnectionAddress is in network byte order.
					options.format("IP=%d", localIP);
					peerUTMPlayer(TheGameSpyChat->getPeer(), hostName.str(), "REQ/", options.str(), PEERFalse);
					options.format("NAT=%d", TheFirewallHelper->getFirewallBehavior());
					peerUTMPlayer(TheGameSpyChat->getPeer(), hostName.str(), "REQ/", options.str(), PEERFalse);

					// refresh the map cache
					TheMapCache->updateCache();

					// switch screens
					TheShell->pop();
					TheShell->push("Menus/GameSpyGameOptionsMenu.wnd");
				}
				else
				{
					// let the user know
					GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"Unable to join game"), NULL);
					DEBUG_LOG(("JoinRoomCallback - Failed to join staging room."));
				}

				// Update buddy location
				if (TheGameSpyChat->getUsingProfile())
				{
					if (TheGameSpyChat->getCurrentGroupRoomID())
					{
						AsciiString s;
						s.format("c&cgenerals://0.0.0.0:0/%d", TheGameSpyChat->getCurrentGroupRoomID());
						gpSetStatus(TheGPConnection, GP_CHATTING, "Chatting", s.str());
					}
					else
					{
						gpSetStatus(TheGPConnection, GP_ONLINE, "Online", "");
					}
				}
				break;
			}
	}

	//*didJoin = (success == PEERJoinSuccess || success == PEERTrue);
}

void createRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param)
{
	DEBUG_LOG(("CreateRoomCallback: success==%d, result==%d", success, result));

	if (roomType != StagingRoom)
	{
		DEBUG_CRASH(("Non-staging room create!"));
		return;
	}

	((GameSpyChat *)TheGameSpyChat)->finishJoiningStagingRoom();

	Int oldGroupID = (Int)param;

	if (success)
	{
		// set up the game info
		UnsignedInt localIP = peerGetLocalIP(TheGameSpyChat->getPeer());
		DEBUG_LOG(("createRoomCallback - peerGetLocalIP returned %d.%d.%d.%d as the local IP",
								localIP >> 24, (localIP >> 16) & 0xff, (localIP >> 8) & 0xff, localIP & 0xff));
//		GetLocalChatConnectionAddress("peerchat.gamespy.com", 6667, localIP);
//		DEBUG_LOG(("createRoomCallback - GetLocalChatConnectionAddress returned %d.%d.%d.%d as the local IP",
//								localIP >> 24, (localIP >> 16) & 0xff, (localIP >> 8) & 0xff, localIP & 0xff));
		localIP = ntohl(localIP); // The IP returned from GetLocalChatConnectionAddress is in network byte order.
		TheGameSpyGame->setLocalIP(localIP);

		UnicodeString name;
		name.translate(TheGameSpyChat->getLoginName());
		TheGameSpyGame->enterGame();
//	TheGameSpyGame->setSeed(GameClientRandomValue(0, INT_MAX - 1));
		TheGameSpyGame->setSeed(GetTickCount());
		GameSlot newSlot;
		newSlot.setState(SLOT_PLAYER, name);
		newSlot.setIP(localIP);
		TheGameSpyGame->setSlot(0,newSlot);

		TheMapCache->updateCache();

		TheGameSpyGame->setMap(TheGlobalData->m_mapName);
		AsciiString asciiMap;
		asciiMap = TheGlobalData->m_mapName;

		asciiMap.toLower();
		std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->find(asciiMap);
		if (it != TheMapCache->end())
		{
			TheGameSpyGame->getSlot(0)->setMapAvailability(true);
			TheGameSpyGame->setMapCRC( it->second.m_CRC );
			TheGameSpyGame->setMapSize( it->second.m_filesize );

			TheGameSpyGame->adjustSlotsForMap(); // BGC- adjust the slots for the new map.
		}


		// change to the proper screen
		TheShell->pop();
		TheShell->push("Menus/GameSpyGameOptionsMenu.wnd");
	}
	else
	{
		// join the lobby again
		TheGameSpyChat->joinGroupRoom(oldGroupID);
		GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"Unable to create game"), NULL);
	}

	// Update buddy location
	if (TheGameSpyChat->getUsingProfile())
	{
		if (TheGameSpyChat->getCurrentGroupRoomID())
		{
			AsciiString s;
			s.format("c&cgenerals://0.0.0.0:0/%d", TheGameSpyChat->getCurrentGroupRoomID());
			gpSetStatus(TheGPConnection, GP_CHATTING, "Chatting", s.str());
		}
		else
		{
			gpSetStatus(TheGPConnection, GP_ONLINE, "Online", "");
		}
	}
}

// Gets called once for each group room when listing group rooms.
// After this has been called for each group room, it will be
// called one more time with groupID==0 and name==NULL.
/////////////////////////////////////////////////////////////////
void ListGroupRoomsCallback(PEER peer, PEERBool success,
														int groupID, GServer server,
														const char * name, int numWaiting,
														int maxWaiting, int numGames,
														int numPlaying, void * param)
{
	DEBUG_LOG(("ListGroupRoomsCallback"));
	if (success)
	{
		if (groupID)
		{
			GroupRoomMap *grMap = TheGameSpyChat->getGroupRooms();
			(*grMap)[groupID].m_name = name;
			(*grMap)[groupID].m_numWaiting = numWaiting;
			(*grMap)[groupID].m_maxWaiting = maxWaiting;
			(*grMap)[groupID].m_numGames = numGames;
			(*grMap)[groupID].m_numPlaying = numPlaying;
			(*grMap)[groupID].m_groupID = groupID;
		}
		else
		{
			// we've got the complete list.
			UpdateGroupRoomList();
		}
	}
}

// Called when peerConnect completes.
static void connectCallback(PEER peer, PEERBool success, void * param)
{
	((GameSpyChat *)TheGameSpyChat)->_connectCallback(peer, success, param);
}

static void nickErrorCallback(PEER peer, int type, const char * nick, void * param)
{
	((GameSpyChat *)TheGameSpyChat)->_nickErrorCallback(peer, type, nick, param);
}

static void GPConnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
	((GameSpyChat *)TheGameSpyChat)->_GPConnectCallback(pconnection, arg, param);
}

static void GPReconnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
	((GameSpyChat *)TheGameSpyChat)->_GPReconnectCallback(pconnection, arg, param);
}

void GameSpyChat::_connectCallback(PEER peer, PEERBool success, void * param)
{
	m_loginTimeout = 0;

	if (!success) {
		GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"Failed to connect"), NULL);
		DEBUG_LOG(("GameSpyChat::_connectCallback - failed to connect."));
	}

	if(!success)
	{
		peerShutdown(m_peer);
		m_peer = NULL;
		gpDisconnect(TheGPConnection);
		gpDestroy(TheGPConnection);

		// Enable controls again
		//EnableLoginControls(TRUE);
		return;
	}

	DEBUG_LOG(("Connected as profile %d (%s)", m_profileID, m_loginName.str()));

	TheGameSpyGame = NEW GameSpyGameInfo;

	// Enable controls again
	//EnableLoginControls(TRUE);

	// the readFromServer() call will set the screen
	if (m_profileID)
	{
		TheGameSpyPlayerInfo->readFromServer();
	}

	TheScriptEngine->signalUIInteract("GameSpyLogin");

	TheShell->popImmediate();
	TheShell->push("Menus/WOLWelcomeMenu.wnd");

	clearGroupRoomList();
	peerListGroupRooms(m_peer, ListGroupRoomsCallback, NULL, PEERFalse);
}

// Called if there's an error with the nick.
void GameSpyChat::_nickErrorCallback(PEER peer, int type, const char * nick, void * param)
{
	// Let the user know.
	/////////////////////
	if(type == PEER_IN_USE)
	{
		AsciiString nickStr = nick;
		AsciiString origName, appendedVal;
		nickStr.nextToken(&origName, "{}");
		nickStr.nextToken(&appendedVal, "{}");
		Int intVal = 1;
		if (!appendedVal.isEmpty())
		{
			intVal = atoi(appendedVal.str()) + 1;
		}
		DEBUG_LOG(("Nickname taken: origName=%s, tries=%d", origName.str(), intVal));

		if (intVal < 10)
		{
			nickStr.format("%s{%d}", origName.str(), intVal);
			// Retry the connect with a similar nick.
			DEBUG_LOG(("GameSpyChat::_nickErrorCallback - nick was taken, setting to %s", nickStr.str()));
			m_loginName = nickStr;
			peerRetryWithNick(peer, nickStr.str());
		}
		else
		{
			GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"That nickname is already taken; please choose another one."), NULL);
			// Cancel the connect.
			peerRetryWithNick(peer, NULL);

			// Enable controls again
			//EnableLoginControls(TRUE);
			m_loginTimeout = 0;
		}
	}
	else
	{
		GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"That nickname contains at least 1 invalid character, please choose another one."), NULL);
		// Cancel the connect.
		peerRetryWithNick(peer, NULL);

		// Enable controls again
		//EnableLoginControls(TRUE);
		m_loginTimeout = 0;
	}
}

void GameSpyChat::_GPConnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
	DEBUG_LOG(("GPConnectCallback:"));
	GPResult *res = (GPResult *)param;
	*res = arg->result;

	setProfileID(arg->profile);

	if (*res != GP_NO_ERROR)
	{
		// couldn't connect.  bummer.
		GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"Error connecting to buddy server"), NULL);
		gpDisconnect(TheGPConnection);
		gpDestroy(TheGPConnection);
		m_loginTimeout = 0;
		return;
	}

	// Connect to chat.
	///////////////////
	peerConnect(m_peer, m_loginName.str(), m_profileID, nickErrorCallback, connectCallback, NULL, PEERFalse);
}

static Bool inGPReconnect = false;
void GameSpyChat::_GPReconnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
	inGPReconnect = false;
	DEBUG_LOG(("GPConnectCallback:"));

	setProfileID(arg->profile);

	if (arg->result != GP_NO_ERROR)
	{
		// couldn't connect.  bummer.
		GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"Error connecting to buddy server"), NULL);
		gpDisconnect(TheGPConnection);
		gpDestroy(TheGPConnection);
		return;
	}
	else
	{
		// yay!  we're back in!
		GSMessageBoxOk(UnicodeString(L"Connected!"), UnicodeString(L"Reonnected to buddy server"), NULL);
	}
}

void GameSpyChat::loginProfile(AsciiString loginName, AsciiString password, AsciiString email)
{
	// Connect to GP.
	///////////////////
	m_profileID = 0;
	gpInitialize(TheGPConnection, 0);
	gpSetCallback(TheGPConnection, GP_ERROR,							(GPCallback)GPErrorCallback,						NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_REQUEST,	(GPCallback)GPRecvBuddyRequestCallback,	NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_MESSAGE,	(GPCallback)GPRecvBuddyMessageCallback,	NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_STATUS,	(GPCallback)GPRecvBuddyStatusCallback,	NULL);

	GPResult res = GP_PARAMETER_ERROR;
	m_loginName = loginName;
	DEBUG_LOG(("GameSpyChat::loginProfile - m_loginName set to %s", m_loginName.str()));
	m_password = password;
	m_email = email;
	gpConnect(TheGPConnection, loginName.str(), email.str(), password.str(), GP_FIREWALL, GP_NON_BLOCKING, (GPCallback)GPConnectCallback, &res);
	/*
	if (res != GP_NO_ERROR)
	{
		// couldn't connect.  bummer.
		GSMessageBoxOk(UnicodeString(L"Error connecting"), UnicodeString(L"Error connecting to buddy server"), NULL);
		gpDisconnect(TheGPConnection);
		gpDestroy(TheGPConnection);
		loginTimeout = 0;
		return;
	}

	// Connect to chat.
	///////////////////
	GameSpyLocalNickname = loginName;
	peerConnect(TheGameSpyChat->getPeer(), loginName.str(), GameSpyLocalProfile, nickErrorCallback, connectCallback, NULL, PEERFalse);
	*/
}

void GameSpyChat::reconnectProfile( void )
{
	if (inGPReconnect)
		return;

	inGPReconnect = true;

	gpInitialize(TheGPConnection, 0);
	gpSetCallback(TheGPConnection, GP_ERROR,							(GPCallback)GPErrorCallback,						NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_REQUEST,	(GPCallback)GPRecvBuddyRequestCallback,	NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_MESSAGE,	(GPCallback)GPRecvBuddyMessageCallback,	NULL);
	gpSetCallback(TheGPConnection, GP_RECV_BUDDY_STATUS,	(GPCallback)GPRecvBuddyStatusCallback,	NULL);

	gpConnect(TheGPConnection, m_loginName.str(), m_email.str(), m_password.str(), GP_FIREWALL, GP_NON_BLOCKING, (GPCallback)GPReconnectCallback, NULL);
}

void GameSpyChat::loginQuick(AsciiString login)
{
	m_profileID = 0; // no buddy stuff

	// Connect to chat.
	///////////////////
	m_loginName = login;
	DEBUG_LOG(("GameSpyChat::loginQuick - setting login to %s", m_loginName));
	peerConnect(m_peer, login.str(), 0, nickErrorCallback, connectCallback, NULL, PEERFalse);
}

void GameSpyChat::login(AsciiString loginName, AsciiString password, AsciiString email)
{
	MutexClass::LockClass m(TheGameSpyMutex);
	if ( ISMAINTHREAD )
	{
		thread.queueLogin(loginName, password, email);
		return;
	}

	// Setup the callbacks.
	///////////////////////
	PEERCallbacks callbacks;
	memset(&callbacks, 0, sizeof(PEERCallbacks));
	callbacks.disconnected = DisconnectedCallback;
	callbacks.readyChanged = ReadyChangedCallback;
	callbacks.roomMessage = RoomMessageCallback;
	callbacks.playerMessage = PlayerMessageCallback;
	callbacks.gameStarted = GameStartedCallback;
	callbacks.playerJoined = PlayerJoinedCallback;
	callbacks.playerLeft = PlayerLeftCallback;
	callbacks.playerChangedNick = PlayerChangedNickCallback;
	callbacks.ping = PingCallback;
	callbacks.crossPing = CrossPingCallback;
	callbacks.roomUTM = RoomUTMCallback;
	callbacks.playerUTM = PlayerUTMCallback;
	callbacks.GOABasic = GOABasicCallback;
	callbacks.GOAInfo = GOAInfoCallback;
	callbacks.GOARules = GOARulesCallback;
	callbacks.GOAPlayers = GOAPlayersCallback;
	//callbacks.globalKeyChanged = GlobalKeyChanged;
	callbacks.param = NULL;

	// Init peer.
	/////////////
	m_peer = peerInitialize(&callbacks);
	if(!m_peer)
	{
		GSMessageBoxOk(UnicodeString(L"No Peer"), UnicodeString(L"No Peer"), NULL);
		return;
	}

	// Setup which rooms to do pings and cross-pings in.
	////////////////////////////////////////////////////
	PEERBool pingRooms[NumRooms];
	PEERBool crossPingRooms[NumRooms];
	pingRooms[TitleRoom] = PEERTrue;
	pingRooms[GroupRoom] = PEERTrue;
	pingRooms[StagingRoom] = PEERFalse;
	crossPingRooms[TitleRoom] = PEERFalse;
	crossPingRooms[GroupRoom] = PEERFalse;
	crossPingRooms[StagingRoom] = PEERTrue;

	// Set the title.
	/////////////////
	if(!peerSetTitle(m_peer, "gmtest", "HA6zkS", "gmtest", "HA6zkS", 15, pingRooms, crossPingRooms))
	{
		GSMessageBoxOk(UnicodeString(L"Error setting title"), UnicodeString(L"Error setting title"), NULL);
		peerShutdown(m_peer);
		m_peer = NULL;
		return;
	}

	//EnableLoginControls( FALSE );
	m_loginTimeout = timeGetTime() + m_loginTimeoutPeriod;
	if (!loginName.isEmpty() && !email.isEmpty() && !password.isEmpty())
	{
		m_usingProfiles = true;
		loginProfile(loginName, password, email);
	}
	else
	{
		m_usingProfiles = false;
		loginQuick(loginName);
	}
}
