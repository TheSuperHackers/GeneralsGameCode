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

#ifndef SYSLOGD_HEADER
#define SYSLOGD_HEADER

#include <stdlib.h>
#include <stdio.h>
#ifndef _WINDOWS
#include <syslog.h>
#endif
#include <string.h>

// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const

#include "odevice.h"

// Windows doesn't have a syslog equivalent (does it?), so this class does little there
class SyslogD : public OutputDevice
{
 public:
   SyslogD(char *ident,int logopt,int facility,int priority);
   virtual int print(const char *str,int len);

 private:
   int priority;
};

#endif
