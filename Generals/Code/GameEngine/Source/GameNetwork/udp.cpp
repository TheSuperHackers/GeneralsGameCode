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

// FILE: Udp.cpp //////////////////////////////////////////////////////////////
// Implementation of UDP socket wrapper class (taken from wnet lib)
// Author: Matthew D. Campbell, July 2001
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/GameEngine.h"
//#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/udp.h"


//-------------------------------------------------------------------------

#ifdef DEBUG_LOGGING

#define CASE(x) case (x): return #x;

AsciiString GetWSAErrorString( Int error )
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
		{
			AsciiString ret;
			ret.format("Not a Winsock error (%d)", error);
			return ret;
		}
	}
	return AsciiString::TheEmptyString; // will not be hit, ever.
}

#undef CASE

#endif // defined(RTS_DEBUG) || defined(RTS_INTERNAL)

//-------------------------------------------------------------------------

UDP::UDP()
{
  fd=0;
}

UDP::~UDP()
{
	if (fd)
		closesocket(fd);
}

Int UDP::Bind(const char *Host,UnsignedShort port)
{
  char hostName[100];
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (isdigit(Host[0]))
    return ( Bind( ntohl(inet_addr(Host)), port) );

  strcpy(hostName, Host);

  hostStruct = gethostbyname(Host);
  if (hostStruct == NULL)
    return (0);
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( Bind(ntohl(hostNode->s_addr),port) );
}

// You must call bind, implicit binding is for sissies
//   Well... you can get implicit binding if you pass 0 for either arg
Int UDP::Bind(UnsignedInt IP,UnsignedShort Port)
{
  int retval;
  int status;

  IP=htonl(IP);
  Port=htons(Port);

  addr.sin_family=AF_INET;
  addr.sin_port=Port;
  addr.sin_addr.s_addr=IP;
  fd=socket(AF_INET,SOCK_DGRAM,DEFAULT_PROTOCOL);
  #ifdef _WINDOWS
  if (fd==SOCKET_ERROR)
    fd=-1;
  #endif
  if (fd==-1)
    return(UNKNOWN);

  retval=bind(fd,(struct sockaddr *)&addr,sizeof(addr));

  #ifdef _WINDOWS
  if (retval==SOCKET_ERROR)
	{
    retval=-1;
		m_lastError = WSAGetLastError();
	}
  #endif
  if (retval==-1)
  {
    status=GetStatus();
    //CERR("Bind failure (" << status << ") IP " << IP << " PORT " << Port )
    return(status);
  }

  int namelen=sizeof(addr);
  getsockname(fd, (struct sockaddr *)&addr, &namelen); 

  myIP=ntohl(addr.sin_addr.s_addr);
  myPort=ntohs(addr.sin_port);

  retval=SetBlocking(FALSE);
  if (retval==-1)
    fprintf(stderr,"Couldn't set nonblocking mode!\n");

  return(OK);
}

Int UDP::getLocalAddr(UnsignedInt &ip, UnsignedShort &port)
{
  ip=myIP;
  port=myPort;
  return(OK);
}


// private function
Int UDP::SetBlocking(Int block)
{
  #ifdef _WINDOWS
   unsigned long flag=1;
   if (block)
     flag=0;
   int retval;
   retval=ioctlsocket(fd,FIONBIO,&flag);
   if (retval==SOCKET_ERROR)
     return(UNKNOWN);
   else
     return(OK);
  #else  // UNIX
   int flags = fcntl(fd, F_GETFL, 0);
   if (block==FALSE)          // set nonblocking
     flags |= O_NONBLOCK;
   else                       // set blocking
     flags &= ~(O_NONBLOCK);

   if (fcntl(fd, F_SETFL, flags) < 0)
   {
     return(UNKNOWN);
   }
   return(OK);
  #endif
}


Int UDP::Write(const unsigned char *msg,UnsignedInt len,UnsignedInt IP,UnsignedShort port)
{
  Int retval;
  struct sockaddr_in to;

  // This happens frequently
  if ((IP==0)||(port==0)) return(ADDRNOTAVAIL);

#ifdef _UNIX
  errno=0;
#endif
  to.sin_port=htons(port);
  to.sin_addr.s_addr=htonl(IP);
  to.sin_family=AF_INET;

  ClearStatus();
  retval=sendto(fd,(const char *)msg,len,0,(struct sockaddr *)&to,sizeof(to));
  #ifdef _WINDOWS
  if (retval==SOCKET_ERROR)
	{
    retval=-1;
		m_lastError = WSAGetLastError();
#ifdef DEBUG_LOGGING
		static Int errCount = 0;
#endif
		DEBUG_ASSERTLOG(errCount++ > 100, ("UDP::Write() - WSA error is %s\n", GetWSAErrorString(WSAGetLastError()).str()));
	}
  #endif
  
  return(retval);
}

Int UDP::Read(unsigned char *msg,UnsignedInt len,sockaddr_in *from)
{
  Int retval;
  int    alen=sizeof(sockaddr_in);

  if (from!=NULL)
  {
    retval=recvfrom(fd,(char *)msg,len,0,(struct sockaddr *)from,&alen);
    #ifdef _WINDOWS
    if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				// failing because of a blocking error isn't really such a bad thing.
				m_lastError = WSAGetLastError();
#ifdef DEBUG_LOGGING
				static Int errCount = 0;
#endif
				DEBUG_ASSERTLOG(errCount++ > 100, ("UDP::Read() - WSA error is %s\n", GetWSAErrorString(WSAGetLastError()).str()));
				retval = -1;
			} else {
				retval = 0;
			}
		}
    #endif
  }
  else
  {
    retval=recvfrom(fd,(char *)msg,len,0,NULL,NULL);
    #ifdef _WINDOWS
    if (retval==SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				// failing because of a blocking error isn't really such a bad thing.
				m_lastError = WSAGetLastError();
#ifdef DEBUG_LOGGING
				static Int errCount = 0;
#endif
				DEBUG_ASSERTLOG(errCount++ > 100, ("UDP::Read() - WSA error is %s\n", GetWSAErrorString(WSAGetLastError()).str()));
				retval = -1;
			} else {
				retval = 0;
			}
		}
    #endif
  }
  return(retval);
}


void UDP::ClearStatus(void)
{
  #ifndef _WINDOWS
  errno=0;
  #endif

	m_lastError = 0;
}

UDP::sockStat UDP::GetStatus(void)
{
	Int status = m_lastError;
 #ifdef _WINDOWS
  //int status=WSAGetLastError();
  if (status==0) return(OK);
  else if (status==WSAEINTR) return(INTR);
  else if (status==WSAEINPROGRESS) return(INPROGRESS);
  else if (status==WSAECONNREFUSED) return(CONNREFUSED);
  else if (status==WSAEINVAL) return(INVAL);
  else if (status==WSAEISCONN) return(ISCONN);
  else if (status==WSAENOTSOCK) return(NOTSOCK);
  else if (status==WSAETIMEDOUT) return(TIMEDOUT);
  else if (status==WSAEALREADY) return(ALREADY);
  else if (status==WSAEWOULDBLOCK) return(WOULDBLOCK);
  else if (status==WSAEBADF) return(BADF);
  else     return((UDP::sockStat)status);
 #else
  //int status=errno;
  if (status==0) return(OK);
  else if (status==EINTR) return(INTR);
  else if (status==EINPROGRESS) return(INPROGRESS);
  else if (status==ECONNREFUSED) return(CONNREFUSED);
  else if (status==EINVAL) return(INVAL);
  else if (status==EISCONN) return(ISCONN);
  else if (status==ENOTSOCK) return(NOTSOCK);
  else if (status==ETIMEDOUT) return(TIMEDOUT);
  else if (status==EALREADY) return(ALREADY);
  else if (status==EAGAIN) return(AGAIN);
  else if (status==EWOULDBLOCK) return(WOULDBLOCK);
  else if (status==EBADF) return(BADF);
  else     return(UNKNOWN);
 #endif
}



/*
//
// Wait for net activity on this socket
//
int UDP::Wait(Int sec,Int usec,fd_set &returnSet)
{
  fd_set inputSet;
 
  FD_ZERO(&inputSet);
  FD_SET(fd,&inputSet);
 
  return(Wait(sec,usec,inputSet,returnSet));
}
*/

/*
//
// Wait for net activity on a list of sockets
//
int UDP::Wait(Int sec,Int usec,fd_set &givenSet,fd_set &returnSet)
{
  Wtime        timeout,timenow,timethen;
  fd_set       backupSet;
  int          retval=0,done,givenMax;
  Bool         noTimeout=FALSE;
  timeval      tv;
 
  returnSet=givenSet;
  backupSet=returnSet;
 
  if ((sec==-1)&&(usec==-1))
    noTimeout=TRUE;
 
  timeout.SetSec(sec);
  timeout.SetUsec(usec);
  timethen+=timeout;
 
  givenMax=fd;
  for (UnsignedInt i=0; i<(sizeof(fd_set)*8); i++)   // i=maxFD+1
  {
    if (FD_ISSET(i,&givenSet))
      givenMax=i;
  }
  ///DBGMSG("WAIT  fd="<<fd<<"  givenMax="<<givenMax);
 
  done=0;
  while( ! done)
  {
    if (noTimeout)
      retval=select(givenMax+1,&returnSet,0,0,NULL);
    else
    {
      timeout.GetTimevalMT(tv);
      retval=select(givenMax+1,&returnSet,0,0,&tv);
    }
 
    if (retval>=0)
      done=1;

    else if ((retval==-1)&&(errno==EINTR))  // in case of signal
    {
      if (noTimeout==FALSE)
      {
        timenow.Update();
        timeout=timethen-timenow;
      }
      if ((noTimeout==FALSE)&&(timenow.GetSec()==0)&&(timenow.GetUsec()==0))
        done=1;
      else
        returnSet=backupSet;
    }
    else  // maybe out of memory?
    {
      done=1;
    }
  }
  ///DBGMSG("Wait retval: "<<retval);
  return(retval);
}
*/




// Set the kernel buffer sizes for incoming, and outgoing packets
//
// Linux seems to have a buffer max of 32767 bytes for this,
//  (which is the default). If you try and set the size to
//  greater than the default it just sets it to 32767.

Int UDP::SetInputBuffer(UnsignedInt bytes)
{
   int retval,arg=bytes;

   retval=setsockopt(fd,SOL_SOCKET,SO_RCVBUF,
     (char *)&arg,sizeof(int));
   if (retval==0)
     return(TRUE);
   else
     return(FALSE);
}

// Same note goes for the output buffer

Int UDP::SetOutputBuffer(UnsignedInt bytes)
{
   int retval,arg=bytes;

   retval=setsockopt(fd,SOL_SOCKET,SO_SNDBUF,
     (char *)&arg,sizeof(int));
   if (retval==0)
     return(TRUE);
   else
     return(FALSE);
}

// Get the system buffer sizes 

int UDP::GetInputBuffer(void)
{
   int retval,arg=0,len=sizeof(int);

   retval=getsockopt(fd,SOL_SOCKET,SO_RCVBUF,
     (char *)&arg,&len);
   return(arg);
}


int UDP::GetOutputBuffer(void)
{
   int retval,arg=0,len=sizeof(int);

   retval=getsockopt(fd,SOL_SOCKET,SO_SNDBUF,
     (char *)&arg,&len);
   return(arg);
}

Int UDP::AllowBroadcasts(Bool status)
{
	int retval;
	BOOL val = status;
	retval = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&val, sizeof(BOOL));
	if (retval == 0)
		return TRUE;
	else
		return FALSE;
}
