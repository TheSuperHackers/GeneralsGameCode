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

#include "global.h"
#include "matcher.h"
#include "encrypt.h"
#include "timezone.h"
#include "debug.h"

#ifdef _WINDOWS
#define usleep(x) Sleep((x)/100000)
#endif

MatcherClass::MatcherClass(void)
{
	m_lastRotation = 0;
	m_baseNick = "matcher";
	m_joinSuccess = false;
	done = 0;

	m_rotateLogs = false;
	quiet = false;
}

Wstring MatcherClass::getString(const Wstring& key)
{
	Wstring res;
	Global.GetString(key, res);
	return res;
}

void logIt(const char *Txt)
{
	// intentionally crash if we can't open it
	FILE *fp = fopen("logIt.txt", "w");
	fputs(Txt, fp);
	fclose(fp);
}

void MatcherClass::readLoop(void)
{
	int delay = -1;
	Global.config.getInt("ROTATEDELAY", delay);
	DBGMSG("ROTATEDELAY: " << delay);

	do
	{
		static time_t lastLogTime = 0;
		time_t now = time(NULL);
		if (now > lastLogTime + 300)
		{
			lastLogTime = now;
			INFMSG("still here" << endl);
		}

		logIt("peerThink\n");
		peerThink(m_peer);

		logIt("peerIsConnected\n");
		if (peerIsConnected(m_peer))
		{
			logIt("checkMatches()\n");
			checkMatches();
			logIt("checkMatches() done\n");
		}
		else
			done = true;

		msleep(1);

		// rotate logs if it's time
		if (delay != -1)
		{
#ifdef _UNIX
			Xtime xtime;
			time_t curtime;
			curtime = time(NULL);
			// get the number of seconds that have passed since midnight
			// of the current day.
			curtime -= TimezoneOffset();
			time_t timeofday = curtime % (delay);
			if ((timeofday > 0) && (timeofday <= 300))
			{
				rotateOutput();
				rotateParanoid();
			}
#endif
		}
	}
	while (!done);
	DBGMSG("Bailing out of readLoop!" << endl);
	INFMSG("Bailing out of readLoop!" << endl);
	ERRMSG("Bailing out of readLoop!" << endl);
}

/////////////////////////////////////////////////////////////////////////////

static void DisconnectedCallback ( PEER peer, const char * reason, void * param)
{
	DBGMSG("Disconnected: " << reason);
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handleDisconnect( reason );
}

static void RoomMessageCallback ( PEER peer, RoomType roomType, const char * nick, const char * message, MessageType messageType, void * param)
{
	DBGMSG("(PUBLIC) " << nick << ": " << message);
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handleRoomMessage( nick, message, messageType );
}

static void PlayerMessageCallback ( PEER peer, const char * nick, const char * message, MessageType messageType, void * param)
{
	DBGMSG("(PRIVATE) " << nick << ": " << message);
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handlePlayerMessage( nick, message, messageType );
}

static void PlayerJoinedCallback ( PEER peer, RoomType roomType, const char * nick, void * param)
{
	DBGMSG(nick << " joined the room");
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handlePlayerJoined( nick );
}

static void PlayerLeftCallback ( PEER peer, RoomType roomType, const char * nick, const char * reason, void * param)
{
	DBGMSG(nick << " left the room");
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handlePlayerLeft( nick );
}

static void PlayerChangedNickCallback ( PEER peer, RoomType roomType, const char * oldNick, const char * newNick, void * param)
{
	INFMSG(oldNick << " changed nicks to " << newNick);
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handlePlayerChangedNick( oldNick, newNick );
}

static void EnumPlayersCallback ( PEER peer, PEERBool success, RoomType roomType, int index, const char * nick, int flags, void * param)
{
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handlePlayerEnum( success == PEERTrue, index, nick, flags);
}

static int s_groupID = 0;
static void ListGroupRoomsCallback ( PEER peer, PEERBool success, int groupID, SBServer server, const char * name, int numWaiting, int maxWaiting, int numGames, int numPlaying, void * param)
{
	if (success && name && !strcasecmp(name, "QuickMatch"))
	{
		s_groupID = groupID;
	}
}

static void ConnectCallback ( PEER peer, PEERBool success, int failureReason, void * param)
{
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handleConnect( success == PEERTrue );
}

static void JoinCallback ( PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void * param)
{
	MatcherClass *matcher = (MatcherClass *)param;
	if (matcher)
		matcher->handleJoin( success == PEERTrue );
}

static void NickErrorCallback ( PEER peer, int type, const char * badNick, int numSuggestedNicks, const const char ** suggestedNicks, void * param)
{
	ERRMSG("Nick error with " << badNick);

	if(type == PEER_IN_USE)
	{
		int len = strlen(badNick);
		std::string nickStr = badNick;
		int newVal = 0;
		if (badNick[len-1] == '}' && badNick[len-3] == '{' && isdigit(badNick[len-2]))
		{
			newVal = badNick[len-2] - '0' + 1;
			nickStr.erase(len-3, 3);
		}

		nickStr.append("{");
		char tmp[2];
		tmp[0] = '0'+newVal;
		tmp[1] = '\0';
		nickStr.append(tmp);
		nickStr.append("}");
		DBGMSG("Nickname taken: was "<<badNick<<", new val = "<<newVal<<", new nick = "<<nickStr.c_str());

		if (newVal < 10)
		{
			// Retry the connect with a similar nick.
			peerRetryWithNick(peer, nickStr.c_str());
		}
		else
		{
			// Cancel the connect.
			peerRetryWithNick(peer, NULL);
			MatcherClass *matcher = (MatcherClass *)param;
			if (matcher)
				matcher->handleNickError( badNick );
		}
	}
	else
	{
		// Cancel the connect.
		peerRetryWithNick(peer, NULL);
		MatcherClass *matcher = (MatcherClass *)param;
		if (matcher)
			matcher->handleNickError( badNick );
	}
}

/////////////////////////////////////////////////////////////////////////////

void callbackEach( CHAT chat, CHATBool success, int index, const char *channel,
                   const char *topic, int numUsers, void *param )
{
	DEBUG_LOG(("Chat channel success: %d", success));
	if (!success)
	{
		return;
	}
	DEBUG_LOG(("Channel[%d]: %s (%s), %d users",
	           index, channel, topic, numUsers));
}

void callbackAll( CHAT chat, CHATBool success, int numChannels, const char **channels,
                  const char **topics, int *numUsers, void *param )
{
	DEBUG_LOG(("Chat channels success: %d", success));
	if (!success)
	{
		return;
	}

	DEBUG_LOG(("%d channels found", numChannels));

	for (int i=0; i<numChannels; ++i)
	{
		DEBUG_LOG(("Channel[%d]: %s (%s), %d users",
		           i, channels[i], topics[i], numUsers[i]));
	}
}

void MatcherClass::handleConnect( bool success )
{
	m_connectSuccess = success;

	//DEBUG_LOG(("Enumerating chat channels"));
	//chatEnumChannels( peerGetChat(m_peer), "", callbackEach, callbackAll, NULL, CHATTrue );
	//DEBUG_LOG(("Done enumerating chat channels"));
}

void MatcherClass::handleGroupRoomList( bool success, int groupID, const char *name )
{
}

void MatcherClass::handleJoin( bool success )
{
	m_joinSuccess = success;
	if (m_joinSuccess)
	{
		DBGMSG("Joined room - listing players");
		peerEnumPlayers(m_peer, GroupRoom, EnumPlayersCallback, this);
	}
}

void MatcherClass::handleNickError( const char *badNick )
{
	exit(1);
}

static void AuthenticateCDKeyCallback
(
    PEER peer,
    int result,
    const char * message,
    void * param
)
{
	bool *val = (bool *)param;
	if (val)
	{
		*val = (result >= 1);
	}
}

void MatcherClass::connectAndLoop(void)
{
	// Game-specific initializations, if neccessary
	init();

	// Check for possible quit from init()-based self-tests
	if (done)
		return ;

	// Defaults.
	////////////
	Wstring title = "gmtest";
	Wstring secretKey = "HA6zkS";
	Wstring serialNo = "";
	m_profileID = 0;
	Global.config.getString("Nick", m_baseNick, "LOGIN");
	DBGMSG("base nick is " << m_baseNick.get());
	m_baseNick.toLower();
	Global.config.getString("Title", title, "LOGIN");
	Global.config.getString("SecretKey", secretKey, "LOGIN");
	Global.config.getInt("ProfileID", m_profileID, "LOGIN");
	Global.config.getString("CDKey", serialNo, "LOGIN");
	PEERCallbacks callbacks;
	PEERBool pingRooms[NumRooms];
	PEERBool crossPingRooms[NumRooms];

	// Setup the callbacks.
	///////////////////////
	memset(&callbacks, 0, sizeof(PEERCallbacks));
	callbacks.disconnected = DisconnectedCallback;
	callbacks.playerChangedNick = PlayerChangedNickCallback;
	callbacks.playerJoined = PlayerJoinedCallback;
	callbacks.playerLeft = PlayerLeftCallback;
	callbacks.roomMessage = RoomMessageCallback;
	callbacks.playerMessage = PlayerMessageCallback;
	callbacks.param = this;

	// Init.
	////////
	m_peer = peerInitialize(&callbacks);
	if(!m_peer)
	{
		ERRMSG("Failed to init peer object" << endl);
		return;
	}

	// Ping/cross-ping in no room.
	/////////////////////////////////
	pingRooms[TitleRoom] = PEERFalse;
	pingRooms[GroupRoom] = PEERFalse;
	pingRooms[StagingRoom] = PEERFalse;
	crossPingRooms[TitleRoom] = PEERFalse;
	crossPingRooms[GroupRoom] = PEERFalse;
	crossPingRooms[StagingRoom] = PEERFalse;

	// Set the title.
	/////////////////
	if(!peerSetTitle(m_peer, title.get(), secretKey.get(), title.get(), secretKey.get(), 0, 30, PEERTrue, pingRooms, crossPingRooms))
	{
		peerShutdown(m_peer);
		ERRMSG("Failed to set the title" << endl);
		return;
	}

	// Connect.
	///////////
	m_connectSuccess = false;
	m_nick = m_baseNick.get();
	peerConnect(m_peer, m_baseNick.get(), m_profileID, NickErrorCallback, ConnectCallback, this, PEERTrue);
	if(!m_connectSuccess)
	{
		peerShutdown(m_peer);
		ERRMSG("Failed to connect" << endl);
		return;
	}

	bool cdOk = false;
	peerAuthenticateCDKey(m_peer, serialNo.get(), AuthenticateCDKeyCallback, &cdOk, PEERTrue);
	if (!cdOk)
	{
		peerShutdown(m_peer);
		ERRMSG("Failed to auth CDKey " << serialNo.get() << endl);
		return;
	}

	m_groupID = 0;
	peerListGroupRooms(m_peer, NULL, ListGroupRoomsCallback, &m_groupID, PEERTrue);
	m_groupID = s_groupID;

	DBGMSG("QuickMatch room is " << m_groupID);

	// Join the title room.
	///////////////////////
	peerJoinGroupRoom(m_peer, m_groupID, JoinCallback, this, PEERTrue);
	if(!m_joinSuccess)
	{
		peerDisconnect(m_peer);
		peerShutdown(m_peer);
		ERRMSG("Failed to join the title room" << endl);
		return;
	}

	// Connected, so lets do our thing
	readLoop();

	peerDisconnect(m_peer);
	peerShutdown(m_peer);
}

