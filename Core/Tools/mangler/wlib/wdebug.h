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
    MT-Safe

The debugging module is pretty good for debugging and it has some message
printing stuff as well.  The basic idea is that you write a class that
inherits from OutputDevice (several are provided) and assign that output
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

#define USE_DEBUG_SEM

#include "wstypes.h"

#include <Utility/iostream_adapter.h>
#include <Utility/sstream_adapter.h>

#if !defined(_WIN32)
// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const
#endif

#ifdef USE_DEBUG_SEM
#include "sem4.h"
#else
#include "critsec.h"
#endif
#include "odevice.h"
#include "streamer.h"
#include "xtime.h"
#include "timezone.h" // MDC
#include "filed.h"

// This is needed because the streams return a pointer.  Every time you
//  change the output device the old stream is deleted, and a new one
//  is created.
// MDC: Added a macro to switch between semaphores & critsecs to debug a
//  problem in Win32.

#ifdef USE_DEBUG_SEM
extern Sem4 DebugLibSemaphore;
#define DEBUGLOCK DebugLibSemaphore.Wait()
#define DEBUGUNLOCK DebugLibSemaphore.Post()
#else
extern CritSec DebugLibSemaphore;
#define DEBUGLOCK DebugLibSemaphore.lock()
#define DEBUGUNLOCK DebugLibSemaphore.unlock()
#endif

// Print an information message
#define INFMSG(X)\
{\
  char     timebuf[40]; \
  Xtime now; \
  now -= TimezoneOffset(); \
  now.FormatTime(timebuf, "mm/dd/yy hh:mm:ss"); \
  DEBUGLOCK; \
  if (MsgManager::infoStream()) \
    (*(MsgManager::infoStream())) << "INF " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
  DEBUGUNLOCK; \
}

// Print a warning message
#define WRNMSG(X)\
{\
  char     timebuf[40]; \
  Xtime now; \
  now -= TimezoneOffset(); \
  now.FormatTime(timebuf, "mm/dd/yy hh:mm:ss"); \
  DEBUGLOCK; \
  if (MsgManager::warnStream()) \
    (*(MsgManager::warnStream())) << "WRN " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
  DEBUGUNLOCK; \
}

// Print an error message
#define ERRMSG(X)\
{\
  char     timebuf[40]; \
  Xtime now; \
  now -= TimezoneOffset(); \
  now.FormatTime(timebuf, "mm/dd/yy hh:mm:ss"); \
  DEBUGLOCK; \
  if (MsgManager::errorStream()) \
    (*(MsgManager::errorStream())) << "ERR " << timebuf << " [" << \
        __FILE__ <<  " " << __LINE__ << "] " << X << endl; \
  DEBUGUNLOCK; \
}


// Just get a stream to the information device, no extra junk
#define INFSTREAM(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::infoStream()) \
    (*(MsgManager::infoStream())) << X;\
  DEBUGUNLOCK; \
}

// Just get a stream to the warning device, no extra junk
#define WRNSTREAM(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::warnStream()) \
    (*(MsgManager::warnStream())) << X;\
  DEBUGUNLOCK; \
}

// Just get a stream to the error device, no extra junk
#define ERRSTREAM(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::errorStream()) \
    (*(MsgManager::errorStream())) << X;\
  DEBUGUNLOCK; \
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

// In Windows, send a copy to the debugger window
#ifdef _WIN32

// Print a variable
#define PVAR(v) \
{ \
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
       "]: " << ##V << " = " << V << endl; \
  strstream __s;\
  __s << __FILE__ << "[" << __LINE__ << \
       "]: " << ##V << " = " << V << '\n' << '\0';\
  OutputDebugString(STRSTREAM_CSTR(__s));\
  DEBUGUNLOCK; \
}


#define DBGMSG(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << "DBG [" << __FILE__ <<  \
    " " << __LINE__ << "] " << X << endl;\
  strstream __s;\
  __s << "DBG [" << __FILE__ <<  \
    " " << __LINE__ << "] " << X << '\n' << '\0';\
  OutputDebugString(STRSTREAM_CSTR(__s));\
  DEBUGUNLOCK; \
}

// Just get a stream to the debugging device, no extra junk
#define DBGSTREAM(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << X;\
  strstream __s;\
  __s << X << '\0';\
  OutputDebugString(STRSTREAM_CSTR(__s));\
  DEBUGUNLOCK; \
}

// Verbosely execute a statement
#define VERBOSE(X)\
{ \
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(DebugManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
     "]: " << ##X << endl; X \
  strstream __s;\
  __s  << __FILE__ << "[" << __LINE__ << \
     "]: " << ##X << '\n' << '\0';\
  OutputDebugString(STRSTREAM_CSTR(__s));\
  DEBUGUNLOCK; \
}

#else // _WIN32

// Print a variable
#define PVAR(v) \
{ \
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
       "]: " << ##V << " = " << V << endl; \
  DEBUGUNLOCK; \
}


#define DBGMSG(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << "DBG [" << __FILE__ <<  \
    " " << __LINE__ << "] " << X << endl;\
  DEBUGUNLOCK; \
}

// Just get a stream to the debugging device, no extra junk
#define DBGSTREAM(X)\
{\
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(MsgManager::debugStream())) << X;\
  DEBUGUNLOCK; \
}

// Verbosely execute a statement
#define VERBOSE(X)\
{ \
  DEBUGLOCK; \
  if (MsgManager::debugStream()) \
    (*(DebugManager::debugStream())) << __FILE__ << "[" << __LINE__ << \
     "]: " << ##X << endl; X \
  DEBUGUNLOCK; \
}
#endif // _WIN32

#endif  // DEBUG

//#undef DEBUGLOCK
//#undef DEBUGUNLOCK

class MsgManager
{
 protected:
   MsgManager();

 public:
   static int                 setAllStreams(OutputDevice *device);
   static int                 ReplaceAllStreams(FileD *output_device, IN char *device_filename, IN char *copy_filename);
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
