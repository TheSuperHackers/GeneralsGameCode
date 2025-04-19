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

/****************************************************************************\
wtime                      Neal Kettler

\****************************************************************************/
#ifndef WTIME_HEADER
#define WTIME_HEADER


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

class Wtime
{
  public:

  enum
  {
    POSITIVE=0,
    NEGATIVE=1
  };

              Wtime();  // init to system time
              Wtime( Wtime &other );
              Wtime( uint32    other );
             ~Wtime();

    void      Update();   // Update members sec & usec to system time
                          

    void      PrintTime(FILE *out) const;
    void      PrintTime(char *out) const;
    void      PrintDate(FILE *out) const;
    void      PrintDate(char *out) const;

    uint32    GetSec(void) const;    // Get member variable 'sec'
    uint32    GetUsec(void) const;   // Get member variable 'usec'
    void      SetSec(uint32 newsec);
    void      SetUsec(uint32 newusec);
    void      Set(uint32 newsec,uint32 newusec);
    bit8      ParseDate(char *in);
    bit8      FormatTime(char *out, const char *format);

    struct timeval   *GetTimeval(void);
    void              GetTimevalMT(struct timeval &tv);

    uint32    GetSecond(void) const; // Second (0- 60) (60 is for a leap second)
    uint32    GetMinute(void) const; // Minute (0 - 59)
    uint32    GetHour(void) const;   // Hour (0-23)
    uint32    GetMDay(void) const;   // Day of Month (1-31)
    uint32    GetWDay(void) const;   // Day of Week  (1-7)
    uint32    GetYDay(void) const;   // Day of Year  (1-366)
    uint32    GetMonth(void) const;  // Month (1-12)
    uint32    GetYWeek(void) const;  // Week of Year (1-53)
    uint32    GetYear(void) const;   // Year (e.g. 1997)

    bit8      GetSign(void) const;  // 0 = pos   1 = neg

    int       Compare(const Wtime &other) const;
    
    // comparisons
    bit8   operator == ( const Wtime &other ) const;
    bit8   operator != ( const Wtime &other ) const;
    bit8   operator  < ( const Wtime &other ) const;
    bit8   operator  > ( const Wtime &other ) const;
    bit8   operator <= ( const Wtime &other ) const;
    bit8   operator >= ( const Wtime &other ) const;

    // assignments
    Wtime   &operator = (const Wtime &other);
    Wtime   &operator = (const uint32    other);

    // math
    // signed
    void        SignedAdd(const Wtime &other);
    void        SignedSubtract(const Wtime &other);

    // unsigned
    Wtime   &operator += (const Wtime &other);
    Wtime   &operator -= (const Wtime &other);
    Wtime    operator +  (Wtime &other);
    Wtime    operator -  (Wtime &other);

    Wtime   &operator += (const uint32 other);
    Wtime   &operator -= (const uint32 other);
    Wtime    operator +  (uint32 other);
    Wtime    operator -  (uint32 other);

  protected:
    uint32    sec;     // seconds since Jan 1, 1970
    uint32    usec;    // microseconds (millionths of a second)
    bit8      sign;    // for time differences 0 = pos 1 = neg
};

#endif
