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

// FILE: GameSpyPersistentStorage.cpp //////////////////////////////////////////////////////
// GameSpy Persistent Storage callbacks, utils, etc
// Author: Matthew D. Campbell, March 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "gamespy/gstats/gpersist.h"

#include "GameClient/Shell.h"
#include "GameClient/MessageBox.h"
#include "GameNetwork/GameSpy.h"
#include "GameNetwork/GameSpyGP.h"
#include "GameNetwork/GameSpyPersistentStorage.h"
#include "GameNetwork/GameSpyThread.h"

static Bool isProfileAuthorized = false;

static Bool gameSpyInitPersistentStorageConnection( void );
static void getPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, char *data, int len, void *instance);
static void setPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, void *instance);


class GameSpyPlayerInfo : public GameSpyPlayerInfoInterface
{
public:
	GameSpyPlayerInfo() { m_locale.clear(); m_wins = m_losses = m_operationCount = 0; m_shouldDisconnect = false; }
	virtual ~GameSpyPlayerInfo() { reset(); }

	virtual void init( void ) { m_locale.clear(); m_wins = m_losses = m_operationCount = 0; queueDisconnect(); };
	virtual void reset( void ) { m_locale.clear(); m_wins = m_losses = m_operationCount = 0; queueDisconnect(); };
	virtual void update( void );

	virtual AsciiString getLocale( void ) { return m_locale; }
	virtual Int getWins( void ) { return m_wins; }
	virtual Int getLosses( void ) { return m_losses; }

	virtual void setLocale( AsciiString locale, Bool setOnServer );
	virtual void setWins( Int wins, Bool setOnServer );
	virtual void setLosses( Int losses, Bool setOnServer );

	virtual void readFromServer( void );
	virtual void threadReadFromServer( void );
	virtual void threadSetLocale( AsciiString val );
	virtual void threadSetWins  ( AsciiString val );
	virtual void threadSetLosses( AsciiString val );

	void queueDisconnect( void ) { 	MutexClass::LockClass m(TheGameSpyMutex); if (IsStatsConnected()) m_shouldDisconnect = true; else m_shouldDisconnect = false; }

private:
	void setValue( AsciiString key, AsciiString val, Bool setOnServer );

	AsciiString m_locale;
	Int m_wins;
	Int m_losses;
	Int m_operationCount;
	Bool m_shouldDisconnect;
};

void GameSpyPlayerInfo::update( void )
{
	if (IsStatsConnected())
	{
		if (m_shouldDisconnect)
		{
			DEBUG_LOG(("Persistent Storage close"));
			CloseStatsConnection();
		}
		else
		{
			PersistThink();
		}
	}
}

void GameSpyPlayerInfo::readFromServer( void )
{
	TheGameSpyThread->queueReadPersistentStatsFromServer();
}

void GameSpyPlayerInfo::threadReadFromServer( void )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	if (gameSpyInitPersistentStorageConnection())
	{
		// get persistent info
		m_operationCount++;
		DEBUG_LOG(("GameSpyPlayerInfo::readFromServer() operation count = %d", m_operationCount));
		GetPersistDataValues(0, TheGameSpyChat->getProfileID(), pd_public_rw, 0, "\\locale\\wins\\losses", getPersistentDataCallback, &m_operationCount);
	}
	else
	{
		//TheGameSpyThread->setNextShellScreen("Menus/WOLWelcomeMenu.wnd");
		//TheShell->pop();
		//TheShell->push("Menus/WOLWelcomeMenu.wnd");
	}
}

void GameSpyPlayerInfo::setLocale( AsciiString locale, Bool setOnServer )
{
	m_locale = locale;

	if (!TheGameSpyChat->getProfileID() || !setOnServer)
		return;

	setValue("locale", m_locale, setOnServer);
}

void GameSpyPlayerInfo::setWins( Int wins, Bool setOnServer )
{
	m_wins = wins;

	if (!TheGameSpyChat->getProfileID() || !setOnServer)
		return;

	AsciiString winStr;
	winStr.format("%d", wins);

	setValue("wins", winStr, setOnServer);
}

void GameSpyPlayerInfo::setLosses( Int losses, Bool setOnServer )
{
	m_losses = losses;

	if (!TheGameSpyChat->getProfileID() || !setOnServer)
		return;

	AsciiString lossesStr;
	lossesStr.format("%d", losses);

	setValue("losses", lossesStr, setOnServer);
}

void GameSpyPlayerInfo::setValue( AsciiString key, AsciiString val, Bool setOnServer )
{
	if (!setOnServer)
		return;

	if (key == "locale")
		TheGameSpyThread->queueUpdateLocale(val);
	else if (key == "wins")
		TheGameSpyThread->queueUpdateWins(val);
	else if (key == "losses")
		TheGameSpyThread->queueUpdateLosses(val);
}

void GameSpyPlayerInfo::threadSetLocale( AsciiString val )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	if (!gameSpyInitPersistentStorageConnection())
		return;

	// set locale info
	AsciiString key = "locale";
	AsciiString str;
	str.format("\\%s\\%s", key.str(), val.str());
	char *writable = strdup(str.str());
	m_operationCount++;
	DEBUG_LOG(("GameSpyPlayerInfo::set%s() operation count = %d", key.str(), m_operationCount));
	SetPersistDataValues(0, TheGameSpyChat->getProfileID(), pd_public_rw, 0, writable, setPersistentDataCallback, &m_operationCount);
	free(writable);
}

void GameSpyPlayerInfo::threadSetWins( AsciiString val )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	if (!gameSpyInitPersistentStorageConnection())
		return;

	// set win info
	AsciiString key = "wins";
	AsciiString str;
	str.format("\\%s\\%s", key.str(), val.str());
	char *writable = strdup(str.str());
	m_operationCount++;
	DEBUG_LOG(("GameSpyPlayerInfo::set%s() operation count = %d", key.str(), m_operationCount));
	SetPersistDataValues(0, TheGameSpyChat->getProfileID(), pd_public_rw, 0, writable, setPersistentDataCallback, &m_operationCount);
	free(writable);
}

void GameSpyPlayerInfo::threadSetLosses( AsciiString val )
{
	MutexClass::LockClass m(TheGameSpyMutex);
	if (!gameSpyInitPersistentStorageConnection())
		return;

	// set loss info
	AsciiString key = "losses";
	AsciiString str;
	str.format("\\%s\\%s", key.str(), val.str());
	char *writable = strdup(str.str());
	m_operationCount++;
	DEBUG_LOG(("GameSpyPlayerInfo::set%s() operation count = %d", key.str(), m_operationCount));
	SetPersistDataValues(0, TheGameSpyChat->getProfileID(), pd_public_rw, 0, writable, setPersistentDataCallback, &m_operationCount);
	free(writable);
}

GameSpyPlayerInfoInterface *TheGameSpyPlayerInfo = NULL;

GameSpyPlayerInfoInterface *createGameSpyPlayerInfo( void )
{
	return NEW GameSpyPlayerInfo;
}









static void persAuthCallback(int localid, int profileid, int authenticated, char *errmsg, void *instance)
{
	DEBUG_LOG(("Auth callback: localid: %d profileid: %d auth: %d err: %s",localid, profileid, authenticated, errmsg));
	isProfileAuthorized = (authenticated != 0);
}

static void getPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, char *data, int len, void *instance)
{
	DEBUG_LOG(("Data get callback: localid: %d profileid: %d success: %d len: %d data: %s",localid, profileid, success, len, data));

	if (!TheGameSpyPlayerInfo)
	{
		//TheGameSpyThread->setNextShellScreen("Menus/WOLWelcomeMenu.wnd");
		//TheShell->pop();
		//TheShell->push("Menus/WOLWelcomeMenu.wnd");
		return;
	}

	AsciiString str = data;
	AsciiString key, val;
	while (!str.isEmpty())
	{
		str.nextToken(&key, "\\");
		str.nextToken(&val, "\\");
		if (!key.isEmpty() && !val.isEmpty())
		{
			if (!key.compareNoCase("locale"))
			{
				TheGameSpyPlayerInfo->setLocale(val, false);
			}
			else if (!key.compareNoCase("wins"))
			{
				TheGameSpyPlayerInfo->setWins(atoi(val.str()), false);
			}
			else if (!key.compareNoCase("losses"))
			{
				TheGameSpyPlayerInfo->setLosses(atoi(val.str()), false);
			}
		}
	}

	// decrement count of active operations
	Int *opCount = (Int *)instance;
	(*opCount) --;
	DEBUG_LOG(("getPersistentDataCallback() operation count = %d", (*opCount)));
	if (!*opCount)
	{
		DEBUG_LOG(("getPersistentDataCallback() queue disconnect"));
		((GameSpyPlayerInfo *)TheGameSpyPlayerInfo)->queueDisconnect();
	}

	const char *keys[3] = { "locale", "wins", "losses" };
	char valueStrings[3][20];
	char *values[3] = { valueStrings[0], valueStrings[1], valueStrings[2] };
	snprintf(values[0], 20, "%s", TheGameSpyPlayerInfo->getLocale().str());
	snprintf(values[1], 20, "%d", TheGameSpyPlayerInfo->getWins());
	snprintf(values[2], 20, "%d", TheGameSpyPlayerInfo->getLosses());
	peerSetGlobalKeys(TheGameSpyChat->getPeer(), 3, (const char **)keys, (const char **)values);
	peerSetGlobalWatchKeys(TheGameSpyChat->getPeer(), GroupRoom,   3, keys, PEERTrue);
	peerSetGlobalWatchKeys(TheGameSpyChat->getPeer(), StagingRoom, 3, keys, PEERTrue);

	// choose next screen
	if (TheGameSpyPlayerInfo->getLocale().isEmpty())
	{
		TheGameSpyThread->setShowLocaleSelect(true);
	}
}

static void setPersistentDataCallback(int localid, int profileid, persisttype_t type, int index, int success, void *instance)
{
	DEBUG_LOG(("Data save callback: localid: %d profileid: %d success: %d", localid, profileid, success));

	Int *opCount = (Int *)instance;
	(*opCount) --;
	DEBUG_LOG(("setPersistentDataCallback() operation count = %d", (*opCount)));
	if (!*opCount)
	{
		DEBUG_LOG(("setPersistentDataCallback() queue disconnect"));
		((GameSpyPlayerInfo *)TheGameSpyPlayerInfo)->queueDisconnect();
	}
}

static Bool gameSpyInitPersistentStorageConnection( void )
{
	if (IsStatsConnected())
		return true;

	isProfileAuthorized = false;
	Int result;

	/*********
	First step, set our game authentication info
	We could do:
		strcpy(gcd_gamename,"gmtest");
		strcpy(gcd_secret_key,"HA6zkS");
	...but this is more secure:
	**********/
	gcd_gamename[0]='g';gcd_gamename[1]='m';gcd_gamename[2]='t';gcd_gamename[3]='e';
	gcd_gamename[4]='s';gcd_gamename[5]='t';gcd_gamename[6]='\0';
	gcd_secret_key[0]='H';gcd_secret_key[1]='A';gcd_secret_key[2]='6';gcd_secret_key[3]='z';
	gcd_secret_key[4]='k';gcd_secret_key[5]='S';gcd_secret_key[6]='\0';

	/*********
	Next, open the stats connection. This may block for
	a 1-2 seconds, so it should be done before the actual game starts.
	**********/
	result = InitStatsConnection(0);

	if (result != GE_NOERROR)
	{
		DEBUG_LOG(("InitStatsConnection returned %d",result));
		return isProfileAuthorized;
	}

	if (TheGameSpyChat->getProfileID())
	{
		char validate[33];

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
		char *munkeeHack = strdup(TheGameSpyChat->getPassword().str()); // GenerateAuth takes a char*, not a const char* :P
		GenerateAuth(GetChallenge(NULL), munkeeHack, validate);
		free (munkeeHack);

		/************
		After we get the validation token, we pass it and the profileid of the user
		we are authenticating into PreAuthenticatePlayerPM.
		We pass the same authentication callback as for the first user, but a different
		localid this time.
		************/
		PreAuthenticatePlayerPM(0, TheGameSpyChat->getProfileID(), validate, persAuthCallback, NULL);
	}
	else
	{
		return isProfileAuthorized;
	}

	UnsignedInt timeoutTime = timeGetTime() + 5000;
	while (!isProfileAuthorized && timeGetTime() < timeoutTime && IsStatsConnected())
	{
		PersistThink();
		msleep(10);
	}

	DEBUG_LOG(("Persistent Storage connect: %d", isProfileAuthorized));
	return isProfileAuthorized;
}

