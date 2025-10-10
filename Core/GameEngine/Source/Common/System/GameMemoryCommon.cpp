/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "PreRTS.h"

#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"

#ifdef MEMORYPOOL_DEBUG

Int theTotalSystemAllocationInBytes = 0;
Int thePeakSystemAllocationInBytes = 0;

constexpr const Int GARBAGE_FILL_VALUE = 0xdeadbeef;

DECLARE_PERF_TIMER(SysMemoryDebugging)
DECLARE_PERF_TIMER(SysMemoryInitFilling)

#endif // MEMORYPOOL_DEBUG

//-----------------------------------------------------------------------------
/**
	this is the low-level allocator that we use to request memory from the OS.
	all (repeat, all) memory allocations in this module should ultimately
	go through this routine (or sysAllocateDoNotZero).

	note: throws ERROR_OUT_OF_MEMORY on failure; never returns null
*/
void* sysAllocateDoNotZero(size_t numBytes)
{
	void* p = ::GlobalAlloc(GMEM_FIXED, numBytes);
	if (!p)
		throw ERROR_OUT_OF_MEMORY;

#ifdef MEMORYPOOL_DEBUG
	{
		USE_PERF_TIMER(SysMemoryDebugging)
		#ifdef USE_FILLER_VALUE
		{
			USE_PERF_TIMER(SysMemoryInitFilling)
			::memset32(p, s_initFillerValue, ::GlobalSize(p));
		}
		#endif
		theTotalSystemAllocationInBytes += ::GlobalSize(p);
		if (thePeakSystemAllocationInBytes < theTotalSystemAllocationInBytes)
			thePeakSystemAllocationInBytes = theTotalSystemAllocationInBytes;
	}
#endif

	return p;
}

//-----------------------------------------------------------------------------
/**
	the counterpart to sysAllocate / sysAllocateDoNotZero; used to free blocks
	allocated by them. it is OK to pass null here (it will just be ignored).
*/
void sysFree(void* p)
{
	if (p)
	{
#ifdef MEMORYPOOL_DEBUG
		{
			USE_PERF_TIMER(MemoryPoolDebugging)
			::memset32(p, GARBAGE_FILL_VALUE, ::GlobalSize(p));
			theTotalSystemAllocationInBytes -= ::GlobalSize(p);
		}
#endif

		::GlobalFree(p);
	}
}

// ----------------------------------------------------------------------------
/**
	fills memory with a 32-bit value (note: assumes the ptr is 4-byte-aligned)
*/
void memset32(void* ptr, Int value, size_t bytesToFill)
{
	Int wordsToFill = bytesToFill>>2;
	bytesToFill -= (wordsToFill<<2);

	Int *p = (Int*)ptr;
	for (++wordsToFill; --wordsToFill; )
		*p++ = value;

	Byte *b = (Byte *)p;
	for (++bytesToFill; --bytesToFill; )
		*b++ = (Byte)value;
}

#ifdef USE_FILLER_VALUE

UnsignedInt s_initFillerValue = 0xf00dcafe; // will be replaced, should never be this value at runtime

void calcFillerValue(Int index)
{
	s_initFillerValue = (index & 3) << 1;
	s_initFillerValue |= 0x01;
	s_initFillerValue |= (~(s_initFillerValue << 4)) & 0xf0;
	s_initFillerValue |= (s_initFillerValue << 8);
	s_initFillerValue |= (s_initFillerValue << 16);
	//DEBUG_LOG(("Setting MemoryPool initFillerValue to %08x (index %d)",s_initFillerValue,index));
}

#endif // USE_FILLER_VALUE
