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

#pragma once

#define allocateBytes(ARGCOUNT,ARGLITERAL)          allocateBytesImplementation(ARGCOUNT)
#define allocateBytesDoNotZero(ARGCOUNT,ARGLITERAL) allocateBytesDoNotZeroImplementation(ARGCOUNT)
#define newInstanceDesc(ARGCLASS,ARGLITERAL)        new ARGCLASS
#define newInstance(ARGCLASS)                       new ARGCLASS
#define MSGNEW(MSG)                                 new
#define NEW                                         new


/**
	The DynamicMemoryAllocator class is used to handle unpredictably-sized
	allocation requests.
*/
class DynamicMemoryAllocator
{
public:

	/// allocate bytes from this pool. (don't call directly; use allocateBytes() macro)
	void *allocateBytesImplementation(Int numBytes);

	/// like allocateBytesImplementation, but zeroes the memory before returning
	void *allocateBytesDoNotZeroImplementation(Int numBytes);

#ifdef MEMORYPOOL_DEBUG
	void debugIgnoreLeaksForThisBlock(void* pBlockPtr);
#endif

	/// free the bytes. (assumes allocated by this dma.)
	void freeBytes(void* pMem);

	/**
		return the actual number of bytes that would be allocated 
		if you tried to allocate the given size.
	*/
	Int getActualAllocationSize(Int numBytes);
};


/**
	The class that manages all the MemoryPools and DynamicMemoryAllocators.
	Usually you will create exactly one of these (TheMemoryPoolFactory)
	and use it for everything.
*/
class MemoryPoolFactory
{
public:

	void memoryPoolUsageReport( const char* filename, FILE *appendToFileInstead = NULL );

#ifdef MEMORYPOOL_DEBUG

	void debugMemoryReport(Int flags, Int startCheckpoint, Int endCheckpoint, FILE *fp = NULL );
	void debugSetInitFillerIndex(Int index);

#endif
};


#define MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
protected: \
	virtual ~ARGCLASS(); \
public: /* include this line at the end to reset visibility to 'public' */ 


#define MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ARGCLASS, ARGPOOLNAME) \
	MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS)


// this is the version for an Abstract Base Class, which will never be instantiated...
#define MEMORY_POOL_GLUE_ABC(ARGCLASS) \
protected: \
	virtual ~ARGCLASS(); \
public: /* include this line at the end to reset visibility to 'public' */ 


/**
	This class is provided as a simple and safe way to integrate C++ object allocation
	into MemoryPool usage. To use it, you must have your class inherit from
	MemoryPoolObject, then put the macro MEMORY_POOL_GLUE(MyClassName, "MyPoolName")
	at the start of your class definition. (This does not create the pool itself -- you
	must create that manually using MemoryPoolFactory::createMemoryPool)
*/
class MemoryPoolObject
{
protected:

	/** ensure that all destructors are virtual */
	virtual ~MemoryPoolObject() { }

public:

	static void deleteInstanceInternal(MemoryPoolObject* mpo) 
	{
		delete mpo;
	}
};

inline void deleteInstance(MemoryPoolObject* mpo)
{
	MemoryPoolObject::deleteInstanceInternal(mpo);
}


/**
	Initialize the memory manager. Construct a new MemoryPoolFactory and 
	DynamicMemoryAllocator and store 'em in the singletons of the relevant
	names. 
*/
extern void initMemoryManager();

/**
	return true if initMemoryManager() has been called.
	return false if only preMainInitMemoryManager() has been called.
*/
extern Bool isMemoryManagerOfficiallyInited();

/**
	Shut down the memory manager. Throw away TheMemoryPoolFactory and 
	TheDynamicMemoryAllocator.
*/
extern void shutdownMemoryManager();

extern MemoryPoolFactory *TheMemoryPoolFactory;
extern DynamicMemoryAllocator *TheDynamicMemoryAllocator;


// TheSuperHackers @info
// The new operator overloads will zero all memory after allocation.
// This replicates the behavior of the original Game Memory implementation and is necessary to avoid crashing the game,
// where data is not properly zero initialized. Disable these operators when fixing those issues.
#ifndef DISABLE_GAMEMEMORY_NEW_OPERATORS

extern void * __cdecl operator new(size_t size);
extern void __cdecl operator delete(void *p);

extern void * __cdecl operator new[](size_t size);
extern void __cdecl operator delete[](void *p);

// additional overloads to account for VC/MFC funky versions
extern void* __cdecl operator new(size_t size, const char *, int);
extern void __cdecl operator delete(void *p, const char *, int);

extern void* __cdecl operator new[](size_t size, const char *, int);
extern void __cdecl operator delete[](void *p, const char *, int);

#endif
