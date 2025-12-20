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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/internal.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Internal header
//////////////////////////////////////////////////////////////////////////////

#pragma once

// make sure we're not omitting the frame pointer
#pragma optimize("y",off)

// We're doing our own little internal asserts for full debug
// builds (until this library is proven & stable)
#ifdef _DEBUG
#  define __ASSERT(x) do { if (!(x)) DebugInternalAssert(__FILE__,__LINE__,#x); } while (0)
#else
#  define __ASSERT(x) do { } while(0)
#endif

/** \internal

  Internal assert function for module internal assertions.

  \param file file name
  \param line line number
  \param expr expression which failed
*/
void DebugInternalAssert(const char *file, int line, const char *expr);

/** \internal

  Allocate the given number of bytes from the Windows heap.
  This function performs a controlled crash on failure.

  \param numBytes number of bytes to allocate
  \return pointer to allocated memory
*/
void *DebugAllocMemory(unsigned numBytes);

/** \internal

  Allocates/reallocates the given memory block to the new
  given size.
  This function performs a controlled crash on failure.

  \param oldPtr pointer to old memory block, may be NULL
  \param newSize new size of block
  \return pointer to reallocated memory
*/
void *DebugReAllocMemory(void *oldPtr, unsigned newSize);

/** \internal

  Frees the allocated memory block.
  This function never fails.

  \param ptr memory block to free
*/
void DebugFreeMemory(void *ptr);

/// \internal Command group: 'debug'
class DebugCmdInterfaceDebug: public DebugCmdInterface
{
public:
  virtual bool Execute(class Debug& dbg, const char *cmd, CommandMode cmdmode,
                       unsigned argn, const char * const * argv);

  virtual void Delete(void)
  {
    this->~DebugCmdInterfaceDebug();
    DebugFreeMemory(this);
  }
};
