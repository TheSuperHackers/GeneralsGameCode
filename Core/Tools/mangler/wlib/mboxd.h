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

#pragma once

#include "odevice.h"

class MboxD : public OutputDevice
{
 public:

   virtual int print(const char *str,int len)
   {
     char *string=new char[len+1];
     memset(string,0,len+1);
     memcpy(string,str,len);
     MessageBox(nullptr,string,"Debug Message", MB_OK | MB_ICONINFORMATION);
     delete[](string);
     return(len);
   }

};
