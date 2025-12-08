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

// FILE: PingThread.cpp //////////////////////////////////////////////////////
// Ping thread
// Author: Matthew D. Campbell, August 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include <winsock.h>	// This one has to be here. Prevents collisions with winsock2.h

#include "GameNetwork/GameSpy/GameResultsThread.h"
#include "mutex.h"
#include "thread.h"

#include "Common/SubsystemInterface.h"

//-------------------------------------------------------------------------

static const Int NumWorkerThreads = 1;

typedef std::queue<GameResultsRequest> RequestQueue;
typedef std::queue<GameResultsResponse> ResponseQueue;
class GameResultsThreadClass;

class GameResultsQueue : public GameResultsInterface
{
public:
	virtual ~GameResultsQueue();
	GameResultsQueue();

	virtual void init() {}
	virtual void reset() {}
	virtual void update() {}

	virtual void startThreads( void );
	virtual void endThreads( void );
	virtual Bool areThreadsRunning( void );

	virtual void addRequest( const GameResultsRequest& req );
	virtual Bool getRequest( GameResultsRequest& resp );

	virtual void addResponse( const GameResultsResponse& resp );
	virtual Bool getResponse( GameResultsResponse& resp );

	virtual Bool areGameResultsBeingSent( void );

private:
	MutexClass m_requestMutex;
	MutexClass m_responseMutex;
	RequestQueue m_requests;
	ResponseQueue m_responses;
	Int m_requestCount;
	Int m_responseCount;

	GameResultsThreadClass *m_workerThreads[NumWorkerThreads];
};

GameResultsInterface* GameResultsInterface::createNewGameResultsInterface( void )
{
	return NEW GameResultsQueue;
}

GameResultsInterface *TheGameResultsQueue;

//-------------------------------------------------------------------------

class GameResultsThreadClass : public ThreadClass
{

public:
	GameResultsThreadClass() : ThreadClass() {}

	void Thread_Function();

private:
	Int sendGameResults( UnsignedInt IP, UnsignedShort port, const std::string& results );
};


//-------------------------------------------------------------------------

GameResultsQueue::GameResultsQueue() : m_requestCount(0), m_responseCount(0)
{
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		m_workerThreads[i] = nullptr;
	}

	startThreads();
}

GameResultsQueue::~GameResultsQueue()
{
	endThreads();
}

void GameResultsQueue::startThreads( void )
{
	endThreads();
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		m_workerThreads[i] = NEW GameResultsThreadClass;
		m_workerThreads[i]->Execute();
	}
}

void GameResultsQueue::endThreads( void )
{
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		delete m_workerThreads[i];
		m_workerThreads[i] = nullptr;
	}
}

Bool GameResultsQueue::areThreadsRunning( void )
{
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		if (m_workerThreads[i])
		{
			if (m_workerThreads[i]->Is_Running())
				return true;
		}
	}
	return false;
}

void GameResultsQueue::addRequest( const GameResultsRequest& req )
{
	MutexClass::LockClass m(m_requestMutex);

	++m_requestCount;
	m_requests.push(req);
}

Bool GameResultsQueue::getRequest( GameResultsRequest& req )
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

void GameResultsQueue::addResponse( const GameResultsResponse& resp )
{
	{
		MutexClass::LockClass m(m_responseMutex);

		++m_responseCount;
		m_responses.push(resp);
	}
}

Bool GameResultsQueue::getResponse( GameResultsResponse& resp )
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

Bool GameResultsQueue::areGameResultsBeingSent( void )
{
	MutexClass::LockClass m(m_requestMutex, 0);
	if (m.Failed())
		return true;

	return m_requestCount > 0;
}

//-------------------------------------------------------------------------

void GameResultsThreadClass::Thread_Function()
{
	try {
	GameResultsRequest req;

	WSADATA wsaData;

	// Fire up winsock (prob already done, but doesn't matter)
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSAStartup( wVersionRequested, &wsaData );

	while ( running )
	{
		// deal with requests
		if (TheGameResultsQueue && TheGameResultsQueue->getRequest(req))
		{
			// resolve the hostname
			const char *hostnameBuffer = req.hostname.c_str();
			UnsignedInt IP = 0xFFFFFFFF;
			if (isdigit(hostnameBuffer[0]))
			{
				IP = inet_addr(hostnameBuffer);
				in_addr hostNode;
				hostNode.s_addr = IP;
				DEBUG_LOG(("sending game results to %s - IP = %s", hostnameBuffer, inet_ntoa(hostNode) ));
			}
			else
			{
				HOSTENT *hostStruct;
				in_addr *hostNode;
				hostStruct = gethostbyname(hostnameBuffer);
				if (hostStruct == nullptr)
				{
					DEBUG_LOG(("sending game results to %s - host lookup failed", hostnameBuffer));

					// Even though this failed to resolve IP, still need to send a
					//   callback.
					IP = 0xFFFFFFFF;   // flag for IP resolve failed
				}
				else
				{
					hostNode = (in_addr *) hostStruct->h_addr;
					IP = hostNode->s_addr;
					DEBUG_LOG(("sending game results to %s IP = %s", hostnameBuffer, inet_ntoa(*hostNode) ));
				}
			}

			int result = sendGameResults( IP, req.port, req.results );
			GameResultsResponse resp;
			resp.hostname = req.hostname;
			resp.port = req.port;
			resp.sentOk = (result == req.results.length());

		}

		// end our timeslice
		Switch_Thread();
	}

	WSACleanup();
	} catch ( ... ) {
		DEBUG_CRASH(("Exception in results thread!"));
	}
}

//-------------------------------------------------------------------------

#ifdef DEBUG_LOGGING
#define CASE(x) case (x): return #x;

static const char *getWSAErrorString( Int error )
{
	switch (error)
	{
		CASE(WSABASEERR)
		CASE(WSAEINTR)
		CASE(WSAEBADF)
		CASE(WSAEACCES)
		CASE(WSAEFAULT)
		CASE(WSAEINVAL)
		CASE(WSAEMFILE)
		CASE(WSAEWOULDBLOCK)
		CASE(WSAEINPROGRESS)
		CASE(WSAEALREADY)
		CASE(WSAENOTSOCK)
		CASE(WSAEDESTADDRREQ)
		CASE(WSAEMSGSIZE)
		CASE(WSAEPROTOTYPE)
		CASE(WSAENOPROTOOPT)
		CASE(WSAEPROTONOSUPPORT)
		CASE(WSAESOCKTNOSUPPORT)
		CASE(WSAEOPNOTSUPP)
		CASE(WSAEPFNOSUPPORT)
		CASE(WSAEAFNOSUPPORT)
		CASE(WSAEADDRINUSE)
		CASE(WSAEADDRNOTAVAIL)
		CASE(WSAENETDOWN)
		CASE(WSAENETUNREACH)
		CASE(WSAENETRESET)
		CASE(WSAECONNABORTED)
		CASE(WSAECONNRESET)
		CASE(WSAENOBUFS)
		CASE(WSAEISCONN)
		CASE(WSAENOTCONN)
		CASE(WSAESHUTDOWN)
		CASE(WSAETOOMANYREFS)
		CASE(WSAETIMEDOUT)
		CASE(WSAECONNREFUSED)
		CASE(WSAELOOP)
		CASE(WSAENAMETOOLONG)
		CASE(WSAEHOSTDOWN)
		CASE(WSAEHOSTUNREACH)
		CASE(WSAENOTEMPTY)
		CASE(WSAEPROCLIM)
		CASE(WSAEUSERS)
		CASE(WSAEDQUOT)
		CASE(WSAESTALE)
		CASE(WSAEREMOTE)
		CASE(WSAEDISCON)
		CASE(WSASYSNOTREADY)
		CASE(WSAVERNOTSUPPORTED)
		CASE(WSANOTINITIALISED)
		CASE(WSAHOST_NOT_FOUND)
		CASE(WSATRY_AGAIN)
		CASE(WSANO_RECOVERY)
		CASE(WSANO_DATA)
		default:
			return "Not a Winsock error";
	}
}

#undef CASE

#endif
//-------------------------------------------------------------------------

Int GameResultsThreadClass::sendGameResults( UnsignedInt IP, UnsignedShort port, const std::string& results )
{
	int error = 0;

	// create the socket
	Int sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (sock < 0)
	{
		DEBUG_LOG(("GameResultsThreadClass::sendGameResults() - socket() returned %d(%s)", sock, getWSAErrorString(sock)));
		return sock;
	}

	// fill in address info
	struct sockaddr_in sockAddr;
	memset( &sockAddr, 0, sizeof( sockAddr ) );
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = IP;
	sockAddr.sin_port = htons(port);

	// Start the connection process....
	if( connect( sock, (struct sockaddr *)&sockAddr, sizeof( sockAddr ) ) == -1 )
	{
		error = WSAGetLastError();
		DEBUG_LOG(("GameResultsThreadClass::sendGameResults() - connect() returned %d(%s)", error, getWSAErrorString(error)));
		if( ( error == WSAEWOULDBLOCK ) || ( error == WSAEINVAL ) || ( error == WSAEALREADY ) )
		{
			return( -1 );
		}

		if( error != WSAEISCONN )
		{
			closesocket( sock );
			return( -1 );
		}
	}

	if (send( sock, results.c_str(), results.length(), 0 ) == SOCKET_ERROR)
	{
		error = WSAGetLastError();
		DEBUG_LOG(("GameResultsThreadClass::sendGameResults() - send() returned %d(%s)", error, getWSAErrorString(error)));
		closesocket(sock);
		return WSAGetLastError();
	}

	closesocket(sock);

	return results.length();
}


//-------------------------------------------------------------------------
