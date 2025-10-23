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

// FILE: PeerThread.cpp //////////////////////////////////////////////////////
// GameSpy Peer (chat) thread
// This thread communicates with GameSpy's chat server
// and talks through a message queue with the rest of
// the game.
// Author: Matthew D. Campbell, June 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Registry.h"
#include "Common/UserPreferences.h"
#include "Common/version.h"
#include "GameNetwork/IPEnumeration.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"

#include "strtok_r.h"
#include "mutex.h"
#include "thread.h"

#include "Common/MiniLog.h"


// enable this for trying to track down why SBServers are losing their keyvals  -MDC 2/20/2003
#undef SERVER_DEBUGGING
#ifdef SERVER_DEBUGGING
void CheckServers(PEER peer);
#endif // SERVER_DEBUGGING

#ifdef DEBUG_LOGGING
//#define PING_TEST
static LogClass s_pingLog("Ping.txt");
#define PING_LOG(x) s_pingLog.log x
#else // DEBUG_LOGGING
#define PING_LOG(x) {}
#endif // DEBUG_LOGGING

#ifdef DEBUG_LOGGING
static LogClass s_stateChangedLog("StateChanged.txt");

#define STATECHANGED_LOG(x) s_stateChangedLog.log x

#else // DEBUG_LOGGING

#define STATECHANGED_LOG(x) {}

#endif // DEBUG_LOGGING

// we should always be using broadcast keys from now on.  Remove the old code sometime when
// we're not in a rush, ok?
// -MDC 2/14/2003
#define USE_BROADCAST_KEYS


int isThreadHosting = 0;
static UnsignedInt s_lastStateChangedHeartbeat = 0;
static Bool s_wantStateChangedHeartbeat = FALSE;
static UnsignedInt s_heartbeatInterval = 10000;

static SOCKET qr2Sock = INVALID_SOCKET;

enum
{
	EXECRC_KEY = NUM_RESERVED_KEYS + 1,
	INICRC_KEY,
	PW_KEY,
	OBS_KEY,
#if !RTS_GENERALS
  USE_STATS_KEY,
#endif
	LADIP_KEY,
	LADPORT_KEY,
	PINGSTR_KEY,
	NUMPLAYER_KEY,
	MAXPLAYER_KEY,
	NUMOBS_KEY,
	NAME__KEY,
	FACTION__KEY,
	COLOR__KEY,
	WINS__KEY,
	LOSSES__KEY
};

#define EXECRC_STR		"exeCRC"
#define INICRC_STR		"iniCRC"
#define PW_STR				"pw"
#define OBS_STR				"obs"
#define USE_STATS_STR "stat"
#define LADIP_STR			"ladIP"
#define LADPORT_STR		"ladPort"
#define PINGSTR_STR		"pings"
#define NUMPLAYER_STR	"numRealPlayers"
#define MAXPLAYER_STR	"maxRealPlayers"
#define NUMOBS_STR		"numObservers"
#define NAME__STR			"name"
#define FACTION__STR	"faction"
#define COLOR__STR		"color"
#define WINS__STR			"wins"
#define LOSSES__STR		"losses"

//-------------------------------------------------------------------------

typedef std::queue<PeerRequest> RequestQueue;
typedef std::queue<PeerResponse> ResponseQueue;
class PeerThreadClass;

class GameSpyPeerMessageQueue : public GameSpyPeerMessageQueueInterface
{
public:
	virtual ~GameSpyPeerMessageQueue();
	GameSpyPeerMessageQueue();
	virtual void startThread( void );
	virtual void endThread( void );
	virtual Bool isThreadRunning( void );
	virtual Bool isConnected( void );
	virtual Bool isConnecting( void );

	virtual void addRequest( const PeerRequest& req );
	virtual Bool getRequest( PeerRequest& req );

	virtual void addResponse( const PeerResponse& resp );
	virtual Bool getResponse( PeerResponse& resp );

	virtual SerialAuthResult getSerialAuthResult( void ) { return m_serialAuth; }
	void setSerialAuthResult( SerialAuthResult result ) { m_serialAuth = result; }

	PeerThreadClass* getThread( void );

private:
	MutexClass m_requestMutex;
	MutexClass m_responseMutex;
	RequestQueue m_requests;
	ResponseQueue m_responses;
	PeerThreadClass *m_thread;

	SerialAuthResult m_serialAuth;
};

GameSpyPeerMessageQueueInterface* GameSpyPeerMessageQueueInterface::createNewMessageQueue( void )
{
	return NEW GameSpyPeerMessageQueue;
}

GameSpyPeerMessageQueueInterface *TheGameSpyPeerMessageQueue;
#define MESSAGE_QUEUE ((GameSpyPeerMessageQueue *)TheGameSpyPeerMessageQueue)

//-------------------------------------------------------------------------

class PeerThreadClass : public ThreadClass
{

public:
	PeerThreadClass() : ThreadClass()
	{
		//Added By Sadullah Nader
		//Initializations inserted
		m_roomJoined = m_allowObservers = m_hasPassword = FALSE;
    m_useStats = TRUE;
		m_exeCRC = m_iniCRC = 0;
		m_gameVersion = 0;
		m_ladderPort = 0;
		m_localRoomID = 0;
		m_maxPlayers = 0;
		m_numObservers = 0;
		m_maxPlayers = 0;
		m_qmGroupRoom = 0;
		m_sawEndOfEnumPlayers = m_sawMatchbot = FALSE;
		m_sawCompleteGameList = FALSE;
		//
		m_isConnecting = m_isConnected = false;
		m_groupRoomID = m_profileID = 0;
		m_nextStagingServer = 1; m_stagingServers.clear();
		m_pingStr = ""; m_mapName = ""; m_ladderIP = ""; m_isHosting = false;
		for (Int i=0; i<MAX_SLOTS; ++i)
		{
			m_playerNames[i] = "";

			//Added by Sadullah Nader
			//Initializations
			m_playerColors[i] = 0;
			m_playerFactions[i] = 0;
			m_playerLosses[i] = 0;
			m_playerProfileID[i] = 0;
			m_playerWins[i] = 0;

			//
		}
	}

	void Thread_Function();

	void markAsDisconnected( void ) { m_isConnecting = m_isConnected = false; }

	void connectCallback( PEER peer, PEERBool success );
	void nickErrorCallback( PEER peer, Int type, const char *nick );

	Bool isConnecting( void ) { return m_isConnecting; }
	Bool isConnected( void ) { return m_isConnected; }

	Int addServerToMap( SBServer server );
	Int removeServerFromMap( SBServer server );
	void clearServers( void );
	SBServer findServerByID( Int id );
	Int findServer( SBServer server );

	// get info about the game we are hosting
	Bool isHosting( void ) { return m_isHosting; }
	void stopHostingAlready(PEER peer);
	Bool hasPassword( void ) { return m_hasPassword; }
	Bool allowObservers( void ) { return m_allowObservers; }
  Bool useStats(void) const { return m_useStats; }
	std::string getMapName( void ) { return m_mapName; }
	UnsignedInt exeCRC( void ) { return m_exeCRC; }
	UnsignedInt iniCRC( void ) { return m_iniCRC; }
	UnsignedInt gameVersion( void ) { return m_gameVersion; }
	std::wstring getLocalStagingServerName( void ) { return m_localStagingServerName; }
	Int getLocalRoomID( void ) { return m_localRoomID; }
	std::string ladderIP( void ) { return m_ladderIP; }
	UnsignedShort ladderPort( void ) { return m_ladderPort; }
	std::string pingStr( void ) { return m_pingStr; }
	std::string getPlayerName(Int idx) { return m_playerNames[idx]; }
	Int getPlayerWins(Int idx) { return m_playerWins[idx]; }
	Int getPlayerLosses(Int idx) { return m_playerLosses[idx]; }
	Int getPlayerProfileID(Int idx) { return m_playerProfileID[idx]; }
	Int getPlayerFaction(Int idx) { return m_playerFactions[idx]; }
	Int getPlayerColor(Int idx) { return m_playerColors[idx]; }
	Int getNumPlayers(void) { return m_numPlayers; }
	Int getMaxPlayers(void) { return m_maxPlayers; }
	Int getNumObservers(void) { return m_numObservers; }

	void roomJoined( Bool val ) { m_roomJoined = val; }
	void setQMGroupRoom( Int groupID ) { m_qmGroupRoom = groupID; }
	void sawEndOfEnumPlayers( void ) { m_sawEndOfEnumPlayers = true; }
	void sawMatchbot( std::string bot ) { m_sawMatchbot = true; m_matchbotName = bot; }
	QMStatus getQMStatus( void ) { return m_qmStatus; }
	void handleQMMatch(PEER peer, Int mapIndex, Int seed, char *playerName[MAX_SLOTS], char *playerIP[MAX_SLOTS], char *playerSide[MAX_SLOTS], char *playerColor[MAX_SLOTS], char *playerNAT[MAX_SLOTS]);
	std::string getQMBotName( void ) { return m_matchbotName; }
	Int getQMGroupRoom( void ) { return m_qmGroupRoom; }
	Int getQMLadder( void ) { return m_qmInfo.QM.ladderID; }

	Int getCurrentGroupRoom(void) { return m_groupRoomID; }

#ifdef USE_BROADCAST_KEYS
	void pushStatsToRoom(PEER peer);
	void getStatsFromRoom(PEER peer, RoomType roomType);
	void trackStatsForPlayer(RoomType roomType, const char *nick, const char *key, const char *val);
	int lookupStatForPlayer(RoomType roomType, const char *nick, const char *key);
	void clearPlayerStats(RoomType roomType);
#endif // USE_BROADCAST_KEYS

	void setSawCompleteGameList(Bool val) { m_sawCompleteGameList = val; }
	Bool getSawCompleteGameList() { return m_sawCompleteGameList; }

private:
	Bool m_isConnecting;
	Bool m_isConnected;
	std::string m_loginName, m_originalName, m_password, m_email;
	Int m_profileID;
	Int m_groupRoomID;
	Bool m_sawCompleteGameList;

#ifdef USE_BROADCAST_KEYS
	enum { NumKeys = 6, ValBufSize = 20 };
	static const char *s_keys[NumKeys];
	static char s_valueBuffers[NumKeys][ValBufSize];
	static const char *s_values[NumKeys];

	typedef std::map<std::string, int> PlayerStatMap;
	PlayerStatMap m_groupRoomStats;
	PlayerStatMap m_stagingRoomStats;
	std::string packStatKey(const char *nick, const char *key);
#endif // USE_BROADCAST_KEYS

	// game-hosting info for GOA callbacks
	Bool m_isHosting;
	Bool m_hasPassword;
	std::string m_mapName;
	std::string m_playerNames[MAX_SLOTS];
	UnsignedInt m_exeCRC;
	UnsignedInt m_iniCRC;
	UnsignedInt m_gameVersion;
	Bool m_allowObservers;
  Bool m_useStats;
	std::string m_pingStr;
	std::string m_ladderIP;
	UnsignedShort m_ladderPort;
	Int m_playerWins[MAX_SLOTS];
	Int m_playerLosses[MAX_SLOTS];
	Int m_playerProfileID[MAX_SLOTS];
	Int m_playerColors[MAX_SLOTS];
	Int m_playerFactions[MAX_SLOTS];
	Int m_numPlayers;
	Int m_maxPlayers;
	Int m_numObservers;

	Int m_nextStagingServer;
	std::map<Int, SBServer> m_stagingServers;
	std::wstring m_localStagingServerName;
	Int m_localRoomID;

	void doQuickMatch( PEER peer );
	QMStatus m_qmStatus;
	PeerRequest m_qmInfo;
	Bool m_roomJoined;
	Int m_qmGroupRoom;
	Bool m_sawEndOfEnumPlayers;
	Bool m_sawMatchbot;
	std::string m_matchbotName;
};

#ifdef USE_BROADCAST_KEYS
const char* PeerThreadClass::s_keys[6] = { "b_locale", "b_wins", "b_losses", "b_points", "b_side", "b_pre" };
char PeerThreadClass::s_valueBuffers[6][20] = { "", "", "", "", "", "" };
const char* PeerThreadClass::s_values[6] = { s_valueBuffers[0], s_valueBuffers[1], s_valueBuffers[2],
	s_valueBuffers[3], s_valueBuffers[4], s_valueBuffers[5]};

void PeerThreadClass::trackStatsForPlayer(RoomType roomType, const char *nick, const char *key, const char *val)
{
	switch (roomType)
	{
		case GroupRoom:
			m_groupRoomStats[packStatKey(nick, key)] = atoi(val);
			break;
		case StagingRoom:
			m_stagingRoomStats[packStatKey(nick, key)] = atoi(val);
			break;
	}
}

std::string PeerThreadClass::packStatKey(const char *nick, const char *key)
{
	std::string s = nick;
	s.append(key);
	return s;
}

int PeerThreadClass::lookupStatForPlayer(RoomType roomType, const char *nick, const char *key)
{
	std::string fullKey = packStatKey(nick, key);
	PlayerStatMap::const_iterator it;
	switch (roomType)
	{
		case GroupRoom:
			it = m_groupRoomStats.find(fullKey);
			if (it != m_groupRoomStats.end())
				return it->second;
			break;
		case StagingRoom:
			it = m_stagingRoomStats.find(fullKey);
			if (it != m_stagingRoomStats.end())
				return it->second;
			break;
	}
	return 0;
}

void PeerThreadClass::clearPlayerStats(RoomType roomType)
{
	switch (roomType)
	{
		case GroupRoom:
			m_groupRoomStats.clear();
			break;
		case StagingRoom:
			m_stagingRoomStats.clear();
			break;
	}
}

void PeerThreadClass::pushStatsToRoom(PEER peer)
{
	DEBUG_LOG(("PeerThreadClass::pushStatsToRoom(): stats are %s=%s,%s=%s,%s=%s,%s=%s,%s=%s,%s=%s",
		s_keys[0], s_values[0],
		s_keys[1], s_values[1],
		s_keys[2], s_values[2],
		s_keys[3], s_values[3],
		s_keys[4], s_values[4],
		s_keys[5], s_values[5]));
	peerSetRoomKeys(peer, GroupRoom, m_loginName.c_str(), 6, s_keys, s_values);
	peerSetRoomKeys(peer, StagingRoom, m_loginName.c_str(), 6, s_keys, s_values);
}

void getRoomKeysCallback(PEER peer, PEERBool success, RoomType roomType, const char *nick, int num, char **keys, char **values, void *param);
void PeerThreadClass::getStatsFromRoom(PEER peer, RoomType roomType)
{
	peerGetRoomKeys(peer, GroupRoom, "*", NumKeys, s_keys, getRoomKeysCallback, this, PEERFalse);
}
#endif // USE_BROADCAST_KEYS

Int PeerThreadClass::addServerToMap( SBServer server )
{
	Int val = m_nextStagingServer++;
	m_stagingServers[val] = server;
	return val;
}

Int PeerThreadClass::removeServerFromMap( SBServer server )
{
	for (std::map<Int, SBServer>::iterator it = m_stagingServers.begin(); it != m_stagingServers.end(); ++it)
	{
		if (it->second == server)
		{
			Int val = it->first;
			m_stagingServers.erase(it);
			return val;
		}
	}

	return 0;
}

void PeerThreadClass::clearServers( void )
{
	m_stagingServers.clear();
}

SBServer PeerThreadClass::findServerByID( Int id )
{
	std::map<Int, SBServer>::iterator it = m_stagingServers.find(id);
	if (it != m_stagingServers.end())
	{
		SBServer server = it->second;
		if (server && !server->keyvals)
		{
			DEBUG_CRASH(("Referencing a missing server!"));
			return 0;
		}
		return it->second;
	}
	return 0;
}

Int PeerThreadClass::findServer( SBServer server )
{
	char tmp[10] = "";
	const char *newName = SBServerGetStringValue(server, "gamename", tmp);
	UnsignedInt newPrivateIP = SBServerGetPrivateInetAddress(server);
	UnsignedShort newPrivatePort = SBServerGetPrivateQueryPort(server);
	UnsignedInt newPublicIP = SBServerGetPublicInetAddress(server);

	SBServer serverToRemove = NULL;

	for (std::map<Int, SBServer>::iterator it = m_stagingServers.begin(); it != m_stagingServers.end(); ++it)
	{
		if (it->second == server)
		{
			return it->first;
		}
		else
		{
			const char *oldName = SBServerGetStringValue(it->second, "gamename", tmp);
			UnsignedInt oldPrivateIP = SBServerGetPrivateInetAddress(it->second);
			UnsignedShort oldPrivatePort = SBServerGetPrivateQueryPort(it->second);
			UnsignedInt oldPublicIP = SBServerGetPublicInetAddress(it->second);
			if (!strcmp(oldName, newName) &&
				oldPrivateIP == newPrivateIP &&
				oldPublicIP == newPublicIP &&
				oldPrivatePort == newPrivatePort)
			{
				serverToRemove = it->second;
			}
		}
	}

	if (serverToRemove)
	{
		// this is the same as another game - it has just migrated to another port.  Remove the old and replace it.
		PeerResponse resp;
		resp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOM;
		resp.stagingRoom.id = removeServerFromMap( serverToRemove );
		resp.stagingRoom.action = PEER_REMOVE;
		resp.stagingRoom.isStaging = TRUE;
		resp.stagingRoom.percentComplete = -1;
		TheGameSpyPeerMessageQueue->addResponse(resp);
	}

	return addServerToMap(server);
}

enum CallbackType
{
	CALLBACK_CONNECT,
	CALLBACK_ERROR,
	CALLBACK_RECVMESSAGE,
	CALLBACK_RECVREQUEST,
	CALLBACK_RECVSTATUS,
};

void connectCallbackWrapper( PEER peer, PEERBool success, int failureReason, void *param )
{
#ifdef SERVER_DEBUGGING
	DEBUG_LOG(("In connectCallbackWrapper()"));
	CheckServers(peer);
#endif // SERVER_DEBUGGING
	if (param != NULL)
	{
		((PeerThreadClass *)param)->connectCallback( peer, success );
	}
}

void nickErrorCallbackWrapper( PEER peer, Int type, const char *nick, int numSuggestedNicks, const gsi_char** suggestedNicks, void *param )
{
	if (param != NULL)
	{
		((PeerThreadClass *)param)->nickErrorCallback( peer, type, nick );
	}
}

static void joinRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param);

//-------------------------------------------------------------------------

GameSpyPeerMessageQueue::GameSpyPeerMessageQueue()
{
	m_thread = NULL;
	m_serialAuth = SERIAL_OK;
}

GameSpyPeerMessageQueue::~GameSpyPeerMessageQueue()
{
	endThread();
}

void GameSpyPeerMessageQueue::startThread( void )
{
	if (!m_thread)
	{
		m_thread = NEW PeerThreadClass;
		m_thread->Execute();
	}
	else
	{
		if (!m_thread->Is_Running())
		{
			m_thread->Execute();
		}
	}
}

void GameSpyPeerMessageQueue::endThread( void )
{
	delete m_thread;
	m_thread = NULL;
}

Bool GameSpyPeerMessageQueue::isThreadRunning( void )
{
	return (m_thread) ? m_thread->Is_Running() : false;
}

Bool GameSpyPeerMessageQueue::isConnected( void )
{
	return (m_thread) ? m_thread->isConnected() : false;
}

Bool GameSpyPeerMessageQueue::isConnecting( void )
{
	return (m_thread) ? m_thread->isConnecting() : false;
}

void GameSpyPeerMessageQueue::addRequest( const PeerRequest& req )
{
	MutexClass::LockClass m(m_requestMutex);
	if (m.Failed())
		return;

	m_requests.push(req);
}

//PeerRequest GameSpyPeerMessageQueue::getRequest( void )
Bool GameSpyPeerMessageQueue::getRequest( PeerRequest& req )
{
	MutexClass::LockClass m(m_requestMutex, 0);
	if (m.Failed())
		return false;

	if (m_requests.empty())
		return false;
	req = m_requests.front();
	m_requests.pop();
	return true;
}

void GameSpyPeerMessageQueue::addResponse( const PeerResponse& resp )
{
	if (resp.nick == "(END)")
		return;

	MutexClass::LockClass m(m_responseMutex);
	if (m.Failed())
		return;

	m_responses.push(resp);
}

//PeerResponse GameSpyPeerMessageQueue::getResponse( void )
Bool GameSpyPeerMessageQueue::getResponse( PeerResponse& resp )
{
	MutexClass::LockClass m(m_responseMutex, 0);
	if (m.Failed())
		return false;

	if (m_responses.empty())
		return false;
	resp = m_responses.front();
	m_responses.pop();
	return true;
}

PeerThreadClass* GameSpyPeerMessageQueue::getThread( void )
{
	return m_thread;
}

//-------------------------------------------------------------------------
static void disconnectedCallback(PEER peer, const char * reason, void * param);
static void roomMessageCallback(PEER peer, RoomType roomType, const char * nick, const char * message, MessageType messageType, void * param);
static void playerMessageCallback(PEER peer, const char * nick, const char * message, MessageType messageType, void * param);
static void playerJoinedCallback(PEER peer, RoomType roomType, const char * nick, void * param);
static void playerLeftCallback(PEER peer, RoomType roomType, const char * nick, const char * reason, void * param);
static void playerChangedNickCallback(PEER peer, RoomType roomType, const char * oldNick, const char * newNick, void * param);
static void playerInfoCallback(PEER peer, RoomType roomType, const char * nick, unsigned int IP, int profileID, void * param);
static void playerFlagsChangedCallback(PEER peer, RoomType roomType, const char * nick, int oldFlags, int newFlags, void * param);
static void listingGamesCallback(PEER peer, PEERBool success, const char * name, SBServer server, PEERBool staging, int msg, Int percentListed, void * param);
static void roomUTMCallback(PEER peer, RoomType roomType, const char * nick, const char * command, const char * parameters, PEERBool authenticated, void * param);
static void playerUTMCallback(PEER peer, const char * nick, const char * command, const char * parameters, PEERBool authenticated, void * param);
static void gameStartedCallback(PEER peer, SBServer server, const char *message, void *param);
static void globalKeyChangedCallback(PEER peer, const char *nick, const char *key, const char *val, void *param);
static void roomKeyChangedCallback(PEER peer, RoomType roomType, const char *nick, const char *key, const char *val, void *param);

// convenience function to set buddy status
static void updateBuddyStatus( GameSpyBuddyStatus status, Int groupRoom = 0, std::string gameName = "" )
{
	if (!TheGameSpyBuddyMessageQueue)
		return;

	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_SETSTATUS;
	switch(status)
	{
		case BUDDY_OFFLINE:
			req.arg.status.status = GP_OFFLINE;
			strcpy(req.arg.status.statusString, "Offline");
			strcpy(req.arg.status.locationString, "");
			break;
		case BUDDY_ONLINE:
			req.arg.status.status = GP_ONLINE;
			strcpy(req.arg.status.statusString, "Online");
			strcpy(req.arg.status.locationString, "");
			break;
		case BUDDY_LOBBY:
			req.arg.status.status = GP_CHATTING;
			strcpy(req.arg.status.statusString, "Chatting");
			sprintf(req.arg.status.locationString, "%d", groupRoom);
			break;
		case BUDDY_STAGING:
			req.arg.status.status = GP_STAGING;
			strcpy(req.arg.status.statusString, "Staging");
			sprintf(req.arg.status.locationString, "%s", gameName.c_str());
			break;
		case BUDDY_LOADING:
			req.arg.status.status = GP_PLAYING;
			strcpy(req.arg.status.statusString, "Loading");
			sprintf(req.arg.status.locationString, "%s", gameName.c_str());
			break;
		case BUDDY_PLAYING:
			req.arg.status.status = GP_PLAYING;
			strcpy(req.arg.status.statusString, "Playing");
			sprintf(req.arg.status.locationString, "%s", gameName.c_str());
			break;
		case BUDDY_MATCHING:
			req.arg.status.status = GP_ONLINE;
			strcpy(req.arg.status.statusString, "Matching");
			strcpy(req.arg.status.locationString, "");
			break;
	}
	DEBUG_LOG(("updateBuddyStatus %d:%s", req.arg.status.status, req.arg.status.statusString));
	TheGameSpyBuddyMessageQueue->addRequest(req);
}

static void createRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param)
{
	Int *s = (Int *)param;
	if (s)
		*s = result;
}

static const char * KeyTypeToString(qr2_key_type type)
{
	switch(type)
	{
	case key_server:
		return "server";
	case key_player:
		return "player";
	case key_team:
		return "team";
	}

	return "Unkown key type";
}

static const char * ErrorTypeToString(qr2_error_t error)
{
	switch(error)
	{
	case e_qrnoerror:
		return "noerror";
	case e_qrwsockerror:
		return "wsockerror";
	case e_qrbinderror:
		return "rbinderror";
	case e_qrdnserror:
		return "dnserror";
	case e_qrconnerror:
		return "connerror";
	}

	return "Unknown error type";
}

static void QRServerKeyCallback
(
	PEER peer,
	int key,
	qr2_buffer_t buffer,
	void * param
)
{
	//DEBUG_LOG(("QR_SERVER_KEY | %d (%s)", key, qr2_registered_key_list[key]));
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("QRServerKeyCallback: bailing because of no thread info"));
		return;
	}

	if (!t->isHosting())
		t->stopHostingAlready(peer);

#ifdef DEBUG_LOGGING
	AsciiString val = "";
#define ADD(x) { qr2_buffer_add(buffer, x); val = x; }
#define ADDINT(x) { qr2_buffer_add_int(buffer, x); val.format("%d",x); }
#else
#define ADD(x) { qr2_buffer_add(buffer, x); }
#define ADDINT(x) { qr2_buffer_add_int(buffer, x); }
#endif

	switch(key)
	{
	case HOSTNAME_KEY:
		ADD(t->getPlayerName(0).c_str());
		break;
	case GAMEVER_KEY:
		ADDINT(t->gameVersion());
		break;
	case EXECRC_KEY:
		ADDINT(t->exeCRC());
		break;
	case INICRC_KEY:
		ADDINT(t->iniCRC());
		break;
	case GAMENAME_KEY:
		{
			std::string tmp = t->getPlayerName(0);
			tmp.append(" ");
			tmp.append(WideCharStringToMultiByte(t->getLocalStagingServerName().c_str()));
			ADD(tmp.c_str());
		}
		break;
	case MAPNAME_KEY:
		ADD(t->getMapName().c_str());
		break;
	case PW_KEY:
		ADDINT(t->hasPassword());
		break;
	case OBS_KEY:
		ADDINT(t->allowObservers());
		break;
#if !RTS_GENERALS
  case USE_STATS_KEY:
    ADDINT(t->useStats());
    break;
#endif
	case LADIP_KEY:
		ADD(t->ladderIP().c_str());
		break;
	case LADPORT_KEY:
		ADDINT(t->ladderPort());
		break;
	case PINGSTR_KEY:
		ADD(t->pingStr().c_str());
		break;
	case NUMPLAYER_KEY:
		ADDINT(t->getNumPlayers());
		break;
	case MAXPLAYER_KEY:
		ADDINT(t->getMaxPlayers());
		break;
	case NUMOBS_KEY:
		ADDINT(t->getNumObservers());
		break;
	default:
		ADD("");
		//DEBUG_LOG(("QR_SERVER_KEY | %d (%s)", key, qr2_registered_key_list[key]));
		break;
	}

	DEBUG_LOG(("QR_SERVER_KEY | %d (%s) = [%s]", key, qr2_registered_key_list[key], val.str()));
}

static void QRPlayerKeyCallback
(
	PEER peer,
	int key,
	int index,
	qr2_buffer_t buffer,
	void * param
)
{
	//DEBUG_LOG(("QR_PLAYER_KEY | %d | %d (%s)", key, index, qr2_registered_key_list[key]));
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("QRPlayerKeyCallback: bailing because of no thread info"));
		return;
	}

	if (!t->isHosting())
		t->stopHostingAlready(peer);

#undef ADD
#undef ADDINT
#ifdef DEBUG_LOGGING
	AsciiString val = "";
#define ADD(x) { qr2_buffer_add(buffer, x); val = x; }
#define ADDINT(x) { qr2_buffer_add_int(buffer, x); val.format("%d",x); }
#else
#define ADD(x) { qr2_buffer_add(buffer, x); }
#define ADDINT(x) { qr2_buffer_add_int(buffer, x); }
#endif

	switch(key)
	{
	case NAME__KEY:
		ADD(t->getPlayerName(index).c_str());
		break;
	case WINS__KEY:
		ADDINT(t->getPlayerWins(index));
		break;
	case LOSSES__KEY:
		ADDINT(t->getPlayerLosses(index));
		break;
	case PID__KEY:
		ADDINT(t->getPlayerProfileID(index));
		break;
	case FACTION__KEY:
		ADDINT(t->getPlayerLosses(index));
		break;
	case COLOR__KEY:
		ADDINT(t->getPlayerLosses(index));
		break;
	default:
		ADD("");
		//DEBUG_LOG(("QR_PLAYER_KEY | %d | %d (%s)", key, index, qr2_registered_key_list[key]));
		break;
	}

	DEBUG_LOG(("QR_PLAYER_KEY | %d | %d (%s) = [%s]", key, index, qr2_registered_key_list[key], val.str()));
}

static void QRTeamKeyCallback
(
	PEER peer,
	int key,
	int index,
	qr2_buffer_t buffer,
	void * param
)
{
	//DEBUG_LOG(("QR_TEAM_KEY | %d | %d", key, index));

	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("QRTeamKeyCallback: bailing because of no thread info"));
		return;
	}
	if (!t->isHosting())
		t->stopHostingAlready(peer);

	// we don't report teams, so this shouldn't get called
	qr2_buffer_add(buffer, "");
}

static void QRKeyListCallback
(
	PEER peer,
	qr2_key_type type,
	qr2_keybuffer_t keyBuffer,
	void * param
)
{
	DEBUG_LOG(("QR_KEY_LIST | %s", KeyTypeToString(type)));

	/*
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("QRKeyListCallback: bailing because of no thread info"));
		return;
	}
	if (!t->isHosting())
		t->stopHostingAlready(peer);
		*/

	// register the keys we use
	switch(type)
	{
	case key_server:
		qr2_keybuffer_add(keyBuffer, HOSTNAME_KEY);
		qr2_keybuffer_add(keyBuffer, GAMEVER_KEY);
		//qr2_keybuffer_add(keyBuffer, GAMENAME_KEY);
		qr2_keybuffer_add(keyBuffer, MAPNAME_KEY);
		qr2_keybuffer_add(keyBuffer, EXECRC_KEY);
		qr2_keybuffer_add(keyBuffer, INICRC_KEY);
		qr2_keybuffer_add(keyBuffer, PW_KEY);
		qr2_keybuffer_add(keyBuffer, OBS_KEY);
#if !RTS_GENERALS
    qr2_keybuffer_add(keyBuffer, USE_STATS_KEY);
#endif
		qr2_keybuffer_add(keyBuffer, LADIP_KEY);
		qr2_keybuffer_add(keyBuffer, LADPORT_KEY);
		qr2_keybuffer_add(keyBuffer, PINGSTR_KEY);
		qr2_keybuffer_add(keyBuffer, NUMPLAYER_KEY);
		qr2_keybuffer_add(keyBuffer, MAXPLAYER_KEY);
		qr2_keybuffer_add(keyBuffer, NUMOBS_KEY);
		break;
	case key_player:
		qr2_keybuffer_add(keyBuffer, NAME__KEY);
		qr2_keybuffer_add(keyBuffer, WINS__KEY);
		qr2_keybuffer_add(keyBuffer, LOSSES__KEY);
		qr2_keybuffer_add(keyBuffer, PID__KEY);
		qr2_keybuffer_add(keyBuffer, FACTION__KEY);
		qr2_keybuffer_add(keyBuffer, COLOR__KEY);
		break;
	case key_team:
		// no custom team keys
		break;
	}
}

static int QRCountCallback
(
	PEER peer,
	qr2_key_type type,
	void * param
)
{
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("QRCountCallback: bailing because of no thread info"));
		return 0;
	}
	if (!t->isHosting())
		t->stopHostingAlready(peer);

	if(type == key_player)
	{
		DEBUG_LOG(("QR_COUNT | %s = %d", KeyTypeToString(type), t->getNumPlayers() + t->getNumObservers()));
		return t->getNumPlayers() + t->getNumObservers();
	}
	else if(type == key_team)
	{
		DEBUG_LOG(("QR_COUNT | %s = %d", KeyTypeToString(type), 0));
		return 0;
	}

	DEBUG_LOG(("QR_COUNT | %s = %d", KeyTypeToString(type), 0));
	return 0;
}

void PeerThreadClass::stopHostingAlready(PEER peer)
{
	isThreadHosting = 0; // debugging
	s_lastStateChangedHeartbeat = 0;
	s_wantStateChangedHeartbeat = FALSE;
	peerStopGame(peer);
	if (qr2Sock != INVALID_SOCKET)
	{
		closesocket(qr2Sock);
		qr2Sock = INVALID_SOCKET;
	}
}

static void QRAddErrorCallback
(
	PEER peer,
	qr2_error_t error,
	char * errorString,
	void * param
)
{
	DEBUG_LOG(("QR_ADD_ERROR | %s | %s", ErrorTypeToString(error), errorString));
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_FAILEDTOHOST;
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

static void QRNatNegotiateCallback
(
	PEER peer,
	int cookie,
	void * param
)
{
	DEBUG_LOG(("QR_NAT_NEGOTIATE | 0x%08X", cookie));
}

static void KickedCallback
(
	PEER peer,
	RoomType roomType,
	const char * nick,
	const char * reason,
	void * param
)
{
	DEBUG_LOG(("Kicked from %d by %s: \"%s\"", roomType, nick, reason));
}

static void NewPlayerListCallback
(
	PEER peer,
	RoomType roomType,
	void * param
)
{
	DEBUG_LOG(("NewPlayerListCallback"));
}

static void AuthenticateCDKeyCallback
(
	PEER peer,
	int result,
	const char * message,
	void * param
)
{
	DEBUG_LOG(("CD Key Result: %s (%d) %X", message, result, param));
#ifdef SERVER_DEBUGGING
	CheckServers(peer);
#endif // SERVER_DEBUGGING
	SerialAuthResult *val = (SerialAuthResult *)param;
	if (val)
	{
		if (result >= 1)
		{
			*val = SERIAL_OK;
		}
		else
		{
			*val = SERIAL_AUTHFAILED;
		}
	}
#ifdef SERVER_DEBUGGING
	CheckServers(peer);
#endif // SERVER_DEBUGGING
}

static SerialAuthResult doCDKeyAuthentication( PEER peer )
{
	SerialAuthResult retval = SERIAL_NONEXISTENT;
	if (!peer)
		return retval;

	AsciiString s = "";
	if (GetStringFromRegistry("\\ergc", "", s) && s.isNotEmpty())
	{
#ifdef SERVER_DEBUGGING
		DEBUG_LOG(("Before peerAuthenticateCDKey()"));
		CheckServers(peer);
#endif // SERVER_DEBUGGING
		peerAuthenticateCDKey(peer, s.str(), AuthenticateCDKeyCallback, &retval, PEERTrue);
#ifdef SERVER_DEBUGGING
		DEBUG_LOG(("After peerAuthenticateCDKey()"));
		CheckServers(peer);
#endif // SERVER_DEBUGGING
	}

	if (retval == SERIAL_OK)
	{
		PSRequest req;
		req.requestType = PSRequest::PSREQUEST_READCDKEYSTATS;
		req.cdkey = s.str();
		TheGameSpyPSMessageQueue->addRequest(req);
	}

	return retval;
}

#define INBUF_LEN 256
void checkQR2Queries( PEER peer, SOCKET sock )
{
	static char indata[INBUF_LEN];
	struct sockaddr_in saddr;
	int saddrlen = sizeof(struct sockaddr_in);
	fd_set set;
	struct timeval timeout = {0,0};
	int error;

	FD_ZERO ( &set );
	FD_SET ( sock, &set );

	while (1)
	{
		error = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (SOCKET_ERROR == error || 0 == error)
			return;
		//else we have data
		error = recvfrom(sock, indata, INBUF_LEN - 1, 0, (struct sockaddr *)&saddr, &saddrlen);
		if (error != SOCKET_ERROR)
		{
			indata[error] = '\0';
			peerParseQuery( peer, indata, error, (sockaddr *)&saddr );
		}
	}
}

static UnsignedInt localIP = 0;

void PeerThreadClass::Thread_Function()
{
	try {

	PEER peer;

	// Setup the callbacks.
	///////////////////////
	PEERCallbacks callbacks;
	memset(&callbacks, 0, sizeof(PEERCallbacks));
	callbacks.disconnected = disconnectedCallback;
	//callbacks.readyChanged = readyChangedCallback;
	callbacks.roomMessage = roomMessageCallback;
	callbacks.playerMessage = playerMessageCallback;
	callbacks.gameStarted = gameStartedCallback;
	callbacks.playerJoined = playerJoinedCallback;
	callbacks.playerLeft = playerLeftCallback;
	callbacks.playerChangedNick = playerChangedNickCallback;
	callbacks.playerFlagsChanged = playerFlagsChangedCallback;
	callbacks.playerInfo = playerInfoCallback;
	callbacks.roomUTM = roomUTMCallback;
	callbacks.playerUTM = playerUTMCallback;
	callbacks.globalKeyChanged = globalKeyChangedCallback;
	callbacks.roomKeyChanged = roomKeyChangedCallback;

	callbacks.qrServerKey = QRServerKeyCallback;
	callbacks.qrPlayerKey = QRPlayerKeyCallback;
	callbacks.qrTeamKey = QRTeamKeyCallback;
	callbacks.qrKeyList = QRKeyListCallback;
	callbacks.qrCount = QRCountCallback;
	callbacks.qrAddError = QRAddErrorCallback;
	callbacks.qrNatNegotiateCallback = QRNatNegotiateCallback;

	callbacks.kicked = KickedCallback;
	callbacks.newPlayerList = NewPlayerListCallback;

	callbacks.param = this;

	m_qmGroupRoom = 0;

	peer = peerInitialize( &callbacks );
	DEBUG_ASSERTCRASH( peer != NULL, ("NULL peer!") );
	m_isConnected = m_isConnecting = false;

	qr2_register_key(EXECRC_KEY, EXECRC_STR);
	qr2_register_key(INICRC_KEY, INICRC_STR);
	qr2_register_key(PW_KEY, PW_STR);
	qr2_register_key(OBS_KEY, OBS_STR);
#if !RTS_GENERALS
  qr2_register_key(USE_STATS_KEY, USE_STATS_STR);
#endif
	qr2_register_key(LADIP_KEY, LADIP_STR);
	qr2_register_key(LADPORT_KEY, LADPORT_STR);
	qr2_register_key(PINGSTR_KEY, PINGSTR_STR);
	qr2_register_key(NUMOBS_KEY, NUMOBS_STR);
	qr2_register_key(NUMPLAYER_KEY, NUMPLAYER_STR);
	qr2_register_key(MAXPLAYER_KEY, MAXPLAYER_STR);
	qr2_register_key(NAME__KEY, NAME__STR "_");
	qr2_register_key(WINS__KEY, WINS__STR "_");
	qr2_register_key(LOSSES__KEY, LOSSES__STR "_");
	qr2_register_key(FACTION__KEY, FACTION__STR "_");
	qr2_register_key(COLOR__KEY, COLOR__STR "_");

#if RTS_GENERALS
	const Int NumKeys = 14;
#else
	const Int NumKeys = 15;
#endif
	unsigned char allKeysArray[NumKeys] = {
		/*
		PID__KEY,
		NAME__KEY,
		WINS__KEY,
		LOSSES__KEY,
		FACTION__KEY,
		COLOR__KEY,
		*/
		MAPNAME_KEY,
		GAMEVER_KEY,
		GAMENAME_KEY,
		EXECRC_KEY,
		INICRC_KEY,
		PW_KEY,
		OBS_KEY,
#if !RTS_GENERALS
    USE_STATS_KEY,
#endif
		LADIP_KEY,
		LADPORT_KEY,
		PINGSTR_KEY,
		NUMOBS_KEY,
		NUMPLAYER_KEY,
		MAXPLAYER_KEY,
		HOSTNAME_KEY
	};

	/*
	const char *allKeys = "\\pid_\\mapname\\gamever\\gamename" \
		"\\" EXECRC_STR "\\" INICRC_STR \
		"\\" PW_STR "\\" OBS_STR "\\" USE_STATS_STR "\\" LADIP_STR "\\" LADPORT_STR \
		"\\" PINGSTR_STR "\\" NUMOBS_STR \
		"\\" NUMPLAYER_STR "\\" MAXPLAYER_STR \
		"\\" NAME__STR "_" "\\" WINS__STR "_" "\\" LOSSES__STR "_" "\\" FACTION__STR "_" "\\" COLOR__STR "_";
	*/

	const char * key = "username";
	peerSetRoomWatchKeys(peer, StagingRoom, 1, &key, PEERTrue);
	peerSetRoomWatchKeys(peer, GroupRoom, 1, &key, PEERTrue);

	m_localRoomID = 0;
	m_localStagingServerName = L"";

	m_qmStatus = QM_IDLE;

	// Setup which rooms to do pings and cross-pings in.
	////////////////////////////////////////////////////
	PEERBool pingRooms[NumRooms];
	PEERBool crossPingRooms[NumRooms];
	pingRooms[TitleRoom] = PEERFalse;
	pingRooms[GroupRoom] = PEERFalse;
	pingRooms[StagingRoom] = PEERFalse;
	crossPingRooms[TitleRoom] = PEERFalse;
	crossPingRooms[GroupRoom] = PEERFalse;
	crossPingRooms[StagingRoom] = PEERFalse;

	/*********
	First step, set our game authentication info
	We could do:
	Generals:
		strcpy(gcd_gamename,"ccgenerals");
		strcpy(gcd_secret_key,"h5T2f6");
	ZeroHour:
		strcpy(gcd_gamename,"ccgenzh");
		strcpy(gcd_secret_key,"D6s9k3");
	or
	Generals:
		strcpy(gcd_gamename,"ccgeneralsb");
		strcpy(gcd_secret_key,"g3T9s2");
	ZeroHour:
		strcpy(gcd_gamename,"ccgeneralsb");
		strcpy(gcd_secret_key,"whatever the key is");
	...but this is more secure:
	**********/
	char gameName[12] = {0};
	char secretKey[7] = {0};
	/**
	gameName[0]='c';gameName[1]='c';gameName[2]='g';gameName[3]='e';
	gameName[4]='n';gameName[5]='e';gameName[6]='r';gameName[7]='a';
	gameName[8]='l';gameName[9]='s';gameName[10]='b';gameName[11]='\0';
	secretKey[0]='g';secretKey[1]='3';secretKey[2]='T';secretKey[3]='9';
	secretKey[4]='s';secretKey[5]='2';secretKey[6]='\0';
	/**/
#if RTS_GENERALS
	gameName[0]='c';gameName[1]='c';gameName[2]='g';gameName[3]='e';
	gameName[4]='n';gameName[5]='e';gameName[6]='r';gameName[7]='a';
	gameName[8]='l';gameName[9]='s';gameName[10]='\0';
	secretKey[0]='h';secretKey[1]='5';secretKey[2]='T';secretKey[3]='2';
	secretKey[4]='f';secretKey[5]='6';secretKey[6]='\0';
#elif RTS_ZEROHOUR
	gameName[0]='c';gameName[1]='c';gameName[2]='g';gameName[3]='e';
	gameName[4]='n';gameName[5]='z';gameName[6]='h';gameName[7]='\0';
	secretKey[0]='D';secretKey[1]='6';secretKey[2]='s';secretKey[3]='9';
	secretKey[4]='k';secretKey[5]='3';secretKey[6]='\0';
#endif
	/**/

	// Set the title.
	/////////////////
	if(!peerSetTitle( peer , gameName, secretKey, gameName, secretKey, GetRegistryVersion(), 30, PEERTrue, pingRooms, crossPingRooms))
	{
		DEBUG_CRASH(("Error setting title"));
		peerShutdown( peer );
		peer = NULL;
		return;
	}

	OptionPreferences pref;
	UnsignedInt preferredIP = INADDR_ANY;
	UnsignedInt selectedIP = pref.getOnlineIPAddress();
	DEBUG_LOG(("Looking for IP %X", selectedIP));
	IPEnumeration IPs;
	EnumeratedIP *IPlist = IPs.getAddresses();
	while (IPlist)
	{
		DEBUG_LOG(("Looking at IP %s", IPlist->getIPstring().str()));
		if (selectedIP == IPlist->getIP())
		{
			preferredIP = IPlist->getIP();
			DEBUG_LOG(("Connecting to GameSpy chat server via IP address %8.8X", preferredIP));
			break;
		}
		IPlist = IPlist->getNext();
	}
	chatSetLocalIP(preferredIP);

	UnsignedInt preferredQRPort = 0;
	AsciiString selectedQRPort = pref["GameSpyQRPort"];
	if (selectedQRPort.isNotEmpty())
	{
		preferredQRPort = atoi(selectedQRPort.str());
	}

	PeerRequest incomingRequest;
	while ( running )
	{
		// deal with requests
		if (TheGameSpyPeerMessageQueue->getRequest(incomingRequest))
		{
			DEBUG_LOG(("TheGameSpyPeerMessageQueue->getRequest() got request of type %d", incomingRequest.peerRequestType));
			switch (incomingRequest.peerRequestType)
			{
			case PeerRequest::PEERREQUEST_LOGIN:
				{
				m_isConnecting = true;
				m_originalName = incomingRequest.nick;
				m_loginName = incomingRequest.nick;
				m_profileID = incomingRequest.login.profileID;
				m_password = incomingRequest.password;
				m_email = incomingRequest.email;
				peerConnect( peer, incomingRequest.nick.c_str(), incomingRequest.login.profileID, nickErrorCallbackWrapper, connectCallbackWrapper, this, PEERTrue );
#ifdef SERVER_DEBUGGING
				DEBUG_LOG(("After peerConnect()"));
				CheckServers(peer);
#endif // SERVER_DEBUGGING
				if (m_isConnected)
				{
					SerialAuthResult ret = doCDKeyAuthentication( peer );
					if (ret != SERIAL_OK)
					{
						m_isConnecting = m_isConnected = false;
						MESSAGE_QUEUE->setSerialAuthResult( ret );
						peerDisconnect( peer );
					}
				}
				m_isConnecting = false;

				// check our connection
				//if (m_isConnected)
				//{
				//	GetLocalChatConnectionAddress("peerchat.gamespy.com", 6667, localIP);
				//}
				}

				break;

			case PeerRequest::PEERREQUEST_LOGOUT:
				m_isConnecting = m_isConnected = false;
				peerDisconnect( peer );
				break;

			case PeerRequest::PEERREQUEST_JOINGROUPROOM:
				m_groupRoomID = incomingRequest.groupRoom.id;
				isThreadHosting = 0; // debugging
				s_lastStateChangedHeartbeat = 0;
				s_wantStateChangedHeartbeat = FALSE;
				peerStopGame( peer );
				peerLeaveRoom( peer, GroupRoom, NULL );
				peerLeaveRoom( peer, StagingRoom, NULL );
				if (qr2Sock != INVALID_SOCKET)
				{
					closesocket(qr2Sock);
					qr2Sock = INVALID_SOCKET;
				}
				m_isHosting = false;
				m_localRoomID = m_groupRoomID;
				DEBUG_LOG(("Requesting to join room %d in thread %X", m_localRoomID, this));
				peerJoinGroupRoom( peer, incomingRequest.groupRoom.id, joinRoomCallback, (void *)this, PEERTrue );
				break;

			case PeerRequest::PEERREQUEST_LEAVEGROUPROOM:
				m_groupRoomID = 0;
				updateBuddyStatus( BUDDY_ONLINE );
				peerLeaveRoom( peer, GroupRoom, NULL );
				peerLeaveRoom( peer, StagingRoom, NULL ); m_isHosting = false;
				break;

			case PeerRequest::PEERREQUEST_JOINSTAGINGROOM:
				{
					m_groupRoomID = 0;
					updateBuddyStatus( BUDDY_ONLINE );
					peerLeaveRoom( peer, GroupRoom, NULL );
					peerLeaveRoom( peer, StagingRoom, NULL ); m_isHosting = false;
					SBServer server = findServerByID(incomingRequest.stagingRoom.id);
					m_localStagingServerName = incomingRequest.text;
					DEBUG_LOG(("Setting m_localStagingServerName to [%ls]", m_localStagingServerName.c_str()));
					m_localRoomID = incomingRequest.stagingRoom.id;
					DEBUG_LOG(("Requesting to join room %d", m_localRoomID));
					if (server)
					{
						peerJoinStagingRoom( peer, server, incomingRequest.password.c_str(), joinRoomCallback, (void *)this, PEERTrue );
					}
					else
					{
						PeerResponse resp;
						resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINSTAGINGROOM;
						resp.joinStagingRoom.id = incomingRequest.stagingRoom.id;
						resp.joinStagingRoom.ok = FALSE;
						resp.joinStagingRoom.result = PEERJoinFailed;
						TheGameSpyPeerMessageQueue->addResponse(resp);
					}
				}
				break;

			case PeerRequest::PEERREQUEST_LEAVESTAGINGROOM:
				m_groupRoomID = 0;
				updateBuddyStatus( BUDDY_ONLINE );
				peerLeaveRoom( peer, GroupRoom, NULL );
				peerLeaveRoom( peer, StagingRoom, NULL );
				isThreadHosting = 0; // debugging
				s_lastStateChangedHeartbeat = 0;
				s_wantStateChangedHeartbeat = FALSE;
				if (m_isHosting)
				{
					peerStopGame( peer );
					if (qr2Sock != INVALID_SOCKET)
					{
						closesocket(qr2Sock);
						qr2Sock = INVALID_SOCKET;
					}
					m_isHosting = false;
				}
				break;

			case PeerRequest::PEERREQUEST_MESSAGEPLAYER:
				{
					std::string s = WideCharStringToMultiByte(incomingRequest.text.c_str());
					peerMessagePlayer( peer, incomingRequest.nick.c_str(), s.c_str(), (incomingRequest.message.isAction)?ActionMessage:NormalMessage );
				}
				break;

			case PeerRequest::PEERREQUEST_MESSAGEROOM:
				{
					std::string s = WideCharStringToMultiByte(incomingRequest.text.c_str());
					peerMessageRoom( peer, (m_groupRoomID)?GroupRoom:StagingRoom, s.c_str(), (incomingRequest.message.isAction)?ActionMessage:NormalMessage );
				}
				break;

			case PeerRequest::PEERREQUEST_PUSHSTATS:
				{
					DEBUG_LOG(("PEERREQUEST_PUSHSTATS: stats are %d,%d,%d,%d,%d,%d",
						incomingRequest.statsToPush.locale, incomingRequest.statsToPush.wins, incomingRequest.statsToPush.losses, incomingRequest.statsToPush.rankPoints, incomingRequest.statsToPush.side, incomingRequest.statsToPush.preorder));

					// Testing alternate way to push stats
#ifdef USE_BROADCAST_KEYS
					snprintf(s_valueBuffers[0], 20, "%d", incomingRequest.statsToPush.locale);
					snprintf(s_valueBuffers[1], 20, "%d", incomingRequest.statsToPush.wins);
					snprintf(s_valueBuffers[2], 20, "%d", incomingRequest.statsToPush.losses);
					snprintf(s_valueBuffers[3], 20, "%d", incomingRequest.statsToPush.rankPoints);
					snprintf(s_valueBuffers[4], 20, "%d", incomingRequest.statsToPush.side);
					snprintf(s_valueBuffers[5], 20, "%d", incomingRequest.statsToPush.preorder);
					pushStatsToRoom(peer);
#else
					const char *keys[6] = { "locale", "wins", "losses", "points", "side", "pre" };
					char valueStrings[6][20];
					char *values[6] = { valueStrings[0], valueStrings[1], valueStrings[2],
						valueStrings[3], valueStrings[4], valueStrings[5]};
					snprintf(values[0], 20, "%d", incomingRequest.statsToPush.locale);
					snprintf(values[1], 20, "%d", incomingRequest.statsToPush.wins);
					snprintf(values[2], 20, "%d", incomingRequest.statsToPush.losses);
					snprintf(values[3], 20, "%d", incomingRequest.statsToPush.rankPoints);
					snprintf(values[4], 20, "%d", incomingRequest.statsToPush.side);
					snprintf(values[5], 20, "%d", incomingRequest.statsToPush.preorder);
					peerSetGlobalKeys(peer, 6, (const char **)keys, (const char **)values);
					peerSetGlobalWatchKeys(peer, GroupRoom,   0, NULL, PEERFalse);
					peerSetGlobalWatchKeys(peer, StagingRoom, 0, NULL, PEERFalse);
					peerSetGlobalWatchKeys(peer, GroupRoom,   6, keys, PEERTrue);
					peerSetGlobalWatchKeys(peer, StagingRoom, 6, keys, PEERTrue);
#endif
				}
				break;

			case PeerRequest::PEERREQUEST_SETGAMEOPTIONS:
				{
					m_mapName = incomingRequest.gameOptsMapName;
					m_numPlayers = incomingRequest.gameOptions.numPlayers;
					m_numObservers = incomingRequest.gameOptions.numObservers;
					m_maxPlayers = incomingRequest.gameOptions.maxPlayers;
					DEBUG_LOG(("peerStateChanged(): Marking game options state as changed - %d players, %d observers", m_numPlayers, m_numObservers));
					for (Int i=0; i<MAX_SLOTS; ++i)
					{
						m_playerNames[i] = incomingRequest.gameOptsPlayerNames[i];
						m_playerWins[i] = incomingRequest.gameOptions.wins[i];
						m_playerLosses[i] = incomingRequest.gameOptions.losses[i];
						m_playerProfileID[i] = incomingRequest.gameOptions.profileID[i];
						m_playerFactions[i] = incomingRequest.gameOptions.faction[i];
						m_playerColors[i] = incomingRequest.gameOptions.color[i];
					}

					s_wantStateChangedHeartbeat = TRUE;

					/*
					peerStateChanged( peer );

#ifdef DEBUG_LOGGING
					static UnsignedInt prev = 0;
					UnsignedInt now = timeGetTime();
					UnsignedInt diff = now - prev;
					prev = now;
#endif
					STATECHANGED_LOG(("peerStateChanged() at time %d (difference of %d ms)", now, diff));
					*/

					peerUTMRoom( peer, StagingRoom, "SL/", incomingRequest.options.c_str(), PEERFalse ); // send the full string to people in the room
				}
				break;

			case PeerRequest::PEERREQUEST_GETEXTENDEDSTAGINGROOMINFO:
				{
					SBServer server = findServerByID( incomingRequest.stagingRoom.id );
					if (server)
					{
						DEBUG_LOG(("Requesting full update on a game"));
						peerUpdateGame( peer, server, PEERTrue );
					}
					else
					{
						DEBUG_LOG(("Tried to update non-existent server!"));
					}
				}
				break;

			case PeerRequest::PEERREQUEST_CREATESTAGINGROOM:
				{
					Int oldGroupID = m_groupRoomID;
					m_groupRoomID = 0;
					updateBuddyStatus( BUDDY_ONLINE );
					if (!incomingRequest.stagingRoomCreation.restrictGameList)
					{
						peerLeaveRoom( peer, GroupRoom, NULL );
						peerLeaveRoom( peer, StagingRoom, NULL );
					}
					m_isHosting = TRUE;

					Int res = PEERJoinFailed;
					if (qr2Sock == INVALID_SOCKET)
					{
						// allocate a port
						if (preferredQRPort < 1024)
						{
							preferredQRPort = 6500 + (ntohl(localIP) & 0xff);
						}
						DEBUG_LOG(("Using %8.8X:%d for QR2", ntohl(localIP), preferredQRPort));
					}
					else
					{
						closesocket(qr2Sock);
						qr2Sock = INVALID_SOCKET;
					}
					qr2Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
					struct sockaddr_in saddr;
					saddr.sin_port=htons(preferredQRPort);
					saddr.sin_addr.s_addr=localIP;
					saddr.sin_family=AF_INET;
					if (bind(qr2Sock, (sockaddr *)&saddr, sizeof(saddr)) != 0)
					{
						DEBUG_LOG(("Could not bind to %d!  Falling back to GameSpy's default port", preferredQRPort));
						closesocket(qr2Sock);
						qr2Sock = INVALID_SOCKET;
						preferredQRPort = 0;
					}
					std::string compositeGame = m_loginName;
					compositeGame.append(" ");
					compositeGame.append(WideCharStringToMultiByte(incomingRequest.text.c_str()));
					m_localStagingServerName = incomingRequest.text;
					m_playerNames[0] = m_loginName;
					peerCreateStagingRoomWithSocket(peer, compositeGame.c_str(), MAX_SLOTS, incomingRequest.password.c_str(), qr2Sock, preferredQRPort, createRoomCallback, (void *)&res, PEERTrue);
					//peerCreateStagingRoomWithSocket(peer, WideCharStringToMultiByte(incomingRequest.text.c_str()).c_str(), MAX_SLOTS, incomingRequest.password.c_str(), qr2Sock, preferredQRPort, createRoomCallback, (void *)&res, PEERTrue);
					DEBUG_LOG(("PEERREQUEST_CREATESTAGINGROOM - creating staging room, name is %ls, passwd is %s, result = %d",
						incomingRequest.text.c_str(), incomingRequest.password.c_str(), res));

					PeerResponse resp;
					resp.peerResponseType = PeerResponse::PEERRESPONSE_CREATESTAGINGROOM;
					resp.createStagingRoom.result = res;
					TheGameSpyPeerMessageQueue->addResponse(resp);

					if (res != PEERJoinSuccess && res != PEERAlreadyInRoom)
					{
						m_localRoomID = oldGroupID;
						DEBUG_LOG(("Requesting to join room %d", m_localRoomID));
						if (incomingRequest.stagingRoomCreation.restrictGameList)
						{
							peerLeaveRoom( peer, StagingRoom, NULL );
						}
						else
						{
							peerJoinGroupRoom( peer, oldGroupID, joinRoomCallback, (void *)this, PEERTrue );
						}
						m_isHosting = FALSE;
						m_localStagingServerName = L"";
						m_playerNames[0] = "";
					}
					else
					{
						if (incomingRequest.stagingRoomCreation.restrictGameList)
						{
							peerLeaveRoom( peer, GroupRoom, NULL );
						}
						isThreadHosting = 1; // debugging
						s_lastStateChangedHeartbeat = timeGetTime(); // wait the full interval before updating state
						s_wantStateChangedHeartbeat = FALSE;
						m_isHosting = TRUE;
						m_allowObservers = incomingRequest.stagingRoomCreation.allowObservers;
            m_useStats = incomingRequest.stagingRoomCreation.useStats;
						m_mapName = "";
						for (Int i=0; i<MAX_SLOTS; ++i)
						{
							m_playerNames[i] = "";
							m_playerWins[i] = 0;
							m_playerLosses[i] = 0;
							m_playerProfileID[i] = 0;
							m_playerFactions[i] = 0;
							m_playerColors[i] = 0;
						}
						if (incomingRequest.password.length() > 0)
							m_hasPassword = true;
						else
							m_hasPassword = false;
						m_playerNames[0] = m_loginName;
						m_exeCRC = incomingRequest.stagingRoomCreation.exeCRC;
						m_iniCRC = incomingRequest.stagingRoomCreation.iniCRC;
						m_gameVersion = incomingRequest.stagingRoomCreation.gameVersion;
						m_localStagingServerName = incomingRequest.text;
						m_ladderIP = incomingRequest.ladderIP;
						m_pingStr = incomingRequest.hostPingStr;
						m_ladderPort = incomingRequest.stagingRoomCreation.ladPort;

#ifdef USE_BROADCAST_KEYS
						pushStatsToRoom(peer);
#endif // USE_BROADCAST_KEYS

						DEBUG_LOG(("Setting m_localStagingServerName to [%ls]", m_localStagingServerName.c_str()));
						updateBuddyStatus( BUDDY_STAGING, 0, WideCharStringToMultiByte(m_localStagingServerName.c_str()) );
					}
				}
				break;

			case PeerRequest::PEERREQUEST_STARTGAMELIST:
				{
					m_sawCompleteGameList = FALSE;
					PeerResponse resp;
					resp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOM;
					resp.stagingRoom.action = PEER_CLEAR;
					resp.stagingRoom.isStaging = TRUE;
					resp.stagingRoom.percentComplete = 0;
					clearServers();
					TheGameSpyPeerMessageQueue->addResponse(resp);
					peerStartListingGames( peer, allKeysArray, NumKeys, (incomingRequest.gameList.restrictGameList?"~":NULL), listingGamesCallback, this );
				}
				break;

			case PeerRequest::PEERREQUEST_STOPGAMELIST:
				{
					peerStopListingGames( peer );
				}
				break;

			case PeerRequest::PEERREQUEST_STARTGAME:
				{
					peerStartGame( peer, NULL, PEER_STOP_REPORTING);
				}
				break;

			case PeerRequest::PEERREQUEST_UTMPLAYER:
				{
					if (incomingRequest.nick.length() > 0)
					{
						peerUTMPlayer( peer, incomingRequest.nick.c_str(), incomingRequest.id.c_str(), incomingRequest.options.c_str(), PEERFalse );
					}
				}
				break;

			case PeerRequest::PEERREQUEST_UTMROOM:
				{
					peerUTMRoom( peer, (incomingRequest.UTM.isStagingRoom)?StagingRoom:GroupRoom, incomingRequest.id.c_str(), incomingRequest.options.c_str(), PEERFalse );
				}
				break;

			case PeerRequest::PEERREQUEST_STARTQUICKMATCH:
				{
					m_qmInfo = incomingRequest;
					doQuickMatch( peer );
				}
				break;

			}
		}

		if (isThreadHosting && s_wantStateChangedHeartbeat)
		{
			UnsignedInt now = timeGetTime();
			if (now > s_lastStateChangedHeartbeat + s_heartbeatInterval)
			{
				s_lastStateChangedHeartbeat = now;
				s_wantStateChangedHeartbeat = FALSE;
				peerStateChanged( peer );

#ifdef DEBUG_LOGGING
				static UnsignedInt prev = 0;
				UnsignedInt now = timeGetTime();
				UnsignedInt diff = now - prev;
				prev = now;
#endif
				STATECHANGED_LOG(("peerStateChanged() at time %d (difference of %d ms)", now, diff));
			}
		}

		// update the network
		PEERBool isConnected = PEERTrue;
		isConnected = peerIsConnected( peer );
		if ( isConnected == PEERTrue )
		{
			if (qr2Sock != INVALID_SOCKET)
			{
				// check hosting activity
				checkQR2Queries( peer, qr2Sock );
			}
			peerThink( peer );
		}

		// end our timeslice
		Switch_Thread();
	}

	DEBUG_LOG(("voluntarily ending peer thread %d", running));
	peerShutdown( peer );

	} catch ( ... ) {
		DEBUG_CRASH(("Exception in peer thread!"));

		try {
			PeerResponse resp;
			resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
			resp.discon.reason = DISCONNECT_LOSTCON;
			TheGameSpyPeerMessageQueue->addResponse(resp);
		}
		catch (...)
		{
		}
	}
}

static void qmProfileIDCallback( PEER peer, PEERBool success, const char *nick, int profileID, void *param )
{
	Int *i = (Int *)param;
	if (!i || !success || !nick)
		return;

	*i = profileID;
}

static Int matchbotProfileID = 0;
void quickmatchEnumPlayersCallback( PEER peer, PEERBool success, RoomType roomType, int index, const char * nick, int flags, void * param )
{
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t || !success || nick == NULL || nick[0] == '\0')
	{
		t->sawEndOfEnumPlayers();
		return;
	}

	Int id = 0;
	peerGetPlayerProfileID(peer, nick, qmProfileIDCallback, &id, PEERTrue);
	DEBUG_LOG(("Saw player %s with id %d (looking for %d)", nick, id, matchbotProfileID));
	if (id == matchbotProfileID)
	{
		t->sawMatchbot(nick);
	}

	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERJOIN;
	resp.nick = nick;
	resp.player.roomType = roomType;
	resp.player.IP = 0;

	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void PeerThreadClass::handleQMMatch(PEER peer, Int mapIndex, Int seed,
																		char *playerName[MAX_SLOTS],
																		char *playerIP[MAX_SLOTS],
																		char *playerSide[MAX_SLOTS],
																		char *playerColor[MAX_SLOTS],
																		char *playerNAT[MAX_SLOTS])
{
	if (m_qmStatus == QM_WORKING)
	{
		m_qmStatus = QM_MATCHED;
		peerLeaveRoom(peer, GroupRoom, "");

		Int i=0;
		for (; i<MAX_SLOTS; ++i)
		{
			if (playerName[i] && stricmp(playerName[i], m_loginName.c_str()))
			{
				peerMessagePlayer( peer, playerName[i], "We're matched!", NormalMessage );
			}
		}

		PeerResponse resp;
		resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
		resp.qmStatus.status = QM_MATCHED;
		for (i=0; i<MAX_SLOTS; ++i)
		{
			if (playerName[i])
			{
				resp.stagingRoomPlayerNames[i] = playerName[i];
				resp.qmStatus.IP[i] = atoi(playerIP[i]);
				resp.qmStatus.side[i] = atoi(playerSide[i]);
				resp.qmStatus.color[i] = atoi(playerColor[i]);
				resp.qmStatus.nat[i] = atoi(playerNAT[i]);
			}
			else
			{
				resp.stagingRoomPlayerNames[i] = "";
				resp.qmStatus.IP[i] = 0;
				resp.qmStatus.side[i] = 0;
				resp.qmStatus.color[i] = 0;
				resp.qmStatus.nat[i] = 0;
			}
		}
		resp.qmStatus.seed = seed;
		resp.qmStatus.mapIdx = mapIndex;
		TheGameSpyPeerMessageQueue->addResponse(resp);
	}
}

void PeerThreadClass::doQuickMatch( PEER peer )
{
	m_qmStatus = QM_JOININGQMCHANNEL;
	Bool done = false;
	matchbotProfileID = m_qmInfo.QM.botID;
	setQMGroupRoom( m_qmInfo.QM.roomID );
	m_sawMatchbot = false;
	updateBuddyStatus( BUDDY_MATCHING );
	while (!done && running)
	{
		if (!peerIsConnected( peer ))
		{
			done = true;
		}
		else
		{
			// update the network
			peerThink( peer );

			// end our timeslice
			Switch_Thread();

			PeerRequest incomingRequest;
			if (TheGameSpyPeerMessageQueue->getRequest(incomingRequest))
			{
				switch (incomingRequest.peerRequestType)
				{
				case PeerRequest::PEERREQUEST_WIDENQUICKMATCHSEARCH:
					{
						if (m_qmStatus != QM_IDLE && m_qmStatus != QM_STOPPED && m_sawMatchbot)
						{
							peerMessagePlayer( peer, m_matchbotName.c_str(), "\\WIDEN", NormalMessage );
						}
					}
					break;
				case PeerRequest::PEERREQUEST_STOPQUICKMATCH:
					{
						m_qmStatus = QM_STOPPED;
						peerLeaveRoom(peer, GroupRoom, "");
						done = true;
					}
					break;
				case PeerRequest::PEERREQUEST_LOGOUT:
					{
						m_qmStatus = QM_STOPPED;
						peerLeaveRoom(peer, GroupRoom, "");
						done = true;
					}
					break;
				case PeerRequest::PEERREQUEST_LEAVEGROUPROOM:
					{
						m_qmStatus = QM_STOPPED;
						peerLeaveRoom(peer, GroupRoom, "");
						done = true;
					}
					break;
				case PeerRequest::PEERREQUEST_UTMPLAYER:
					{
						peerUTMPlayer( peer, incomingRequest.nick.c_str(), incomingRequest.id.c_str(), incomingRequest.options.c_str(), PEERFalse );
					}
					break;
				default:
					{
						DEBUG_CRASH(("Unanticipated request %d to peer thread!", incomingRequest.peerRequestType));
					}
					break;
				}
			}

			if (!done)
			{
				// do the next bit of QM
				switch (m_qmStatus)
				{
				case QM_JOININGQMCHANNEL:
					{
						PeerResponse resp;
						resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
						resp.qmStatus.status = QM_JOININGQMCHANNEL;
						TheGameSpyPeerMessageQueue->addResponse(resp);

						m_groupRoomID = m_qmGroupRoom;
						peerLeaveRoom( peer, GroupRoom, NULL );
						peerLeaveRoom( peer, StagingRoom, NULL ); m_isHosting = false;
						m_localRoomID = m_groupRoomID;
						m_roomJoined = false;
						DEBUG_LOG(("Requesting to join room %d in thread %X", m_localRoomID, this));
						peerJoinGroupRoom( peer, m_localRoomID, joinRoomCallback, (void *)this, PEERTrue );
						if (m_roomJoined)
						{
							resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
							resp.qmStatus.status = QM_LOOKINGFORBOT;
							TheGameSpyPeerMessageQueue->addResponse(resp);

							m_qmStatus = QM_LOOKINGFORBOT;
							m_sawMatchbot = false;
							m_sawEndOfEnumPlayers = false;
							peerEnumPlayers( peer, GroupRoom, quickmatchEnumPlayersCallback, this );
						}
						else
						{
							resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
							resp.qmStatus.status = QM_COULDNOTFINDBOT;
							TheGameSpyPeerMessageQueue->addResponse(resp);
							done = true;
							m_qmStatus = QM_STOPPED;
						}
					}
					break;
				case QM_LOOKINGFORBOT:
					{
						if (m_sawEndOfEnumPlayers)
						{
							if (m_sawMatchbot)
							{
								char buf[64];
								std::string msg = "\\CINFO";
								snprintf(buf, 64, "\\Widen\\%d", m_qmInfo.QM.widenTime);
								msg.append(buf);
								snprintf(buf, 64, "\\LadID\\%d", m_qmInfo.QM.ladderID);
								msg.append(buf);
								snprintf(buf, 64, "\\LadPass\\%d", m_qmInfo.QM.ladderPassCRC);
								msg.append(buf);
								snprintf(buf, 64, "\\PointsMin\\%d", m_qmInfo.QM.minPointPercentage);
								msg.append(buf);
								snprintf(buf, 64, "\\PointsMax\\%d", m_qmInfo.QM.maxPointPercentage);
								msg.append(buf);
								snprintf(buf, 64, "\\Points\\%d", m_qmInfo.QM.points);
								msg.append(buf);
								snprintf(buf, 64, "\\Discons\\%d", m_qmInfo.QM.discons);
								msg.append(buf);
								snprintf(buf, 64, "\\DisconMax\\%d", m_qmInfo.QM.maxDiscons);
								msg.append(buf);
								snprintf(buf, 64, "\\NumPlayers\\%d", m_qmInfo.QM.numPlayers);
								msg.append(buf);
								snprintf(buf, 64, "\\Pings\\%s", m_qmInfo.QM.pings);
								msg.append(buf);
								snprintf(buf, 64, "\\IP\\%d", ntohl(peerGetLocalIP(peer)));// not ntohl(localIP), as we need EXTERNAL address for proper NAT negotiation!
								msg.append(buf);
								snprintf(buf, 64, "\\Side\\%d", m_qmInfo.QM.side);
								msg.append(buf);
								snprintf(buf, 64, "\\Color\\%d", m_qmInfo.QM.color);
								msg.append(buf);
								snprintf(buf, 64, "\\NAT\\%d", m_qmInfo.QM.NAT);
								msg.append(buf);
								snprintf(buf, 64, "\\EXE\\%d", m_qmInfo.QM.exeCRC);
								msg.append(buf);
								snprintf(buf, 64, "\\INI\\%d", m_qmInfo.QM.iniCRC);
								msg.append(buf);
								buf[0] = 0;
								msg.append("\\Maps\\");
								for (size_t i=0; i<m_qmInfo.qmMaps.size(); ++i)
								{
									if (m_qmInfo.qmMaps[i])
										msg.append("1");
									else
										msg.append("0");
								}
								DEBUG_LOG(("Sending QM options of [%s] to %s", msg.c_str(), m_matchbotName.c_str()));
								peerMessagePlayer( peer, m_matchbotName.c_str(), msg.c_str(), NormalMessage );
								m_qmStatus = QM_WORKING;
								PeerResponse resp;
								resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
								resp.qmStatus.status = QM_SENTINFO;
								TheGameSpyPeerMessageQueue->addResponse(resp);
							}
							else
							{
								// no QM bot.  Bail.
								PeerResponse resp;
								resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
								resp.qmStatus.status = QM_COULDNOTFINDBOT;
								TheGameSpyPeerMessageQueue->addResponse(resp);

								m_qmStatus = QM_STOPPED;
								peerLeaveRoom(peer, GroupRoom, "");
								done = true;
							}
						}
					}
					break;
				case QM_WORKING:
					{
					}
					break;
				case QM_MATCHED:
					{
						// leave QM channel, and clean up.  Our work here is done.
						peerLeaveRoom( peer, GroupRoom, NULL );
						peerLeaveRoom( peer, StagingRoom, NULL ); m_isHosting = false;

						m_qmStatus = QM_STOPPED;
						peerLeaveRoom(peer, GroupRoom, "");
						done = true;
					}
					break;
				case QM_INCHANNEL:
					{
					}
					break;
				case QM_NEGOTIATINGFIREWALLS:
					{
					}
					break;
				case QM_STARTINGGAME:
					{
					}
					break;
				case QM_COULDNOTFINDCHANNEL:
					{
					}
					break;
				case QM_COULDNOTNEGOTIATEFIREWALLS:
					{
					}
					break;
				}
			}
		}
	}
	updateBuddyStatus( BUDDY_ONLINE );
}

static void getPlayerProfileIDCallback(PEER peer,  PEERBool success,  const char * nick,  int profileID,  void * param)
{
	if (success && param != NULL)
	{
		*((Int *)param) = profileID;
	}
}

static void stagingRoomPlayerEnum( PEER peer, PEERBool success, RoomType roomType, int index, const char * nick, int flags, void * param )
{
	DEBUG_LOG(("Enum: success=%d, index=%d, nick=%s, flags=%d", success, index, nick, flags));
	if (!nick || !success)
		return;

	Int id = 0;
	peerGetPlayerProfileID(peer, nick, getPlayerProfileIDCallback, &id, PEERTrue);
	DEBUG_ASSERTCRASH(id != 0, ("Failed to fetch player ID!"));

	PeerResponse *resp = (PeerResponse *)param;
	if (flags & PEER_FLAG_OP)
	{
		resp->joinStagingRoom.isHostPresent = TRUE;
	}
	if (index < MAX_SLOTS)
	{
		resp->stagingRoomPlayerNames[index] = nick;
	}

	if (id)
	{
		PSRequest req;
		req.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
		req.player.id = id;
		TheGameSpyPSMessageQueue->addRequest(req);
	}
}

static void joinRoomCallback(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param)
{
	DEBUG_LOG(("JoinRoomCallback: success==%d, result==%d", success, result));
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
		return;
	DEBUG_LOG(("Room id was %d from thread %X", t->getLocalRoomID(), t));
	DEBUG_LOG(("Current staging server name is [%ls]", t->getLocalStagingServerName().c_str()));
	DEBUG_LOG(("Room type is %d (GroupRoom=%d, StagingRoom=%d, TitleRoom=%d)", roomType, GroupRoom, StagingRoom, TitleRoom));

#ifdef USE_BROADCAST_KEYS
	if (success)
	{
		t->pushStatsToRoom(peer);
		t->getStatsFromRoom(peer, roomType);
	}
#endif // USE_BROADCAST_KEYS

	switch (roomType)
	{
		case GroupRoom:
			{
#ifdef USE_BROADCAST_KEYS
				t->clearPlayerStats(GroupRoom);
#endif // USE_BROADCAST_KEYS
				PeerResponse resp;
				resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINGROUPROOM;
				resp.joinGroupRoom.id = t->getLocalRoomID();
				resp.joinGroupRoom.ok = success;
				TheGameSpyPeerMessageQueue->addResponse(resp);
				t->roomJoined(success == PEERTrue);
				DEBUG_LOG(("Entered group room %d, qm is %d", t->getLocalRoomID(), t->getQMGroupRoom()));
				if ((!t->getQMGroupRoom()) || (t->getQMGroupRoom() != t->getLocalRoomID()))
				{
					DEBUG_LOG(("Updating buddy status"));
					updateBuddyStatus( BUDDY_LOBBY, t->getLocalRoomID() );
				}
			}
			break;
		case StagingRoom:
			{
#ifdef USE_BROADCAST_KEYS
				t->clearPlayerStats(StagingRoom);
#endif // USE_BROADCAST_KEYS
				PeerResponse resp;
				resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINSTAGINGROOM;
				resp.joinStagingRoom.id = t->getLocalRoomID();
				resp.joinStagingRoom.ok = success;
				resp.joinStagingRoom.result = result;
				if (success)
				{
					DEBUG_LOG(("joinRoomCallback() - game name is now '%ls'", t->getLocalStagingServerName().c_str()));
					updateBuddyStatus( BUDDY_STAGING, 0, WideCharStringToMultiByte(t->getLocalStagingServerName().c_str()) );
				}

				resp.joinStagingRoom.isHostPresent = FALSE;
				DEBUG_LOG(("Enum of staging room players"));
				peerEnumPlayers(peer, StagingRoom, stagingRoomPlayerEnum, &resp);
				DEBUG_LOG(("Host %s present", (resp.joinStagingRoom.isHostPresent)?"is":"is not"));

				TheGameSpyPeerMessageQueue->addResponse(resp);
			}
			break;
	}
}

// Gets called once for each group room when listing group rooms.
// After this has been called for each group room, it will be
// called one more time with groupID==0 and name==NULL.
/////////////////////////////////////////////////////////////////
static void listGroupRoomsCallback(PEER peer, PEERBool success,
														int groupID, SBServer server,
														const char * name, int numWaiting,
														int maxWaiting, int numGames,
														int numPlaying, void * param)
{
	DEBUG_LOG(("listGroupRoomsCallback, success=%d, server=%X, groupID=%d", success, server, groupID));
#ifdef SERVER_DEBUGGING
	CheckServers(peer);
#endif // SERVER_DEBUGGING
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t)
	{
		DEBUG_LOG(("No thread!  Bailing!"));
		return;
	}

	if (success)
	{
		DEBUG_LOG(("Saw group room of %d (%s) at address %X %X", groupID, name, server, (server)?server->keyvals:0));
		PeerResponse resp;
		resp.peerResponseType = PeerResponse::PEERRESPONSE_GROUPROOM;
		resp.groupRoom.id = groupID;
		resp.groupRoom.numWaiting = numWaiting;
		resp.groupRoom.maxWaiting = maxWaiting;
		resp.groupRoom.numGames = numGames;
		resp.groupRoom.numPlaying = numPlaying;
		if (name)
		{
			resp.groupRoomName = name;
			//t->setQMGroupRoom(groupID);
		}
		TheGameSpyPeerMessageQueue->addResponse(resp);
#ifdef SERVER_DEBUGGING
		CheckServers(peer);
		DEBUG_LOG((""));
#endif // SERVER_DEBUGGING
	}
	else
	{
		DEBUG_LOG(("Failure!"));
	}
}

void PeerThreadClass::connectCallback( PEER peer, PEERBool success )
{
	PeerResponse resp;
	if(!success)
	{
		//updateBuddyStatus( BUDDY_OFFLINE );
		resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
		resp.discon.reason = DISCONNECT_COULDNOTCONNECT;
		TheGameSpyPeerMessageQueue->addResponse(resp);
		return;
	}

	updateBuddyStatus( BUDDY_ONLINE );

	m_isConnected = true;
	DEBUG_LOG(("Connected as profile %d (%s)", m_profileID, m_loginName.c_str()));
	resp.peerResponseType = PeerResponse::PEERRESPONSE_LOGIN;
	resp.player.profileID = m_profileID;
	resp.nick = m_loginName;
	GetLocalChatConnectionAddress("peerchat.gamespy.com", 6667, localIP);
	chatSetLocalIP(localIP);
	resp.player.internalIP = ntohl(localIP);
	resp.player.externalIP = ntohl(peerGetLocalIP(peer));
	TheGameSpyPeerMessageQueue->addResponse(resp);

	PSRequest psReq;
	psReq.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
	psReq.player.id = m_profileID;
	psReq.nick = m_originalName;
	psReq.email = m_email;
	psReq.password = m_password;
	TheGameSpyPSMessageQueue->addRequest(psReq);

#ifdef SERVER_DEBUGGING
	DEBUG_LOG(("Before peerListGroupRooms()"));
	CheckServers(peer);
#endif // SERVER_DEBUGGING
	peerListGroupRooms( peer, NULL, listGroupRoomsCallback, this, PEERTrue );
#ifdef SERVER_DEBUGGING
	DEBUG_LOG(("After peerListGroupRooms()"));
	CheckServers(peer);
#endif // SERVER_DEBUGGING
}

void PeerThreadClass::nickErrorCallback( PEER peer, Int type, const char *nick )
{
	if(type == PEER_IN_USE)
	{
		Int len = strlen(nick);
		std::string nickStr = nick;
		Int newVal = 0;
		if (nick[len-1] == '}' && nick[len-3] == '{' && isdigit(nick[len-2]))
		{
			newVal = nick[len-2] - '0' + 1;
			nickStr.erase(len-3, 3);
		}

		DEBUG_LOG(("Nickname taken: was %s, new val = %d, new nick = %s", nick, newVal, nickStr.c_str()));

		if (newVal < 10)
		{
			nickStr.append("{");
			char tmp[2];
			tmp[0] = '0'+newVal;
			tmp[1] = '\0';
			nickStr.append(tmp);
			nickStr.append("}");
			// Retry the connect with a similar nick.
			m_loginName = nickStr;
			peerRetryWithNick(peer, nickStr.c_str());
		}
		else
		{
			PeerResponse resp;
			resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
			resp.discon.reason = DISCONNECT_NICKTAKEN;
			TheGameSpyPeerMessageQueue->addResponse(resp);

			// Cancel the connect.
			peerRetryWithNick(peer, NULL);
		}
	}
	else
	{
		PeerResponse resp;
		resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
		resp.discon.reason = DISCONNECT_BADNICK;
		TheGameSpyPeerMessageQueue->addResponse(resp);

		// Cancel the connect.
		peerRetryWithNick(peer, NULL);
	}
}

void disconnectedCallback(PEER peer, const char * reason, void * param)
{
	DEBUG_LOG(("disconnectedCallback(): reason was '%s'", reason));
	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (t)
		t->markAsDisconnected();
	//updateBuddyStatus( BUDDY_OFFLINE );
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
	resp.discon.reason = DISCONNECT_LOSTCON;
	SerialAuthResult res = TheGameSpyPeerMessageQueue->getSerialAuthResult();
	switch (res)
	{
		case SERIAL_NONEXISTENT:
			resp.discon.reason = DISCONNECT_SERIAL_NOT_PRESENT;
			break;
		case SERIAL_AUTHFAILED:
			resp.discon.reason = DISCONNECT_SERIAL_INVALID;
			break;
		case SERIAL_BANNED:
			resp.discon.reason = DISCONNECT_SERIAL_BANNED;
			break;
	}
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void roomMessageCallback(PEER peer, RoomType roomType, const char * nick, const char * message, MessageType messageType, void * param)
{
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_MESSAGE;
	resp.nick = nick;
	resp.text = MultiByteToWideCharSingleLine(message);
	resp.message.isPrivate = FALSE;
	resp.message.isAction = (messageType == ActionMessage);
	TheGameSpyPeerMessageQueue->addResponse(resp);
	DEBUG_LOG(("Saw text [%hs] (%ls) %d chars Orig was %s (%d chars)", nick, resp.text.c_str(), resp.text.length(), message, strlen(message)));

	UnsignedInt IP;
	peerGetPlayerInfoNoWait(peer, nick, &IP, &resp.message.profileID);

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (t && (t->getQMStatus() != QM_IDLE && t->getQMStatus() != QM_STOPPED))
	{
		if (resp.message.profileID == matchbotProfileID)
		{
			char *lastStr = NULL;
			char *cmd = strtok_r((char *)message, " ", &lastStr);
			if ( cmd && strcmp(cmd, "MBOT:POOLSIZE") == 0 )
			{
				Int poolSize = 0;

				while (1)
				{
					char *poolStr = strtok_r(NULL, " ", &lastStr);
					char *sizeStr = strtok_r(NULL, " ", &lastStr);
					if (poolStr && sizeStr)
					{
						Int pool = atoi(poolStr);
						Int size = atoi(sizeStr);
						if (pool == t->getQMLadder())
						{
							poolSize = size;
							break;
						}
					}
					else
						break;
				}

				PeerResponse resp;
				resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
				resp.qmStatus.status = QM_POOLSIZE;
				resp.qmStatus.poolSize = poolSize;
				TheGameSpyPeerMessageQueue->addResponse(resp);
			}
		}
	}
}

void gameStartedCallback( PEER peer, SBServer server, const char *message, void *param )
{
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_GAMESTART;
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void playerMessageCallback(PEER peer, const char * nick, const char * message, MessageType messageType, void * param)
{
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_MESSAGE;
	resp.nick = nick;
	resp.text = MultiByteToWideCharSingleLine(message);
	resp.message.isPrivate = TRUE;
	resp.message.isAction = (messageType == ActionMessage);
	UnsignedInt IP;
	peerGetPlayerInfoNoWait(peer, nick, &IP, &resp.message.profileID);
	TheGameSpyPeerMessageQueue->addResponse(resp);


	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (t && (t->getQMStatus() != QM_IDLE && t->getQMStatus() != QM_STOPPED))
	{
		if (resp.message.isPrivate && resp.message.profileID == matchbotProfileID)
		{
			char *lastStr = NULL;
			char *cmd = strtok_r((char *)message, " ", &lastStr);
			if ( cmd && strcmp(cmd, "MBOT:MATCHED") == 0 )
			{
				char *mapNumStr = strtok_r(NULL, " ", &lastStr);
				char *seedStr = strtok_r(NULL, " ", &lastStr);
				char *playerStr[MAX_SLOTS];
				char *playerIPStr[MAX_SLOTS];
				char *playerSideStr[MAX_SLOTS];
				char *playerColorStr[MAX_SLOTS];
				char *playerNATStr[MAX_SLOTS];
				Int numPlayers = 0;
				for (Int i=0; i<MAX_SLOTS; ++i)
				{
					playerStr[i] = strtok_r(NULL, " ", &lastStr);
					playerIPStr[i] = strtok_r(NULL, " ", &lastStr);
					playerSideStr[i] = strtok_r(NULL, " ", &lastStr);
					playerColorStr[i] = strtok_r(NULL, " ", &lastStr);
					playerNATStr[i] = strtok_r(NULL, " ", &lastStr);
					if (playerNATStr[i])
					{
						++numPlayers;
					}
					else
					{
						playerStr[i] = NULL;
						playerIPStr[i] = NULL;
						playerSideStr[i] = NULL;
						playerColorStr[i] = NULL;
						playerNATStr[i] = NULL;
					}
				}

				if (numPlayers > 1)
				{
					// woohoo!  got everything needed for a match!
					DEBUG_LOG(("Saw %d-player QM match: map index = %s, seed = %s", numPlayers, mapNumStr, seedStr));
					t->handleQMMatch(peer, atoi(mapNumStr), atoi(seedStr), playerStr, playerIPStr, playerSideStr, playerColorStr, playerNATStr);
				}
			}
			else if ( cmd && strcmp(cmd, "MBOT:WORKING") == 0 )
			{
				Int poolSize = 0;
				char *poolStr = strtok_r(NULL, " ", &lastStr);
				if (poolStr)
					poolSize = atoi(poolStr);
				PeerResponse resp;
				resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
				resp.qmStatus.status = QM_WORKING;
				resp.qmStatus.poolSize = poolSize;
				TheGameSpyPeerMessageQueue->addResponse(resp);
			}
			else if ( cmd && strcmp(cmd, "MBOT:WIDENINGSEARCH") == 0 )
			{
				PeerResponse resp;
				resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
				resp.qmStatus.status = QM_WIDENINGSEARCH;
				TheGameSpyPeerMessageQueue->addResponse(resp);
			}
		}
	}
}

void roomUTMCallback(PEER peer, RoomType roomType, const char * nick, const char * command, const char * parameters, PEERBool authenticated, void * param)
{
	DEBUG_LOG(("roomUTMCallback: %s says %s = [%s]", nick, command, parameters));
	if (roomType != StagingRoom)
		return;
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_ROOMUTM;
	resp.nick = nick;
	resp.command = command;
	resp.commandOptions = parameters;
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void playerUTMCallback(PEER peer, const char * nick, const char * command, const char * parameters, PEERBool authenticated, void * param)
{
	DEBUG_LOG(("playerUTMCallback: %s says %s = [%s]", nick, command, parameters));
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERUTM;
	resp.nick = nick;
	resp.command = command;
	resp.commandOptions = parameters;
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

static void getPlayerInfo(PeerThreadClass *t, PEER peer, const char *nick, Int& id, UnsignedInt& IP,
													std::string& locale, Int& wins, Int& losses, Int& rankPoints, Int& side, Int& preorder,
													RoomType roomType, Int& flags)
{
	if (!t || !nick)
		return;
	peerGetPlayerInfoNoWait(peer, nick, &IP, &id);
#ifdef USE_BROADCAST_KEYS
	//locale.printf
	Int localeIndex = t->lookupStatForPlayer(roomType, nick, "b_locale");
	AsciiString tmp;
	tmp.format("%d", localeIndex);
	locale = tmp.str();

	wins = t->lookupStatForPlayer(roomType, nick, "b_wins");
	losses = t->lookupStatForPlayer(roomType, nick, "b_losses");
	rankPoints = t->lookupStatForPlayer(roomType, nick, "b_points");
	side = t->lookupStatForPlayer(roomType, nick, "b_side");
	preorder = t->lookupStatForPlayer(roomType, nick, "b_pre");
#else // USE_BROADCAST_KEYS
	const char *s;
	s = peerGetGlobalWatchKey(peer, nick, "locale");
	locale = (s)?s:"";
	s = peerGetGlobalWatchKey(peer, nick, "wins");
	wins = atoi((s)?s:"");
	s = peerGetGlobalWatchKey(peer, nick, "losses");
	losses = atoi((s)?s:"");
	s = peerGetGlobalWatchKey(peer, nick, "points");
	rankPoints = atoi((s)?s:"");
	s = peerGetGlobalWatchKey(peer, nick, "side");
	side = atoi((s)?s:"");
	s = peerGetGlobalWatchKey(peer, nick, "pre");
	preorder = atoi((s)?s:"");
#endif // USE_BROADCAST_KEYS
	flags = 0;
	peerGetPlayerFlags(peer, nick, roomType, &flags);
	DEBUG_LOG(("getPlayerInfo(%d) - %s has locale %s, wins:%d, losses:%d, rankPoints:%d, side:%d, preorder:%d",
		id, nick, locale.c_str(), wins, losses, rankPoints, side, preorder));
}

static void roomKeyChangedCallback(PEER peer, RoomType roomType, const char *nick, const char *key, const char *val, void *param)
{
#ifdef USE_BROADCAST_KEYS
	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t || !nick || !key || !val)
	{
		DEBUG_ASSERTCRASH(nick && strcmp(nick,"(END)")==0, ("roomKeyChangedCallback bad values = nick:%X:%s, key:%X:%s, val:%X:%s", nick, nick, key, key, val, val));
		return;
	}

#ifdef DEBUG_LOGGING
	if (strcmp(key, "username") && strcmp(key, "b_flags"))
	{
		DEBUG_LOG(("roomKeyChangedCallback() - %s set %s=%s", nick, key, val));
	}
#endif

	t->trackStatsForPlayer(roomType, nick, key, val);

	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERINFO;
	resp.nick = nick;
	resp.player.roomType = roomType;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		resp.player.roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
#endif // USE_BROADCAST_KEYS
}

#ifdef USE_BROADCAST_KEYS
void getRoomKeysCallback(PEER peer, PEERBool success, RoomType roomType, const char *nick, int num, char **keys, char **values, void *param)
{
	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t || !nick || !num || !success || !keys || !values)
	{
		DEBUG_ASSERTCRASH(!nick || strcmp(nick,"(END)")==0, ("getRoomKeysCallback bad key/value %X/%X, nick=%s", keys, values, nick));
		return;
	}

	for (Int i=0; i<num; ++i)
	{
		t->trackStatsForPlayer(roomType, nick, keys[i], values[i]);
	}

	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERINFO;
	resp.nick = nick;
	resp.player.roomType = roomType;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		resp.player.roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
}
#endif // USE_BROADCAST_KEYS

static void globalKeyChangedCallback(PEER peer, const char *nick, const char *key, const char *val, void *param)
{
	if (!nick)
		return;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERINFO;
	resp.nick = nick;
	resp.player.roomType = t->getCurrentGroupRoom()?GroupRoom:StagingRoom;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		resp.player.roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void playerJoinedCallback(PEER peer, RoomType roomType, const char * nick, void * param)
{
	if (!nick)
		return;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERJOIN;
	resp.nick = nick;
	resp.player.roomType = roomType;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

void playerLeftCallback(PEER peer, RoomType roomType, const char * nick, const char * reason, void * param)
{
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERLEFT;
	resp.nick = nick;
	resp.player.roomType = roomType;
	resp.player.profileID = 0;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);

	if (t->getQMStatus() != QM_IDLE && t->getQMStatus() != QM_STOPPED)
	{
		if (!stricmp(t->getQMBotName().c_str(), nick))
		{
			// matchbot left - bail
			PeerResponse resp;
			resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
			resp.qmStatus.status = QM_COULDNOTFINDBOT;
			TheGameSpyPeerMessageQueue->addResponse(resp);

			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_STOPQUICKMATCH;
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
}

void playerChangedNickCallback(PEER peer, RoomType roomType, const char * oldNick, const char * newNick, void * param)
{
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERCHANGEDNICK;
	resp.nick = newNick;
	resp.oldNick = oldNick;
	resp.player.roomType = roomType;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	getPlayerInfo(t, peer, newNick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

static void playerInfoCallback(PEER peer, RoomType roomType, const char * nick, unsigned int IP, int profileID, void * param)
{
	if (!nick)
		return;
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERINFO;
	resp.nick = nick;
	resp.player.roomType = roomType;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		roomType, resp.player.flags);
DEBUG_LOG(("**GS playerInfoCallback name=%s, local=%s", nick, resp.locale.c_str() ));
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

static void playerFlagsChangedCallback(PEER peer, RoomType roomType, const char * nick, int oldFlags, int newFlags, void * param)
{
	if (!nick)
		return;
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERCHANGEDFLAGS;
	resp.nick = nick;
	resp.player.roomType = roomType;

	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(t, ("No Peer thread!"));
	if (!t)
		return;

	getPlayerInfo(t, peer, nick, resp.player.profileID, resp.player.IP,
		resp.locale, resp.player.wins, resp.player.losses,
		resp.player.rankPoints, resp.player.side, resp.player.preorder,
		roomType, resp.player.flags);
	TheGameSpyPeerMessageQueue->addResponse(resp);
}

#ifdef DEBUG_LOGGING
/*
static void enumFunc(char *key, char *val, void *param)
{
	DEBUG_LOG(("  [%s] = [%s]", key, val));
}
*/
#endif

static void listingGamesCallback(PEER peer, PEERBool success, const char * name, SBServer server, PEERBool staging, int msg, Int percentListed, void * param)
{
	PeerThreadClass *t = (PeerThreadClass *)param;
	if (!t || !success)
		return;

#ifdef DEBUG_LOGGING
	AsciiString cmdStr = "<Unknown>";
	switch(msg)
	{
		case PEER_ADD:
			cmdStr = "PEER_ADD";
			break;
		case PEER_UPDATE:
			cmdStr = "PEER_UPDATE";
			break;
		case PEER_REMOVE:
			cmdStr = "PEER_REMOVE";
			break;
		case PEER_CLEAR:
			cmdStr = "PEER_CLEAR";
			break;
		case PEER_COMPLETE:
			cmdStr = "PEER_COMPLETE";
			break;
	}
	DEBUG_LOG(("listingGamesCallback() - doing command %s on server %X", cmdStr.str(), server));
#endif // DEBUG_LOGGING

//	PeerThreadClass *t = (PeerThreadClass *)param;
	DEBUG_ASSERTCRASH(name || msg==PEER_CLEAR || msg==PEER_COMPLETE, ("Game has no name!"));
	if (!t || !success || (!name && (msg == PEER_ADD || msg == PEER_UPDATE)))
	{
		DEBUG_LOG(("Bailing from listingGamesCallback() - success=%d, name=%X, server=%X, msg=%X", success, name, server, msg));
		return;
	}
	if (!name)
		name = "bogus";

	if (server && (msg == PEER_ADD || msg == PEER_UPDATE))
	{
		DEBUG_ASSERTCRASH(server->keyvals, ("Looking at an already-freed server for msg type %d!", msg));
		if (!server->keyvals)
		{
			msg = PEER_REMOVE;
		}
	}

	if (server && success && (msg == PEER_ADD || msg == PEER_UPDATE))
	{
		DEBUG_LOG(("Game name is '%s'", name));
		const char *newname = SBServerGetStringValue(server, "gamename", (char *)name);
#if RTS_GENERALS
		if (strcmp(newname, "ccgenerals"))
#elif RTS_ZEROHOUR
		if (strcmp(newname, "ccgenzh"))
#endif
			name = newname;
		DEBUG_LOG(("Game name is now '%s'", name));
	}

	DEBUG_LOG(("listingGamesCallback - got percent complete %d", percentListed));
	if (percentListed == 100)
	{
		if (!t->getSawCompleteGameList())
		{
			t->setSawCompleteGameList(TRUE);
			PeerResponse completeResp;
			completeResp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOMLISTCOMPLETE;
			TheGameSpyPeerMessageQueue->addResponse(completeResp);
		}
	}

	AsciiString gameName = name;
	AsciiString tmp = gameName;
	AsciiString hostName;
	tmp.nextToken(&hostName, " ");
	const char *firstSpace = gameName.find(' ');
	if(firstSpace)
	{
		gameName.set(firstSpace + 1);
		//gameName.trim();
		DEBUG_LOG(("Hostname/Gamename split leaves '%s' hosting '%s'", hostName.str(), gameName.str()));
	}
	PeerResponse resp;
	resp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOM;
	resp.stagingRoom.action = msg;
	resp.stagingRoom.isStaging = staging;
	resp.stagingRoom.percentComplete = percentListed;

	if (server && (msg == PEER_ADD || msg == PEER_UPDATE))
	{
		Bool hasPassword = (Bool)SBServerGetIntValue(server, PW_STR, FALSE);
		Bool allowObservers = (Bool)SBServerGetIntValue(server, OBS_STR, FALSE);
    Bool usesStats = (Bool)SBServerGetIntValue(server, USE_STATS_STR, TRUE);
		const char *verStr = SBServerGetStringValue(server, "gamever", "000000");
		const char *exeStr = SBServerGetStringValue(server, EXECRC_STR, "000000");
		const char *iniStr = SBServerGetStringValue(server, INICRC_STR, "000000");
		const char *ladIPStr = SBServerGetStringValue(server, LADIP_STR, "000000");
		const char *pingStr = SBServerGetStringValue(server, PINGSTR_STR, "FFFFFFFFFFFFFFFF");
		UnsignedShort ladPort = (UnsignedShort)SBServerGetIntValue(server, LADPORT_STR, 0);
		UnsignedInt verVal = strtoul(verStr, NULL, 10);
		UnsignedInt exeVal = strtoul(exeStr, NULL, 10);
		UnsignedInt iniVal = strtoul(iniStr, NULL, 10);
		resp.stagingRoom.requiresPassword = hasPassword;
		resp.stagingRoom.allowObservers = allowObservers;
    resp.stagingRoom.useStats = usesStats;
		resp.stagingRoom.version = verVal;
		resp.stagingRoom.exeCRC = exeVal;
		resp.stagingRoom.iniCRC = iniVal;
		resp.stagingServerLadderIP = ladIPStr;
		resp.stagingServerPingString = pingStr;
		resp.stagingRoom.ladderPort = ladPort;
		resp.stagingRoom.numPlayers = SBServerGetIntValue(server, NUMPLAYER_STR, 0);
		resp.stagingRoom.numObservers = SBServerGetIntValue(server, NUMOBS_STR, 0);
		resp.stagingRoom.maxPlayers = SBServerGetIntValue(server, MAXPLAYER_STR, 8);
		resp.stagingRoomMapName = SBServerGetStringValue(server, "mapname", "");
		for (Int i=0; i<MAX_SLOTS; ++i)
		{
			resp.stagingRoomPlayerNames[i] = SBServerGetPlayerStringValue(server, i, NAME__STR, "");
			resp.stagingRoom.wins[i] = SBServerGetPlayerIntValue(server, i, WINS__STR, 0);
			resp.stagingRoom.losses[i] = SBServerGetPlayerIntValue(server, i, LOSSES__STR, 0);
			resp.stagingRoom.profileID[i] = SBServerGetPlayerIntValue(server, i, "pid", 0);
			resp.stagingRoom.color[i] = SBServerGetPlayerIntValue(server, i, COLOR__STR, 0);
			resp.stagingRoom.faction[i] = SBServerGetPlayerIntValue(server, i, FACTION__STR, 0);
#ifdef DEBUG_LOGGING
			if (resp.stagingRoomPlayerNames[i].length())
			{
				DEBUG_LOG(("Player %d raw stuff: [%s] [%d] [%d] [%d]", i, resp.stagingRoomPlayerNames[i].c_str(), resp.stagingRoom.wins[i], resp.stagingRoom.losses[i], resp.stagingRoom.profileID[i]));
			}
#endif
		}
		if (resp.stagingRoomPlayerNames[0].empty())
		{
			resp.stagingRoomPlayerNames[0] = hostName.str();
		}
		DEBUG_ASSERTCRASH(resp.stagingRoomPlayerNames[0].empty() == false, ("No host!"));
		DEBUG_LOG(("Raw stuff: [%s] [%s] [%s] [%d] [%d] [%d]", verStr, exeStr, iniStr, hasPassword, allowObservers, usesStats));
		DEBUG_LOG(("Raw stuff: [%s] [%s] [%d]", pingStr, ladIPStr, ladPort));
		DEBUG_LOG(("Saw game with stuff %s %d %X %X %X %s", resp.stagingRoomMapName.c_str(), hasPassword, verVal, exeVal, iniVal, SBServerGetStringValue(server, "password", "missing")));
#ifdef PING_TEST
	PING_LOG(("%s", pingStr));
#endif
	}

	if (msg == PEER_ADD || msg == PEER_UPDATE)
	{
		if (!resp.stagingRoom.exeCRC || !resp.stagingRoom.iniCRC)
		{
			if (SBServerHasBasicKeys(server))
			{
				DEBUG_LOG(("Server %x does not have basic keys", server));
				return;
			}
			else
			{
				DEBUG_LOG(("Server %x has basic keys, yet has no info", server));
			}
			if (msg == PEER_UPDATE)
			{
				PeerRequest req;
				req.peerRequestType = PeerRequest::PEERREQUEST_GETEXTENDEDSTAGINGROOMINFO;
				req.stagingRoom.id = t->findServer( server );
				DEBUG_LOG(("Add/update a 0/0 server %X (%d, %s) - requesting full update to see if that helps.",
					server, resp.stagingRoom.id, gameName.str()));
				TheGameSpyPeerMessageQueue->addRequest(req);
			}
			return; // don't actually try to list it.
		}
	}

	switch (msg)
	{
		case PEER_CLEAR:
			t->clearServers();
			break;
		case PEER_ADD:
		case PEER_UPDATE:
			resp.stagingRoom.id = t->findServer( server );
			DEBUG_LOG(("Add/update on server %X (%d, %s)", server, resp.stagingRoom.id, gameName.str()));
			resp.stagingServerName = MultiByteToWideCharSingleLine( gameName.str() );
			DEBUG_LOG(("Server had basic=%d, full=%d", SBServerHasBasicKeys(server), SBServerHasFullKeys(server)));
#ifdef DEBUG_LOGGING
			//SBServerEnumKeys(server, enumFunc, NULL);
#endif
			break;
		case PEER_REMOVE:
			DEBUG_LOG(("Removing server %X (%d)", server, resp.stagingRoom.id));
			resp.stagingRoom.id = t->removeServerFromMap( server );
			break;
	}

	TheGameSpyPeerMessageQueue->addResponse(resp);
}

//-------------------------------------------------------------------------

