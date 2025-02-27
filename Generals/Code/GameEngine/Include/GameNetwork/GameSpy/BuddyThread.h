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

// FILE: BuddyThread.h //////////////////////////////////////////////////////
// Generals GameSpy BuddyList thread class interface
// Author: Matthew D. Campbell, June 2002

#pragma once

#include "gamespy/gp/gp.h"

#define MAX_BUDDY_CHAT_LEN 128

// this class encapsulates a request for the buddy thread
class BuddyRequest
{
public:
	enum
	{
		BUDDYREQUEST_LOGIN,				// attempt to login
		BUDDYREQUEST_RELOGIN,			// log in after being disconnected
		BUDDYREQUEST_LOGOUT,			// log out if connected
		BUDDYREQUEST_MESSAGE,
		BUDDYREQUEST_LOGINNEW,		// attempt to create a new nick and login
		//BUDDYREQUEST_DELETELOGIN,
		BUDDYREQUEST_ADDBUDDY,		// add someone to your buddy list
		BUDDYREQUEST_DELBUDDY,		// delete someone from your buddy list
		BUDDYREQUEST_OKADD,				// allow someone to add you to their buddy list
		BUDDYREQUEST_DENYADD,			// don't allow someone to add you to their buddy list
		BUDDYREQUEST_SETSTATUS,		// Set our status
		BUDDYREQUEST_DELETEACCT,	// Delete our account
	} buddyRequestType;

	union
	{
		struct
		{
			GPProfile recipient;
			WideChar text[MAX_BUDDY_CHAT_LEN];
		} message;

		struct
		{
			char nick[GP_NICK_LEN];
			char email[GP_EMAIL_LEN];
			char password[GP_PASSWORD_LEN];
			Bool hasFirewall;
		} login;

		struct
		{
			GPProfile id;
			WideChar text[MAX_BUDDY_CHAT_LEN];
		} addbuddy;

		struct
		{
			GPProfile id;
		} profile;

		struct
		{
			GPEnum status;
			char statusString[GP_STATUS_STRING_LEN];
			char locationString[GP_LOCATION_STRING_LEN];
		} status;

	} arg;
};

//-------------------------------------------------------------------------

// this class encapsulates an action the buddy thread wants from the UI
class BuddyResponse
{
public:
	enum
	{
		BUDDYRESPONSE_LOGIN,
		BUDDYRESPONSE_DISCONNECT,
		BUDDYRESPONSE_MESSAGE,
		BUDDYRESPONSE_REQUEST,
		BUDDYRESPONSE_STATUS,
	} buddyResponseType;

	GPProfile profile;
	GPResult result;

	union
	{
		struct
		{
			UnsignedInt date;
			char nick[GP_NICK_LEN];
			WideChar text[MAX_BUDDY_CHAT_LEN];
		} message;

		struct
		{
			char nick[GP_NICK_LEN];
			char email[GP_EMAIL_LEN];
			char countrycode[GP_COUNTRYCODE_LEN];
			WideChar text[GP_REASON_LEN];
		} request;

		struct
		{
			//GPResult result;
			GPErrorCode errorCode;
			char errorString[MAX_BUDDY_CHAT_LEN];
			GPEnum fatal;
		} error;

		struct
		{
			char nick[GP_NICK_LEN];
			char email[GP_EMAIL_LEN];
			char countrycode[GP_COUNTRYCODE_LEN];
			char location[GP_LOCATION_STRING_LEN];
			GPEnum status;
			char statusString[GP_STATUS_STRING_LEN];
		} status;
	} arg;
};

//-------------------------------------------------------------------------

// this is the actual message queue used to pass messages between threads
class GameSpyBuddyMessageQueueInterface
{
public:
	virtual ~GameSpyBuddyMessageQueueInterface() {}
	virtual void startThread( void ) = 0;
	virtual void endThread( void ) = 0;
	virtual Bool isThreadRunning( void ) = 0;
	virtual Bool isConnected( void ) = 0;
	virtual Bool isConnecting( void ) = 0;

	virtual void addRequest( const BuddyRequest& req ) = 0;
	virtual Bool getRequest( BuddyRequest& req ) = 0;

	virtual void addResponse( const BuddyResponse& resp ) = 0;
	virtual Bool getResponse( BuddyResponse& resp ) = 0;

	virtual GPProfile getLocalProfileID( void ) = 0;

	static GameSpyBuddyMessageQueueInterface* createNewMessageQueue( void );
};

extern GameSpyBuddyMessageQueueInterface *TheGameSpyBuddyMessageQueue;
