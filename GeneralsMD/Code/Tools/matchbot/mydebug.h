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

#ifndef MYDEBUG_HEADER
#define MYDEBUG_HEADER

#define USE_SEM

#include "wstypes.h"

#include <Utility/iostream_adapter.h>
#include <Utility/sstream_adapter.h>

#ifdef USE_SEM
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
// MDC: Changed from semaphores to critical sections because Windows doesn't provide
//      a good way of doing semaphores (I think)
// MDC: Never mind, they seem to be working now!  I'm leaving it in, though, so anyone can
//      flip a switch between the two.

#ifdef USE_SEM
extern Sem4 MyDebugLibSemaphore;
#define MYDEBUGLOCK MyDebugLibSemaphore.Wait()
#define MYDEBUGUNLOCK MyDebugLibSemaphore.Post()
#else
extern CritSec MyDebugLibSemaphore;
#define MYDEBUGLOCK MyDebugLibSemaphore.lock()
#define MYDEBUGUNLOCK MyDebugLibSemaphore.unlock()
#endif

// Print an information message
#define PARANOIDMSG(X)\
{\
char     timebuf[40]; \
Xtime now; \
now -= TimezoneOffset(); \
now.FormatTime(timebuf, "mm/dd/yy hh:mm:ss"); \
MYDEBUGLOCK; \
if (MyMsgManager::paranoidStream()) \
(*(MyMsgManager::paranoidStream())) << "HACK " << timebuf << " [" << \
__FILE__ <<  " " << __LINE__ << "] " << X << endl; \
MYDEBUGUNLOCK; \
}

// Just get a stream to the information device, no extra junk
#define PARANOIDSTREAM(X)\
{\
MYDEBUGLOCK; \
if (MyMsgManager::paranoidStream()) \
(*(MyMsgManager::paranoidStream())) << X;\
MYDEBUGUNLOCK; \
}


//#undef MYDEBUGLOCK
//#undef MYDEBUGUNLOCK

class MyMsgManager
{
protected:
	MyMsgManager();

public:
	static int                 setAllStreams(OutputDevice *device);
	static int                 setParanoidStream(OutputDevice *device);
	static int                 ReplaceAllStreams(FileD *output_device, const char *device_filename, const char *copy_filename);

	static void                enableParanoid(int flag);

	static ostream            *paranoidStream(void);
};

#endif
