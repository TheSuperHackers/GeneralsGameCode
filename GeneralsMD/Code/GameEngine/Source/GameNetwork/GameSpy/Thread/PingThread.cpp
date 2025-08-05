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

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include <winsock.h>	// This one has to be here. Prevents collisions with windsock2.h

#include "GameNetwork/GameSpy/PingThread.h"
#include "mutex.h"
#include "thread.h"

#include "Common/StackDump.h"
#include "Common/SubsystemInterface.h"

//-------------------------------------------------------------------------

static const Int NumWorkerThreads = 10;

typedef std::queue<PingRequest> RequestQueue;
typedef std::queue<PingResponse> ResponseQueue;
class PingThreadClass;

class Pinger : public PingerInterface
{
public:
	virtual ~Pinger();
	Pinger();
	virtual void startThreads( void );
	virtual void endThreads( void );
	virtual Bool areThreadsRunning( void );

	virtual void addRequest( const PingRequest& req );
	virtual Bool getRequest( PingRequest& resp );

	virtual void addResponse( const PingResponse& resp );
	virtual Bool getResponse( PingResponse& resp );

	virtual Bool arePingsInProgress( void );
	virtual Int getPing( AsciiString hostname );

	virtual void clearPingMap( void );
	virtual AsciiString getPingString( Int timeout );

private:
	MutexClass m_requestMutex;
	MutexClass m_responseMutex;
	MutexClass m_pingMapMutex;
	RequestQueue m_requests;
	ResponseQueue m_responses;
	Int m_requestCount;
	Int m_responseCount;

	std::map<std::string, Int> m_pingMap;

	PingThreadClass *m_workerThreads[NumWorkerThreads];
};

PingerInterface* PingerInterface::createNewPingerInterface( void )
{
	return NEW Pinger;
}

PingerInterface *ThePinger;

//-------------------------------------------------------------------------

class PingThreadClass : public ThreadClass
{

public:
	PingThreadClass() : ThreadClass() {}

	void Thread_Function();

private:
	Int doPing( UnsignedInt IP, Int timeout );
};


//-------------------------------------------------------------------------

Pinger::Pinger() : m_requestCount(0), m_responseCount(0)
{
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		m_workerThreads[i] = NULL;
	}
}

Pinger::~Pinger()
{
	endThreads();
}

void Pinger::startThreads( void )
{
	endThreads();
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		m_workerThreads[i] = NEW PingThreadClass;
		m_workerThreads[i]->Execute();
	}
}

void Pinger::endThreads( void )
{
	for (Int i=0; i<NumWorkerThreads; ++i)
	{
		if (m_workerThreads[i])
		{
			delete m_workerThreads[i];
			m_workerThreads[i] = NULL;
		}
	}
}

Bool Pinger::areThreadsRunning( void )
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

void Pinger::addRequest( const PingRequest& req )
{
	MutexClass::LockClass m(m_requestMutex);

	++m_requestCount;
	m_requests.push(req);
}

Bool Pinger::getRequest( PingRequest& req )
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

void Pinger::addResponse( const PingResponse& resp )
{
	{
		MutexClass::LockClass m(m_pingMapMutex);

		m_pingMap[resp.hostname] = resp.avgPing;
	}
	{
		MutexClass::LockClass m(m_responseMutex);

		++m_responseCount;
		m_responses.push(resp);
	}
}

Bool Pinger::getResponse( PingResponse& resp )
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

Bool Pinger::arePingsInProgress( void )
{
	return (m_requestCount != m_responseCount);
}

Int Pinger::getPing( AsciiString hostname )
{
	MutexClass::LockClass m(m_pingMapMutex, 0);
	if (m.Failed())
		return false;

	std::map<std::string, Int>::const_iterator it = m_pingMap.find(hostname.str());
	if (it != m_pingMap.end())
		return it->second;

	return -1;
}

void Pinger::clearPingMap( void )
{
	MutexClass::LockClass m(m_pingMapMutex);
	m_pingMap.clear();
}

AsciiString Pinger::getPingString( Int timeout )
{
	MutexClass::LockClass m(m_pingMapMutex);

	AsciiString pingString;
	AsciiString tmp;
	for (std::map<std::string, Int>::const_iterator it = m_pingMap.begin(); it != m_pingMap.end(); ++it)
	{
		Int ping = it->second;
		if (ping < 0 || ping > timeout)
			ping = timeout;
		ping = ping * 255 / timeout;
		tmp.format("%2.2X", ping);
		pingString.concat(tmp);
	}
	return pingString;
}

//-------------------------------------------------------------------------

void PingThreadClass::Thread_Function()
{
	try {
	_set_se_translator( DumpExceptionInfo ); // Hook that allows stack trace.
	PingRequest req;

	WSADATA wsaData;

	// Fire up winsock (prob already done, but doesn't matter)
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSAStartup( wVersionRequested, &wsaData );

	while ( running )
	{
		// deal with requests
		if (ThePinger->getRequest(req))
		{
			// resolve the hostname
			const char *hostnameBuffer = req.hostname.c_str();
			UnsignedInt IP = 0xFFFFFFFF;
			if (isdigit(hostnameBuffer[0]))
			{
				IP = inet_addr(hostnameBuffer);
				in_addr hostNode;
				hostNode.s_addr = IP;
				DEBUG_LOG(("pinging %s - IP = %s", hostnameBuffer, inet_ntoa(hostNode) ));
			}
			else
			{
				HOSTENT *hostStruct;
				in_addr *hostNode;
				hostStruct = gethostbyname(hostnameBuffer);
				if (hostStruct == NULL)
				{
					DEBUG_LOG(("pinging %s - host lookup failed", hostnameBuffer));

					// Even though this failed to resolve IP, still need to send a
					//   callback.
					IP = 0xFFFFFFFF;   // flag for IP resolve failed
				}
				else
				{
					hostNode = (in_addr *) hostStruct->h_addr;
					IP = hostNode->s_addr;
					DEBUG_LOG(("pinging %s IP = %s", hostnameBuffer, inet_ntoa(*hostNode) ));
				}
			}

			// do ping
			Int totalPing = 0;
			Int goodReps = 0;
			Int reps = req.repetitions;
			while (reps-- && running && IP != 0xFFFFFFFF)
			{
				Int ping = doPing(IP, req.timeout);
				if (ping >= 0)
				{
					totalPing += ping;
					++goodReps;
				}

				// end our timeslice
				Switch_Thread();
			}
			if (!goodReps)
				totalPing = -1;
			else
				totalPing = totalPing / goodReps;

			PingResponse resp;
			resp.hostname = req.hostname;
			resp.avgPing = totalPing;
			resp.repetitions = goodReps;
			ThePinger->addResponse(resp);
		}

		// end our timeslice
		Switch_Thread();
	}

	WSACleanup();
	} catch ( ... ) {
		DEBUG_CRASH(("Exception in ping thread!"));
	}
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

HANDLE WINAPI IcmpCreateFile(VOID); /* INVALID_HANDLE_VALUE on error */
BOOL WINAPI IcmpCloseHandle(HANDLE IcmpHandle); /* FALSE on error */

/* Note 2: For the most part, you can refer to RFC 791 for detials
 * on how to fill in values for the IP option information structure.
 */
typedef struct ip_option_information
{
   UnsignedByte Ttl;         /* Time To Live (used for traceroute) */
   UnsignedByte Tos;         /* Type Of Service (usually 0) */
   UnsignedByte Flags;       /* IP header flags (usually 0) */
   UnsignedByte OptionsSize; /* Size of options data (usually 0, max 40) */
   UnsignedByte FAR *OptionsData;   /* Options data buffer */
}
IPINFO, *PIPINFO, FAR *LPIPINFO;


/* Note 1: The Reply Buffer will have an array of ICMP_ECHO_REPLY
 * structures, followed by options and the data in ICMP echo reply
 * datagram received. You must have room for at least one ICMP
 * echo reply structure, plus 8 bytes for an ICMP header.
 */
typedef struct icmp_echo_reply
{
   UnsignedInt Address;     /* source address */
   ////////UnsignedInt Status;      /* IP status value (see below) */
   UnsignedInt RTTime;      /* Round Trip Time in milliseconds */
   UnsignedShort DataSize;   /* reply data size */
   UnsignedShort Reserved;   /* */
   void FAR *Data;     /* reply data buffer */
   struct ip_option_information Options; /* reply options */
}
ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;


DWORD WINAPI IcmpSendEcho(
   HANDLE IcmpHandle,   /* handle returned from IcmpCreateFile() */
   UnsignedInt DestAddress,  /* destination IP address (in network order) */
   LPVOID RequestData,  /* pointer to buffer to send */
   WORD RequestSize,    /* length of data in buffer */
   LPIPINFO RequestOptns,   /* see Note 2 */
   LPVOID ReplyBuffer,  /* see Note 1 */
   DWORD ReplySize,     /* length of reply (must allow at least 1 reply) */
   DWORD Timeout       /* time in milliseconds to wait for reply */
);


#define IP_STATUS_BASE 11000
#define IP_SUCCESS 0
#define IP_BUF_TOO_SMALL (IP_STATUS_BASE + 1)
#define IP_DEST_NET_UNREACHABLE (IP_STATUS_BASE + 2)
#define IP_DEST_HOST_UNREACHABLE (IP_STATUS_BASE + 3)
#define IP_DEST_PROT_UNREACHABLE (IP_STATUS_BASE + 4)
#define IP_DEST_PORT_UNREACHABLE (IP_STATUS_BASE + 5)
#define IP_NO_RESOURCES (IP_STATUS_BASE + 6)
#define IP_BAD_OPTION (IP_STATUS_BASE + 7)
#define IP_HW_ERROR (IP_STATUS_BASE + 8)
#define IP_PACKET_TOO_BIG (IP_STATUS_BASE + 9)
#define IP_REQ_TIMED_OUT (IP_STATUS_BASE + 10)
#define IP_BAD_REQ (IP_STATUS_BASE + 11)
#define IP_BAD_ROUTE (IP_STATUS_BASE + 12)
#define IP_TTL_EXPIRED_TRANSIT (IP_STATUS_BASE + 13)
#define IP_TTL_EXPIRED_REASSEM (IP_STATUS_BASE + 14)
#define IP_PARAM_PROBLEM (IP_STATUS_BASE + 15)
#define IP_SOURCE_QUENCH (IP_STATUS_BASE + 16)
#define IP_OPTION_TOO_BIG (IP_STATUS_BASE + 17)
#define IP_BAD_DESTINATION (IP_STATUS_BASE + 18)
#define IP_ADDR_DELETED (IP_STATUS_BASE + 19)
#define IP_SPEC_MTU_CHANGE (IP_STATUS_BASE + 20)
#define IP_MTU_CHANGE (IP_STATUS_BASE + 21)
#define IP_UNLOAD (IP_STATUS_BASE + 22)
#define IP_GENERAL_FAILURE (IP_STATUS_BASE + 50)
#define MAX_IP_STATUS IP_GENERAL_FAILURE
#define IP_PENDING (IP_STATUS_BASE + 255)


#define BUFSIZE     8192
#define DEFAULT_LEN 32
#define LOOPLIMIT   4
#define DEFAULT_TTL 64

Int PingThreadClass::doPing(UnsignedInt IP, Int timeout)
{
   /*
    * Initialize default settings
    */

   IPINFO stIPInfo, *lpstIPInfo;
   HANDLE hICMP, hICMP_DLL;
   int i, j, nDataLen, nLoopLimit, nTimeOut, nTTL, nTOS;
   DWORD dwReplyCount;
   ///////IN_ADDR stDestAddr;
   BOOL fRet, fDontStop;
   ///BOOL fTraceRoute;

   nDataLen = DEFAULT_LEN;
   nLoopLimit = LOOPLIMIT;
   nTimeOut = timeout;
   fDontStop = FALSE;
   lpstIPInfo = NULL;
   nTTL = DEFAULT_TTL;
   nTOS = 0;

   Int pingTime = -1;  // in case of error

   char achReqData[BUFSIZE];
   char achRepData[sizeof(ICMPECHO) + BUFSIZE];


   HANDLE ( WINAPI *lpfnIcmpCreateFile )( VOID ) = NULL;
   BOOL ( WINAPI *lpfnIcmpCloseHandle )( HANDLE ) = NULL;
   DWORD (WINAPI *lpfnIcmpSendEcho)(HANDLE, DWORD, LPVOID, WORD, LPVOID,
                                    LPVOID, DWORD, DWORD) = NULL;


   /*
    *  Load the ICMP.DLL
    */
   hICMP_DLL = LoadLibrary("ICMP.DLL");
   if (hICMP_DLL == 0)
   {
      DEBUG_LOG(("LoadLibrary() failed: Unable to locate ICMP.DLL!"));
      goto cleanup;
   }

   /*
    * Get pointers to ICMP.DLL functions
    */
   lpfnIcmpCreateFile = (void * (__stdcall *)(void))GetProcAddress( (HINSTANCE)hICMP_DLL, "IcmpCreateFile");
   lpfnIcmpCloseHandle = (int (__stdcall *)(void *))GetProcAddress( (HINSTANCE)hICMP_DLL, "IcmpCloseHandle");
   lpfnIcmpSendEcho = (unsigned long (__stdcall *)(void *, unsigned long, void *, unsigned short,
                       void *, void *, unsigned long, unsigned long))GetProcAddress( (HINSTANCE)hICMP_DLL, "IcmpSendEcho" );

   if ((!lpfnIcmpCreateFile) ||
         (!lpfnIcmpCloseHandle) ||
         (!lpfnIcmpSendEcho))
   {
      DEBUG_LOG(("GetProcAddr() failed for at least one function."));
      goto cleanup;
   }


   /*
    * IcmpCreateFile() - Open the ping service
    */
   hICMP = (HANDLE) lpfnIcmpCreateFile();
   if (hICMP == INVALID_HANDLE_VALUE)
   {
      DEBUG_LOG(("IcmpCreateFile() failed"));
      goto cleanup;
   }

   /*
    * Init data buffer printable ASCII
    *  32 (space) through 126 (tilde)
    */
   for (j = 0, i = 32; j < nDataLen; j++, i++)
   {
      if (i >= 126)
         i = 32;
      achReqData[j] = i;
   }

   /*
   * Init IPInfo structure
   */
   lpstIPInfo = &stIPInfo;
   stIPInfo.Ttl = nTTL;
   stIPInfo.Tos = nTOS;
   stIPInfo.Flags = 0;
   stIPInfo.OptionsSize = 0;
   stIPInfo.OptionsData = NULL;



   /*
    * IcmpSendEcho() - Send the ICMP Echo Request
    *                   and read the Reply
    */
   dwReplyCount = lpfnIcmpSendEcho(
                     hICMP,
                     IP,
                     achReqData,
                     nDataLen,
                     lpstIPInfo,
                     achRepData,
                     sizeof(achRepData),
                     nTimeOut);
   if (dwReplyCount != 0)
   {
      //////////IN_ADDR stDestAddr;
      DWORD dwStatus;

      pingTime = (*(UnsignedInt *) & (achRepData[8]));

      // I've seen the ping time bigger than the timeout by a little
      //   bit.  How lame.
      if (pingTime > timeout)
         pingTime = timeout;

      dwStatus = *(DWORD *) & (achRepData[4]);
      if (dwStatus != IP_SUCCESS)
      {
         DEBUG_LOG(("ICMPERR: %d", dwStatus));
      }

   }
   else
   {
      DEBUG_LOG(("IcmpSendEcho() failed: %d", dwReplyCount));
      // Ok we didn't get a packet, just say everything's OK
      //  and the time was -1
      pingTime = -1;
      goto cleanup;
   }


   /*
    * IcmpCloseHandle - Close the ICMP handle
    */
   fRet = lpfnIcmpCloseHandle(hICMP);
   if (fRet == FALSE)
   {
      DEBUG_LOG(("Error closing ICMP handle"));
   }

   // Say what you will about goto's but it's handy for stuff like this
cleanup:

   // Shut down...
   if (hICMP_DLL)
      FreeLibrary((HINSTANCE)hICMP_DLL);

   return pingTime;
}


//-------------------------------------------------------------------------
