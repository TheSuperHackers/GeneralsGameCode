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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwmemlog.cpp                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 2:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WWMemoryLogClass::Allocate_Memory -- allocates memory                                     *
 *   WWMemoryLogClass::Release_Memory -- frees memory                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "wwmemlog.h"
#include "wwdebug.h"
#include "Vector.H"
#include "FastAllocator.h"

#define USE_FAST_ALLOCATOR

#ifdef STEVES_NEW_CATCHER
	#define DISABLE_MEMLOG	1
#else //STEVES_NEW_CATCHER
#ifdef PARAM_EDITING_ON
	#define DISABLE_MEMLOG	1
#else //PARAM_EDITING_ON
	#define DISABLE_MEMLOG	1
#endif //PARAM_EDITING_ON
#endif //STEVES_NEW_CATCHER*/

#ifdef USE_FAST_ALLOCATOR
	#define ALLOC_MEMORY(n) FastAllocatorGeneral::Get_Allocator()->Alloc(n)
	#define FREE_MEMORY(p) FastAllocatorGeneral::Get_Allocator()->Free(p)
#else
	#define ALLOC_MEMORY(n) ::malloc(n)
	#define FREE_MEMORY(p) ::free(p)
#endif


/*
** Enable one of the following #defines to specify which thread-sychronization
** method to use.
*/
#ifdef _WIN32
#include <windows.h>
#define MEMLOG_USE_MUTEX					0
#define MEMLOG_USE_CRITICALSECTION		1
#define MEMLOG_USE_FASTCRITICALSECTION	0
#else
#undef DISABLE_MEMLOG
#define DISABLE_MEMLOG						1
#define MEMLOG_USE_MUTEX					0
#define MEMLOG_USE_CRITICALSECTION		0
#define MEMLOG_USE_FASTCRITICALSECTION	0
#endif

#if (DISABLE_MEMLOG == 0)
bool WWMemoryLogClass::IsMemoryLogEnabled=true;
#else
bool WWMemoryLogClass::IsMemoryLogEnabled=false;
#endif

static unsigned AllocateCount;
static unsigned FreeCount;

/*
** Name for each memory category.  I'm padding the array with some "undefined" strings in case
** someone forgets to set the name when adding a new category.
*/
static const char *const _MemoryCategoryNames[] =
{
	"UNKNOWN",
	"Geometry",
	"Animation",
	"Texture",
	"Pathfind",
	"Vis",
	"Sound",
	"CullingData",
	"Strings",
	"GameData",
	"PhysicsData",
	"W3dData",
	"StaticAllocations",
	"GameInit",
	"Renderer",
	"Network",
	"BINK",
	"<undefined>",
	"<undefined>",
	"<undefined>",
	"<undefined>",
};


/**
** MemoryCounterClass
** This object will store statistics for each memory category.  It can provide things like
** the current amount of allocated memory and the peak amount of allocated memory.
*/
class MemoryCounterClass
{
public:
	MemoryCounterClass(void) : CurrentAllocation(0), PeakAllocation(0) { }

	void		Memory_Allocated(int size)						{ CurrentAllocation+=size; PeakAllocation = max(PeakAllocation,CurrentAllocation); }
	void		Memory_Released(int size)						{ CurrentAllocation-=size; }

	int		Get_Current_Allocated_Memory(void)			{ return CurrentAllocation; }
	int		Get_Peak_Allocated_Memory(void)				{ return PeakAllocation; }

protected:
	int		CurrentAllocation;
	int		PeakAllocation;
};



/**
** ActiveCategoryStackClass
** This object is used to keep track of the "active memory category".  Whenever memory is allocated
** it will be charged to the current active memory category.  To be thread-safe, there will be
** one ActiveCategoryStack per thread that is encountered in the program.
*/
const int MAX_CATEGORY_STACK_DEPTH = 1024;
class ActiveCategoryStackClass : public VectorClass<int>
{
public:
	ActiveCategoryStackClass(void) :
		VectorClass<int>(MAX_CATEGORY_STACK_DEPTH),
		ThreadID(-1),
		Count(0)
	{ }

																				// If object was created but not Init'd, ThreadID will be -1 and Count == 0
																				// If object was created and Init'd, ThreadID will not be -1.  We expect Count to return to 1 after all Pop's
	~ActiveCategoryStackClass(void)									{ WWASSERT((ThreadID == -1 && Count == 0) || (ThreadID != -1 && Count == 1)); }

	ActiveCategoryStackClass & operator = (const ActiveCategoryStackClass & that);

	bool		operator == (const ActiveCategoryStackClass &)	{ return false; }
	bool		operator != (const ActiveCategoryStackClass &)	{ return true; }

	void		Init(int thread_id)										{ ThreadID = thread_id; Count = 0; Push(MEM_UNKNOWN); }
	void		Set_Thread_ID(int id)									{ WWASSERT(ThreadID != -1); ThreadID = id; }
	int		Get_Thread_ID(void)										{ return ThreadID; }

	void		Push(int active_category)								{ WWASSERT(ThreadID != -1); (*this)[Count] = active_category; Count++; }
	void		Pop(void)													{ WWASSERT(ThreadID != -1) ; Count--; }
	int		Current(void)												{ WWASSERT(ThreadID != -1); return (*this)[Count-1]; }

protected:

	int		ThreadID;
	int		Count;
};



/**
** ActiveCategoryClass
** This is a dynamic vector of ActiveCategoryStackClasses which adds a new stack each time
** a new thread is encountered.  It also is able to return to you the active category for
** the currently active thread automatically.
*/
const int MAX_CATEGORY_STACKS = 256;		// maximum number of threads we expect to encounter...

class ActiveCategoryClass : public VectorClass<ActiveCategoryStackClass>
{
public:

	ActiveCategoryClass(void) : VectorClass<ActiveCategoryStackClass>(MAX_CATEGORY_STACKS), Count(0) { Get_Active_Stack().Push(MEM_STATICALLOCATION); }

	void		Push(int active_category)	{ Get_Active_Stack().Push(active_category); }
	void		Pop(void)						{ Get_Active_Stack().Pop(); }
	int		Current(void)					{ return Get_Active_Stack().Current(); }

protected:

	ActiveCategoryStackClass & Get_Active_Stack(void);

	int		Count;
};


/**
** MemLogClass
** This class ties all of the logging datastructures together into a single object
** which can be created on demand when the first 'new' call is encountered.
*/
class MemLogClass
{
public:

	int				Get_Current_Allocated_Memory(int category);
	int				Get_Peak_Allocated_Memory(int category);

	/*
	** Interface for recording allocations and de-allocations
	*/
	int				Register_Memory_Allocated(int size);
	void				Register_Memory_Released(int category,int size);

	void				Push_Active_Category(int category);
	void				Pop_Active_Category(void);

	void				Init();

private:

	MemoryCounterClass		_MemoryCounters[MEM_COUNT];
	ActiveCategoryClass		_ActiveCategoryTracker;

};


/**
** Static Variables
** _TheMemLog - object which encapsulates all logging. will be allocated on first use
** _MemLogMutex - handle to the mutex used to arbtirate access to the logging data structures
** _MemLogLockCounter - count of the active mutex locks.
*/
static MemLogClass *				_TheMemLog = NULL;
static bool							_MemLogAllocated = false;

#if MEMLOG_USE_MUTEX
static void *						_MemLogMutex = NULL;
static int							_MemLogLockCounter = 0;
#endif

#if MEMLOG_USE_CRITICALSECTION
static bool							_MemLogCriticalSectionAllocated = false;
static char							_MemLogCriticalSectionHandle[sizeof(CRITICAL_SECTION)];
#endif

#if MEMLOG_USE_FASTCRITICALSECTION
volatile unsigned					_MemLogSemaphore = 0;
#endif

/*
** Use this code to get access to the mutex...
*/
WWINLINE void * Get_Mem_Log_Mutex(void)
{
#if MEMLOG_USE_MUTEX

	if (_MemLogMutex == NULL) {
		_MemLogMutex=CreateMutex(NULL,false,NULL);
		WWASSERT(_MemLogMutex);
	}
	return _MemLogMutex;

#endif

#if MEMLOG_USE_CRITICALSECTION

	if (_MemLogCriticalSectionAllocated == false) {
		InitializeCriticalSection((CRITICAL_SECTION*)_MemLogCriticalSectionHandle);
		_MemLogCriticalSectionAllocated = true;
	}
	return _MemLogCriticalSectionHandle;

#endif

#if DISABLE_MEMLOG
	return NULL;
#endif
}

WWINLINE void Lock_Mem_Log_Mutex(void)
{
#if MEMLOG_USE_MUTEX

	void * mutex = Get_Mem_Log_Mutex();
#ifdef DEBUG_CRASHING
	int res =
#endif
		WaitForSingleObject(mutex,INFINITE);
	WWASSERT(res==WAIT_OBJECT_0);
	_MemLogLockCounter++;
#endif

#if MEMLOG_USE_CRITICALSECTION

	Get_Mem_Log_Mutex();
	EnterCriticalSection((CRITICAL_SECTION*)_MemLogCriticalSectionHandle);

#endif

#if MEMLOG_USE_FASTCRITICALSECTION

	volatile unsigned& nFlag=_MemLogSemaphore;

	#define ts_lock _emit 0xF0
	assert(((unsigned)&nFlag % 4) == 0);

	__asm mov ebx, [nFlag]
	__asm ts_lock
	__asm bts dword ptr [ebx], 0
	__asm jc The_Bit_Was_Previously_Set_So_Try_Again
	return;

	The_Bit_Was_Previously_Set_So_Try_Again:
	ThreadClass::Switch_Thread();
	__asm mov ebx, [nFlag]
	__asm ts_lock
	__asm bts dword ptr [ebx], 0
	__asm jc  The_Bit_Was_Previously_Set_So_Try_Again

#endif
}

WWINLINE void Unlock_Mem_Log_Mutex(void)
{
#if MEMLOG_USE_MUTEX

	void * mutex = Get_Mem_Log_Mutex();
	_MemLogLockCounter--;
#ifdef DEBUG_CRASHING
	int res=
#endif
		ReleaseMutex(mutex);
	WWASSERT(res);

#endif
#if MEMLOG_USE_CRITICALSECTION

	Get_Mem_Log_Mutex();
	LeaveCriticalSection((CRITICAL_SECTION*)_MemLogCriticalSectionHandle);

#endif

#if MEMLOG_USE_FASTCRITICALSECTION
	_MemLogSemaphore = 0;
#endif
}

class MemLogMutexLockClass
{
public:
	MemLogMutexLockClass(void) { Lock_Mem_Log_Mutex(); }
	~MemLogMutexLockClass(void) { Unlock_Mem_Log_Mutex(); }
};



/***************************************************************************************************
**
** ActiveCategoryStackClass Implementation
**
***************************************************************************************************/
ActiveCategoryStackClass &
ActiveCategoryStackClass::operator = (const ActiveCategoryStackClass & that)
{
	if (this != &that) {
		VectorClass<int>::operator == (that);
		ThreadID = that.ThreadID;
		Count = that.Count;
	}
	return *this;
}


/***************************************************************************************************
**
** ActiveCategoryClass Implementation
**
***************************************************************************************************/
ActiveCategoryStackClass & ActiveCategoryClass::Get_Active_Stack(void)
{
	int current_thread = ::GetCurrentThreadId();

	/*
	** If we already have an allocated category stack for the current thread,
	** just return its active category.
	*/
	for (int i=0; i<Count; i++) {
		ActiveCategoryStackClass & cat_stack = (*this)[i];
		if (cat_stack.Get_Thread_ID() == current_thread) {
			return cat_stack;
		}
	}

	/*
	** If we fall through to here, we need to allocate a new category stack
	** for this thread.
	*/
	(*this)[Count].Init(current_thread);
	Count++;
	return (*this)[Count-1];
}


/***************************************************************************************************
**
** MemLogClass Implementation
**
***************************************************************************************************/
int MemLogClass::Get_Current_Allocated_Memory(int category)
{
	MemLogMutexLockClass lock;
	return _MemoryCounters[category].Get_Current_Allocated_Memory();
}

int MemLogClass::Get_Peak_Allocated_Memory(int category)
{
	MemLogMutexLockClass lock;
	return _MemoryCounters[category].Get_Peak_Allocated_Memory();
}

void MemLogClass::Init()
{
	{
		MemLogMutexLockClass lock;
		WWASSERT(_ActiveCategoryTracker.Current()==MEM_STATICALLOCATION);
	}
	Pop_Active_Category();	// Remove staticallocation state forever
}

int MemLogClass::Register_Memory_Allocated(int size)
{
	MemLogMutexLockClass lock;

	int active_category = _ActiveCategoryTracker.Current();
	WWASSERT((active_category >= 0) && (active_category < MEM_COUNT));
	_MemoryCounters[active_category].Memory_Allocated(size);

	return active_category;
}

void MemLogClass::Register_Memory_Released(int category,int size)
{
	MemLogMutexLockClass lock;
	_MemoryCounters[category].Memory_Released(size);
}

void MemLogClass::Push_Active_Category(int category)
{
	MemLogMutexLockClass lock;
	WWASSERT((category >= 0) && (category < MEM_COUNT));
	_ActiveCategoryTracker.Push(category);
}

void MemLogClass::Pop_Active_Category(void)
{
	MemLogMutexLockClass lock;
	_ActiveCategoryTracker.Pop();
}



/***************************************************************************************************
**
** WWMemoryLogClass Implementation
**
***************************************************************************************************/

int WWMemoryLogClass::Get_Category_Count(void)
{
	return MEM_COUNT;
}

const char * WWMemoryLogClass::Get_Category_Name(int category)
{
	return _MemoryCategoryNames[category];
}

int WWMemoryLogClass::Get_Current_Allocated_Memory(int category)
{
	return Get_Log()->Get_Current_Allocated_Memory(category);
}

int WWMemoryLogClass::Get_Peak_Allocated_Memory(int category)
{
	return Get_Log()->Get_Peak_Allocated_Memory(category);
}

void WWMemoryLogClass::Push_Active_Category(int category)
{
#if (DISABLE_MEMLOG == 0)
	Get_Log()->Push_Active_Category(category);
#endif //(DISABLE_MEMLOG == 0)
}

void WWMemoryLogClass::Pop_Active_Category(void)
{
#if (DISABLE_MEMLOG == 0)
	Get_Log()->Pop_Active_Category();
#endif //(DISABLE_MEMLOG == 0)
}

int WWMemoryLogClass::Register_Memory_Allocated(int size)
{
	return Get_Log()->Register_Memory_Allocated(size);
}

void WWMemoryLogClass::Register_Memory_Released(int category,int size)
{
	Get_Log()->Register_Memory_Released(category,size);
}


static void _MemLogCleanup(void)
{
	delete _TheMemLog;
	_TheMemLog = NULL;
}


MemLogClass * WWMemoryLogClass::Get_Log(void)
{
	MemLogMutexLockClass lock;

	if (_TheMemLog == NULL) {
		//assert(!_MemLogAllocated);
		_TheMemLog = W3DNEW MemLogClass;

#ifdef STEVES_NEW_CATCHER
		/*
		** This was me trying to be clever and fix the memory leak in the memlog. Unfortunately, the Get_Log member can be called
		** during the process of exiting the process (IYSWIM) and you get it trying to re-allocate the MemLogClass I just freed.
		** Solution is just to disable memlog when I'm trying to find memory leaks. ST - 6/18/2001 9:51PM
		*/
		if (!_MemLogAllocated) {
			atexit(&Release_Log);
		}
		_MemLogAllocated = true;
#endif //STEVES_NEW_CATCHER
	}
	return _TheMemLog;
}


/***********************************************************************************************
 * WWMemoryLogClass::Release_Log -- Free the memory used by WWMemoryLogClass so it doesn't leak*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: Called as part of _onexit processing                                              *
 *                                                                                             *
 *           It's messy, but I assume there's a reason it's not statically allocated...        *
 *           OK, now I get it                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/13/2001 8:55PM ST : Created                                                             *
 *=============================================================================================*/
void WWMemoryLogClass::Release_Log(void)
{
	MemLogMutexLockClass lock;

	delete _TheMemLog;
	_TheMemLog = NULL;
}


/***************************************************************************************************
**
** Allocating and Freeing memory
**
** PLEASE NOTE: The user is expected to implement global new and delete functions in his own
** code which call WWMemoryLogClass::Allocate_Memory and WWMemoryLogClass::Release_Memory.
** This was the only solution I could come up given that some APPS have their own new and delete
** functions or enable the CRT ones.  It was also not an option to move this entire system into
** the APP because I wanted all of our LIBs to participate in the memory usage logging...
**
***************************************************************************************************/

const int WWMEMLOG_KEY0 = (unsigned('G')<<24) | (unsigned('g')<<16) | (unsigned('0')<<8) | unsigned('l');
const int WWMEMLOG_KEY1 = (unsigned('~')<<24) | (unsigned('_')<<16) | (unsigned('d')<<8) | unsigned('3');


/**
** MemoryLogStruct
** This structure is added to the beginning of each memory allocation to facilitate
** tracking which category the memory belongs to when it is freed.  The size of
** this struct is also 16 bytes so that we wont be seriously affecting the alignment
** of allocated memory...
*/
struct MemoryLogStruct
{
	MemoryLogStruct(int category,int size) :
		Key0(WWMEMLOG_KEY0),
		Key1(WWMEMLOG_KEY1),
		Category(category),
		Size(size)
	{}

	bool		Is_Valid_Memory_Log(void)	{ return ((Key0 == WWMEMLOG_KEY0) && (Key1 == WWMEMLOG_KEY1)); }

	int		Key0;				// if this is not equal to WWMEMLOG_KEY0 then we don't have a valid log
	int		Key1;				// should be equal to WWMEMLOG_KEY1
	int		Category;		// category this memory belongs to
	int		Size;				// size of the allocation
};



/***********************************************************************************************
 * WWMemoryLogClass::Allocate_Memory -- allocates memory                                       *
 *                                                                                             *
 *    This function adds a header to the memory allocated so that when the memory is freed     *
 *    the proper memory category size can be decremented.  The application using this logging  *
 *    system should call this function from inside its overloaded 'new' operator.              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void * WWMemoryLogClass::Allocate_Memory(size_t size)
{
	AllocateCount++;
#if DISABLE_MEMLOG
	return ALLOC_MEMORY(size);
#else

	__declspec( thread ) static bool reentrancy_test = false;
	MemLogMutexLockClass lock;

	if (reentrancy_test) {
		return ALLOC_MEMORY(size);
	} else {
		reentrancy_test = true;

		/*
		** Allocate space for the requested buffer + our logging structure
		*/
		void * ptr = ALLOC_MEMORY(size + sizeof(MemoryLogStruct));

		if (ptr != NULL) {
			/*
			** Record this allocation
			*/
			int active_category = WWMemoryLogClass::Register_Memory_Allocated(size);

			/*
			** Write our logging structure into the beginning of the buffer.  I'm using
			** placement new syntax to initialize the log structure right in the memory buffer
			*/
			new(ptr) MemoryLogStruct(active_category,size);

			/*
			** Return the allocated memory to the user, skipping past our log structure.
			*/
			reentrancy_test = false;
			return (void*)(((char *)ptr) + sizeof(MemoryLogStruct));

		} else {
			reentrancy_test = false;
			return ptr;
		}

	}
#endif //DISABLE_MEMLOG
}


/***********************************************************************************************
 * WWMemoryLogClass::Release_Memory -- frees memory                                            *
 *                                                                                             *
 *    This function checks for a wwmemlog header and decrements the relevant memory category.  *
 *    It should be called in the application's custom delete operator.                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void WWMemoryLogClass::Release_Memory(void *ptr)
{
	FreeCount++;
#if DISABLE_MEMLOG
	FREE_MEMORY(ptr);
#else

	MemLogMutexLockClass lock;
	if (ptr) {

		/*
		** Check if this memory is preceeded by a valid MemoryLogStruct
		*/
		MemoryLogStruct * memlog = (MemoryLogStruct*)((char*)ptr - sizeof(MemoryLogStruct));
		if (memlog->Is_Valid_Memory_Log()) {

			/*
			** Valid MemoryLogStruct found, track the de-allocation and pass on
			** to the built-in free function.
			*/
			WWMemoryLogClass::Register_Memory_Released(memlog->Category,memlog->Size);
			FREE_MEMORY((void*)memlog);

		} else {

			/*
			** No valid MemoryLogStruct found, just call free on the memory.
			*/
			FREE_MEMORY(ptr);
		}
	}

#endif //DISABLE_MEMLOG
}

// Reset allocate and free counters

void WWMemoryLogClass::Reset_Counters()
{
	AllocateCount=0;
	FreeCount=0;
}

// Return allocate count since last reset
int WWMemoryLogClass::Get_Allocate_Count()
{
	return AllocateCount;
}

// Return allocate count since last reset
int WWMemoryLogClass::Get_Free_Count()
{
	return FreeCount;
}

void WWMemoryLogClass::Init()
{
	Get_Log()->Init();
}
