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

/*****************************************************************************\
wdebug                        Neal Kettler

MT-LEVEL
    MT-UnSafe! (UNIX version is safe though)

The debugging module is pretty good for debugging and it has some message
printing stuff as well.  The basic idea is that you write a class that
inherits from OutputDevice (severall are provided) and assign that output
device to a stream.  There are seperate streams for debugging, information,
warning, and error messages.  Each one can have a seperate output device,
or they can all have the same one.  Debugging messages only get compiled
in if your module defines 'DEBUG'. If you don't define debug, then not even
the text of the debugging message gets into the binary.   All the other
output streams get printed regardless of whether DEBUG is defined.

Sample usage:
FileD debug_device("gameres.debug");     // create a file device
MsgManager::setDebugStream(&debug_device);
DBGMSG("This debug message #" << 1 << " you use C++ streams");

Note that since these are defines you really don't need to put a semicolon
at the end, and it can be bad in situations like this:

if (x)
  DBGMSG("Stuff is broken");
else
  DBGMSG("Stuff is NOT broken");

This won't compile, read the code until you figure it out.  Only then
will you be ready to leave grasshopper.

\*****************************************************************************/

#pragma once

#include <Utility/iostream_adapter.h>
#include "odevice.h"
#include "streamer.h"
#include <time.h>



// Print an information message
#define INFMSG(X)\
{\
  char     timebuf[40]; \
  time_t   clock=time(NULL); \
  cftime(timebuf,"%D %T",&clock); \
  if (MsgManager::infoStream()) \
    (*(MsgManager::infoStream())) << "INF " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
}

// Print a warning message
#define WRNMSG(X)\
{\
  char     timebuf[40]; \
  time_t   clock=time(NULL); \
  cftime(timebuf,"%D %T",&clock); \
  if (MsgManager::warnStream()) \
    (*(MsgManager::warnStream())) << "WRN " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
}

// Print an error message
#define ERRMSG(X)\
{\
  char     timebuf[40]; \
  time_t   clock=time(NULL); \
  strcpy(timebuf,ctime(&clock)); \
  if (MsgManager::errorStream()) \
    (*(MsgManager::errorStream())) << "ERR " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
}


// Just get a stream to the information device, no extra junk
#define INFSTREAM(X)\
{\
  if (MsgManager::infoStream()) \
    (*(MsgManager::infoStream())) << X;\
}

// Just get a stream to the warning device, no extra junk
#define WRNSTREAM(X)\
{\
  if (MsgManager::warnStream()) \
    (*(MsgManager::warnStream())) << X;\
}

// Just get a stream to the error device, no extra junk
#define ERRSTREAM(X)\
{\
  if (MsgManager::errorStream()) \
    (*(MsgManager::errorStream())) << X;\
}

#ifndef DEBUG

// No debugging, no debug messages.
// Note that anything enclosed in "DBG()" will NOT get executed
// unless DEBUG is defined.
// They are defined to {} for consistency when DEBUG is defined

#define DBG(X)
#define DBGSTREAM(X)  {}
#define PVAR(v)       {}
#define DBGMSG(X)     {}
#define VERBOSE(X)    {}

#else  // DEBUG _is_ defined

// Execute only if in debugging mode
#define DBG(X) X

// Print a variable
#define PVAR(v) \
{ \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
       "]: " << ##V << " = " << V << endl; \
}


#define DBGMSG(X)\
{\
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << "DBG [" << __FILE__ <<  \
    " " << __LINE__ << "] " << X << endl;\
}

// Just get a stream to the debugging device, no extra junk
#define DBGSTREAM(X)\
{\
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << X;\
}

// Verbosely execute a statement
#define VERBOSE(X)\
{ \
  if (MsgManager::debugStream()) \
    (*(DebugManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
     "]: " << ##X << endl; X \
}

#endif  // DEBUG


class MsgManager
{
 protected:
   MsgManager();

 public:
   static int                 setAllStreams(OutputDevice *device);
   static int                 setDebugStream(OutputDevice *device);
   static int                 setInfoStream(OutputDevice *device);
   static int                 setWarnStream(OutputDevice *device);
   static int                 setErrorStream(OutputDevice *device);

   static void                enableDebug(int flag);
   static void                enableInfo(int flag);
   static void                enableWarn(int flag);
   static void                enableError(int flag);

   static ostream            *debugStream(void);
   static ostream            *infoStream(void);
   static ostream            *warnStream(void);
   static ostream            *errorStream(void);
};
