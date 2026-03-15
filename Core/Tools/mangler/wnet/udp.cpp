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

#include "udp.h"
#include "wlib/wdebug.h"

UDP::UDP()
{
  fd=0;
}

UDP::~UDP()
{
}

sint32 UDP::Bind(char *Host,uint16 port)
{
  char hostName[100];
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (isdigit(Host[0]))
    return ( Bind( ntohl(inet_addr(Host)), port) );

  strcpy(hostName, Host);

  hostStruct = gethostbyname(Host);
  if (hostStruct == nullptr)
    return (0);
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( Bind(ntohl(hostNode->s_addr),port) );
}

// You must call bind, implicit binding is for sissies
//   Well... you can get implicit binding if you pass 0 for either arg
sint32 UDP::Bind(uint32 IP,uint16 Port)
{
  int retval;
  int status;

  IP=htonl(IP);
  Port=htons(Port);

  addr.sin_family=AF_INET;
  addr.sin_port=Port;
  addr.sin_addr.s_addr=IP;
  fd=socket(AF_INET,SOCK_DGRAM,DEFAULT_PROTOCOL);
  #ifdef _WIN32
  if (fd==SOCKET_ERROR)
    fd=-1;
  #endif
  if (fd==-1)
    return(UNKNOWN);

  retval=bind(fd,(struct sockaddr *)&addr,sizeof(addr));

  #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
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

bit8 UDP::getLocalAddr(uint32 &ip, uint16 &port)
{
  ip=myIP;
  port=myPort;
  return(OK);
}


// private function
sint32 UDP::SetBlocking(bit8 block)
{
  #ifdef _WIN32
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


sint32 UDP::Write(uint8 *msg,uint32 len,uint32 IP,uint16 port)
{
  sint32 retval;
  struct sockaddr_in to;

  // This happens frequently
  if ((IP==0)||(port==0)) return(ADDRNOTAVAIL);

  errno=0;
  to.sin_port=htons(port);
  to.sin_addr.s_addr=htonl(IP);
  to.sin_family=AF_INET;

  ClearStatus();
  retval=sendto(fd,(char *)msg,len,0,(struct sockaddr *)&to,sizeof(to));
  #ifdef _WIN32
  if (retval==SOCKET_ERROR)
    retval=-1;
  #endif

  return(retval);
}

sint32 UDP::Read(uint8 *msg,uint32 len,sockaddr_in *from)
{
  sint32 retval;
  int    alen=sizeof(sockaddr_in);

  if (from!=nullptr)
  {
    retval=recvfrom(fd,(char *)msg,len,0,(struct sockaddr *)from,&alen);
    #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
    #endif
  }
  else
  {
    retval=recvfrom(fd,(char *)msg,len,0,nullptr,nullptr);
    #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
    #endif
  }
  return(retval);
}


void UDP::ClearStatus(void)
{
  #ifndef _WIN32
  errno=0;
  #endif
}

UDP::sockStat UDP::GetStatus(void)
{
 #ifdef _WIN32
  int status=WSAGetLastError();
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
  else     return(UNKNOWN);
 #else
  int status=errno;
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



//
// Wait for net activity on this socket
//
int UDP::Wait(sint32 sec,sint32 usec,fd_set &returnSet)
{
  fd_set inputSet;

  FD_ZERO(&inputSet);
  FD_SET(fd,&inputSet);

  return(Wait(sec,usec,inputSet,returnSet));
}

//
// Wait for net activity on a list of sockets
//
int UDP::Wait(sint32 sec,sint32 usec,fd_set &givenSet,fd_set &returnSet)
{
  Wtime        timeout,timenow,timethen;
  fd_set       backupSet;
  int          retval=0,done,givenMax;
  bit8         noTimeout=FALSE;
  timeval      tv;

  returnSet=givenSet;
  backupSet=returnSet;

  if ((sec==-1)&&(usec==-1))
    noTimeout=TRUE;

  timeout.SetSec(sec);
  timeout.SetUsec(usec);
  timethen+=timeout;

  givenMax=fd;
  for (uint32 i=0; i<(sizeof(fd_set)*8); i++)   // i=maxFD+1
  {
    if (FD_ISSET(i,&givenSet))
      givenMax=i;
  }
  ///DBGMSG("WAIT  fd="<<fd<<"  givenMax="<<givenMax);

  done=0;
  while( ! done)
  {
    if (noTimeout)
      retval=select(givenMax+1,&returnSet,0,0,nullptr);
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





// Set the kernel buffer sizes for incoming, and outgoing packets
//
// Linux seems to have a buffer max of 32767 bytes for this,
//  (which is the default). If you try and set the size to
//  greater than the default it just sets it to 32767.

bit8 UDP::SetInputBuffer(uint32 bytes)
{
  #ifndef _WIN32
   int retval,arg=bytes;

   retval=setsockopt(fd,SOL_SOCKET,SO_RCVBUF,
     (char *)&arg,sizeof(int));
   if (retval==0)
     return(TRUE);
   else
     return(FALSE);
  #else
    return(FALSE);
  #endif
}

// Same note goes for the output buffer

bit8 UDP::SetOutputBuffer(uint32 bytes)
{
  #ifndef _WIN32
   int retval,arg=bytes;

   retval=setsockopt(fd,SOL_SOCKET,SO_SNDBUF,
     (char *)&arg,sizeof(int));
   if (retval==0)
     return(TRUE);
   else
     return(FALSE);
  #else
    return(FALSE);
  #endif
}

// Get the system buffer sizes

int UDP::GetInputBuffer(void)
{
  #ifndef _WIN32
   int retval,arg=0,len=sizeof(int);

   retval=getsockopt(fd,SOL_SOCKET,SO_RCVBUF,
     (char *)&arg,&len);
   return(arg);
  #else
    return(0);
  #endif
}


int UDP::GetOutputBuffer(void)
{
  #ifndef _WIN32
   int retval,arg=0,len=sizeof(int);

   retval=getsockopt(fd,SOL_SOCKET,SO_SNDBUF,
     (char *)&arg,&len);
   return(arg);
  #else
    return(0);
  #endif
}
