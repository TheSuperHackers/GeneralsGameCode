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

#include <malloc.h>

#include "Common/GameMemoryNull.h"

static Bool theMainInitFlag = false;

// ----------------------------------------------------------------------------
// PUBLIC DATA 
// ----------------------------------------------------------------------------

MemoryPoolFactory *TheMemoryPoolFactory = NULL;
DynamicMemoryAllocator *TheDynamicMemoryAllocator = NULL;

//-----------------------------------------------------------------------------
// METHODS for DynamicMemoryAllocator
//-----------------------------------------------------------------------------

/**
	allocate a chunk-o-bytes from this DMA and return it, but don't bother zeroing
	out the block. if unable to allocate, throw ERROR_OUT_OF_MEMORY. this
	function will never return null.

  added code to make sure we're on a DWord boundary, throw exception if not
*/
void *DynamicMemoryAllocator::allocateBytesDoNotZeroImplementation(Int numBytes)
{
	void *p = malloc(numBytes);
	if (p == NULL)
		throw ERROR_OUT_OF_MEMORY;
	return p;
}

/**
	allocate a chunk-o-bytes from this DMA and return it, and zero out the contents first.
	if unable to allocate, throw ERROR_OUT_OF_MEMORY. 
	this function will never return null.
*/
void *DynamicMemoryAllocator::allocateBytesImplementation(Int numBytes)
{
	void* p = allocateBytesDoNotZeroImplementation(numBytes);	// throws on failure
	memset(p, 0, numBytes);
	return p;
}

/**
	free a chunk-o-bytes allocated by this dma. it's ok to pass null.
*/
void DynamicMemoryAllocator::freeBytes(void* pBlockPtr)
{
	free(pBlockPtr);
}

Int DynamicMemoryAllocator::getActualAllocationSize(Int numBytes)
{
	return numBytes;
}

#ifdef MEMORYPOOL_DEBUG
void DynamicMemoryAllocator::debugIgnoreLeaksForThisBlock(void* pBlockPtr)
{
}
#endif

//-----------------------------------------------------------------------------
// METHODS for MemoryPoolFactory
//-----------------------------------------------------------------------------

void MemoryPoolFactory::memoryPoolUsageReport( const char* filename, FILE *appendToFileInstead )
{
}

#ifdef MEMORYPOOL_DEBUG
void MemoryPoolFactory::debugMemoryReport(Int flags, Int startCheckpoint, Int endCheckpoint, FILE *fp )
{
}
void MemoryPoolFactory::debugSetInitFillerIndex(Int index)
{
}
#endif

//-----------------------------------------------------------------------------
// GLOBAL FUNCTIONS
//-----------------------------------------------------------------------------

/**
	Initialize the memory manager, and create TheMemoryPoolFactory and TheDynamicMemoryAllocator.
*/
void initMemoryManager()
{
	if (TheMemoryPoolFactory == NULL && TheDynamicMemoryAllocator == NULL)
	{
		TheMemoryPoolFactory = new (malloc(sizeof MemoryPoolFactory)) MemoryPoolFactory;
		TheDynamicMemoryAllocator = new (malloc(sizeof DynamicMemoryAllocator)) DynamicMemoryAllocator;

		DEBUG_INIT(DEBUG_FLAGS_DEFAULT);
		DEBUG_LOG(("*** Initialized the Null Memory Manager"));
	}
	else
	{
			DEBUG_CRASH(("Null Memory Manager is already initialized"));
	}

	theMainInitFlag = true;
}

//-----------------------------------------------------------------------------
Bool isMemoryManagerOfficiallyInited()
{
	return theMainInitFlag;
}

//-----------------------------------------------------------------------------
/**
	shutdown the memory manager and discard all memory. Note: if preMainInitMemoryManager()
	was called prior to initMemoryManager(), this call will do nothing.
*/
void shutdownMemoryManager()
{
	if (TheDynamicMemoryAllocator != NULL)
	{
		TheDynamicMemoryAllocator->~DynamicMemoryAllocator();
		free((void *)TheDynamicMemoryAllocator);
		TheDynamicMemoryAllocator = NULL;
	}

	if (TheMemoryPoolFactory != NULL)
	{
		TheMemoryPoolFactory->~MemoryPoolFactory();
		free((void *)TheMemoryPoolFactory);
		TheMemoryPoolFactory = NULL;
	}

	theMainInitFlag = false;

	DEBUG_SHUTDOWN();
}


#ifndef DISABLE_GAMEMEMORY_NEW_OPERATORS

extern void * __cdecl operator new(size_t size)
{
	void *p = malloc(size);
	if (p == NULL)
		throw ERROR_OUT_OF_MEMORY;
	memset(p, 0, size);
	return p;
}

extern void __cdecl operator delete(void *p)
{
	free(p);
}

extern void * __cdecl operator new[](size_t size)
{
	void *p = malloc(size);
	if (p == NULL)
		throw ERROR_OUT_OF_MEMORY;
	memset(p, 0, size);
	return p;
}

extern void __cdecl operator delete[](void *p)
{
	free(p);
}

// additional overloads to account for VC/MFC funky versions
extern void* __cdecl operator new(size_t size, const char *, int)
{
	void *p = malloc(size);
	if (p == NULL)
		throw ERROR_OUT_OF_MEMORY;
	memset(p, 0, size);
	return p;
}

extern void __cdecl operator delete(void *p, const char *, int)
{
	free(p);
}

extern void* __cdecl operator new[](size_t size, const char *, int)
{
	void *p = malloc(size);
	if (p == NULL)
		throw ERROR_OUT_OF_MEMORY;
	memset(p, 0, size);
	return p;
}

extern void __cdecl operator delete[](void *p, const char *, int)
{
	free(p);
}

#endif
