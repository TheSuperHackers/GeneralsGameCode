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

#include "syslogd.h"

SyslogD::SyslogD(char *ident,int logopt,int facility,int _priority)
{
#ifndef _WINDOWS
  openlog(ident,logopt,facility);
  priority=_priority;
#endif
}

int SyslogD::print(const char *str, int len)
{
#ifndef _WINDOWS
  char *temp_str=new char[len+1];
  memset(temp_str,0,len+1);
  strncpy(temp_str,str,len);
  syslog(priority,temp_str);
  delete[](temp_str);
#endif
  return(len);
}
