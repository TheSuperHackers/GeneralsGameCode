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

// FILE: GameSpyGP.cpp //////////////////////////////////////////////////////
// GameSpy GP callbacks, utils, etc
// Author: Matthew D. Campbell, February 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/GameText.h"
#include "GameNetwork/GameSpy.h"
#include "GameNetwork/GameSpyGP.h"
#include "GameNetwork/GameSpyOverlay.h"

GPConnection TheGPConnectionObj;
GPConnection *TheGPConnection = &TheGPConnectionObj;
GPProfile GameSpyLocalProfile = 0;
char GameSpyProfilePassword[64];

void GPRecvBuddyMessageCallback(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyMessageCallback: message from %d is %s", arg->profile, arg->message));

	//gpGetInfo(pconn, arg->profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, nullptr);
	//printf("MESSAGE (%d): %s: %s\n", msgCount,whois, arg->message);
}

static void buddyTryReconnect( void )
{
	TheGameSpyChat->reconnectProfile();
}

void GPErrorCallback(GPConnection * pconnection, GPErrorArg * arg, void * param)
{
	DEBUG_LOG(("GPErrorCallback"));

	AsciiString errorCodeString;
	AsciiString resultString;

	#define RESULT(x) case x: resultString = #x; break;
	switch(arg->result)
	{
		RESULT(GP_NO_ERROR)
		RESULT(GP_MEMORY_ERROR)
		RESULT(GP_PARAMETER_ERROR)
		RESULT(GP_NETWORK_ERROR)
		RESULT(GP_SERVER_ERROR)
	default:
		resultString = "Unknown result!";
	}
	#undef RESULT

	#define ERRORCODE(x) case x: errorCodeString = #x; break;
	switch(arg->errorCode)
	{
		ERRORCODE(GP_GENERAL)
		ERRORCODE(GP_PARSE)
		ERRORCODE(GP_NOT_LOGGED_IN)
		ERRORCODE(GP_BAD_SESSKEY)
		ERRORCODE(GP_DATABASE)
		ERRORCODE(GP_NETWORK)
		ERRORCODE(GP_FORCED_DISCONNECT)
		ERRORCODE(GP_CONNECTION_CLOSED)
		ERRORCODE(GP_LOGIN)
		ERRORCODE(GP_LOGIN_TIMEOUT)
		ERRORCODE(GP_LOGIN_BAD_NICK)
		ERRORCODE(GP_LOGIN_BAD_EMAIL)
		ERRORCODE(GP_LOGIN_BAD_PASSWORD)
		ERRORCODE(GP_LOGIN_BAD_PROFILE)
		ERRORCODE(GP_LOGIN_PROFILE_DELETED)
		ERRORCODE(GP_LOGIN_CONNECTION_FAILED)
		ERRORCODE(GP_LOGIN_SERVER_AUTH_FAILED)
		ERRORCODE(GP_NEWUSER)
		ERRORCODE(GP_NEWUSER_BAD_NICK)
		ERRORCODE(GP_NEWUSER_BAD_PASSWORD)
		ERRORCODE(GP_UPDATEUI)
		ERRORCODE(GP_UPDATEUI_BAD_EMAIL)
		ERRORCODE(GP_NEWPROFILE)
		ERRORCODE(GP_NEWPROFILE_BAD_NICK)
		ERRORCODE(GP_NEWPROFILE_BAD_OLD_NICK)
		ERRORCODE(GP_UPDATEPRO)
		ERRORCODE(GP_UPDATEPRO_BAD_NICK)
		ERRORCODE(GP_ADDBUDDY)
		ERRORCODE(GP_ADDBUDDY_BAD_FROM)
		ERRORCODE(GP_ADDBUDDY_BAD_NEW)
		ERRORCODE(GP_ADDBUDDY_ALREADY_BUDDY)
		ERRORCODE(GP_AUTHADD)
		ERRORCODE(GP_AUTHADD_BAD_FROM)
		ERRORCODE(GP_AUTHADD_BAD_SIG)
		ERRORCODE(GP_STATUS)
		ERRORCODE(GP_BM)
		ERRORCODE(GP_BM_NOT_BUDDY)
		ERRORCODE(GP_GETPROFILE)
		ERRORCODE(GP_GETPROFILE_BAD_PROFILE)
		ERRORCODE(GP_DELBUDDY)
		ERRORCODE(GP_DELBUDDY_NOT_BUDDY)
		ERRORCODE(GP_DELPROFILE)
		ERRORCODE(GP_DELPROFILE_LAST_PROFILE)
		ERRORCODE(GP_SEARCH)
		ERRORCODE(GP_SEARCH_CONNECTION_FAILED)
	default:
		errorCodeString = "Unknown error code!";
	}
	#undef ERRORCODE

	if(arg->fatal)
	{
		DEBUG_LOG(( "-----------"));
		DEBUG_LOG(( "GP FATAL ERROR"));
		DEBUG_LOG(( "-----------"));

		// if we're still connected to the chat server, tell the user.  He can always hit the buddy
		// button to try reconnecting.  Oh yes, also hide the buddy popup.
		GameSpyCloseOverlay(GSOVERLAY_BUDDY);
		if (TheGameSpyChat->isConnected())
		{
			GSMessageBoxYesNo(TheGameText->fetch("GUI:GPErrorTitle"), TheGameText->fetch("GUI:GPDisconnected"), buddyTryReconnect, nullptr);
		}
	}
	else
	{
		DEBUG_LOG(( "-----"));
		DEBUG_LOG(( "GP ERROR"));
		DEBUG_LOG(( "-----"));
	}
	DEBUG_LOG(( "RESULT: %s (%d)", resultString.str(), arg->result));
	DEBUG_LOG(( "ERROR CODE: %s (0x%X)", errorCodeString.str(), arg->errorCode));
	DEBUG_LOG(( "ERROR STRING: %s", arg->errorString));
}

void GPRecvBuddyStatusCallback(GPConnection * connection, GPRecvBuddyStatusArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyStatusCallback: info on %d is in %d", arg->profile, arg->index));

	//GameSpyUpdateBuddyOverlay();
}

void GPRecvBuddyRequestCallback(GPConnection * connection, GPRecvBuddyRequestArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyRequestCallback: %d wants to be our buddy because '%s'", arg->profile, arg->reason));
}
