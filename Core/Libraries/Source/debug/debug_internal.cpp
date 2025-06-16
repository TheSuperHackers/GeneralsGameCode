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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_internal.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Implementation of internal code
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"

void DebugInternalAssert(const char *file, int line, const char *expr)
{
  // dangerous as well but since this function is used in this
  // module only we know how long stuff can get
  char buf[512];
  wsprintf(buf,"File %s, line %i:\n%s",file,line,expr);
  MessageBox(NULL,buf,"Internal assert failed",
                        MB_OK|MB_ICONSTOP|MB_TASKMODAL|MB_SETFOREGROUND);
  
  // stop right now!
  TerminateProcess(GetCurrentProcess(),666);
}

void *DebugAllocMemory(unsigned numBytes)
{
  HGLOBAL h=GlobalAlloc(GMEM_FIXED,numBytes);
  if (!h)
    DCRASH_RELEASE("Debug mem alloc failed");
  return (void *)h;
}

void *DebugReAllocMemory(void *oldPtr, unsigned newSize)
{
  // Windows doesn't like ReAlloc with NULL handle/ptr...
  if (!oldPtr)
    return newSize?DebugAllocMemory(newSize):0;

  // Shrinking to 0 size is basically freeing memory
  if (!newSize)
  {
    GlobalFree((HGLOBAL)oldPtr);
    return 0;
  }

  // now try GlobalReAlloc first
  HGLOBAL h=GlobalReAlloc((HGLOBAL)oldPtr,newSize,0);
  if (!h)
  {
    // this failed (Windows doesn't like ReAlloc'ing larger
    // fixed memory blocks) - go with Alloc/Free instead
    h=GlobalAlloc(GMEM_FIXED,newSize);
    if (!h)
      DCRASH_RELEASE("Debug mem realloc failed");
    unsigned oldSize=GlobalSize((HGLOBAL)oldPtr);
    memcpy((void *)h,oldPtr,oldSize<newSize?oldSize:newSize);
    GlobalFree((HGLOBAL)oldPtr);
  }

  return (void *)h;
}

void DebugFreeMemory(void *ptr)
{
  if (ptr)
    GlobalFree((HGLOBAL)ptr);
}
