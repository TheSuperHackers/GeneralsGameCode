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

\****************************************************************************/

#ifndef TCP_HEADER
#define TCP_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#ifdef _WINDOWS

#include <winsock.h>
#include <io.h>
#define close _close
#define read  _read
#define write _write

#else  //UNIX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>

typedef signed int SOCKET;

#endif


#ifdef AIX
#include <sys/select.h>
#endif

#define DEFAULT_PROTOCOL 0

#include "wlib/wstypes.h"
#include "wlib/wdebug.h"
#include "wlib/wtime.h"

class TCP
{

// DATA ---------------

private:
  int         mode;               // client or server
  sint32      fd;                 // the primary FD

  uint32      myIP;               // after bind myIP & myPort will be
  uint16      myPort;             //   whatever we bound to

  struct sockaddr_in addr;
  int         maxFD;              // value of the biggest FD
  int         clientCount;        // how many clients open


  sint32      inputDelay;         // default delay for semi-blocking reads
  sint32      outputDelay;        // default delay for semi-blocking writes 

  enum ConnectionState
  {
     CLOSED,
     CONNECTING,
     CONNECTED
  }           connectionState;     // What state is client FD in 

public:

  enum
  {
    CLIENT = 1,
    SERVER = 2
  };

  // These defines specify a system independent way to
  //   get error codes for socket services.
  enum
  {
    OK,                 // Everything's cool
    UNKNOWN,            // There was an error of unknown type
    ISCONN,             // The socket is already connected
    INPROGRESS,         // The socket is non-blocking and the operation
                        //   isn't done yet
    ALREADY,            // The socket is already attempting a connection
                        //   but isn't done yet
    AGAIN,              // Try again.
    ADDRINUSE,          // Address already in use
    ADDRNOTAVAIL,       // That address is not available on the remote host
    BADF,               // Not a valid FD
    CONNREFUSED,        // Connection was refused
    INTR,               // Operation was interrupted
    NOTSOCK,            // FD wasn't a socket
    PIPE,               // That operation just made a SIGPIPE
    WOULDBLOCK,         // That operation would block
    INVAL,              // Invalid
    TIMEDOUT            // Timeout
  };

  // for client list (if this is a server)
  fd_set clientList;


// CODE ----------------

public:
          TCP(int newMode);
          TCP(int newMode,sint16 socket);
         ~TCP();
  bit8    Bind(uint32 IP,uint16 port,bit8 reuseAddr=FALSE);
  bit8    Bind(char *Host,uint16 port,bit8 reuseAddr=FALSE);

  sint32  GetMaxFD(void);

  bit8    Connect(uint32 IP,uint16 port);
  bit8    Connect(char *Host,uint16 port);
  bit8    ConnectAsync(uint32 IP,uint16 port);
  bit8    ConnectAsync(char *Host,uint16 port);

  bit8    IsConnected(sint32 whichFD=0);

  sint32  GetFD(void);
  sint32  GetClientCount(void) { return(clientCount); }

  // Get IP or Port of a connected endpoint
  uint32  GetRemoteIP(sint32 whichFD=0);
  uint16  GetRemotePort(sint32 whichFD=0);

  sint32  GetConnection(void);
  sint32  GetConnection(struct sockaddr *clientAddr);
  void    WaitWrite(sint32 whichFD=0);
  bit8    CanWrite(sint32 whichFD=0);
  sint32  Write(const uint8 *msg,uint32 len,sint32 whichFD=0);
  sint32  WriteNB(uint8 *msg,uint32 len,sint32 whichFD=0);
  sint32  EncapsulatedWrite(uint8 *msg,uint32 len,sint32 whichFD=0);
  sint32  WriteString(char *msg,sint32 whichFD=0);
  sint32  Printf(sint32 whichFD,const char *format,...);
  sint32  Read(uint8 *msg,uint32 len,sint32 whichFD=0);
  sint32  TimedRead(uint8 *msg,uint32 len,int seconds,sint32 whichFD=0);
  sint32  Peek(uint8 *msg,uint32 len,sint32 whichFD=0);
  sint32  EncapsulatedRead(uint8 *msg,uint32 len,sint32 whichFD=0);

  char   *Gets(char *string,int n,int whichFD=0);

  // Wait on all sockets (or a specified one)
  //   return when ready for reading (or timeout occurs)
  int     Wait(sint32 sec,sint32 usec,fd_set &returnSet,sint32 whichFD=0);
  int     Wait(sint32 sec,sint32 usec,fd_set &inputSet,fd_set &returnSet);

  int     GetStatus(void);
  void    ClearStatus(void);

  //sint32  GetSockStatus(sint32 whichFD=0);

  // give up ownership of the socket without closing it
  void    DisownSocket(void);

  sint32  Close(sint32 whichFD=0);
  sint32  CloseAll(void);   // close all sockets (same as close for client)

  sint32  SetBlocking(bit8 block,sint32 whichFD=0);

  // Set default delays for semi-blocking reads & writes
  // default input = 5, output = 5
  // this is new and not used everywhere
  //
  bit8    SetInputDelay(sint32 delay) { inputDelay=delay; return(TRUE); };
  bit8    SetOutputDelay(sint32 delay) { outputDelay=delay; return(TRUE); };

};

#endif
