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

/****************************************************************************\
TCP                   Neal Kettler        neal@westwood.com
******************************************************************************

A general purpose TCP class that can be used in either CLIENT or
SERVER mode.  Note that this uses non-blocking sockets.

The FD_* macros:

  FD_CLR(int fd, fd_set *set);     // clear a single FD
  FD_ISSET(int fd, fd_set *set);   // check whether a single FD is set
  FD_SET(int fd, fd_set *set);     // set a single FD
  FD_ZERO(fd_set * set);           // clear the entire set

NOTE:  The fd_set returned by 'Wait' is static, don't call delete
on it!


If you are writing a CLIENT:
   The last argument to many functions is an integer whichFD, this is used
only by SERVER mode, so you can omit this argument.  Sample Code:

fd_set *fdSet;
uint8  *buff=new uint8[1024];
int     retval;
TCP tcp(CLIENT);

tcp.Bind((uint32)0,(uint16)0); // let system pick local IP and a Port for you
tcp.Connect("tango",13);       // can connect by name or "10.1.1.10"
                               // or the integer in host byte order

fdSet=tcp.Wait(10,0);          // wait for UP TO 10 sec and 0 microseconds
if (FD_ISSET(tcp.GetFD(),fdSet))   // Is there something to read?
{
  retval=tcp.Read(buff,1024);  // Read something
                               //  Retval will contain the number of
                               //  bytes read, or...
                               //  0 = remote end closed connection
                               // -1 = nothing to read
  fprintf(stderr,"%s",buff);
}
else
  fprintf(stderr,"Nothing was read!\n");


If you are writing a SERVER:

    The structure called 'clientList' contains all the File Descriptors
that have connected to the server.  Make sure you look at the FD_*
functions so you can use this sort of structure.  When you are writing
a server, you need to specify the 'whichFD' arguments to all the
functions. Sample Code:

fd_set *fdSet;
uint8  *buff=new uint8[1024];
int     retval;
TCP     tcp(SERVER);

tcp.Bind((uint32)0,(uint16)2121);    // You need to bind to a well defined
                              //  port number or nobody will know where
                              //  to connect to.

while (1)
{
  fdSet=tcp.Wait(-1,-1);        // Wait until there is something on the socket
  if (FD_ISSET(tcp.GetFD(),fdSet))  // somebody must want a connection
  {
    retval=tcp.GetConnection(); // Get a connection if somebody's trying
    if (retval!=-1)
    {
      tcp.Write("Hello World!\n",strlen("Hello World!\n"),retval);
      tcp.Close(retval);
    }
  }
}


\****************************************************************************/


#include "tcp.h"
#include <stdarg.h>

#ifndef _WIN32
#include <errno.h>
#define closesocket close
#endif

// newMode should be either CLIENT or SERVER
TCP::TCP(int new_mode)
{
  mode=CLIENT;
  maxFD=0;
  fd = -1;
  clientCount=0;
  if ((new_mode==CLIENT)||(new_mode==SERVER))
    mode=new_mode;
  FD_ZERO(&clientList);
  connectionState=CLOSED;
  inputDelay=5;
  outputDelay=5;
}

// Create a TCP object on a pre-existing socket
TCP::TCP(int new_mode,sint16 socket)
{
  sint32 retval;

  mode=CLIENT;
  maxFD= socket;
  fd = socket;
  clientCount=0;
  if ((new_mode==CLIENT)||(new_mode==SERVER))
    mode=new_mode;
  FD_ZERO(&clientList);

  inputDelay=5;
  outputDelay=5;


  retval=SetBlocking(FALSE,socket);      // set to NB mode
  //DBGMSG("Setblocking: "<<retval);

  connectionState=CLOSED;
  if (mode==CLIENT)               // determine what state the socket is in
  {
    connectionState=CONNECTING;   // this is used when state is unsure
    if (IsConnected(socket))
      connectionState=CONNECTED;
    else
      connectionState=CLOSED;
  }
  //DBGMSG("Connstate = "<<connectionState);
}


TCP::~TCP()
{
   CloseAll();
}


int TCP::GetFD()
{
  return(fd);
}


// private function
sint32 TCP::SetBlocking(bit8 block,sint32 whichFD)
{
   if (whichFD==0)
     whichFD=fd;

   #ifdef _WIN32
   unsigned long flag=1;
   if (block)
     flag=0;
   int retval;
   retval=ioctlsocket(whichFD,FIONBIO,&flag);
   if (retval==SOCKET_ERROR)
     return(-1);
   else
     return(0);
   #else
   int flags = fcntl(whichFD, F_GETFL, 0);
   if (block==FALSE)          // set nonblocking
     flags |= O_NONBLOCK;
   else                       // set blocking
     flags &= ~(O_NONBLOCK);

   if (fcntl(whichFD, F_SETFL, flags) < 0)
   {
     return(-1);
   }
   return(0);
   #endif
}


sint32 TCP::GetMaxFD(void)
{
  if (mode==CLIENT)
    return(fd);
  else if (mode==SERVER)
    return(maxFD);
  else
    return(-1);
}

// Only specify whichFD if this is a server application
sint32 TCP::Write(const uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval;

  if (whichFD==0)
  {
    if (mode==SERVER)
      assert(FALSE);
    whichFD=fd;
  }
  SetBlocking(TRUE,whichFD);
  retval=send(whichFD,(const char *)msg,len,0);
  #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
  #endif
  SetBlocking(FALSE,whichFD);
  return(retval);
}


// Only specify whichFD if this is a server application
// NON BLOCKING WRITE
sint32 TCP::WriteNB(uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval;

  if (whichFD==0)
  {
    if (mode==SERVER)
      assert(FALSE);
    whichFD=fd;
  }
  retval=send(whichFD,(const char *)msg,len,0);
  #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
  #endif
  return(retval);
}


// Encapsulate data for lame ass proxys that won't pass 0's or 255's through
//   0 goes to 1,1
//   1 goes to 1,2
// 255 goes to 1,3
// everything else is the same
sint32 TCP::EncapsulatedWrite(uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval;
  uint32 i,bytesSent=0;
  uint8  data,one=1;

  if (mode==CLIENT)
    whichFD=fd;
  SetBlocking(TRUE,whichFD);
  for (i=0; i<len; i++)
  {
    data=msg[i];
    if ((data>1)&&(data<255))
    {
      retval=send(whichFD,(char *)&data,1,0);
      if (retval<1)
      {
        SetBlocking(FALSE,whichFD);
        return(i);
      }
      bytesSent++;
    }
    else
    {
      retval=send(whichFD,(char *)&one,1,0);
      if (retval<1)
      {
        SetBlocking(FALSE,whichFD);
        return(i);
      }
      if (data==0)
        data=1;
      else if (data==1)
        data=2;
      else if (data==255)
        data=3;

      retval=send(whichFD,(char *)&data,1,0);
      if (retval<1)
      {
        SetBlocking(FALSE,whichFD);
        return(i);
      }
      bytesSent+=2;
    }
  }
  SetBlocking(FALSE,whichFD);
  ///fprintf(stderr,"\n\nENCAP SENT %d\n\n",bytesSent);
  return(len);
}


// Make sure string is '\0' terminated
sint32 TCP::WriteString(char *msg,sint32 whichFD)
{
  if (mode==CLIENT)
    whichFD=fd;

  WaitWrite(whichFD);

  sint32 retval;

  if (mode==CLIENT)
  {
    SetBlocking(TRUE,fd);
    retval=send(fd,msg,strlen(msg),0);
    SetBlocking(FALSE,fd);
    return(retval);
  }
  else if (mode==SERVER)
  {
    if ((whichFD<=maxFD) && (FD_ISSET(whichFD,&clientList)))
    {
      SetBlocking(TRUE,whichFD);
      retval=send(whichFD,msg,strlen(msg),0);
      SetBlocking(FALSE,whichFD);
      return(retval);
    }
  }
  return(-1);
}


// only use for strings up to 1024 chars!
sint32 TCP::Printf(sint32 whichFD,const char *format,...)
{
  va_list arg;
  char string[1024];
  sint32 retval;
  va_start(arg,format);
  vsprintf(string,format,arg);
  va_end(arg);

  if (mode==CLIENT)
    whichFD=fd;

  WaitWrite(fd);
  if (mode==CLIENT)
  {
    SetBlocking(TRUE,whichFD);
    retval=send(fd,string,strlen(string),0);
    SetBlocking(FALSE,whichFD);
    return(retval);
  }
  else if (mode==SERVER)
  {
    if ((whichFD<=maxFD) && (FD_ISSET(whichFD,&clientList)))
    {
      SetBlocking(TRUE,whichFD);
      retval=send(whichFD,string,strlen(string),0);
      SetBlocking(FALSE,whichFD);
      return(retval);
    }
  }
  return(-1);
}



// Returns 0 on failure
// Returns IP in host byte order!
uint32 TCP::GetRemoteIP(sint32 whichFD)
{
  struct sockaddr_in sin;
  int    sinSize=sizeof(sin);

  if (mode==CLIENT)
  {
    if(getpeername(fd,(sockaddr *)&sin,&sinSize)==0)
      return(ntohl(sin.sin_addr.s_addr));
  }
  else if (mode==SERVER)
  {
    if(getpeername(whichFD,(sockaddr *)&sin,&sinSize)==0)
      return(ntohl(sin.sin_addr.s_addr));
  }
  return(0);
}


// Returns 0 on failure
// Returns Port in host byte order!
uint16 TCP::GetRemotePort(sint32 whichFD)
{
  struct sockaddr_in sin;
  int    sinSize=sizeof(sin);

  if (mode==CLIENT)
  {
    if(getpeername(fd,(sockaddr *)&sin,&sinSize)==0)
      return(ntohs(sin.sin_port));
  }
  else if (mode==SERVER)
  {
    if(getpeername(whichFD,(sockaddr *)&sin,&sinSize)==0)
      return(ntohs(sin.sin_port));
  }
  return(0);
}


// Is the FD connected?
bit8 TCP::IsConnected(sint32 whichFD)
{
  struct sockaddr_in sin;
  int    sinSize=sizeof(sin);

  if (mode==CLIENT)
    whichFD=fd;

  if (mode==CLIENT)
  {
    if (connectionState==CONNECTED)
      return(TRUE);
    if (connectionState==CLOSED)
      return(FALSE);
  }

  // only get here if state==CONNECTING
  if(getpeername(whichFD,(sockaddr *)&sin,&sinSize)==0)
    if ( (sin.sin_addr.s_addr!=htonl(0)) && (CanWrite(whichFD)) )
    {
      connectionState=CONNECTED;
      return(TRUE);
    }
  return(FALSE);
}


// Not portable?
/**************
sint32 TCP::GetSockStatus(sint32 whichFD)
{
  sint32 retval;
  int status,size=sizeof(int);

  if (whichFD==0)
    whichFD=fd;
  retval=getsockopt(whichFD,SOL_SOCKET,SO_ERROR,(char *)&status,&size);
  if (retval==-1)
    return(-1);
  return(status);
}
*******************/



// The TCP equivalent of fgets()
char *TCP::Gets(char *string,int n,int whichFD)
{
  char c;
  int  retval,i=0;
  fd_set fdSet;

  if (whichFD==0)
    whichFD=GetFD();

  if (whichFD <= 0)
    return(nullptr);

  memset(string,0,n);

  while(1)
  {
    if (i==n)
      return(string);

    Wait(inputDelay,0,fdSet,whichFD);   // inputDelay = 5 sec or so
    if (! FD_ISSET(whichFD,&fdSet))
    {
      DBGMSG("Gets timeout: " << inputDelay);
      return(nullptr);
    }

    retval=Read((unsigned char *)&c,1,whichFD);
    if ((retval>0)&&(c!=0))
    {
      string[i]=c;
      if (c=='\n')
        return(string);
      i++;
    }
    else if ((retval==0)&&(i==0))
    {
      DBGMSG("Remote endpoint closed (1)");
      return(nullptr);
    }
    else if (retval==0)
      return(string);
  }
  return(string);
}


// only specify whichFD if this is a server
sint32 TCP::Read(uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval;
  //DBGMSG("In read, mode: "<<mode<<"  FD: "<<fd);
  if (mode==CLIENT)
  {
    retval=recv(fd,(char *)msg,len,0);
    ////////DBGMSG("READ: "<<retval << "   ON FD: " << fd << "  LEN: "<< len);
    if (retval==0)
      Close();
    return(retval);
  }
  else if (mode==SERVER)
  {
    if ((whichFD<=maxFD) && (FD_ISSET(whichFD,&clientList)))
    {
      retval=recv(whichFD,(char *)msg,len,0);
      if (retval==0)
      {
        Close(whichFD);
      }
      return(retval);
    }
    else
    {
      return(0);  // closed
    }
  }
  return(-1);
}


// only specify whichFD if this is a server
// Try and read 'len' bytes until the timer goes out.
// This is effectively a blocking call, but it's still useful
// in threaded environments.
sint32 TCP::TimedRead(uint8 *msg,uint32 len,int seconds,sint32 whichFD)
{
  fd_set    set;
  sint32    bytes_read=0;
  sint32    retval;

  time_t stop_time=time(nullptr)+seconds;
  while ((time(nullptr)<=stop_time)&&((uint32)bytes_read<len))
  {
    Wait(1,0,set,whichFD);
    //DBGMSG("Calling read");
    retval=Read(msg+bytes_read,len-bytes_read,whichFD);
    if (retval==0)    // they closed
    {
      DBGMSG("Remote close!");
      return(bytes_read);
    }
    else if (retval>0)
      bytes_read+=retval;
    // otherwise some error
  }
  return(bytes_read);
}



// only specify whichFD if this is a server
// Peek at data in system buffer
sint32 TCP::Peek(uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval;
  if (mode==CLIENT)
  {
    retval=recv(fd,(char *)msg,len,MSG_PEEK);
    if (retval==0)
      Close();
    return(retval);
  }
  else if (mode==SERVER)
  {
    if ((whichFD<=maxFD) && (FD_ISSET(whichFD,&clientList)))
    {
      retval=recv(whichFD,(char *)msg,len,MSG_PEEK);
      if (retval==0)
        Close(whichFD);
      return(retval);
    }
    else
     return(0);  // closed
  }
  return(-1);
}


// only specify whichFD if this is a server
// (this is used for non-8 bit clean pipes, you probably don't
//   want to use it!)
sint32 TCP::EncapsulatedRead(uint8 *msg,uint32 len,sint32 whichFD)
{
  sint32 retval,bytesRead=0;
  uint32 i;
  char data;

  if (mode==CLIENT)
    whichFD=fd;
  else if (mode==SERVER)
  {
   if ((whichFD>maxFD) || (!FD_ISSET(whichFD,&clientList)))
     return(0);
  }
  else
    return(-1);

  for (i=0; i<len; i++)
  {
    retval=recv(fd,&data,1,0);
    if (retval==0)
    {
      Close();
      return(bytesRead);
    }
    if (retval==1)
    {
      bytesRead++;
      if (data==1)
      {
        retval=0;
        while(retval!=1)
        {
          retval=recv(fd,&data,1,0);
          if (retval==0)
          {
            Close();
            return(bytesRead);
          }
        }
        if (data==1)
          data=0;
        else if (data==2)
          data=1;
        else if (data==3)
          data=(char)255;
      }
      msg[i]=data;
    }
    if (retval==-1)
      return(bytesRead);
  }
  return(bytesRead);
}


sint32 TCP::CloseAll(void)
{
  int i;

  if (mode==CLIENT)
    return(Close());

  for(i=0; i<=maxFD; i++)
  {
    if ((i!=fd)&&(FD_ISSET(i,&clientList)))
      Close(i);
  }
  return(Close(fd));  // close the master fd last
}

//
// For clients this is used to give up ownership of a socket.
//  Often used so the destructor won't call close on a socket.
//
void TCP::DisownSocket(void)
{
  if (mode==CLIENT)
  {
    fd=-1;
    connectionState=CLOSED;
  }
}

// for a server 0 = master FD, or a client FD can be passed in
// for a client the whichFD argument is ignored completely
sint32 TCP::Close(sint32 whichFD)
{
  int i;
  if (mode==CLIENT)
  {
    connectionState=CLOSED;
    if(fd != -1)
    {
      sint32 retval = closesocket(fd);
      fd = -1;
      return retval;
    }
  }
  else if (mode==SERVER)
  {
     if (whichFD==0)
     {
       if (shutdown(fd,2)==0)
         return(closesocket(fd));
       else
         return(-1);
     }
     else if ((whichFD<=maxFD) && (FD_ISSET(whichFD,&clientList)))
     {
       if (whichFD==maxFD)  // make sure maxFD is still correct
       {
          for (i=maxFD; i>=0; i--)
            if (FD_ISSET(i,&clientList))
            {
               maxFD=i;
               break;
            }
       }
       FD_CLR((uint32)whichFD,&clientList);
       clientCount--;
       return(closesocket(whichFD));
     }
  }
  return(-1);
}



// if 'sec' AND 'usec' are -1 then this will sleep until
// there is socket activity

int TCP::Wait(sint32 sec,sint32 usec,fd_set &returnSet,sint32 whichFD)
{
  fd_set inputSet;

  FD_ZERO(&inputSet);

  if (mode==SERVER)
  {
    if (whichFD==0)
    {
      inputSet=clientList;
      if (fd > 0)
        FD_SET(fd,&inputSet);
    }
    else if (whichFD > 0)
      FD_SET(whichFD,&inputSet);
  }
  else if (mode==CLIENT)
  {
    if (whichFD==0)
      whichFD=fd;
    if (whichFD > 0)
      FD_SET(whichFD,&inputSet);
  }

  return(Wait(sec,usec,inputSet,returnSet));
}

int TCP::Wait(sint32 sec,sint32 usec,fd_set &givenSet,fd_set &returnSet)
{
  Wtime        timeout;
  Wtime        timenow;
  Wtime        timethen;
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

  givenMax=maxFD;
  for (uint32 i=0; i<(sizeof(fd_set)*8); i++)   // i=maxFD+1
  {
    if (FD_ISSET(i,&givenSet))
      givenMax=i;
  }

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
  return(retval);
}


void TCP::WaitWrite(sint32 whichFD)
{
  fd_set       backupSet;
  int          retval=0,done;
  fd_set       outputSet;

  if (whichFD==0)
    whichFD=fd;

  if (whichFD==-1)
    return;

  FD_ZERO(&outputSet);
  FD_SET(whichFD,&outputSet);
  backupSet=outputSet;

  done=0;
  while( ! done)
  {
    retval=select(maxFD+1,0,&outputSet,0,nullptr);

    if (retval>=0)
      done=1;

    else if ((retval==-1)&&(errno==EINTR))  // in case of signal
      outputSet=backupSet;
    else  // maybe out of memory?
      done=1;
  }
}

// Can a FD be written to?
bit8 TCP::CanWrite(sint32 whichFD)
{
  int          retval=0;
  fd_set       outputSet;
  Wtime     timeout;
  timeval      tv;

  timeout.SetSec(0);
  timeout.SetUsec(0);

  if (whichFD==0)
    whichFD=fd;

  FD_ZERO(&outputSet);
  FD_SET(whichFD,&outputSet);

  timeout.GetTimevalMT(tv);
  retval=select(whichFD+1,0,&outputSet,0,&tv);
  if (retval>0)
    return(TRUE);
  else
    return(FALSE);
}


bit8 TCP::Bind(char *Host,uint16 port,bit8 reuseAddr)
{
  char hostName[100];
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (isdigit(Host[0]))
    return ( Bind( ntohl(inet_addr(Host)), port,reuseAddr) );

  strcpy(hostName, Host);

  hostStruct = gethostbyname(Host);
  if (hostStruct == nullptr)
    return (0);
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( Bind(ntohl(hostNode->s_addr),port,reuseAddr) );
}


// You must call bind, implicit binding is for sissies
//   Well... you can get implicit binding if you pass 0 for either arg

bit8 TCP::Bind(uint32 IP,uint16 Port,bit8 reuseAddr)
{
  int retval;
  int status;

  IP=htonl(IP);
  Port=htons(Port);

  addr.sin_family=AF_INET;
  addr.sin_port=Port;
  addr.sin_addr.s_addr=IP;
  fd=socket(AF_INET,SOCK_STREAM,DEFAULT_PROTOCOL);
  if (fd==-1)
    return(FALSE);

  retval=SetBlocking(FALSE,fd);
  if (retval==-1)
    ERRMSG("Couldn't set nonblocking mode!");

  if (reuseAddr==TRUE)
  {
    uint32 opval;

    #ifdef SO_REUSEPORT
/******************  this may make the socket get garbage data??
    opval=1;
    retval=setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&opval,sizeof(opval));
    if (retval!=0)
      fprintf(stderr,"Could not set socket to SO_REUSEPORT\n");
**********************/
    #endif
    #ifdef SO_REUSEADDR
    opval=1;
    retval=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&opval,sizeof(opval));
    if (retval!=0)
      fprintf(stderr,"Could not set socket to SO_REUSEADDR\n");
    #endif
  }

  retval=bind(fd,(struct sockaddr *)&addr,sizeof(addr));
  #ifdef _WIN32
    if (retval==SOCKET_ERROR)
      retval=-1;
  #endif

  if (retval==-1)
  {
    status=GetStatus();
    DBGMSG("Bind failure (" << status << ") IP "<< IP <<" PORT "<< ntohs(Port));
    return(FALSE);
  }
  myIP=IP;
  myPort=Port;
  maxFD=fd;

  if (mode==SERVER)
    listen(fd,64);  //Solaris needs lots of listen slots for some reason

  return(TRUE);
}


// This is only for clients

bit8 TCP::Connect(char *Host,uint16 port)
{
  char hostName[100];
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (isdigit(Host[0]))
    return ( Connect( ntohl(inet_addr(Host)), port) );

  strcpy(hostName, Host);

  hostStruct = gethostbyname(Host);
  if (hostStruct == nullptr)
  {ERRMSG("Can't resolve host");return (0);}
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( Connect(ntohl(hostNode->s_addr),port) );
}

bit8 TCP::Connect(uint32 IP,uint16 Port)
{
  int tries,result;
  struct timeval sleep_time;
  struct sockaddr_in serverAddr;
  int status;

  IP=htonl(IP);
  Port=htons(Port);

  serverAddr.sin_family=AF_INET;
  serverAddr.sin_port=Port;
  serverAddr.sin_addr.s_addr=IP;

  if (mode!=CLIENT)
  {ERRMSG("Can't connect in server mode");return(FALSE);}

  tries=0;
  result=-1;


  // try 10 connects with a greater and greater sleep time after each one
  // this can go on for upto 5.4 seconds
  while ((tries < 10) && (result == -1))
  {
    ClearStatus();
    result = connect(fd,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
    status=GetStatus();

    #ifdef _WIN32
      if (result==SOCKET_ERROR)
        result=-1;
    #endif

    if ((status == ISCONN) && (result == -1))
    {
      result = 0;
    }
    if (result == -1)
    {
      if ((status!=INPROGRESS)&&(status!=ALREADY)&&(status!=AGAIN)&&
        (status!=WOULDBLOCK))
      {
        Close();
        Bind(myIP,myPort);
      }
      tries++;
      sleep_time.tv_sec = 0;
      sleep_time.tv_usec = (100000*(tries+1));
      #ifdef WIN32
        Sleep((sleep_time.tv_usec)/1000);
      #else
        select(0, 0, 0, 0, &sleep_time);
      #endif
    }
  }

  if (result == -1)
  {
    return(FALSE);
  }
  connectionState=CONNECTED;
  return (TRUE);
}



// Asynchronous Connection
bit8 TCP::ConnectAsync(char *Host,uint16 port)
{
  char hostName[100];
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (isdigit(Host[0]))
    return ( ConnectAsync( ntohl(inet_addr(Host)), port) );

  strcpy(hostName, Host);

  hostStruct = gethostbyname(Host);
  if (hostStruct == nullptr)
    return (0);
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( ConnectAsync(ntohl(hostNode->s_addr),port) );
}

// Asynchronous Connection
bit8 TCP::ConnectAsync(uint32 IP,uint16 Port)
{
  int result;
  struct sockaddr_in serverAddr;
  int status,connectErrno;
  int retval;

  IP=htonl(IP);
  Port=htons(Port);

  serverAddr.sin_family=AF_INET;
  serverAddr.sin_port=Port;
  serverAddr.sin_addr.s_addr=IP;

  if (mode!=CLIENT)
    return(FALSE);

  result=-1;

  if (connectionState==CONNECTING)
  {
    if (IsConnected(fd))
    {
      DBGMSG("CONNECTION COMPLETE at point 1");
      connectionState=CONNECTED;
      return(TRUE);
    }
    else
      return(TRUE);  // Still trying
  }

  ClearStatus();
  result = connect(fd,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
  connectErrno=errno;
  status=GetStatus();

  #ifdef _WIN32
    if (result==SOCKET_ERROR)
    {
      DBGMSG("Socket error 1  " << status);
      result=-1;
    }
  #endif

  // If we have a bogus FD, try again after closing and re-binding
  if ((result==-1)&&((status==BADF)||(status==NOTSOCK)||(status==INVAL)))
  {
    Close();
    retval=Bind(myIP,myPort);
    DBGMSG("BIND = "<<retval);
    ClearStatus();
    result = connect(fd,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
    status=GetStatus();
    #ifdef _WIN32
      if (result==SOCKET_ERROR)
      {
        DBGMSG("Socket error 2  " << status);
        result=-1;
      }
    #endif
  }

  if (result==-1)
  {
    if ((status==ISCONN)||(status==INPROGRESS)||(status==ALREADY)||
      (status==WOULDBLOCK))
    {
      connectionState=CONNECTING;
      return(TRUE);   // The socket's trying to connect
    }
    else  // Must be a "real" problem
    {
      Close();
      DBGMSG("Fail " << connectErrno << " " << status);
      connectionState=CLOSED;
      return(FALSE);
    }
  }
  //printf("Connected for real\n");
  connectionState=CONNECTED;
  return(TRUE);
}




void TCP::ClearStatus(void)
{
  #ifndef _WIN32
  errno=0;
  #endif
}

int TCP::GetStatus(void)
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


// this is only for servers

sint32 TCP::GetConnection(void)
{
  if (mode!=SERVER)
    return(-1);

  sint32 clientFD;
  struct sockaddr_in clientAddr;
  int addrlen=sizeof(clientAddr);

  clientFD=accept(fd,(struct sockaddr *)&clientAddr,&addrlen);
  if (clientFD!=-1)
  {
    if (clientFD>maxFD)
      maxFD=clientFD;
    FD_SET(clientFD,&clientList);
    clientCount++;
  }
  return(clientFD);
}

sint32 TCP::GetConnection(struct sockaddr *clientAddr)
{
  if (mode!=SERVER)
    return(-1);

  sint32 clientFD;
  int addrlen=sizeof(struct sockaddr);

  clientFD=accept(fd,(struct sockaddr *)clientAddr,&addrlen);
  if (clientFD!=-1)
  {
    if (clientFD>maxFD)
      maxFD=clientFD;
    FD_SET(clientFD,&clientList);
    clientCount++;
  }
  return(clientFD);
}

