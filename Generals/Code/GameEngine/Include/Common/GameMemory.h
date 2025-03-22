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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Memory.h 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C); 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Memory.h
//
// Created:    Steven Johnson, August 2001
//
// Desc:       Memory manager
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _GAME_MEMORY_H_ 
#define _GAME_MEMORY_H_

// Turn off memory pool checkpointing for now.
#define DISABLE_MEMORYPOOL_CHECKPOINTING 1

#if (defined(_DEBUG) || defined(_INTERNAL)) && !defined(MEMORYPOOL_DEBUG_CUSTOM_NEW) && !defined(DISABLE_MEMORYPOOL_DEBUG_CUSTOM_NEW)
	#define MEMORYPOOL_DEBUG_CUSTOM_NEW
#endif

//#if (defined(_DEBUG) || defined(_INTERNAL)) && !defined(MEMORYPOOL_DEBUG) && !defined(DISABLE_MEMORYPOOL_DEBUG)
#if (defined(_DEBUG)) && !defined(MEMORYPOOL_DEBUG) && !defined(DISABLE_MEMORYPOOL_DEBUG)
	#define MEMORYPOOL_DEBUG
#endif

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

#include <new.h>
#include <stdio.h>
#ifdef MEMORYPOOL_OVERRIDE_MALLOC
	#include <malloc.h>
#endif

// USER INCLUDES //////////////////////////////////////////////////////////////

#include "Lib/BaseType.h"
#include "Common/Debug.h"
#include "Common/Errors.h"

// MACROS //////////////////////////////////////////////////////////////////

#ifdef MEMORYPOOL_DEBUG

	// by default, enable free-block-retention for checkpointing in debug mode
	#ifndef DISABLE_MEMORYPOOL_CHECKPOINTING
		#define MEMORYPOOL_CHECKPOINTING
	#endif

	// by default, enable bounding walls in debug mode (unless we have specifically disabled them)
	#ifndef DISABLE_MEMORYPOOL_BOUNDINGWALL
		#define MEMORYPOOL_BOUNDINGWALL
	#endif

	#define DECLARE_LITERALSTRING_ARG1										const char * debugLiteralTagString
	#define PASS_LITERALSTRING_ARG1												debugLiteralTagString
	#define DECLARE_LITERALSTRING_ARG2										, const char * debugLiteralTagString
	#define PASS_LITERALSTRING_ARG2												, debugLiteralTagString

	#define MP_LOC_SUFFIX																/*" [" DEBUG_FILENLINE "]"*/

	#define allocateBlock(ARGLITERAL)										allocateBlockImplementation(ARGLITERAL MP_LOC_SUFFIX)
	#define allocateBlockDoNotZero(ARGLITERAL)					allocateBlockDoNotZeroImplementation(ARGLITERAL MP_LOC_SUFFIX)
	#define allocateBytes(ARGCOUNT,ARGLITERAL)					allocateBytesImplementation(ARGCOUNT, ARGLITERAL MP_LOC_SUFFIX)
	#define allocateBytesDoNotZero(ARGCOUNT,ARGLITERAL)	allocateBytesDoNotZeroImplementation(ARGCOUNT, ARGLITERAL MP_LOC_SUFFIX)
	#define newInstanceDesc(ARGCLASS,ARGLITERAL)				new(ARGCLASS::ARGCLASS##_GLUE_NOT_IMPLEMENTED, ARGLITERAL MP_LOC_SUFFIX) ARGCLASS
	#define newInstance(ARGCLASS)												new(ARGCLASS::ARGCLASS##_GLUE_NOT_IMPLEMENTED, __FILE__) ARGCLASS

	#if !defined(MEMORYPOOL_STACKTRACE) && !defined(DISABLE_MEMORYPOOL_STACKTRACE)
		#define MEMORYPOOL_STACKTRACE
	#endif

	// flags for the memory-report options.
	enum 
	{

#ifdef MEMORYPOOL_CHECKPOINTING
		// ------------------------------------------------------
		// you usually won't use the _REPORT bits directly; see below for more convenient combinations.

		// you must set at least one of the 'allocate' bits.
		_REPORT_CP_ALLOCATED_BEFORE			= 0x0001,
		_REPORT_CP_ALLOCATED_BETWEEN		= 0x0002,
		_REPORT_CP_ALLOCATED_DONTCARE		= (_REPORT_CP_ALLOCATED_BEFORE|_REPORT_CP_ALLOCATED_BETWEEN),

		// you must set at least one of the 'freed' bits.
		_REPORT_CP_FREED_BEFORE					= 0x0010,
		_REPORT_CP_FREED_BETWEEN				= 0x0020,
		_REPORT_CP_FREED_NEVER					= 0x0040,	// ie, still in existence
		_REPORT_CP_FREED_DONTCARE				= (_REPORT_CP_FREED_BEFORE|_REPORT_CP_FREED_BETWEEN|_REPORT_CP_FREED_NEVER),
		// ------------------------------------------------------
#endif // MEMORYPOOL_CHECKPOINTING

#ifdef MEMORYPOOL_STACKTRACE
		/** display the stacktrace for allocation location for all blocks found. 
			this bit may be mixed-n-matched with any other flag.
		*/
		REPORT_CP_STACKTRACE		= 0x0100,
#endif
		
		/** display stats for each pool, in addition to each block.
			(this is useful for finding suitable allocation counts for the pools.)
			this bit may be mixed-n-matched with any other flag.
		*/
		REPORT_POOLINFO					= 0x0200, 

		/** report on the overall memory situation (including all pools and dma's).
			this bit may be mixed-n-matched with any other flag.
		*/
		REPORT_FACTORYINFO			= 0x0400,	

		/** report on pools that have overflowed their initial allocation.
			this bit may be mixed-n-matched with any other flag.
		*/
		REPORT_POOL_OVERFLOW		= 0x0800,	

		/** simple-n-cheap leak checking */
		REPORT_SIMPLE_LEAKS			= 0x1000,

#ifdef MEMORYPOOL_CHECKPOINTING
		/** report on blocks that were allocated between the checkpoints.
		 (don't care if they were freed or not.)
		*/
		REPORT_CP_ALLOCATES	= (_REPORT_CP_ALLOCATED_BETWEEN | _REPORT_CP_FREED_DONTCARE),	

		/** report on blocks that were freed between the checkpoints.
		 (don't care when they were allocated.)
		*/
		REPORT_CP_FREES			= (_REPORT_CP_ALLOCATED_DONTCARE | _REPORT_CP_FREED_BETWEEN),	

		/** report on blocks that were allocated between the checkpoints, and still exist
		 (note that this reports *potential* leaks -- some such blocks may be desired)
		*/
		REPORT_CP_LEAKS			= (_REPORT_CP_ALLOCATED_BETWEEN | _REPORT_CP_FREED_NEVER),

		/** report on blocks that existed before checkpoint #1 and still exist now.
		*/
		REPORT_CP_LONGTERM		= (_REPORT_CP_ALLOCATED_BEFORE | _REPORT_CP_FREED_NEVER),
		
		/** report on blocks that were allocated-and-freed between the checkpoints.
		*/
		REPORT_CP_TRANSIENT		= (_REPORT_CP_ALLOCATED_BETWEEN | _REPORT_CP_FREED_BETWEEN),

		/** report on all blocks that currently exist
		*/
		REPORT_CP_EXISTING		= (_REPORT_CP_ALLOCATED_BEFORE | _REPORT_CP_ALLOCATED_BETWEEN | _REPORT_CP_FREED_NEVER),

		/** report on all blocks that have ever existed (!) (or at least, since the last call
			to debugResetCheckpoints)
		*/
		REPORT_CP_ALL					= (_REPORT_CP_ALLOCATED_DONTCARE | _REPORT_CP_FREED_DONTCARE)
#endif // MEMORYPOOL_CHECKPOINTING
		
	};

#else

	#define DECLARE_LITERALSTRING_ARG1
	#define PASS_LITERALSTRING_ARG1	
	#define DECLARE_LITERALSTRING_ARG2
	#define PASS_LITERALSTRING_ARG2

	#define allocateBlock(ARGLITERAL)										allocateBlockImplementation()
	#define allocateBlockDoNotZero(ARGLITERAL)					allocateBlockDoNotZeroImplementation()
	#define allocateBytes(ARGCOUNT,ARGLITERAL)					allocateBytesImplementation(ARGCOUNT)
	#define allocateBytesDoNotZero(ARGCOUNT,ARGLITERAL)	allocateBytesDoNotZeroImplementation(ARGCOUNT)
	#define newInstanceDesc(ARGCLASS,ARGLITERAL)				new(ARGCLASS::ARGCLASS##_GLUE_NOT_IMPLEMENTED) ARGCLASS
	#define newInstance(ARGCLASS)												new(ARGCLASS::ARGCLASS##_GLUE_NOT_IMPLEMENTED) ARGCLASS

#endif

// FORWARD REFERENCES /////////////////////////////////////////////////////////

class MemoryPoolSingleBlock;
class MemoryPoolBlob;
class MemoryPool;
class MemoryPoolFactory;
class DynamicMemoryAllocator;
class BlockCheckpointInfo;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
/**
	This class is purely a convenience used to pass optional arguments to initMemoryManager(),
	and by extension, to createDynamicMemoryAllocator(). You can specify how many sub-pools you
	want, what size each is, what the allocation counts are to be, etc. Most apps will
	construct an array of these to pass to initMemoryManager() and never use it elsewhere.
*/
struct PoolInitRec
{
	const char *poolName;					///< name of the pool; by convention, "dmaPool_XXX" where XXX is allocationSize
	Int allocationSize;						///< size, in bytes, of the pool.
	Int initialAllocationCount;		///< initial number of blocks to allocate.
	Int overflowAllocationCount;	///< when the pool runs out of space, allocate more blocks in this increment
};

enum 
{
	MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS = 8	///< The max number of subpools allowed in a DynamicMemoryAllocator
};

#ifdef MEMORYPOOL_CHECKPOINTING
// ----------------------------------------------------------------------------
/**
	This class exists purely for coding convenience, and should never be used by external code.
	It simply allows MemoryPool and DynamicMemoryAllocator to share checkpoint-related
	code in a seamless way.
*/
class Checkpointable
{
private:
	BlockCheckpointInfo	*m_firstCheckpointInfo;		///< head of the linked list of checkpoint infos for this pool/dma
	Bool								m_cpiEverFailed;					///< flag to detect if we ran out of memory accumulating checkpoint info.

protected:

	Checkpointable();
	~Checkpointable();

	/// create a new checkpoint info and add it to the list.
	BlockCheckpointInfo *debugAddCheckpointInfo(
		const char *debugLiteralTagString,
		Int allocCheckpoint,
		Int blockSize
	);

public:
	/// dump a checkpoint report to logfile
	void debugCheckpointReport(Int flags, Int startCheckpoint, Int endCheckpoint, const char *poolName);
	/// reset all the checkpoints for this pool/dma
	void debugResetCheckpoints();
};
#endif

// ----------------------------------------------------------------------------
/**
	A MemoryPool provides a way to efficiently allocate objects of the same (or similar)
	size. We allocate large a large chunk of memory (a "blob") and subdivide it into
	even-size chunks, doling these out as needed. If the first blob gets full, we allocate
	additional blobs as necessary. A given pool can allocate blocks of only one size;
	if you need a different size, you should use a different pool.
*/
class MemoryPool 
#ifdef MEMORYPOOL_CHECKPOINTING
	: public Checkpointable
#endif
{
private:

	MemoryPoolFactory	*m_factory;									///< the factory that created us
	MemoryPool				*m_nextPoolInFactory;				///< linked list node, managed by factory
	const char				*m_poolName;								///< name of this pool. (literal string; must not be freed)
	Int								m_allocationSize;						///< size of the blocks allocated by this pool, in bytes
	Int								m_initialAllocationCount;		///< number of blocks to be allocated in initial blob
	Int								m_overflowAllocationCount;	///< number of blocks to be allocated in any subsequent blob(s)
	Int								m_usedBlocksInPool;					///< total number of blocks in use in the pool.
	Int								m_totalBlocksInPool;				///< total number of blocks in all blobs of this pool (used or not).
	Int								m_peakUsedBlocksInPool;			///< high-water mark of m_usedBlocksInPool
	MemoryPoolBlob		*m_firstBlob;								///< head of linked list: first blob for this pool.
	MemoryPoolBlob		*m_lastBlob;								///< tail of linked list: last blob for this pool. (needed for efficiency)
	MemoryPoolBlob		*m_firstBlobWithFreeBlocks;	///< first blob in this pool that has at least one unallocated block.

private:
	/// create a new blob with the given number of blocks.
	MemoryPoolBlob* createBlob(Int allocationCount);

	/// destroy a blob.
	Int freeBlob(MemoryPoolBlob *blob);

public:

	// 'public' funcs that are really only for use by MemoryPoolFactory
	MemoryPool *getNextPoolInList();					///< return next pool in linked list
	void addToList(MemoryPool **pHead);				///< add this pool to head of the linked list
	void removeFromList(MemoryPool **pHead);	///< remove this pool from the linked list
	#ifdef MEMORYPOOL_DEBUG
		static void debugPoolInfoReport( MemoryPool *pool, FILE *fp = NULL );	///< dump a report about this pool to the logfile
		const char *debugGetBlockTagString(void *pBlock);		///< return the tagstring for the given block (assumed to belong to this pool)
		void debugMemoryVerifyPool();												///< perform internal consistency check on this pool.
		Int debugPoolReportLeaks( const char* owner );
	#endif
	#ifdef MEMORYPOOL_CHECKPOINTING
		void debugResetCheckpoints();												///< throw away all checkpoint information for this pool.
	#endif

public:

	MemoryPool();

	/// initialize the given memory pool.
	void init(MemoryPoolFactory *factory, const char *poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount);

	~MemoryPool();

	/// allocate a block from this pool. (don't call directly; use allocateBlock() macro)
	void *allocateBlockImplementation(DECLARE_LITERALSTRING_ARG1);

	/// same as allocateBlockImplementation, but memory returned is not zeroed
	void *allocateBlockDoNotZeroImplementation(DECLARE_LITERALSTRING_ARG1);
	
	/// free the block. it is OK to pass null.
	void freeBlock(void *pMem);

	/// return the factory that created (and thus owns) this pool.
	MemoryPoolFactory *getOwningFactory();

	/// return the name of this pool. the result is a literal string and must not be freed.
	const char *getPoolName();

	/// return the block allocation size of this pool.
	Int getAllocationSize();

	/// return the number of free (available) blocks in this pool.
	Int getFreeBlockCount();

	/// return the number of blocks in use in this pool.
	Int getUsedBlockCount();

	/// return the total number of blocks in this pool. [ == getFreeBlockCount() + getUsedBlockCount() ]
	Int getTotalBlockCount();

	/// return the high-water mark for getUsedBlockCount()
	Int getPeakBlockCount();

	/// return the initial allocation count for this pool
	Int getInitialBlockCount();

	Int countBlobsInPool();

	/// if this pool has any empty blobs, return them to the system.
	Int releaseEmpties();

	/// destroy all blocks and blobs in this pool.
	void reset();

	#ifdef MEMORYPOOL_DEBUG
		/// return true iff this block was allocated by this pool.
		Bool debugIsBlockInPool(void *pBlock);
	#endif
};

// ----------------------------------------------------------------------------
/**
	The DynamicMemoryAllocator class is used to handle unpredictably-sized
	allocation requests. It basically allocates a number of (private) MemoryPools,
	then routes request to the smallest-size pool that will satisfy the request.
	(Requests too large for any of the pool are routed to the system memory allocator.)
	You should normally use this in place of malloc/free or (global) new/delete.
*/
class DynamicMemoryAllocator 
#ifdef MEMORYPOOL_CHECKPOINTING
	: public Checkpointable
#endif
{
private:
	MemoryPoolFactory					*m_factory;						///< the factory that created us
	DynamicMemoryAllocator		*m_nextDmaInFactory;	///< linked list node, managed by factory
	Int												m_numPools;						///< number of subpools (up to MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS)
	Int												m_usedBlocksInDma;		///< total number of blocks allocated, from subpools and "raw"
	MemoryPool								*m_pools[MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS];	///< the subpools
	MemoryPoolSingleBlock			*m_rawBlocks;					///< linked list of "raw" blocks allocated directly from system

	/// return the best pool for the given allocSize, or null if none are suitable
	MemoryPool *findPoolForSize(Int allocSize);

public:

	// 'public' funcs that are really only for use by MemoryPoolFactory

	DynamicMemoryAllocator *getNextDmaInList();						///< return next dma in linked list
	void addToList(DynamicMemoryAllocator **pHead);				///< add this dma to the list
	void removeFromList(DynamicMemoryAllocator **pHead);	///< remove this dma from the list
	#ifdef MEMORYPOOL_DEBUG
		Int debugCalcRawBlockBytes(Int *numBlocks);												///< calculate the number of bytes in "raw" (non-subpool) blocks
		void debugMemoryVerifyDma();												///< perform internal consistency check
		const char *debugGetBlockTagString(void *pBlock);		///< return the tagstring for the given block (assumed to belong to this dma)
		void debugDmaInfoReport( FILE *fp = NULL );					///< dump a report about this pool to the logfile
		Int debugDmaReportLeaks();
	#endif
	#ifdef MEMORYPOOL_CHECKPOINTING
		void debugResetCheckpoints();												///< toss all checkpoint information
	#endif

public:

	DynamicMemoryAllocator();

	/// initialize the dma. pass 0/null for numSubPool/parms to get some reasonable default subpools.
	void init(MemoryPoolFactory *factory, Int numSubPools, const PoolInitRec pParms[]);
	
	~DynamicMemoryAllocator();

	/// allocate bytes from this pool. (don't call directly; use allocateBytes() macro)
	void *allocateBytesImplementation(Int numBytes DECLARE_LITERALSTRING_ARG2);

	/// like allocateBytesImplementation, but zeroes the memory before returning
	void *allocateBytesDoNotZeroImplementation(Int numBytes DECLARE_LITERALSTRING_ARG2);

#ifdef MEMORYPOOL_DEBUG
	void debugIgnoreLeaksForThisBlock(void* pBlockPtr);
#endif

	/// free the bytes. (assumes allocated by this dma.)
	void freeBytes(void* pMem);

	/**
		return the actual number of bytes that would be allocated 
		if you tried to allocate the given size. (It will generally be slightly
		larger than you request.) This lets you use extra space if you're gonna get it anyway...
		The idea is that you will call this before doing a memory allocation, to see if
		you got any extra "bonus" space.
	*/
	Int getActualAllocationSize(Int numBytes);

	/// destroy all allocations performed by this DMA.
	void reset();

	Int getDmaMemoryPoolCount() const { return m_numPools; }
	MemoryPool* getNthDmaMemoryPool(Int i) const { return m_pools[i]; }

	#ifdef MEMORYPOOL_DEBUG

		/// return true iff this block was allocated by this dma
		Bool debugIsBlockInDma(void *pBlock);

		/// return true iff the pool is a subpool of this dma
		Bool debugIsPoolInDma(MemoryPool *pool);

	#endif	// MEMORYPOOL_DEBUG
};

// ----------------------------------------------------------------------------
#ifdef MEMORYPOOL_DEBUG
enum { MAX_SPECIAL_USED = 256 };
#endif

// ----------------------------------------------------------------------------
/**
	The class that manages all the MemoryPools and DynamicMemoryAllocators.
	Usually you will create exactly one of these (TheMemoryPoolFactory)
	and use it for everything.
*/
class MemoryPoolFactory
{
private:
	MemoryPool								*m_firstPoolInFactory;		///< linked list of pools
	DynamicMemoryAllocator		*m_firstDmaInFactory;			///< linked list of dmas
#ifdef MEMORYPOOL_CHECKPOINTING
	Int												m_curCheckpoint;					///< most recent checkpoint value
#endif
#ifdef MEMORYPOOL_DEBUG
	Int												m_usedBytes;							///< total bytes in use
	Int												m_physBytes;							///< total bytes allocated to all pools (includes unused blocks)
	Int												m_peakUsedBytes;					///< high-water mark of m_usedBytes
	Int												m_peakPhysBytes;					///< high-water mark of m_physBytes
	Int												m_usedBytesSpecial[MAX_SPECIAL_USED];
	Int												m_usedBytesSpecialPeak[MAX_SPECIAL_USED];
	Int												m_physBytesSpecial[MAX_SPECIAL_USED];
	Int												m_physBytesSpecialPeak[MAX_SPECIAL_USED];
#endif

public:

		// 'public' funcs that are really only for use by MemoryPool and friends
	#ifdef MEMORYPOOL_DEBUG
		/// adjust the usedBytes and physBytes variables by the given amoun ts.
		void adjustTotals(const char* tagString, Int usedDelta, Int physDelta);
	#endif
	#ifdef MEMORYPOOL_CHECKPOINTING
		/// return the current checkpoint value.
		Int getCurCheckpoint() { return m_curCheckpoint; }
	#endif

public:
	
	MemoryPoolFactory();
	void init();
	~MemoryPoolFactory();

	/// create a new memory pool with the given settings. if a pool with the given name already exists, return it.
	MemoryPool *createMemoryPool(const PoolInitRec *parms);

	/// overloaded version of createMemoryPool with explicit parms.
	MemoryPool *createMemoryPool(const char *poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount);
	
	/// return the pool with the given name. if no such pool exists, return null.
	MemoryPool *findMemoryPool(const char *poolName);

	/// destroy the given pool.
	void destroyMemoryPool(MemoryPool *pMemoryPool);

	/// create a DynamicMemoryAllocator with subpools with the given parms.
	DynamicMemoryAllocator *createDynamicMemoryAllocator(Int numSubPools, const PoolInitRec pParms[]);

	/// destroy the given DynamicMemoryAllocator.
	void destroyDynamicMemoryAllocator(DynamicMemoryAllocator *dma);

	/// destroy the contents of all pools and dmas. (the pools and dma's are not destroyed, just reset)
	void reset();

	void memoryPoolUsageReport( const char* filename, FILE *appendToFileInstead = NULL );

	#ifdef MEMORYPOOL_DEBUG

		/// perform internal consistency checking
		void debugMemoryVerify();

		/// return true iff the block was allocated by any pool or dma owned by this factory.
		Bool debugIsBlockInAnyPool(void *pBlock);

		/// return the tag string for the block. 
		const char *debugGetBlockTagString(void *pBlock);

		/// dump a report with the given options to the logfile.
		void debugMemoryReport(Int flags, Int startCheckpoint, Int endCheckpoint, FILE *fp = NULL );

		void debugSetInitFillerIndex(Int index);

	#endif
	#ifdef MEMORYPOOL_CHECKPOINTING
		
		/// set a new checkpoint.
		Int debugSetCheckpoint();

		/// reset all checkpoint information.
		void debugResetCheckpoints();

	#endif
};

// how many bytes are we allowed to 'waste' per pool allocation before the debug code starts yelling at us...
#define MEMORY_POOL_OBJECT_ALLOCATION_SLOP	16

// ----------------------------------------------------------------------------
#define GCMP_FIND(ARGCLASS, ARGPOOLNAME) \
private: \
	static MemoryPool *getClassMemoryPool() \
	{ \
		/* \
			Note that this static variable will be initialized exactly once: the first time \
			control flows over this section of code. This allows us to neatly resolve the \
			order-of-execution problem for static variables, ensuring this is not executed \
			prior to the initialization of TheMemoryPoolFactory. \
		*/ \
		DEBUG_ASSERTCRASH(TheMemoryPoolFactory, ("TheMemoryPoolFactory is NULL\n")); \
		static MemoryPool *The##ARGCLASS##Pool = TheMemoryPoolFactory->findMemoryPool(ARGPOOLNAME); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool, ("Pool \"%s\" not found (did you set it up in initMemoryPools?)\n", ARGPOOLNAME)); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool->getAllocationSize() >= sizeof(ARGCLASS), ("Pool \"%s\" is too small for this class (currently %d, need %d)\n", ARGPOOLNAME, The##ARGCLASS##Pool->getAllocationSize(), sizeof(ARGCLASS))); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool->getAllocationSize() <= sizeof(ARGCLASS)+MEMORY_POOL_OBJECT_ALLOCATION_SLOP, ("Pool \"%s\" is too large for this class (currently %d, need %d)\n", ARGPOOLNAME, The##ARGCLASS##Pool->getAllocationSize(), sizeof(ARGCLASS))); \
		return The##ARGCLASS##Pool; \
	} 

// ----------------------------------------------------------------------------
#define GCMP_CREATE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW) \
private: \
	static MemoryPool *getClassMemoryPool() \
	{ \
		/* \
			Note that this static variable will be initialized exactly once: the first time \
			control flows over this section of code. This allows us to neatly resolve the \
			order-of-execution problem for static variables, ensuring this is not executed \
			prior to the initialization of TheMemoryPoolFactory. \
		*/ \
		DEBUG_ASSERTCRASH(TheMemoryPoolFactory, ("TheMemoryPoolFactory is NULL\n")); \
		static MemoryPool *The##ARGCLASS##Pool = TheMemoryPoolFactory->createMemoryPool(ARGPOOLNAME, sizeof(ARGCLASS), ARGINITIAL, ARGOVERFLOW); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool, ("Pool \"%s\" not found (did you set it up in initMemoryPools?)\n", ARGPOOLNAME)); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool->getAllocationSize() >= sizeof(ARGCLASS), ("Pool \"%s\" is too small for this class (currently %d, need %d)\n", ARGPOOLNAME, The##ARGCLASS##Pool->getAllocationSize(), sizeof(ARGCLASS))); \
		DEBUG_ASSERTCRASH(The##ARGCLASS##Pool->getAllocationSize() <= sizeof(ARGCLASS)+MEMORY_POOL_OBJECT_ALLOCATION_SLOP, ("Pool \"%s\" is too large for this class (currently %d, need %d)\n", ARGPOOLNAME, The##ARGCLASS##Pool->getAllocationSize(), sizeof(ARGCLASS))); \
		return The##ARGCLASS##Pool; \
	} 
	
// ----------------------------------------------------------------------------
#define MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
protected: \
	virtual ~ARGCLASS(); \
public: \
	enum ARGCLASS##MagicEnum { ARGCLASS##_GLUE_NOT_IMPLEMENTED = 0 }; \
public: \
	inline void *operator new(size_t s, ARGCLASS##MagicEnum e DECLARE_LITERALSTRING_ARG2) \
	{ \
		DEBUG_ASSERTCRASH(s == sizeof(ARGCLASS), ("The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up correctly")); \
		return ARGCLASS::getClassMemoryPool()->allocateBlockImplementation(PASS_LITERALSTRING_ARG1); \
	} \
public: \
	/* \
		Note that this delete operator can't be called directly; it is called \
		only if the analogous new operator is called, AND the constructor \
		throws an exception... \
	*/ \
	inline void operator delete(void *p, ARGCLASS##MagicEnum e DECLARE_LITERALSTRING_ARG2) \
	{ \
		ARGCLASS::getClassMemoryPool()->freeBlock(p); \
	} \
protected: \
	/* \
		Make normal new and delete protected, so they can't be called by the outside world. \
		Note that delete is funny, in that it can still be called by the class itself; \
		this is safe but not recommended, for consistency purposes. More problematically, \
		it can be called by another class that has declared itself 'friend' to us. \
		In theory, this shouldn't work, since it may not use the right operator-delete, \
		and thus the wrong memory pool; in practice, it seems the right delete IS called \
		in MSVC -- it seems to make operator delete virtual if the destructor is also virtual. \
		At any rate, this is undocumented behavior as far as I can tell, so we put a big old \
		crash into operator delete telling people to do the right thing and call deleteInstance \
		instead -- it'd be nice if we could catch this at compile time, but catching it at \
		runtime seems to be the best we can do... \
	*/ \
	inline void *operator new(size_t s) \
	{ \
		DEBUG_CRASH(("This operator new should normally never be called... please use new(char*) instead.")); \
		DEBUG_ASSERTCRASH(s == sizeof(ARGCLASS), ("The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up correctly")); \
		throw ERROR_BUG; \
		return 0; \
	} \
	inline void operator delete(void *p) \
	{ \
		DEBUG_CRASH(("Please call deleteInstance instead of delete.")); \
		ARGCLASS::getClassMemoryPool()->freeBlock(p); \
	} \
private: \
	virtual MemoryPool *getObjectMemoryPool() \
	{ \
		return ARGCLASS::getClassMemoryPool(); \
	} \
public: /* include this line at the end to reset visibility to 'public' */ 

// ----------------------------------------------------------------------------
#define MEMORY_POOL_GLUE(ARGCLASS, ARGPOOLNAME) \
	MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
	GCMP_FIND(ARGCLASS, ARGPOOLNAME)

// ----------------------------------------------------------------------------
#define MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW) \
	MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
	GCMP_CREATE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW)

// ----------------------------------------------------------------------------
#define MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ARGCLASS, ARGPOOLNAME) \
	MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
	GCMP_CREATE(ARGCLASS, ARGPOOLNAME, -1, -1)

// ----------------------------------------------------------------------------
// this is the version for an Abstract Base Class, which will never be instantiated...
#define MEMORY_POOL_GLUE_ABC(ARGCLASS) \
protected: \
	virtual ~ARGCLASS(); \
public: \
	enum ARGCLASS##MagicEnum { ARGCLASS##_GLUE_NOT_IMPLEMENTED = 0 }; \
protected: \
	inline void *operator new(size_t s, ARGCLASS##MagicEnum e DECLARE_LITERALSTRING_ARG2) \
	{ \
		DEBUG_CRASH(("this should be impossible to call (abstract base class)")); \
		DEBUG_ASSERTCRASH(s == sizeof(ARGCLASS), ("The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up correctly")); \
		throw ERROR_BUG; \
		return 0; \
	} \
protected: \
	inline void operator delete(void *p, ARGCLASS##MagicEnum e DECLARE_LITERALSTRING_ARG2) \
	{ \
		DEBUG_CRASH(("this should be impossible to call (abstract base class)")); \
	} \
protected: \
	inline void *operator new(size_t s) \
	{ \
		DEBUG_CRASH(("this should be impossible to call (abstract base class)")); \
		DEBUG_ASSERTCRASH(s == sizeof(ARGCLASS), ("The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up correctly")); \
		throw ERROR_BUG; \
		return 0; \
	} \
	inline void operator delete(void *p) \
	{ \
		DEBUG_CRASH(("this should be impossible to call (abstract base class)")); \
	} \
private: \
	virtual MemoryPool *getObjectMemoryPool() \
	{ \
		throw ERROR_BUG; \
		return 0; \
	} \
public: /* include this line at the end to reset visibility to 'public' */ 

// ----------------------------------------------------------------------------
/**
	Sometimes you want to make a class's destructor protected so that it can only
	be destroyed under special circumstances. MemoryPoolObject short-circuits this
	by making the destructor always be protected, and the true delete technique
	(namely, deleteInstance) always public by default. You can simulate the behavior
	you really want by including this macro 
*/
#define MEMORY_POOL_DELETEINSTANCE_VISIBILITY(ARGVIS)\
ARGVIS:	void deleteInstance() { MemoryPoolObject::deleteInstance(); } public: 


// ----------------------------------------------------------------------------
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

protected: 
	inline void *operator new(size_t s) { DEBUG_CRASH(("This should be impossible")); return 0; }
	inline void operator delete(void *p) { DEBUG_CRASH(("This should be impossible")); }

protected: 

	virtual MemoryPool *getObjectMemoryPool() = 0;
	
public: 

	void deleteInstance() 
	{	
		if (this)
		{
			MemoryPool *pool = this->getObjectMemoryPool(); // save this, since the dtor will nuke our vtbl
			this->~MemoryPoolObject();	// it's virtual, so the right one will be called.
			pool->freeBlock((void *)this); 
		}
	} 
};

// ----------------------------------------------------------------------------
/**
	A simple utility class to ensure exception safety; this holds a MemoryPoolObject
	and deletes it in its destructor. Especially useful for iterators!
*/
class MemoryPoolObjectHolder
{
private:
	MemoryPoolObject *m_mpo;
public:
	MemoryPoolObjectHolder(MemoryPoolObject *mpo = NULL) : m_mpo(mpo) { }
	void hold(MemoryPoolObject *mpo) { DEBUG_ASSERTCRASH(!m_mpo, ("already holding")); m_mpo = mpo; }
	void release() { m_mpo = NULL; }
	~MemoryPoolObjectHolder() { m_mpo->deleteInstance(); }
};


// INLINING ///////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
inline MemoryPoolFactory *MemoryPool::getOwningFactory() { return m_factory; }
inline MemoryPool *MemoryPool::getNextPoolInList() { return m_nextPoolInFactory; }
inline const char *MemoryPool::getPoolName() { return m_poolName; }
inline Int MemoryPool::getAllocationSize() { return m_allocationSize; }
inline Int MemoryPool::getFreeBlockCount() { return getTotalBlockCount() - getUsedBlockCount(); }
inline Int MemoryPool::getUsedBlockCount() { return m_usedBlocksInPool; }
inline Int MemoryPool::getTotalBlockCount() { return m_totalBlocksInPool; }
inline Int MemoryPool::getPeakBlockCount() { return m_peakUsedBlocksInPool; }
inline Int MemoryPool::getInitialBlockCount() { return m_initialAllocationCount; }

// ----------------------------------------------------------------------------
inline DynamicMemoryAllocator *DynamicMemoryAllocator::getNextDmaInList() { return m_nextDmaInFactory; }

// EXTERNALS //////////////////////////////////////////////////////////////////

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
	similar to initMemoryManager, but this should be used if the memory manager must be initialized
	prior to main() (e.g., from a static constructor). If preMainInitMemoryManager() is called prior
	to initMemoryManager(), then subsequent calls to either are quietly ignored, AS IS any subsequent
	call to shutdownMemoryManager() [since there's no safe way to ensure that shutdownMemoryManager
	will execute after all static destructors].

	(Note: this function is actually not externally visible, but is documented here for clarity.)
*/
/* extern void preMainInitMemoryManager(); */

/**
	Shut down the memory manager. Throw away TheMemoryPoolFactory and 
	TheDynamicMemoryAllocator.
*/
extern void shutdownMemoryManager();

extern MemoryPoolFactory *TheMemoryPoolFactory;
extern DynamicMemoryAllocator *TheDynamicMemoryAllocator;

/**
	This function is declared in this header, but is not defined anywhere -- you must provide
	it in your code. It is called by initMemoryManager() or preMainInitMemoryManager() in order
	to get the specifics of the subpool for the dynamic memory allocator. (If you just want
	some defaults, set both return arguments to zero.) The reason for this odd setup is that 
	we may need to init the memory manager prior to main() [due to static C++ ctors] and
	this allows us a way to get the necessary parameters.
*/
extern void userMemoryManagerGetDmaParms(Int *numSubPools, const PoolInitRec **pParms);

/**
	This function is declared in this header, but is not defined anywhere -- you must provide
	it in your code. It is called by initMemoryManager() or preMainInitMemoryManager() in order
	to initialize the pools to be used. (You can define an empty function if you like.)
*/
extern void userMemoryManagerInitPools();

/**
	This function is declared in this header, but is not defined anywhere -- you must provide
	it in your code. It is called by createMemoryPool to adjust the allocation size(s) for a 
	given pool. Note that the counts are in-out parms!
*/
extern void userMemoryAdjustPoolSize(const char *poolName, Int& initialAllocationCount, Int& overflowAllocationCount);

#ifdef __cplusplus

#ifndef _OPERATOR_NEW_DEFINED_

	#define _OPERATOR_NEW_DEFINED_

	extern void * __cdecl operator new		(size_t size);
	extern void __cdecl operator delete		(void *p);

	extern void * __cdecl operator new[]	(size_t size);
	extern void __cdecl operator delete[]	(void *p);

	// additional overloads to account for VC/MFC funky versions
	extern void* __cdecl operator new(size_t nSize, const char *, int);
	extern void __cdecl operator delete(void *, const char *, int);

	extern void* __cdecl operator new[](size_t nSize, const char *, int);
	extern void __cdecl operator delete[](void *, const char *, int);

#if defined(_MSC_VER) && _MSC_VER < 1300
	// additional overloads for 'placement new'
	//inline void* __cdecl operator new							(size_t s, void *p) { return p; }
	//inline void __cdecl operator delete						(void *, void *p)		{ }
	inline void* __cdecl operator new[]						(size_t s, void *p) { return p; }
	inline void __cdecl operator delete[]					(void *, void *p)		{ }
#endif

#endif

#ifdef MEMORYPOOL_DEBUG_CUSTOM_NEW
	#define MSGNEW(MSG)		new(MSG, 0)
	#define NEW						new(__FILE__, __LINE__)
#else
	#define MSGNEW(MSG)		new
	#define NEW						new
#endif

#endif

class STLSpecialAlloc 
{
public:
	static void* allocate(size_t __n);
	static void deallocate(void* __p, size_t);
};

#define EMPTY_DTOR(CLASS) inline CLASS::~CLASS() { }

#endif // _GAME_MEMORY_H_
