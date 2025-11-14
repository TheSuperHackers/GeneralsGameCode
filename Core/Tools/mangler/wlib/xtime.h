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
xtime                      Neal Kettler

This is version 2 of the Wtime library (now xtime).  It now supports
time storage from the year 0 to well after the sun
will have gone supernova (OK, OK I admit it'll break in the
year 5 Million.)

The call to update the current time will break in 2038.
Hopefully by then somebody will replace the lame time()
function :-)
\****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#else
#include <sys/timeb.h>
#include <winsock.h>
#endif

#include <time.h>
#include <string.h>

#include "wstypes.h"

class Xtime
{
  public:

              Xtime();  // init to system time
              Xtime( Xtime &other );
              Xtime( time_t other );   // 1970-2038
             ~Xtime();

    void      addSeconds(sint32 seconds);

    bit8      getTime(int &month, int &mday, int &year, int &hour, int &minute,
                        int &second) const;

    bit8      setTime(int month, int mday, int year, int hour, int minute,
                        int second);

    void      update();   // Update members sec & usec to system time
                          // This will break after 2038

/********
    void      PrintTime(FILE *out) const;
    void      PrintTime(char *out) const;
    void      PrintDate(FILE *out) const;
    void      PrintDate(char *out) const;
**********/

    sint32    getDay(void) const;    // Get days since year 0
    sint32    getMsec(void) const;   // Get milliseconds into the day

    void      setDay(sint32 day);
    void      setMsec(sint32 msec);

    void      set(sint32 newday, sint32 newmsec);
    bit8      ParseDate(char *in);
    bit8      FormatTime(char *out, const char *format);

    bit8      getTimeval(struct timeval &tv);

    // All of these may return -1 if the time is invalid
    int    getSecond(void) const; // Second (0-60) (60 is for a leap second)
    int    getMinute(void) const; // Minute (0-59)
    int    getHour(void) const;   // Hour (0-23)
    int    getMDay(void) const;   // Day of Month (1-31)
    int    getWDay(void) const;   // Day of Week  (1-7)
    int    getYDay(void) const;   // Day of Year  (1-366)  (366 = leap yr)
    int    getMonth(void) const;  // Month (1-12)
    int    getYWeek(void) const;  // Week of Year (1-53)
    int    getYear(void) const;   // Year (e.g. 1997)

    // Modify the time components.  Return FALSE if fail
    bit8      setSecond(sint32 sec);
    bit8      setMinute(sint32 min);
    bit8      setHour(sint32 hour);
    bit8      setYear(sint32 year);
    bit8      setMonth(sint32 month);
    bit8      setMDay(sint32 mday);

    void      normalize(void);  // move msec overflows to the day

    // Compare two times
    int       compare(const Xtime &other) const;

    // comparisons
    bit8   operator == ( const Xtime &other ) const;
    bit8   operator != ( const Xtime &other ) const;
    bit8   operator  < ( const Xtime &other ) const;
    bit8   operator  > ( const Xtime &other ) const;
    bit8   operator <= ( const Xtime &other ) const;
    bit8   operator >= ( const Xtime &other ) const;

    // assignments
    Xtime   &operator = (const Xtime &other);
    Xtime   &operator = (const time_t other);

    // signed
    Xtime   &operator += (const Xtime &other);
    Xtime   &operator -= (const Xtime &other);
    Xtime    operator +  (Xtime &other);
    Xtime    operator -  (Xtime &other);

    Xtime   &operator += (const time_t other);
    Xtime   &operator -= (const time_t other);
    Xtime    operator +  (time_t other);
    Xtime    operator -  (time_t other);

  protected:
    sint32    day_;    // days since Jan 1, 0
    sint32    msec_;   // milliseconds  (thousandths of a sec)
};
