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

#ifndef STREAMER_HEADER
#define STREAMER_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Utility/iostream_adapter.h>
#include <string.h>

// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const

#include "odevice.h"

#ifndef STREAMER_BUFSIZ
// This limits the number of characters that can be sent to a single 'print'
// call.  If your debug message is bigger than this, it will get split over
// multiple 'print' calls.  That's usually not a problem.
 #define STREAMER_BUFSIZ 2048
#endif


// Provide a streambuf interface for a class that can 'print'
class Streamer : public streambuf
{
 public:
               Streamer();
    virtual   ~Streamer();

    int        setOutputDevice(OutputDevice *output_device);

 protected:
    // Virtual methods from streambuf
    int       xsputn(const char* s, int n); // buffer some characters
    int       overflow(int = EOF);          // flush buffer and make more room
    int       underflow(void);              // Does nothing
    int       sync();

    int       doallocate();                 // allocate a buffer


    OutputDevice  *Output_Device;
    char* Buf;
};

#endif
