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


#pragma once

#ifndef UDP_HEADER
#define UDP_HEADER

#ifdef _UNIX
#include <errno.h>
#endif

#ifdef _WINDOWS
#include <winsock.h>
#include <io.h>
//#define close _close
//#define read  _read
//#define write _write

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
#endif

#ifdef AIX
#include <sys/select.h>
#endif

#include "Lib/BaseType.h"

#define DEFAULT_PROTOCOL 0

//#include "wlib/wstypes.h"
//#include "wlib/wtime.h"

class UDP
{
 // DATA
 private:
  Int       fd;
  UnsignedInt       myIP;
  UnsignedShort       myPort;
  struct       sockaddr_in  addr;

 public:
  // These defines specify a system independent way to
  //   get error codes for socket services.
  enum sockStat CPP_11(: Int)
  {
    OK           =  0,     // Everything's cool
    UNKNOWN      = -1,     // There was an error of unknown type
    ISCONN       = -2,     // The socket is already connected
    INPROGRESS   = -3,     // The socket is non-blocking and the operation
                           //   isn't done yet
    ALREADY      = -4,     // The socket is already attempting a connection
                           //   but isn't done yet
    AGAIN        = -5,     // Try again.
    ADDRINUSE    = -6,     // Address already in use
    ADDRNOTAVAIL = -7,     // That address is not available on the remote host
    BADF         = -8,     // Not a valid FD
    CONNREFUSED  = -9,     // Connection was refused
    INTR         =-10,     // Operation was interrupted
    NOTSOCK      =-11,     // FD wasn't a socket
    PIPE         =-12,     // That operation just made a SIGPIPE
    WOULDBLOCK   =-13,     // That operation would block
    INVAL        =-14,     // Invalid
    TIMEDOUT     =-15      // Timeout
  };

// CODE
 private:
  Int           SetBlocking(Int block);

	Int m_lastError;

 public:
                   UDP();
                  ~UDP();
  Int           Bind(UnsignedInt IP,UnsignedShort port);
  Int           Bind(const char *Host,UnsignedShort port);
  Int           Write(const unsigned char *msg,UnsignedInt len,UnsignedInt IP,UnsignedShort port);
  Int           Read(unsigned char *msg,UnsignedInt len,sockaddr_in *from);
  sockStat         GetStatus(void);
  void             ClearStatus(void);
  //int              Wait(Int sec,Int usec,fd_set &returnSet);
  //int              Wait(Int sec,Int usec,fd_set &givenSet,fd_set &returnSet);

  Int             getLocalAddr(UnsignedInt &ip, UnsignedShort &port);
  Int           getFD(void) { return(fd); }

  Int             SetInputBuffer(UnsignedInt bytes);
  Int             SetOutputBuffer(UnsignedInt bytes);
  int              GetInputBuffer(void);
  int              GetOutputBuffer(void);
	Int						AllowBroadcasts(Bool status);
};

#ifdef DEBUG_LOGGING
AsciiString GetWSAErrorString( Int error );
#endif

#endif
