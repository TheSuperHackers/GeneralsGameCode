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

// FILE: PersistentStorageThread.cpp //////////////////////////////////////////////////////
// GameSpy Persistent Storage thread
// This thread communicates with GameSpy's persistent storage server
// and talks through a message queue with the rest of
// the game.
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/UserPreferences.h"
#include "Common/PlayerTemplate.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#include "mutex.h"
#include "thread.h"

#include "Common/StackDump.h"
#include "Common/SubsystemInterface.h"


//-------------------------------------------------------------------------

PSRequest::PSRequest()
{
	player.reset();
	requestType = PSREQUEST_READPLAYERSTATS;
	addDiscon = addDesync = FALSE;
	lastHouse = -1;
}

//-------------------------------------------------------------------------

#define DEBUG_MAP(x) for (it = stats.x.begin(); it != stats.x.end(); ++it) \
{ \
	if (it->second > 0) \
	{ \
		DEBUG_LOG(("%s(%d): %d\n", #x, it->first, it->second)); \
	} \
}

static void debugDumpPlayerStats( const PSPlayerStats& stats )
{
	DEBUG_LOG(("-----------------------------------------\n"));
	DEBUG_LOG(("Tracking player stats for player %d:\n", stats.id));
	PerGeneralMap::const_iterator it;
	DEBUG_MAP(wins);
	DEBUG_MAP(losses);
	DEBUG_MAP(games);
	DEBUG_MAP(duration);
	DEBUG_MAP(unitsKilled);
	DEBUG_MAP(unitsLost);
	DEBUG_MAP(unitsBuilt);
	DEBUG_MAP(buildingsKilled);
	DEBUG_MAP(buildingsLost);
	DEBUG_MAP(buildingsBuilt);
	DEBUG_MAP(earnings);
	DEBUG_MAP(techCaptured);
	DEBUG_MAP(discons);
	DEBUG_MAP(desyncs);
	DEBUG_MAP(surrenders);
	DEBUG_MAP(gamesOf2p);
	DEBUG_MAP(gamesOf3p);
	DEBUG_MAP(gamesOf4p);
	DEBUG_MAP(gamesOf5p);
	DEBUG_MAP(gamesOf6p);
	DEBUG_MAP(gamesOf7p);
	DEBUG_MAP(gamesOf8p);
	DEBUG_MAP(customGames);
	DEBUG_MAP(QMGames);
	
	if (stats.locale > 0)
	{
		DEBUG_LOG(("Locale: %d\n", stats.locale));
	}
	
	if (stats.gamesAsRandom > 0)
	{
		DEBUG_LOG(("gamesAsRandom: %d\n", stats.gamesAsRandom));
	}

	if (stats.options.length())
	{
		DEBUG_LOG(("Options: %s\n", stats.options.c_str()));
	}

	if (stats.systemSpec.length())
	{
		DEBUG_LOG(("systemSpec: %s\n", stats.systemSpec.c_str()));
	}

	if (stats.lastFPS > 0.0f)
	{
		DEBUG_LOG(("lastFPS: %g\n", stats.lastFPS));
	}

	if (stats.battleHonors > 0)
	{
		DEBUG_LOG(("battleHonors: %x\n", stats.battleHonors));
	}
	if (stats.challengeMedals > 0)
	{
		DEBUG_LOG(("challengeMedals: %x\n", stats.challengeMedals));
	}
	if (stats.lastGeneral >= 0)
	{
		DEBUG_LOG(("lastGeneral: %d\n", stats.lastGeneral));
	}
	if (stats.gamesInRowWithLastGeneral >= 0)
	{
		DEBUG_LOG(("gamesInRowWithLastGeneral: %d\n", stats.gamesInRowWithLastGeneral));
	}
	if (stats.builtSCUD >= 0)
	{
		DEBUG_LOG(("builtSCUD: %d\n", stats.builtSCUD));
	}
	if (stats.builtNuke >= 0)
	{
		DEBUG_LOG(("builtNuke: %d\n", stats.builtNuke));
	}
	if (stats.builtParticleCannon >= 0)
	{
		DEBUG_LOG(("builtParticleCannon: %d\n", stats.builtParticleCannon));
	}

	if (stats.winsInARow >= 0)
	{
		DEBUG_LOG(("winsInARow: %d\n", stats.winsInARow));
	}
	if (stats.maxWinsInARow >= 0)
	{
		DEBUG_LOG(("maxWinsInARow: %d\n", stats.maxWinsInARow));
	}
	if (stats.disconsInARow >= 0)
	{
		DEBUG_LOG(("disconsInARow: %d\n", stats.disconsInARow));
	}
	if (stats.maxDisconsInARow >= 0)
	{
		DEBUG_LOG(("maxDisconsInARow: %d\n", stats.maxDisconsInARow));
	}
	if (stats.lossesInARow >= 0)
	{
		DEBUG_LOG(("lossesInARow: %d\n", stats.lossesInARow));
	}
	if (stats.maxLossesInARow >= 0)
	{
		DEBUG_LOG(("maxLossesInARow: %d\n", stats.maxLossesInARow));
	}
	if (stats.desyncsInARow >= 0)
	{
		DEBUG_LOG(("desyncsInARow: %d\n", stats.desyncsInARow));
	}
	if (stats.maxDesyncsInARow >= 0)
	{
		DEBUG_LOG(("maxDesyncsInARow: %d\n", stats.maxDesyncsInARow));
	}

	if (stats.lastLadderPort >= 0)
	{
		DEBUG_LOG(("lastLadderPort: %d\n", stats.lastLadderPort));
	}

	if (stats.lastLadderHost.length())
	{
		DEBUG_LOG(("lastLadderHost: %s\n", stats.lastLadderHost.c_str()));
	}



}

//-------------------------------------------------------------------------

#define INCORPORATE_MAP(x) for (it = other.x.begin(); it != other.x.end(); ++it) \
{ \
	if (it->second > 0) \
	{ \
		x[it->first] = it->second; \
	} \
}

void PSPlayerStats::incorporate( const PSPlayerStats& other )
{
	PerGeneralMap::const_iterator it;
	INCORPORATE_MAP(wins);
	INCORPORATE_MAP(losses);
	INCORPORATE_MAP(games);
	INCORPORATE_MAP(duration);
	INCORPORATE_MAP(unitsKilled);
	INCORPORATE_MAP(unitsLost);
	INCORPORATE_MAP(unitsBuilt);
	INCORPORATE_MAP(buildingsKilled);
	INCORPORATE_MAP(buildingsLost);
	INCORPORATE_MAP(buildingsBuilt);
	INCORPORATE_MAP(earnings);
	INCORPORATE_MAP(techCaptured);

	//GS  Clear all disconnects so that we don't retain any that were
	//previously reported as 1 by updateAdditionalGameSpyDisconnections
	discons.clear();
	INCORPORATE_MAP(discons);

	INCORPORATE_MAP(desyncs);
	INCORPORATE_MAP(surrenders);
	INCORPORATE_MAP(gamesOf2p);
	INCORPORATE_MAP(gamesOf3p);
	INCORPORATE_MAP(gamesOf4p);
	INCORPORATE_MAP(gamesOf5p);
	INCORPORATE_MAP(gamesOf6p);
	INCORPORATE_MAP(gamesOf7p);
	INCORPORATE_MAP(gamesOf8p);
	INCORPORATE_MAP(customGames);
	INCORPORATE_MAP(QMGames);
	
	if (other.locale > 0)
	{
		locale = other.locale;
	}
	
	if (other.gamesAsRandom > 0)
	{
		gamesAsRandom = other.gamesAsRandom;
	}

	if (other.options.length())
	{
		options = other.options;
	}

	if (other.systemSpec.length())
	{
		systemSpec = other.systemSpec;
	}

	if (other.lastFPS > 0.0f)
	{
		lastFPS = other.lastFPS;
	}

	if (other.battleHonors > 0)
	{
		battleHonors |= other.battleHonors;
	}
	if (other.challengeMedals > 0)
	{
		challengeMedals |= other.challengeMedals;
	}
	if (other.lastGeneral >= 0)
	{
		lastGeneral = other.lastGeneral;
	}
	if (other.gamesInRowWithLastGeneral >= 0)
	{
		gamesInRowWithLastGeneral = other.gamesInRowWithLastGeneral;
	}
	if (other.builtParticleCannon >= 0)
	{
		builtParticleCannon = other.builtParticleCannon;
	}
	if (other.builtNuke >= 0)
	{
		builtNuke = other.builtNuke;
	}
	if (other.builtSCUD >= 0)
	{
		builtSCUD = other.builtSCUD;
	}
	if (other.winsInARow >= 0)
	{
		winsInARow = other.winsInARow;
	}
	if (other.maxWinsInARow >= 0)
	{
		maxWinsInARow = other.maxWinsInARow;
	}
	if (other.lossesInARow >= 0)
	{
		lossesInARow = other.lossesInARow;
	}
	if (other.maxLossesInARow >= 0)
	{
		maxLossesInARow = other.maxLossesInARow;
	}
	if (other.disconsInARow >= 0)
	{
		disconsInARow = other.disconsInARow;
	}
	if (other.maxDisconsInARow >= 0)
	{
		maxDisconsInARow = other.maxDisconsInARow;
	}
	if (other.desyncsInARow >= 0)
	{
		desyncsInARow = other.desyncsInARow;
	}
	if (other.maxDesyncsInARow >= 0)
	{
		maxDesyncsInARow = other.maxDesyncsInARow;
	}
	if (other.lastLadderPort >= 0)
	{
		lastLadderPort = other.lastLadderPort;
	}
	if (other.lastLadderHost.length())
	{
		lastLadderHost = other.lastLadderHost;
	}
}

PSPlayerStats::PSPlayerStats( const PSPlayerStats& other )
{
	incorporate(other);
	id = other.id;
	locale = other.locale;
	gamesAsRandom = other.gamesAsRandom;
	options = other.options;
	systemSpec = other.systemSpec;
	lastFPS = other.lastFPS;
	lastGeneral = other.lastGeneral;
	gamesInRowWithLastGeneral = other.gamesInRowWithLastGeneral;
	builtParticleCannon = other.builtParticleCannon;
	builtNuke = other.builtNuke;
	builtSCUD = other.builtSCUD;
	challengeMedals = other.challengeMedals;
	battleHonors = other.battleHonors;
	winsInARow = other.winsInARow;
	maxWinsInARow = other.maxWinsInARow;
	lossesInARow = other.lossesInARow;
	maxLossesInARow = other.maxLossesInARow;
	disconsInARow = other.disconsInARow;
	maxDisconsInARow = other.maxDisconsInARow;
	desyncsInARow = other.desyncsInARow;
	maxDesyncsInARow = other.maxDesyncsInARow;
	lastLadderHost = other.lastLadderHost;
	lastLadderPort = other.lastLadderPort;
}

//-------------------------------------------------------------------------

typedef std::queue<PSRequest> RequestQueue;
typedef std::queue<PSResponse> ResponseQueue;
class PSThreadClass;

class GameSpyPSMessageQueue : public GameSpyPSMessageQueueInterface
{
public:
	virtual ~GameSpyPSMessageQueue();
	GameSpyPSMessageQueue();
	virtual void startThread( void );
	virtual void endThread( void );
	virtual Bool isThreadRunning( void );

	virtual void addRequest( const PSRequest& req );
	virtual Bool getRequest( PSRequest& req );

	virtual void addResponse( const PSResponse& resp );
	virtual Bool getResponse( PSResponse& resp );

	virtual void trackPlayerStats( PSPlayerStats stats );
	virtual PSPlayerStats findPlayerStatsByID( Int id );

	PSThreadClass* getThread( void );

	Int getLocalPlayerID(void) { return m_localPlayerID; }
	void setLocalPlayerID(Int localPlayerID) { m_localPlayerID = localPlayerID; }

	std::string getEmail() { return m_email; }
	std::string getNick() { return m_nick; }
	std::string getPassword() { return m_password; }

	void setEmail(std::string email) { m_email = email; }
	void setNick(std::string nick) { m_nick = nick; }
	void setPassword(std::string password) { m_password = password; }

private:
	MutexClass m_requestMutex;
	MutexClass m_responseMutex;
	RequestQueue m_requests;
	ResponseQueue m_responses;
	PSThreadClass *m_thread;
	Int m_localPlayerID;

	std::string m_email;
	std::string m_nick;
	std::string m_password;

	std::map<Int, PSPlayerStats> m_playerStats;
};

GameSpyPSMessageQueueInterface* GameSpyPSMessageQueueInterface::createNewMessageQueue( void )
{
	return NEW GameSpyPSMessageQueue;
}

GameSpyPSMessageQueueInterface *TheGameSpyPSMessageQueue = NULL;
#define MESSAGE_QUEUE ((GameSpyPSMessageQueue *)TheGameSpyPSMessageQueue)

//-------------------------------------------------------------------------

class PSThreadClass : public ThreadClass
{

public:
	PSThreadClass() : ThreadClass() 
	{ 
		m_loginOK = m_sawLocalData = m_doneTryingToLogin = false; 
		m_opCount = 0; 
	}

	void Thread_Function();

	void persAuthCallback( Bool val ) { m_loginOK = val; m_doneTryingToLogin = true; }
	void decrOpCount( void ) { --m_opCount; }
	void incrOpCount( void ) { ++m_opCount; }
	Int getOpCount( void ) { return m_opCount; }
	Bool sawLocalPlayerData( void ) { return m_sawLocalData; }
	void gotLocalPlayerData( void ) { m_sawLocalData = TRUE; }

private:
	Bool tryConnect( void );
	Bool tryLogin( Int id, std::string nick, std::string password, std::string email );
	Bool m_loginOK;
	Bool m_doneTryingToLogin;
	Int m_opCount;
	Bool m_sawLocalData;
};


//-------------------------------------------------------------------------

GameSpyPSMessageQueue::GameSpyPSMessageQueue()
{
	m_thread = NULL;
	m_localPlayerID = 0;
}

GameSpyPSMessageQueue::~GameSpyPSMessageQueue()
{
	endThread();
}

void GameSpyPSMessageQueue::startThread( void )
{
	if (!m_thread)
	{
		m_thread = NEW PSThreadClass;
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

void GameSpyPSMessageQueue::endThread( void )
{
	if (m_thread)
		delete m_thread;
	m_thread = NULL;
}

Bool GameSpyPSMessageQueue::isThreadRunning( void )
{
	return (m_thread) ? m_thread->Is_Running() : false;
}

void GameSpyPSMessageQueue::addRequest( const PSRequest& req )
{
	MutexClass::LockClass m(m_requestMutex);
	if (m.Failed())
		return;

	m_requests.push(req);
}

Bool GameSpyPSMessageQueue::getRequest( PSRequest& req )
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

void GameSpyPSMessageQueue::addResponse( const PSResponse& resp )
{
	MutexClass::LockClass m(m_responseMutex);
	if (m.Failed())
		return;

	m_responses.push(resp);
}

Bool GameSpyPSMessageQueue::getResponse( PSResponse& resp )
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

PSThreadClass* GameSpyPSMessageQueue::getThread( void )
{
	return m_thread;
}

void GameSpyPSMessageQueue::trackPlayerStats( PSPlayerStats stats )
{
#ifdef DEBUG_LOGGING
	debugDumpPlayerStats( stats );
	DEBUG_ASSERTCRASH(stats.id != 0, ("Tracking stats with ID of 0\n"));
#endif
	PSPlayerStats newStats;
	std::map<Int, PSPlayerStats>::iterator it = m_playerStats.find(stats.id);
	if (it != m_playerStats.end())
	{
		newStats = it->second;
		newStats.incorporate(stats);
		m_playerStats[stats.id] = newStats;
	}
	else
	{
		m_playerStats[stats.id] = stats;
	}
}

PSPlayerStats GameSpyPSMessageQueue::findPlayerStatsByID( Int id )
{
	std::map<Int, PSPlayerStats>::iterator it = m_playerStats.find(id);
	if (it != m_playerStats.end())
	{
		return it->second;
	}

	PSPlayerStats empty;
	empty.id = 0;
	return empty;
}

//-------------------------------------------------------------------------

Bool PSThreadClass::tryConnect( void )
{
	Int result;

	DEBUG_LOG(("m_opCount = %d - opening connection\n", m_opCount));

	if (IsStatsConnected())
	{
		DEBUG_LOG(("connection already open!\n"));
		return true;
	}

	// this may block for 1-2 seconds (according to GS) so it's nice we're not in the UI thread :)
	result = InitStatsConnection(0);

#ifdef DEBUG_LOGGING
	static const char *retValStrings[6] = {
		"GE_NOERROR",
		"GE_NOSOCKET",
		"GE_NODNS",
		"GE_NOCONNECT",
		"GE_BUSY",
		"GE_DATAERROR"
	};
#endif // DEBUG_LOGGING

	if (result != GE_NOERROR)
	{
		DEBUG_LOG(("InitStatsConnection() returned %d (%s)\n", result, retValStrings[result]));
		return false;
	}
	else
	{
		DEBUG_LOG(("InitStatsConnection() succeeded\n"));
	}

	return true;
}

static void persAuthCallback(int localid, int profileid, int authenticated, char *errmsg, void *instance)
{
	PSThreadClass *t = (PSThreadClass *)instance;
	DEBUG_LOG(("Auth callback: localid: %d profileid: %d auth: %d err: %s\n",localid, profileid, authenticated, errmsg));
	if (t)
		t->persAuthCallback(authenticated != 0);
}

Bool PSThreadClass::tryLogin( Int id, std::string nick, std::string password, std::string email )
{
	char validate[33];
	DEBUG_LOG(("PSThreadClass::tryLogin id = %d, nick = %s, password = %s, email = %s\n", id, nick.c_str(), password.c_str(), email.c_str()));
	/***********
	We'll go ahead and start the authentication, using a Presence & Messaging SDK
	profileid / password.  To generate the new validation token, we'll need to pass
	in the password for the profile we are authenticating.
	Again, if this is done in a client/server setting, with the Persistent Storage
	access being done on the server, and the P&M SDK is used on the client, the
	server will need to send the challenge (GetChallenge(NULL)) to the client, the
	client will create the validation token using GenerateAuth, and send it
	back to the server for use in PreAuthenticatePlayerPM
	***********/
	char *munkeeHack = strdup(password.c_str()); // GenerateAuth takes a char*, not a const char* :P
	GenerateAuth(GetChallenge(NULL), munkeeHack, validate);
	free (munkeeHack);

	/************
	After we get the validation token, we pass it and the profileid of the user
	we are authenticating into PreAuthenticatePlayerPM.
	We pass the same authentication callback as for the first user, but a different
	localid this time.
	************/
	m_loginOK = false;
	m_doneTryingToLogin = false;
	PreAuthenticatePlayerPM(id, id, validate, ::persAuthCallback, this);
	while (!m_doneTryingToLogin && IsStatsConnected())
		PersistThink();
	DEBUG_LOG(("Persistant Storage Login success %d\n", m_loginOK));
	return m_loginOK;
}

static void getPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, char *data, int len, void *instance)
{
	DEBUG_LOG(("Data get callback: localid: %d profileid: %d success: %d len: %d data: %s\n",localid, profileid, success, len, data));
	PSThreadClass *t = (PSThreadClass *)instance;
	if (!t)
		return;

	t->decrOpCount();

	PSResponse resp;

	if (!success)
	{
		resp.responseType = PSResponse::PSRESPONSE_COULDNOTCONNECT;
		resp.player.id = profileid;
		TheGameSpyPSMessageQueue->addResponse(resp);
		if (!t->getOpCount() && !t->sawLocalPlayerData())
		{
			// we haven't gotten stats for ourselves - try again
			DEBUG_LOG(("Requesting retry for reading local player's stats\n"));
			PSRequest req;
			req.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
			req.player.id = MESSAGE_QUEUE->getLocalPlayerID();
			TheGameSpyPSMessageQueue->addRequest(req);
		}
		return;
	}

	if (profileid == MESSAGE_QUEUE->getLocalPlayerID())
	{
		t->gotLocalPlayerData();
		DEBUG_LOG(("getPersistentDataCallback() - got local player info\n"));

		// check if we have discons we should update on the server
		UserPreferences pref;
		AsciiString userPrefFilename;
		userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", MESSAGE_QUEUE->getLocalPlayerID());
		DEBUG_LOG(("using the file %s\n", userPrefFilename.str()));
		pref.load(userPrefFilename);
		Int addedInDesyncs2 = pref.getInt("0", 0);
		DEBUG_LOG(("addedInDesyncs2 = %d\n", addedInDesyncs2));
		if (addedInDesyncs2 < 0)
			addedInDesyncs2 = 10;
		Int addedInDesyncs3 = pref.getInt("1", 0);
		DEBUG_LOG(("addedInDesyncs3 = %d\n", addedInDesyncs3));
		if (addedInDesyncs3 < 0)
			addedInDesyncs3 = 10;
		Int addedInDesyncs4 = pref.getInt("2", 0);
		DEBUG_LOG(("addedInDesyncs4 = %d\n", addedInDesyncs4));
		if (addedInDesyncs4 < 0)
			addedInDesyncs4 = 10;
		Int addedInDiscons2 = pref.getInt("3", 0);
		DEBUG_LOG(("addedInDiscons2 = %d\n", addedInDiscons2));
		if (addedInDiscons2 < 0)
			addedInDiscons2 = 10;
		Int addedInDiscons3 = pref.getInt("4", 0);
		DEBUG_LOG(("addedInDiscons3 = %d\n", addedInDiscons3));
		if (addedInDiscons3 < 0)
			addedInDiscons3 = 10;
		Int addedInDiscons4 = pref.getInt("5", 0);
		DEBUG_LOG(("addedInDiscons4 = %d\n", addedInDiscons4));
		if (addedInDiscons4 < 0)
			addedInDiscons4 = 10;

		DEBUG_LOG(("addedInDesync=%d,%d,%d, addedInDiscon=%d,%d,%d\n",
			addedInDesyncs2, addedInDesyncs3, addedInDesyncs4,
			addedInDiscons2, addedInDiscons3, addedInDiscons4));

		if (addedInDesyncs2 || addedInDesyncs3 || addedInDesyncs4 || addedInDiscons2 || addedInDiscons3 || addedInDiscons4)
		{
			DEBUG_LOG(("We have a previous discon we can attempt to update!  Bummer...\n"));

			PSRequest req;
			req.requestType = PSRequest::PSREQUEST_UPDATEPLAYERSTATS;
			req.email = MESSAGE_QUEUE->getEmail();
			req.nick = MESSAGE_QUEUE->getNick();
			req.password = MESSAGE_QUEUE->getPassword();
			req.player = GameSpyPSMessageQueueInterface::parsePlayerKVPairs((len)?data:"");
			req.player.id = profileid;
			req.addDesync = FALSE;
			req.addDiscon = FALSE;
			req.lastHouse = 0;
			TheGameSpyPSMessageQueue->addRequest(req);
		}
	}

	resp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
	resp.player = GameSpyPSMessageQueueInterface::parsePlayerKVPairs((len)?data:"");
	resp.player.id = profileid;

	TheGameSpyPSMessageQueue->addResponse(resp);
}

static void setPersistentDataLocaleCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, void *instance)
{
	DEBUG_LOG(("Data save callback: localid: %d profileid: %d success: %d\n", localid, profileid, success));

	PSThreadClass *t = (PSThreadClass *)instance;
	if (!t)
		return;

	t->decrOpCount();
}

static void setPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, void *instance)
{
	DEBUG_LOG(("Data save callback: localid: %d profileid: %d success: %d\n", localid, profileid, success));

	PSThreadClass *t = (PSThreadClass *)instance;
	if (!t)
		return;

	if (success)
	{
		UserPreferences pref;
		AsciiString userPrefFilename;
		userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", profileid);
		DEBUG_LOG(("setPersistentDataCallback - writing stats to file %s\n", userPrefFilename.str()));
		pref.load(userPrefFilename);
		pref.clear();
		pref.write();
	}
	t->decrOpCount();
}

struct CDAuthInfo
{
	Bool success;
	Bool done;
	Int id;
};

void preAuthCDCallback(int localid, int profileid, int authenticated, char *errmsg, void *instance)
{
	DEBUG_LOG(("preAuthCDCallback(): profileid: %d auth: %d err: %s\n", profileid, authenticated, errmsg));

	CDAuthInfo *authInfo = (CDAuthInfo *)instance;
	authInfo->success = authenticated;
	authInfo->done = TRUE;
	authInfo->id = profileid;
}

static void getPreorderCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, char *data, int len, void *instance)
{
	PSThreadClass *t = (PSThreadClass *)instance;
	if (!t)
		return;

	t->decrOpCount();

	PSResponse resp;

	if (!success)
	{
		DEBUG_LOG(("Failed getPreorderCallback()\n"));
		return;
	}

	resp.responseType = PSResponse::PSRESPONSE_PREORDER;
	resp.preorder = (data && strcmp(data, "\\preorder\\1") == 0);
	DEBUG_LOG(("getPreorderCallback() - data was '%s'\n", data));

	TheGameSpyPSMessageQueue->addResponse(resp);
}

void PSThreadClass::Thread_Function()
{
	try {
	_set_se_translator( DumpExceptionInfo ); // Hook that allows stack trace.
	/*********
	First step, set our game authentication info
	We could do:
		strcpy(gcd_gamename,"ccgenerals");
		strcpy(gcd_secret_key,"h5T2f6");
	or
		strcpy(gcd_gamename,"ccgeneralsb");
		strcpy(gcd_secret_key,"g3T9s2");
	...but this is more secure:
	**********/
	/**
	gcd_gamename[0]='c';gcd_gamename[1]='c';gcd_gamename[2]='g';gcd_gamename[3]='e';
	gcd_gamename[4]='n';gcd_gamename[5]='e';gcd_gamename[6]='r';gcd_gamename[7]='a';
	gcd_gamename[8]='l';gcd_gamename[9]='s';gcd_gamename[10]='b';gcd_gamename[11]='\0';
	gcd_secret_key[0]='g';gcd_secret_key[1]='3';gcd_secret_key[2]='T';gcd_secret_key[3]='9';
	gcd_secret_key[4]='s';gcd_secret_key[5]='2';gcd_secret_key[6]='\0';
	/**/
	gcd_gamename[0]='c';gcd_gamename[1]='c';gcd_gamename[2]='g';gcd_gamename[3]='e';
	gcd_gamename[4]='n';gcd_gamename[5]='e';gcd_gamename[6]='r';gcd_gamename[7]='a';
	gcd_gamename[8]='l';gcd_gamename[9]='s';gcd_gamename[10]='\0';
	gcd_secret_key[0]='h';gcd_secret_key[1]='5';gcd_secret_key[2]='T';gcd_secret_key[3]='2';
	gcd_secret_key[4]='f';gcd_secret_key[5]='6';gcd_secret_key[6]='\0';
	/**/
	
	//strcpy(StatsServerHostname, "sdkdev.gamespy.com");

	PSRequest req;
	while ( running )
	{
		// deal with requests
		if (TheGameSpyPSMessageQueue->getRequest(req))
		{
			switch (req.requestType)
			{
			case PSRequest::PSREQUEST_SENDGAMERESTOGAMESPY:
				{
					if (tryConnect())
					{
						NewGame(0);
#ifdef DEBUG_LOGGING
						Int res = 
#endif // DEBUG_LOGGING
							SendGameSnapShot(NULL, req.results.c_str(), SNAP_FINAL);
						DEBUG_LOG(("Just sent game results - res was %d\n", res));
						FreeGame(NULL);
					}
				}
				break;
			case PSRequest::PSREQUEST_READPLAYERSTATS:
				{
					Bool initialConnection = FALSE;
					if (!MESSAGE_QUEUE->getLocalPlayerID())
					{
						MESSAGE_QUEUE->setLocalPlayerID(req.player.id); // first request is for ourselves
						MESSAGE_QUEUE->setEmail(req.email);
						MESSAGE_QUEUE->setNick(req.nick);
						MESSAGE_QUEUE->setPassword(req.password);
						DEBUG_LOG(("Setting email/nick/password = %s/%s/%s\n", req.email.c_str(), req.nick.c_str(), req.password.c_str()));
						initialConnection = TRUE;
					}
					DEBUG_LOG(("Processing PSRequest::PSREQUEST_READPLAYERSTATS\n"));
					if (tryConnect())
					{
						DEBUG_LOG(("Successful login\n"));
						incrOpCount();
						gsi_char keys[] = "";
						GetPersistDataValues(0, req.player.id, pd_public_rw, 0, keys, getPersistentDataCallback, this);
					}
					else
					{
						DEBUG_LOG(("Unsuccessful login - retry=%d\n", initialConnection));
						PSResponse resp;
						resp.responseType = PSResponse::PSRESPONSE_COULDNOTCONNECT;
						resp.player.id = req.player.id;
						TheGameSpyPSMessageQueue->addResponse(resp);
						if (initialConnection)
						{
							// we haven't gotten stats for ourselves - try again
							DEBUG_LOG(("Requesting retry for reading local player's stats\n"));
							PSRequest req;
							req.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
							req.player.id = MESSAGE_QUEUE->getLocalPlayerID();
							TheGameSpyPSMessageQueue->addRequest(req);
						}
					}
				}
				break;
			case PSRequest::PSREQUEST_UPDATEPLAYERLOCALE:
				{
					DEBUG_LOG(("Processing PSRequest::PSREQUEST_UPDATEPLAYERLOCALE\n"));
					if (tryConnect() && tryLogin(req.player.id, req.nick, req.password, req.email))
					{
						char kvbuf[256];
						sprintf(kvbuf, "\\locale\\%d", req.player.locale);
						incrOpCount();
						SetPersistDataValues(0, req.player.id, pd_public_rw, 0, kvbuf, setPersistentDataLocaleCallback, this);
					}
				}
				break;
			case PSRequest::PSREQUEST_UPDATEPLAYERSTATS:
				{
					/*
					** NOTE THAT THIS IS HIGHLY DEPENDENT ON INI ORDERING FOR THE PLAYERTEMPLATES!!!
					*/
					DEBUG_LOG(("Processing PSRequest::PSREQUEST_UPDATEPLAYERSTATS\n"));
					UserPreferences pref;
					AsciiString userPrefFilename;
					userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", MESSAGE_QUEUE->getLocalPlayerID());
					DEBUG_LOG(("using the file %s\n", userPrefFilename.str()));
					pref.load(userPrefFilename);
					Int addedInDesyncs2 = pref.getInt("0", 0);
					DEBUG_LOG(("addedInDesyncs2 = %d\n", addedInDesyncs2));
					if (addedInDesyncs2 < 0)
						addedInDesyncs2 = 10;
					Int addedInDesyncs3 = pref.getInt("1", 0);
					DEBUG_LOG(("addedInDesyncs3 = %d\n", addedInDesyncs3));
					if (addedInDesyncs3 < 0)
						addedInDesyncs3 = 10;
					Int addedInDesyncs4 = pref.getInt("2", 0);
					DEBUG_LOG(("addedInDesyncs4 = %d\n", addedInDesyncs4));
					if (addedInDesyncs4 < 0)
						addedInDesyncs4 = 10;
					Int addedInDiscons2 = pref.getInt("3", 0);
					DEBUG_LOG(("addedInDiscons2 = %d\n", addedInDiscons2));
					if (addedInDiscons2 < 0)
						addedInDiscons2 = 10;
					Int addedInDiscons3 = pref.getInt("4", 0);
					DEBUG_LOG(("addedInDiscons3 = %d\n", addedInDiscons3));
					if (addedInDiscons3 < 0)
						addedInDiscons3 = 10;
					Int addedInDiscons4 = pref.getInt("5", 0);
					DEBUG_LOG(("addedInDiscons4 = %d\n", addedInDiscons4));
					if (addedInDiscons4 < 0)
						addedInDiscons4 = 10;

					DEBUG_LOG(("req.addDesync=%d, req.addDiscon=%d, addedInDesync=%d,%d,%d, addedInDiscon=%d,%d,%d\n",
						req.addDesync, req.addDiscon, addedInDesyncs2, addedInDesyncs3, addedInDesyncs4,
						addedInDiscons2, addedInDiscons3, addedInDiscons4));

					if (req.addDesync || req.addDiscon)
					{
						AsciiString val;
						if (req.lastHouse == 2)
						{
							val.format("%d", addedInDesyncs2 + req.addDesync);
							pref["0"] = val;
							val.format("%d", addedInDiscons2 + req.addDiscon);
							pref["3"] = val;
							DEBUG_LOG(("house 2 req.addDesync || req.addDiscon: %d %d\n",
								addedInDesyncs2 + req.addDesync, addedInDiscons2 + req.addDiscon));
						}
						else if (req.lastHouse == 3)
						{
							val.format("%d", addedInDesyncs3 + req.addDesync);
							pref["1"] = val;
							val.format("%d", addedInDiscons3 + req.addDiscon);
							pref["4"] = val;
							DEBUG_LOG(("house 3 req.addDesync || req.addDiscon: %d %d\n",
								addedInDesyncs3 + req.addDesync, addedInDiscons3 + req.addDiscon));
						}
						else
						{
							val.format("%d", addedInDesyncs4 + req.addDesync);
							pref["2"] = val;
							val.format("%d", addedInDiscons4 + req.addDiscon);
							pref["5"] = val;
							DEBUG_LOG(("house 4 req.addDesync || req.addDiscon: %d %d\n",
								addedInDesyncs4 + req.addDesync, addedInDiscons4 + req.addDiscon));
						}
						pref.write();
						if (req.password.size() == 0)
							return;
					}
					if (!req.player.id)
					{
						DEBUG_LOG(("Bailing because ID is NULL!\n"));
						return;
					}
					req.player.desyncs[2] += addedInDesyncs2;
					req.player.games[2] += addedInDesyncs2;
					req.player.discons[2] += addedInDiscons2;
					req.player.games[2] += addedInDiscons2;
					req.player.desyncs[3] += addedInDesyncs3;
					req.player.games[3] += addedInDesyncs3;
					req.player.discons[3] += addedInDiscons3;
					req.player.games[3] += addedInDiscons3;
					req.player.desyncs[4] += addedInDesyncs4;
					req.player.games[4] += addedInDesyncs4;
					req.player.discons[4] += addedInDiscons4;
					req.player.games[4] += addedInDiscons4;
					DEBUG_LOG(("House2: %d/%d/%d, House3: %d/%d/%d, House4: %d/%d/%d\n",
						req.player.desyncs[2], req.player.discons[2], req.player.games[2],
						req.player.desyncs[3], req.player.discons[3], req.player.games[3],
						req.player.desyncs[4], req.player.discons[4], req.player.games[4]
						));
					if (tryConnect() && tryLogin(req.player.id, req.nick, req.password, req.email))
					{
						DEBUG_LOG(("Logged in!\n"));
						if (TheGameSpyPSMessageQueue)
							TheGameSpyPSMessageQueue->trackPlayerStats(req.player);

						char *munkeeHack = strdup(GameSpyPSMessageQueueInterface::formatPlayerKVPairs(req.player).c_str()); // GS takes a char* for some reason
						incrOpCount();
						DEBUG_LOG(("Setting values %s\n", munkeeHack));
						SetPersistDataValues(0, req.player.id, pd_public_rw, 0, munkeeHack, setPersistentDataCallback, this);
						free(munkeeHack);
					}
					else
					{
						DEBUG_LOG(("Cannot connect!\n"));
						//if (IsStatsConnected())
							//CloseStatsConnection();
					}
				}
				break;
			case PSRequest::PSREQUEST_READCDKEYSTATS:
				{
					DEBUG_LOG(("Processing PSRequest::PSREQUEST_READCDKEYSTATS\n"));
					if (tryConnect())
					{
						incrOpCount();
						CDAuthInfo cdAuthInfo;
						cdAuthInfo.done = FALSE;
						cdAuthInfo.success = FALSE;
						cdAuthInfo.id = 0;
						char cdkeyHash[33] = "";
						char validationToken[33] = "";
						char *munkeeHack = strdup(req.cdkey.c_str()); // GenerateAuth takes a char*, not a const char* :P

						GenerateAuth(GetChallenge(NULL), munkeeHack, validationToken); // validation token
						GenerateAuth("", munkeeHack, cdkeyHash); // cdkey hash

						free (munkeeHack);

						PreAuthenticatePlayerCD( 0, "preorder", cdkeyHash, validationToken, preAuthCDCallback , &cdAuthInfo);

						while (running && IsStatsConnected() && !cdAuthInfo.done)
							PersistThink();

						DEBUG_LOG(("Looking for preorder status for %d (success=%d, done=%d) from CDKey %s with hash %s\n",
							cdAuthInfo.id, cdAuthInfo.success, cdAuthInfo.done, req.cdkey.c_str(), cdkeyHash));
						if (cdAuthInfo.done && cdAuthInfo.success)
						{
							gsi_char keys[] = "\\preorder";
							GetPersistDataValues(0, cdAuthInfo.id, pd_public_ro, 0, keys, getPreorderCallback, this);
						}
						else
							decrOpCount();
					}
				}
				break;
			}
		}

		// update the network
		if (IsStatsConnected())
		{
			PersistThink();
			if (m_opCount <= 0)
			{
				DEBUG_ASSERTCRASH(m_opCount == 0, ("Negative operations pending!!!"));
				DEBUG_LOG(("m_opCount = %d - closing connection\n", m_opCount));
				CloseStatsConnection();
				m_opCount = 0;
			}
		}

		// end our timeslice
		Switch_Thread();
	}

	if (IsStatsConnected())
		CloseStatsConnection();
	} catch ( ... ) {
		DEBUG_CRASH(("Exception in storage thread!"));
	}
}

//-------------------------------------------------------------------------
PSPlayerStats::PSPlayerStats( void )
{
	reset();
}

void PSPlayerStats::reset( void )
{
	id = 0;
	locale = 0;
	gamesAsRandom = 0;
	lastFPS = 0;
	lastGeneral = 0;
	gamesInRowWithLastGeneral = 0;
	builtNuke = 0;
	builtSCUD = 0;
	builtParticleCannon = 0;
	challengeMedals = 0;
	battleHonors = 0;
	winsInARow = 0;
	maxWinsInARow = 0;
	lossesInARow = 0;
	maxLossesInARow = 0;
	disconsInARow = 0;
	maxDisconsInARow = 0;
	desyncsInARow = 0;
	maxDesyncsInARow = 0;
	lastLadderPort = 0;

	//Added By Sadullah Nader
	maxQMwinsInARow = 0;
	QMwinsInARow = 0;
	//
}

//-------------------------------------------------------------------------
#define CHECK(x) if (k == #x && generalMarker >= 0) { s.x[generalMarker] = atoi(v.c_str()); continue; }

PSPlayerStats GameSpyPSMessageQueueInterface::parsePlayerKVPairs( std::string kvPairs )
{
	PSPlayerStats s;
	kvPairs.append("\\");

	Int offset = 0;
	while (1)
	{
		Int firstMarker = kvPairs.find_first_of('\\', offset);
		if (firstMarker < 0)
			break;
		Int secondMarker = kvPairs.find_first_of('\\', firstMarker + 1);
		if (secondMarker < 0)
			break;
		Int thirdMarker = kvPairs.find_first_of('\\', secondMarker + 1);
		if (thirdMarker < 0)
			break;
		Int generalMarker = kvPairs.find_last_not_of("0123456789", secondMarker - 1);
		std::string k, v, g;
		if (generalMarker == secondMarker - 1)
		{
			k = kvPairs.substr(firstMarker + 1, secondMarker - firstMarker - 1);
			generalMarker = -1;
		}
		else
		{
			k = kvPairs.substr(firstMarker + 1, generalMarker - firstMarker);
			g = kvPairs.substr(generalMarker + 1, secondMarker - generalMarker - 1);
			generalMarker = atoi(g.c_str());
		}
		v = kvPairs.substr(secondMarker + 1, thirdMarker - secondMarker - 1);
		//DEBUG_LOG(("%d [%s] [%s]\n", generalMarker, k.c_str(), v.c_str()));
		offset = thirdMarker - 1;

		CHECK(wins);
		CHECK(losses);
		CHECK(games);
		CHECK(duration);
		CHECK(unitsKilled);
		CHECK(unitsLost);
		CHECK(unitsBuilt);
		CHECK(buildingsKilled);
		CHECK(buildingsLost);
		CHECK(buildingsBuilt);
		CHECK(earnings);
		CHECK(techCaptured);
		CHECK(discons);
		CHECK(desyncs);
		CHECK(surrenders);
		CHECK(gamesOf2p);
		CHECK(gamesOf3p);
		CHECK(gamesOf4p);
		CHECK(gamesOf5p);
		CHECK(gamesOf6p);
		CHECK(gamesOf7p);
		CHECK(gamesOf8p);
		CHECK(customGames);
		CHECK(QMGames);
		
		if (k == "locale" && generalMarker < 0)
		{
			s.locale = atoi(v.c_str());
			continue;
		}

		if (k == "random" && generalMarker < 0)
		{
			s.gamesAsRandom = atoi(v.c_str());
			continue;
		}

		if (k == "options" && generalMarker < 0)
		{
			s.options = v;
			continue;
		}

		if (k == "systemSpec" && generalMarker < 0)
		{
			s.systemSpec = v;
			continue;
		}

		if (k == "fps" && generalMarker < 0)
		{
			s.lastFPS = atof(v.c_str());
			continue;
		}
		
		if (k == "lastGeneral" && generalMarker < 0)
		{
			s.lastGeneral = atoi(v.c_str());
			continue;
		}
		if (k == "genInRow" && generalMarker < 0)
		{
			s.gamesInRowWithLastGeneral = atoi(v.c_str());
			continue;
		}
		if (k == "builtNuke" && generalMarker < 0)
		{
			s.builtNuke = atoi(v.c_str());
			continue;
		}
		if (k == "builtSCUD" && generalMarker < 0)
		{
			s.builtSCUD = atoi(v.c_str());
			continue;
		}
		if (k == "builtCannon" && generalMarker < 0)
		{
			s.builtParticleCannon = atoi(v.c_str());
			continue;
		}
		if (k == "challenge" && generalMarker < 0)
		{
			s.challengeMedals = atoi(v.c_str());
			continue;
		}
		if (k == "battle" && generalMarker < 0)
		{
			s.battleHonors = atoi(v.c_str());
			continue;
		}

		if (k == "WinRow" && generalMarker < 0)
		{
			s.winsInARow = atoi(v.c_str());
			continue;
		}
		if (k == "WinRowMax" && generalMarker < 0)
		{
			s.maxWinsInARow = atoi(v.c_str());
			continue;
		}

		if (k == "LossRow" && generalMarker < 0)
		{
			s.lossesInARow = atoi(v.c_str());
			continue;
		}
		if (k == "LossRowMax" && generalMarker < 0)
		{
			s.maxLossesInARow = atoi(v.c_str());
			continue;
		}

		if (k == "DSRow" && generalMarker < 0)
		{
			s.desyncsInARow = atoi(v.c_str());
			continue;
		}
		if (k == "DSRowMax" && generalMarker < 0)
		{
			s.maxDesyncsInARow = atoi(v.c_str());
			continue;
		}

		if (k == "DCRow" && generalMarker < 0)
		{
			s.disconsInARow = atoi(v.c_str());
			continue;
		}
		if (k == "DCRowMax" && generalMarker < 0)
		{
			s.maxDisconsInARow = atoi(v.c_str());
			continue;
		}

		if (k == "ladderPort" && generalMarker < 0)
		{
			s.lastLadderPort = atoi(v.c_str());
			continue;
		}
		if (k == "ladderHost" && generalMarker < 0)
		{
			s.lastLadderHost = v;
			continue;
		}

		//DEBUG_ASSERTCRASH(generalMarker >= 0, ("Unknown KV Pair in persistent storage: [%s] = [%s]\n", k.c_str(), v.c_str()));
		//DEBUG_ASSERTCRASH(generalMarker  < 0, ("Unknown KV Pair in persistent storage for PlayerTemplate %d: [%s] = [%s]\n", generalMarker, k.c_str(), v.c_str()));
	}

	return s;
}

#define ITERATE_OVER(x) for (it = stats.x.begin(); it != stats.x.end(); ++it) \
{ \
	if (it->second > 0) \
	{ \
		sprintf(kvbuf, "\\" #x "%d\\%d", it->first, it->second); \
		s.append(kvbuf); \
	} \
}

std::string GameSpyPSMessageQueueInterface::formatPlayerKVPairs( PSPlayerStats stats )
{
	char kvbuf[256];
	std::string s = "";
	PerGeneralMap::iterator it;

	ITERATE_OVER(wins);
	ITERATE_OVER(losses);
	ITERATE_OVER(games);
	ITERATE_OVER(duration);
	ITERATE_OVER(unitsKilled);
	ITERATE_OVER(unitsLost);
	ITERATE_OVER(unitsBuilt);
	ITERATE_OVER(buildingsKilled);
	ITERATE_OVER(buildingsLost);
	ITERATE_OVER(buildingsBuilt);
	ITERATE_OVER(earnings);
	ITERATE_OVER(techCaptured);
	
	//GS  Report all disconnects, even if zero, because might have been 
	//previously reported as 1 by updateAdditionalGameSpyDisconnections
//	ITERATE_OVER(discons);
	for (Int ptIdx = 0; ptIdx < ThePlayerTemplateStore->getPlayerTemplateCount(); ++ptIdx)
	{
//		const PlayerTemplate* pTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(ptIdx);
//		const GeneralPersona* pGeneral = TheChallengeGenerals->getGeneralByTemplateName(pTemplate->getName());
//		BOOL isReported = pGeneral ? pGeneral->isStartingEnabled() : FALSE;
//		if( !isReported )
//			continue;  //don't report unplayable templates (observer, boss, etc.)

		sprintf(kvbuf, "\\discons%d\\%d", ptIdx, stats.discons[ptIdx]);
		s.append(kvbuf);
	}

	ITERATE_OVER(desyncs);
	ITERATE_OVER(surrenders);
	ITERATE_OVER(gamesOf2p);
	ITERATE_OVER(gamesOf3p);
	ITERATE_OVER(gamesOf4p);
	ITERATE_OVER(gamesOf5p);
	ITERATE_OVER(gamesOf6p);
	ITERATE_OVER(gamesOf7p);
	ITERATE_OVER(gamesOf8p);
	ITERATE_OVER(customGames);
	ITERATE_OVER(QMGames);
	
	if (stats.locale > 0)
	{
		sprintf(kvbuf, "\\locale\\%d", stats.locale);
		s.append(kvbuf);
	}

	if (stats.gamesAsRandom > 0)
	{
		sprintf(kvbuf, "\\random\\%d", stats.gamesAsRandom);
		s.append(kvbuf);
	}

	if (stats.options.length())
	{
		snprintf(kvbuf, 256, "\\options\\%s", stats.options.c_str());
		s.append(kvbuf);
	}

	if (stats.systemSpec.length())
	{
		snprintf(kvbuf, 256, "\\systemSpec\\%s", stats.systemSpec.c_str());
		s.append(kvbuf);
	}

	if (stats.lastFPS > 0.0f)
	{
		sprintf(kvbuf, "\\fps\\%g", stats.lastFPS);
		s.append(kvbuf);
	}
	if (stats.lastGeneral >= 0)
	{
		sprintf(kvbuf, "\\lastGeneral\\%d", stats.lastGeneral);
		s.append(kvbuf);
	}
	if (stats.gamesInRowWithLastGeneral >= 0)
	{
		sprintf(kvbuf, "\\genInRow\\%d", stats.gamesInRowWithLastGeneral);
		s.append(kvbuf);
	}
	if (stats.builtParticleCannon >= 0)
	{
		sprintf(kvbuf, "\\builtCannon\\%d", stats.builtParticleCannon);
		s.append(kvbuf);
	}
	if (stats.builtNuke >= 0)
	{
		sprintf(kvbuf, "\\builtNuke\\%d", stats.builtNuke);
		s.append(kvbuf);
	}
	if (stats.builtSCUD >= 0)
	{
		sprintf(kvbuf, "\\builtSCUD\\%d", stats.builtSCUD);
		s.append(kvbuf);
	}
	if (stats.challengeMedals > 0)
	{
		sprintf(kvbuf, "\\challenge\\%d", stats.challengeMedals);
		s.append(kvbuf);
	}
	if (stats.battleHonors > 0)
	{
		sprintf(kvbuf, "\\battle\\%d", stats.battleHonors);
		s.append(kvbuf);
	}

	//if (stats.winsInARow > 0)
	{
		sprintf(kvbuf, "\\WinRow\\%d", stats.winsInARow);
		s.append(kvbuf);
	}
	if (stats.maxWinsInARow > 0)
	{
		sprintf(kvbuf, "\\WinRowMax\\%d", stats.maxWinsInARow);
		s.append(kvbuf);
	}

	//if (stats.lossesInARow > 0)
	{
		sprintf(kvbuf, "\\LossRow\\%d", stats.lossesInARow);
		s.append(kvbuf);
	}
	if (stats.maxLossesInARow > 0)
	{
		sprintf(kvbuf, "\\LossRowMax\\%d", stats.maxLossesInARow);
		s.append(kvbuf);
	}

	//if (stats.disconsInARow > 0)
	{
		sprintf(kvbuf, "\\DCRow\\%d", stats.disconsInARow);
		s.append(kvbuf);
	}
	if (stats.maxDisconsInARow > 0)
	{
		sprintf(kvbuf, "\\DCRowMax\\%d", stats.maxDisconsInARow);
		s.append(kvbuf);
	}

	//if (stats.desyncsInARow > 0)
	{
		sprintf(kvbuf, "\\DSRow\\%d", stats.desyncsInARow);
		s.append(kvbuf);
	}
	if (stats.maxDesyncsInARow > 0)
	{
		sprintf(kvbuf, "\\DSRowMax\\%d", stats.maxDesyncsInARow);
		s.append(kvbuf);
	}

	if (stats.lastLadderPort > 0)
	{
		sprintf(kvbuf, "\\ladderPort\\%d", stats.lastLadderPort);
		s.append(kvbuf);
	}
	if (stats.lastLadderHost.length())
	{
		snprintf(kvbuf, 256, "\\ladderHost\\%s", stats.lastLadderHost.c_str());
		s.append(kvbuf);
	}

	DEBUG_LOG(("Formatted persistent values as '%s'\n", s.c_str()));
	return s;
}

//-------------------------------------------------------------------------
