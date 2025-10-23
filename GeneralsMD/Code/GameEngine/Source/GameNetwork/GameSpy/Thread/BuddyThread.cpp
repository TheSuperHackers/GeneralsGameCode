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

// FILE: BuddyThread.cpp //////////////////////////////////////////////////////
// GameSpy Presence & Messaging (Buddy) thread
// This thread communicates with GameSpy's buddy server
// and talks through a message queue with the rest of
// the game.
// Author: Matthew D. Campbell, June 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"

#include "mutex.h"
#include "thread.h"


//-------------------------------------------------------------------------

typedef std::queue<BuddyRequest> RequestQueue;
typedef std::queue<BuddyResponse> ResponseQueue;
class BuddyThreadClass;

class GameSpyBuddyMessageQueue : public GameSpyBuddyMessageQueueInterface
{
public:
	virtual ~GameSpyBuddyMessageQueue();
	GameSpyBuddyMessageQueue();
	virtual void startThread( void );
	virtual void endThread( void );
	virtual Bool isThreadRunning( void );
	virtual Bool isConnected( void );
	virtual Bool isConnecting( void );

	virtual void addRequest( const BuddyRequest& req );
	virtual Bool getRequest( BuddyRequest& req );

	virtual void addResponse( const BuddyResponse& resp );
	virtual Bool getResponse( BuddyResponse& resp );

	virtual GPProfile getLocalProfileID( void );

	BuddyThreadClass* getThread( void );

private:
	MutexClass m_requestMutex;
	MutexClass m_responseMutex;
	RequestQueue m_requests;
	ResponseQueue m_responses;
	BuddyThreadClass *m_thread;
};

GameSpyBuddyMessageQueueInterface* GameSpyBuddyMessageQueueInterface::createNewMessageQueue( void )
{
	return NEW GameSpyBuddyMessageQueue;
}

GameSpyBuddyMessageQueueInterface *TheGameSpyBuddyMessageQueue;
#define MESSAGE_QUEUE ((GameSpyBuddyMessageQueue *)TheGameSpyBuddyMessageQueue)

//-------------------------------------------------------------------------

class BuddyThreadClass : public ThreadClass
{

public:
	BuddyThreadClass() : ThreadClass() { m_isNewAccount = m_isdeleting = m_isConnecting = m_isConnected = false; m_profileID = 0; m_lastErrorCode = 0; }

	void Thread_Function();

	void errorCallback( GPConnection *con, GPErrorArg *arg );
	void messageCallback( GPConnection *con, GPRecvBuddyMessageArg *arg );
	void connectCallback( GPConnection *con, GPConnectResponseArg *arg );
	void requestCallback( GPConnection *con, GPRecvBuddyRequestArg *arg );
	void statusCallback( GPConnection *con, GPRecvBuddyStatusArg *arg );

	Bool isConnecting( void ) { return m_isConnecting; }
	Bool isConnected( void ) { return m_isConnected; }

	GPProfile getLocalProfileID( void ) { return m_profileID; }

private:
	Bool m_isNewAccount;
	Bool m_isConnecting;
	Bool m_isConnected;
	GPProfile m_profileID;
	Int m_lastErrorCode;
	Bool m_isdeleting;
	std::string m_nick, m_email, m_pass;
};

enum CallbackType
{
	CALLBACK_CONNECT,
	CALLBACK_ERROR,
	CALLBACK_RECVMESSAGE,
	CALLBACK_RECVREQUEST,
	CALLBACK_RECVSTATUS,
};

void callbackWrapper( GPConnection *con, void *arg, void *param )
{
	CallbackType info = (CallbackType)(Int)param;
	BuddyThreadClass *thread = MESSAGE_QUEUE->getThread() ? MESSAGE_QUEUE->getThread() : NULL /*(TheGameSpyBuddyMessageQueue)?TheGameSpyBuddyMessageQueue->getThread():NULL*/;
	if (!thread)
		return;

	switch (info)
	{
		case CALLBACK_CONNECT:
			thread->connectCallback( con, (GPConnectResponseArg *)arg );
			break;
		case CALLBACK_ERROR:
			thread->errorCallback( con, (GPErrorArg *)arg );
			break;
		case CALLBACK_RECVMESSAGE:
			thread->messageCallback( con, (GPRecvBuddyMessageArg *)arg );
			break;
		case CALLBACK_RECVREQUEST:
			thread->requestCallback( con, (GPRecvBuddyRequestArg *)arg );
			break;
		case CALLBACK_RECVSTATUS:
			thread->statusCallback( con, (GPRecvBuddyStatusArg *)arg );
			break;
	}
}

//-------------------------------------------------------------------------

GameSpyBuddyMessageQueue::GameSpyBuddyMessageQueue()
{
	m_thread = NULL;
}

GameSpyBuddyMessageQueue::~GameSpyBuddyMessageQueue()
{
	endThread();
}

void GameSpyBuddyMessageQueue::startThread( void )
{
	if (!m_thread)
	{
		m_thread = NEW BuddyThreadClass;
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

void GameSpyBuddyMessageQueue::endThread( void )
{
	delete m_thread;
	m_thread = NULL;
}

Bool GameSpyBuddyMessageQueue::isThreadRunning( void )
{
	return (m_thread) ? m_thread->Is_Running() : false;
}

Bool GameSpyBuddyMessageQueue::isConnected( void )
{
	return (m_thread) ? m_thread->isConnected() : false;
}

Bool GameSpyBuddyMessageQueue::isConnecting( void )
{
	return (m_thread) ? m_thread->isConnecting() : false;
}

void GameSpyBuddyMessageQueue::addRequest( const BuddyRequest& req )
{
	MutexClass::LockClass m(m_requestMutex);
	if (m.Failed())
		return;

	m_requests.push(req);
}

Bool GameSpyBuddyMessageQueue::getRequest( BuddyRequest& req )
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

void GameSpyBuddyMessageQueue::addResponse( const BuddyResponse& resp )
{
	MutexClass::LockClass m(m_responseMutex);
	if (m.Failed())
		return;

	m_responses.push(resp);
}

Bool GameSpyBuddyMessageQueue::getResponse( BuddyResponse& resp )
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

BuddyThreadClass* GameSpyBuddyMessageQueue::getThread( void )
{
	return m_thread;
}

GPProfile GameSpyBuddyMessageQueue::getLocalProfileID( void )
{
	return (m_thread) ? m_thread->getLocalProfileID() : 0;
}

//-------------------------------------------------------------------------

void BuddyThreadClass::Thread_Function()
{
	try {
	GPConnection gpCon;
	GPConnection *con = &gpCon;
#if RTS_GENERALS
	const int productID = 675;
#elif RTS_ZEROHOUR
	const int productID = 823;
#endif
	gpInitialize( con, productID, 0, GP_PARTNERID_GAMESPY );
	m_isConnected = m_isConnecting = false;

	gpSetCallback( con, GP_ERROR,								callbackWrapper,	(void *)CALLBACK_ERROR );
	gpSetCallback( con, GP_RECV_BUDDY_MESSAGE,	callbackWrapper,	(void *)CALLBACK_RECVMESSAGE );
	gpSetCallback( con, GP_RECV_BUDDY_REQUEST,	callbackWrapper,	(void *)CALLBACK_RECVREQUEST );
	gpSetCallback( con, GP_RECV_BUDDY_STATUS,		callbackWrapper,	(void *)CALLBACK_RECVSTATUS );

	GPEnum lastStatus = GP_OFFLINE;
	std::string lastStatusString;

	BuddyRequest incomingRequest;
	while ( running )
	{
		// deal with requests
		if (TheGameSpyBuddyMessageQueue->getRequest(incomingRequest))
		{
			switch (incomingRequest.buddyRequestType)
			{
			case BuddyRequest::BUDDYREQUEST_LOGIN:
				m_isConnecting = true;
				m_nick = incomingRequest.arg.login.nick;
				m_email = incomingRequest.arg.login.email;
				m_pass = incomingRequest.arg.login.password;
				m_isConnected = (gpConnect( con, incomingRequest.arg.login.nick, incomingRequest.arg.login.email,
					incomingRequest.arg.login.password, (incomingRequest.arg.login.hasFirewall)?GP_FIREWALL:GP_NO_FIREWALL,
					GP_BLOCKING, callbackWrapper, (void *)CALLBACK_CONNECT ) == GP_NO_ERROR);
				m_isConnecting = false;
				break;

			case BuddyRequest::BUDDYREQUEST_RELOGIN:
				m_isConnecting = true;
				m_isConnected = (gpConnect( con, m_nick.c_str(), m_email.c_str(), m_pass.c_str(), GP_FIREWALL,
					GP_BLOCKING, callbackWrapper, (void *)CALLBACK_CONNECT ) == GP_NO_ERROR);
				m_isConnecting = false;
				break;
			case BuddyRequest::BUDDYREQUEST_DELETEACCT:
				m_isdeleting =  true;
				// TheSuperHackers @tweak OmniBlade API was updated since Generals released to require a callback. Passing -1 will make our wrapper ignore this.
				gpDeleteProfile( con, callbackWrapper, (void *)(-1) );
				break;
			case BuddyRequest::BUDDYREQUEST_LOGOUT:
				m_isConnecting = m_isConnected = false;
				gpDisconnect( con );
				break;
			case BuddyRequest::BUDDYREQUEST_MESSAGE:
				{
					std::string s = WideCharStringToMultiByte( incomingRequest.arg.message.text );
					DEBUG_LOG(("Sending a buddy message to %d [%s]", incomingRequest.arg.message.recipient, s.c_str()));
					gpSendBuddyMessage( con, incomingRequest.arg.message.recipient, s.c_str() );
				}
				break;
			case BuddyRequest::BUDDYREQUEST_LOGINNEW:
				{
					m_isConnecting = true;
					m_nick = incomingRequest.arg.login.nick;
					m_email = incomingRequest.arg.login.email;
					m_pass = incomingRequest.arg.login.password;
					m_isNewAccount = TRUE;
					// TheSuperHackers @tweak OmniBlade API was updated since Generals release to require uniquenick which is the same as nick and cdkey is an empty string here.
					m_isConnected = (gpConnectNewUser( con, incomingRequest.arg.login.nick, incomingRequest.arg.login.nick, incomingRequest.arg.login.email,
						incomingRequest.arg.login.password, "", (incomingRequest.arg.login.hasFirewall)?GP_FIREWALL:GP_NO_FIREWALL,
						GP_BLOCKING, callbackWrapper, (void *)CALLBACK_CONNECT ) == GP_NO_ERROR);
					if (m_isNewAccount) // if we didn't re-login
					{
						gpSetInfoMask( con, GP_MASK_NONE ); // don't share info
					}
					m_isConnecting = false;
				}
				break;
			case BuddyRequest::BUDDYREQUEST_ADDBUDDY:
				{
					std::string s = WideCharStringToMultiByte( incomingRequest.arg.addbuddy.text );
					gpSendBuddyRequest( con, incomingRequest.arg.addbuddy.id, s.c_str() );
				}
				break;
			case BuddyRequest::BUDDYREQUEST_DELBUDDY:
				{
					gpDeleteBuddy( con, incomingRequest.arg.profile.id );
				}
				break;
			case BuddyRequest::BUDDYREQUEST_OKADD:
				{
					gpAuthBuddyRequest( con, incomingRequest.arg.profile.id );
				}
				break;
			case BuddyRequest::BUDDYREQUEST_DENYADD:
				{
					gpDenyBuddyRequest( con, incomingRequest.arg.profile.id );
				}
				break;
			case BuddyRequest::BUDDYREQUEST_SETSTATUS:
				{
					//don't blast our 'Loading' status with 'Online'.
					if (lastStatus == GP_PLAYING && lastStatusString == "Loading" && incomingRequest.arg.status.status == GP_ONLINE)
						break;

					DEBUG_LOG(("BUDDYREQUEST_SETSTATUS: status is now %d:%s/%s",
						incomingRequest.arg.status.status, incomingRequest.arg.status.statusString, incomingRequest.arg.status.locationString));
					gpSetStatus( con, incomingRequest.arg.status.status, incomingRequest.arg.status.statusString,
						incomingRequest.arg.status.locationString );
					lastStatus = incomingRequest.arg.status.status;
					lastStatusString = incomingRequest.arg.status.statusString;
				}
				break;
			}
		}

		// update the network
		GPEnum isConnected = GP_CONNECTED;
		GPResult res = GP_NO_ERROR;
		res = gpIsConnected( con, &isConnected );
		if ( isConnected == GP_CONNECTED && res == GP_NO_ERROR )
			gpProcess( con );

		// end our timeslice
		Switch_Thread();
	}

	gpDestroy( con );
	} catch ( ... ) {
		DEBUG_CRASH(("Exception in buddy thread!"));
	}
}

void BuddyThreadClass::errorCallback( GPConnection *con, GPErrorArg *arg )
{
	// log the error
	DEBUG_LOG(("GPErrorCallback"));
	m_lastErrorCode = arg->errorCode;

	char errorCodeString[256];
	char resultString[256];

	#define RESULT(x) case x: strcpy(resultString, #x); break;
	switch(arg->result)
	{
		RESULT(GP_NO_ERROR)
		RESULT(GP_MEMORY_ERROR)
		RESULT(GP_PARAMETER_ERROR)
		RESULT(GP_NETWORK_ERROR)
		RESULT(GP_SERVER_ERROR)
	default:
		strcpy(resultString, "Unknown result!");
	}
	#undef RESULT

	#define ERRORCODE(x) case x: strcpy(errorCodeString, #x); break;
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
		strcpy(errorCodeString, "Unknown error code!");
	}
	#undef ERRORCODE

	if(arg->fatal)
	{
		DEBUG_LOG(( "-----------"));
		DEBUG_LOG(( "GP FATAL ERROR"));
		DEBUG_LOG(( "-----------"));
	}
	else
	{
		DEBUG_LOG(( "-----"));
		DEBUG_LOG(( "GP ERROR"));
		DEBUG_LOG(( "-----"));
	}
	DEBUG_LOG(( "RESULT: %s (%d)", resultString, arg->result));
	DEBUG_LOG(( "ERROR CODE: %s (0x%X)", errorCodeString, arg->errorCode));
	DEBUG_LOG(( "ERROR STRING: %s", arg->errorString));

	if (arg->fatal == GP_FATAL)
	{
		BuddyResponse errorResponse;
		errorResponse.buddyResponseType = BuddyResponse::BUDDYRESPONSE_DISCONNECT;
		errorResponse.result = arg->result;
		errorResponse.arg.error.errorCode = arg->errorCode;
		errorResponse.arg.error.fatal = arg->fatal;
		strlcpy(errorResponse.arg.error.errorString, arg->errorString, MAX_BUDDY_CHAT_LEN);
		m_isConnecting = m_isConnected = false;
		TheGameSpyBuddyMessageQueue->addResponse( errorResponse );
		if (m_isdeleting)
		{
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_LOGOUT;
			TheGameSpyPeerMessageQueue->addRequest( req );
			m_isdeleting = false;
		}
	}
}

static void getNickForMessage( GPConnection *con, GPGetInfoResponseArg *arg, void *param )
{
	BuddyResponse *resp = (BuddyResponse *)param;
	strcpy(resp->arg.message.nick, arg->nick);
}

void BuddyThreadClass::messageCallback( GPConnection *con, GPRecvBuddyMessageArg *arg )
{
	BuddyResponse messageResponse;
	messageResponse.buddyResponseType = BuddyResponse::BUDDYRESPONSE_MESSAGE;
	messageResponse.profile = arg->profile;

	// get info about the person asking to be our buddy
	gpGetInfo( con, arg->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)getNickForMessage, &messageResponse);

	std::wstring s = MultiByteToWideCharSingleLine( arg->message );
	wcslcpy(messageResponse.arg.message.text, s.c_str(), MAX_BUDDY_CHAT_LEN);
	messageResponse.arg.message.date = arg->date;
	DEBUG_LOG(("Got a buddy message from %d [%ls]", arg->profile, s.c_str()));
	TheGameSpyBuddyMessageQueue->addResponse( messageResponse );
}

void BuddyThreadClass::connectCallback( GPConnection *con, GPConnectResponseArg *arg )
{
	BuddyResponse loginResponse;
	if (arg->result == GP_NO_ERROR)
	{
		loginResponse.buddyResponseType = BuddyResponse::BUDDYRESPONSE_LOGIN;
		loginResponse.result = arg->result;
		loginResponse.profile = arg->profile;
		TheGameSpyBuddyMessageQueue->addResponse( loginResponse );
		m_profileID = arg->profile;

		if (!TheGameSpyPeerMessageQueue->isConnected() && !TheGameSpyPeerMessageQueue->isConnecting())
		{
			DEBUG_LOG(("Buddy connect: trying chat connect"));
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_LOGIN;
			req.nick = m_nick;
			req.password = m_pass;
			req.email = m_email;
			req.login.profileID = arg->profile;
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
	else
	{
		if (!TheGameSpyPeerMessageQueue->isConnected() && !TheGameSpyPeerMessageQueue->isConnecting())
		{
			if (m_lastErrorCode == GP_NEWUSER_BAD_NICK)
			{
				m_isNewAccount = FALSE;
				// they just hit 'create account' instead of 'log in'.  Fix them.
				DEBUG_LOG(("User Error: Create Account instead of Login.  Fixing them..."));
				BuddyRequest req;
				req.buddyRequestType = BuddyRequest::BUDDYREQUEST_LOGIN;
				strcpy(req.arg.login.nick, m_nick.c_str());
				strcpy(req.arg.login.email, m_email.c_str());
				strcpy(req.arg.login.password, m_pass.c_str());
				req.arg.login.hasFirewall = true;
				TheGameSpyBuddyMessageQueue->addRequest( req );
				return;
			}
			DEBUG_LOG(("Buddy connect failed (%d/%d): posting a failed chat connect", arg->result, m_lastErrorCode));
			PeerResponse resp;
			resp.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
			resp.discon.reason = DISCONNECT_COULDNOTCONNECT;
			switch (m_lastErrorCode)
			{
			case GP_LOGIN_TIMEOUT:
				resp.discon.reason = DISCONNECT_GP_LOGIN_TIMEOUT;
				break;
			case GP_LOGIN_BAD_NICK:
				resp.discon.reason = DISCONNECT_GP_LOGIN_BAD_NICK;
				break;
			case GP_LOGIN_BAD_EMAIL:
				resp.discon.reason = DISCONNECT_GP_LOGIN_BAD_EMAIL;
				break;
			case GP_LOGIN_BAD_PASSWORD:
				resp.discon.reason = DISCONNECT_GP_LOGIN_BAD_PASSWORD;
				break;
			case GP_LOGIN_BAD_PROFILE:
				resp.discon.reason = DISCONNECT_GP_LOGIN_BAD_PROFILE;
				break;
			case GP_LOGIN_PROFILE_DELETED:
				resp.discon.reason = DISCONNECT_GP_LOGIN_PROFILE_DELETED;
				break;
			case GP_LOGIN_CONNECTION_FAILED:
				resp.discon.reason = DISCONNECT_GP_LOGIN_CONNECTION_FAILED;
				break;
			case GP_LOGIN_SERVER_AUTH_FAILED:
				resp.discon.reason = DISCONNECT_GP_LOGIN_SERVER_AUTH_FAILED;
				break;
			case GP_NEWUSER_BAD_NICK:
				resp.discon.reason = DISCONNECT_GP_NEWUSER_BAD_NICK;
				break;
			case GP_NEWUSER_BAD_PASSWORD:
				resp.discon.reason = DISCONNECT_GP_NEWUSER_BAD_PASSWORD;
				break;
			case GP_NEWPROFILE_BAD_NICK:
				resp.discon.reason = DISCONNECT_GP_NEWPROFILE_BAD_NICK;
				break;
			case GP_NEWPROFILE_BAD_OLD_NICK:
				resp.discon.reason = DISCONNECT_GP_NEWPROFILE_BAD_OLD_NICK;
				break;
			}
			TheGameSpyPeerMessageQueue->addResponse(resp);
		}
	}
}

// -----------------------

static void getInfoResponseForRequest( GPConnection *con, GPGetInfoResponseArg *arg, void *param )
{
	BuddyResponse *resp = (BuddyResponse *)param;
	resp->profile = arg->profile;
	strcpy(resp->arg.request.nick, arg->nick);
	strcpy(resp->arg.request.email, arg->email);
	strcpy(resp->arg.request.countrycode, arg->countrycode);
}

void BuddyThreadClass::requestCallback( GPConnection *con, GPRecvBuddyRequestArg *arg )
{
	BuddyResponse response;
	response.buddyResponseType = BuddyResponse::BUDDYRESPONSE_REQUEST;
	response.profile = arg->profile;

	// get info about the person asking to be our buddy
	gpGetInfo( con, arg->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)getInfoResponseForRequest, &response);

	std::wstring s = MultiByteToWideCharSingleLine( arg->reason );
	wcslcpy(response.arg.request.text, s.c_str(), GP_REASON_LEN);

	TheGameSpyBuddyMessageQueue->addResponse( response );
}

// -----------------------

static void getInfoResponseForStatus(GPConnection * connection, GPGetInfoResponseArg * arg, void * param)
{
	BuddyResponse *resp = (BuddyResponse *)param;
	resp->profile = arg->profile;
	strcpy(resp->arg.status.nick, arg->nick);
	strcpy(resp->arg.status.email, arg->email);
	strcpy(resp->arg.status.countrycode, arg->countrycode);
}

void BuddyThreadClass::statusCallback( GPConnection *con, GPRecvBuddyStatusArg *arg )
{
	BuddyResponse response;

	// get user's name
	response.buddyResponseType = BuddyResponse::BUDDYRESPONSE_STATUS;
	gpGetInfo( con, arg->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)getInfoResponseForStatus, &response);

	// get user's status
	GPBuddyStatus status;
	gpGetBuddyStatus( con, arg->index, &status );
	strcpy(response.arg.status.location, status.locationString);
	strcpy(response.arg.status.statusString, status.statusString);
	response.arg.status.status = status.status;
	DEBUG_LOG(("Got buddy status for %d(%s) - status %d", status.profile, response.arg.status.nick, status.status));

	// relay to UI
	TheGameSpyBuddyMessageQueue->addResponse( response );
}


//-------------------------------------------------------------------------

