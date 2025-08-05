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

#include "monod.h"

MonoD::MonoD(void)
{
#ifdef _WIN32
  unsigned long retval;
  handle = CreateFile("\\\\.\\MONO", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle != INVALID_HANDLE_VALUE)
  {
    DeviceIoControl(handle, (DWORD)IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0,
                     &retval,0);
  }
#endif
}

MonoD::~MonoD()
{
  #ifdef _WIN32
    CloseHandle(handle);
    handle=NULL;
  #endif
}

int MonoD::print(const char *str, int len)
{
  #ifdef _WIN32
    unsigned long retval;
    WriteFile(handle, str, len, &retval, NULL);
    //DeviceIoControl(handle, (DWORD)IOCTL_MONO_PRINT_RAW, (void *)str, len, NULL, 0,
    //                 &retval,0);
    return(len);
  #else
    for (int i=0; i<len; i++)
      fprintf(stderr,"%c",str[i]);
    return(len);
  #endif
}
