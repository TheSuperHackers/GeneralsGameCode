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

#ifndef SEM4_HEADER
#define SEM4_HEADER

#include <limits.h>
#ifndef _WINDOWS
#include <unistd.h>
#endif
#include "wstypes.h"

#ifdef _REENTRANT
#ifndef _WINDOWS
#include <semaphore.h>
#else
#include <windows.h>
#endif // _WINDOWS
#endif // _REENTRANT

// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const

class Sem4
{
 private:
  #ifdef _REENTRANT
#ifndef _WINDOWS
  sem_t sem;
#else
  HANDLE sem;
#endif
  #endif
 public:
               Sem4();
               Sem4(uint32 value);
              ~Sem4();

  sint32       Wait(void) const;
  sint32       TryWait(void) const;
  sint32       Post(void) const;
  sint32       GetValue(int *sval) const;
  sint32       Destroy(void);
};

#endif
