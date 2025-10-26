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

///////////////////////////////////////////////////////////////////////////////
// FastAllocator.h
//
// Implements simple and fast memory allocators. Includes sample code for
// STL and overriding of specific or global new/delete.
//
// Allocators are very fast and prevent memory fragmentation. They use up
// a little more space on average than generic new/malloc free/delete.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#define MEMORY_OVERWRITE_TEST



///////////////////////////////////////////////////////////////////////////////
// Include files
//
#include "always.h"
#include "wwdebug.h"
#include "mutex.h"
#include <malloc.h>
#include <stddef.h> //size_t & ptrdiff_t definition
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//
class FastFixedAllocator;    //Allocates and deletes items of a fixed size.
class FastAllocatorGeneral;  //Allocates and deletes items of any size. Can use as a fast replacement for global new/delete.



///////////////////////////////////////////////////////////////////////////////
// StackAllocator
//
// Introduction:
//    This templated class allows you to allocate a dynamically sized temporary
//    block of memory local to a function in a way that causes it to use some
//    stack space instead of using the (very slow) malloc function. You don't
//    want to call malloc/new to allocate temporary blocks of memory when speed
//    is important. This class is only practical if you are allocating arrays of
//    objects (especially smaller objects). If you have a class whose size is
//    40K, you aren't going to be able to put even one of them on the stack.
//    If you don't need an array but instead just need a single object, then you
//    can simply create your object on the stack and be done with it. However,
//    there are some times when you need a temporary array of objects (or just
//    an array of chars or pointers) but don't know the size beforehand. That's
//    where this class is useful.
//
// Parameters:
//    The class takes three templated parameters: T, nStackCount, bConstruct.
//      T           -- The type of object being allocated. Examples include char,
//                     cRZString, int*, cRZRect*.
//      nStackCount -- How many items to allocate on the stack before switching
//                     to a heap allocation. Note that if you want to write
//                     portable code, you need to use no more than about 2K
//                     of stack space. So make sure nStackCount*sizeof(T) < 2048.
//      bConstruct  -- This is normally 1, but setting it to 0 is a hint to the
//                     compiler that you are constructing a type that has no
//                     constructor. Testing showed that the VC++ compiler wasn't
//                     completely able to optimize on its own. In particular, it
//                     was smart enough to remove the while loop, but not smart
//                     enough to remore the pTArrayEnd assignment.
//
// Benchmarks:
//    Testing one one machine showed that allocating and freeing a block of
//    2048 bytes via malloc/free took 8700 and 8400 clock ticks respectively.
//    Using this stack allocator to do the same thing tooko 450 and 375 clock
//    ticks respectively.
//
// Usage and examples:
//    Since we are using a local allocator and not overloading global or class
//    operator new, you have to directory call StackAllocator::New instead
//    of operator new. So instead of saying:
//       SomeStruct* pStructArray = new SomeStruct[13];
//    you say:
//       SomeStruct* pStructArray = stackAllocator.New(13);
//
//    void Example(int nSize){
//       StackAllocator<char, 512> stackAllocator;    //Create an instance
//       char* pArray = stackAllocator.New(nSize);    //Allocate memory, it will auto-freed.
//       memset(pArray, 0, nSize*sizeof(char));       //Do something with the memory.
//    }
//
//    void Example(int nSize){
//       StackAllocator<int*, 512, 0> stackAllocator; //Create an instance. We use the 'construct' hint feature here.
//       int** pArray = stackAllocator.New(nSize);    //Allocate memory.
//       memset(pArray, 0, nSize*sizeof(int*));       //Do something with the memory.
//       stackAllocator.Delete(pArray);               //In this example, we explicity free the memory.
//    }
//
//    void Example(int nSize){
//       StackAllocator<cRZRect, 200> stackAllocator;       //Create an instance
//       cRZRect* pRectArray = stackAllocator.New(nSize);   //Allocate memory, it will auto-freed.
//       pRectArray[0].SetRect(0,0,1,1);                    //Do something with the memory.
//    }
//
//    void Example(int nSize){
//       StackAllocator<char, 200> stackAllocator;    //Create an instance
//       char* pArray  = stackAllocator.New(nSize);   //Allocate memory.
//       char* pArray2 = stackAllocator.New(nSize);   //Allocate some additional memory.
//       memset(pArray, 0, nSize*sizeof(char));       //Do something with the memory.
//       stackAllocator.Delete(pArray);               //Delete the memory
//       pArray = stackAllocator.New(nSize*2);        //Allocate some new memory. We'll let the allocator free it.
//       memset(pArray, 0, nSize*2*sizeof(char));     //Do something with the new memory.
//       stackAllocator.Delete(pArray2);              //Delete the additional memory.
//    }
//
template<class T, int nStackCount, int bConstruct=1>
class StackAllocator{
public:
   StackAllocator() : mnAllocCount(-1), mpTHeap(NULL){}
  ~StackAllocator(){
      if(mnAllocCount != -1){ //If there is anything to do...
         if(mpTHeap)
            delete mpTHeap;
         else{
            if(bConstruct){ //Since this constant, the comparison gets optimized away.
               T* pTArray = (T*)mTArray;
               const T* const pTArrayEnd = pTArray + mnAllocCount;
               while(pTArray < pTArrayEnd){
                  pTArray->~T(); //Call the destructor on the object directly.
                  ++pTArray;
               }
            }
         }
      }
   }

   T* New(unsigned nCount){
      if(mnAllocCount == -1){
         mnAllocCount = nCount;
         if(nCount < nStackCount){ //If the request is small enough to come from the stack...
            //We call the constructors of all the objects here.
            if(bConstruct){ //Since this constant, the comparison gets optimized away.
               T* pTArray = (T*)mTArray;
               const T* const pTArrayEnd = pTArray + nCount;
               while(pTArray < pTArrayEnd){
                  //Use the placement operator new. This simply calls the constructor
                  //of T with 'this' set to the input address. Note that we don't put
                  //a '()' after the T this is because () causes trivial types like int
                  //and class* to be assigned zero/NULL. We don't want that.
                  new(pTArray)T;
                  ++pTArray;
               }
            }
            return (T*)mTArray;
         }
         //Else the request is too big. So let's use (the slower) operator new.
         return (mpTHeap = new T[nCount]); //The compiler will call the constructors here.
      }
      //Else we are being used. Let's be nice and allocate something anyway.
      return new T[nCount];
   }

   void Delete(T* pT){
      if(pT == (T*)mTArray){ //If the allocation came from our stack...
         if(bConstruct){ //Since this constant, the comparison gets optimized away.
            T* pTArray = (T*)mTArray;
            const T* const pTArrayEnd = pTArray + mnAllocCount;
            while(pTArray < pTArrayEnd){
               pTArray->~T(); //Call the destructor on the object directly.
               ++pTArray;
            }
         }
         mnAllocCount = -1;
      }
      else if(pT == mpTHeap){ //If the allocation came from our heap...
         delete[] mpTHeap;    //The compiler will call the destructors here.
         mpTHeap      = NULL; //We clear these out so that we can possibly
         mnAllocCount = -1;   //  use the allocator again.
      }
      else //Else the allocation came from the external heap.
         delete[] pT;
   }

protected:
   int  mnAllocCount;                     //Count of objects allocated. -1 means that nothing is allocated. We don't use zero because zero is a legal allocation count in C++.
   T*   mpTHeap;                          //This is normally NULL, but gets used of the allocation request is too high.
   char mTArray[nStackCount*sizeof(T)];   //This is our stack memory.
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// class FastFixedAllocator
//
class FastFixedAllocator
{
public:
	FastFixedAllocator(unsigned int n=0);
  ~FastFixedAllocator();
   void  Init(unsigned int n); //Useful for setting allocation size *after* construction,
                               //but before first use.
	void* Alloc();
	void  Free(void* pAlloc);

	unsigned Get_Heap_Size() const { return TotalHeapSize; }
	unsigned Get_Allocated_Size() const { return TotalAllocatedSize; }
	unsigned Get_Allocation_Count() const { return TotalAllocationCount; }

protected:
	struct Link
	{
		Link* next;
	};

   struct Chunk
	{
		enum {
			size = 8*1024-16
		};

		Chunk* next;
		char mem[size];
	};
	Chunk* chunks;
	unsigned int esize;
	unsigned TotalHeapSize;
	unsigned TotalAllocatedSize;
	unsigned TotalAllocationCount;
	Link*  head;
	void   grow();
};

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void* FastFixedAllocator::Alloc()
{
	TotalAllocationCount++;
	TotalAllocatedSize+=esize;
   if (head==0) {
      grow();
	}
   Link* p = head;
   head    = p->next;
   return p;
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void FastFixedAllocator::Free(void* pAlloc)
{
	TotalAllocationCount--;
	TotalAllocatedSize-=esize;
   Link* p = static_cast<Link*>(pAlloc);
   p->next = head;
   head    = p;
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE FastFixedAllocator::FastFixedAllocator(unsigned int n) : esize(1), TotalHeapSize(0), TotalAllocatedSize(0), TotalAllocationCount(0)
{
   head   = 0;
   chunks = 0;
   Init(n);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE FastFixedAllocator::~FastFixedAllocator()
{
   Chunk* n = chunks;
   while(n){
      Chunk* p = n;
      n = n->next;
      delete p;
   }
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void FastFixedAllocator::Init(unsigned int n)
{
   esize = (n<sizeof(Link*) ? sizeof(Link*) : n);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void FastFixedAllocator::grow()
{
   Chunk* n = new Chunk;
   n->next  = chunks;
   chunks   = n;
	TotalHeapSize+=sizeof(Chunk);

   const int nelem = Chunk::size/esize;
   char* start = n->mem;
   char* last = &start[(nelem-1)*esize];
   for(char* p = start; p<last; p+=esize)
      reinterpret_cast<Link*>(p)->next = reinterpret_cast<Link*>(p+esize);
   reinterpret_cast<Link*>(last)->next = 0;
   head = reinterpret_cast<Link*>(start);
}

///////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////
// class FastAllocatorGeneral
//
// This class works by putting sizes into fixed size buckets. Each fixed size
// bucket is a FastFixedAllocator.
//
class FastAllocatorGeneral
{
	enum {
		MAX_ALLOC_SIZE=2048,
		ALLOC_STEP=16
	};
public:
	FastAllocatorGeneral();
	void* Alloc(unsigned int n);
	void  Free(void* pAlloc);
  	void* Realloc(void* pAlloc, unsigned int n);

	unsigned Get_Total_Heap_Size();
	unsigned Get_Total_Allocated_Size();
	unsigned Get_Total_Allocation_Count();
	unsigned Get_Total_Actual_Memory_Usage() { return ActualMemoryUsage; }

	static FastAllocatorGeneral* Get_Allocator();

protected:
   FastFixedAllocator allocators[MAX_ALLOC_SIZE/ALLOC_STEP];
	FastCriticalSectionClass CriticalSections[MAX_ALLOC_SIZE/ALLOC_STEP];
	bool MemoryLeakLogEnabled;

	unsigned AllocatedWithMalloc;
	unsigned AllocatedWithMallocCount;
	unsigned ActualMemoryUsage;
};
///////////////////////////////////////////////////////////////////////////////

WWINLINE unsigned FastAllocatorGeneral::Get_Total_Heap_Size()
{
	int size=AllocatedWithMalloc;
	for (int i=0;i<MAX_ALLOC_SIZE/ALLOC_STEP;++i) {
		FastCriticalSectionClass::LockClass lock(CriticalSections[i]);
		size+=allocators[i].Get_Heap_Size();
	}
	return size;
}

WWINLINE unsigned FastAllocatorGeneral::Get_Total_Allocated_Size()
{
	int size=AllocatedWithMalloc;
	for (int i=0;i<MAX_ALLOC_SIZE/ALLOC_STEP;++i) {
		FastCriticalSectionClass::LockClass lock(CriticalSections[i]);
		size+=allocators[i].Get_Allocated_Size();
	}
	return size;
}

WWINLINE unsigned FastAllocatorGeneral::Get_Total_Allocation_Count()
{
	int count=AllocatedWithMallocCount;
	for (int i=0;i<MAX_ALLOC_SIZE/ALLOC_STEP;++i) {
		FastCriticalSectionClass::LockClass lock(CriticalSections[i]);
		count+=allocators[i].Get_Allocation_Count();
	}
	return count;
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void* FastAllocatorGeneral::Alloc(unsigned int n)
{
   void* pMemory;
	static int re_entrancy=0;
	re_entrancy++;

   //We actually allocate n+4 bytes. We store the # allocated
   //in the first 4 bytes, and return the ptr to the rest back
   //to the user.
   n += sizeof(unsigned int);
#ifdef MEMORY_OVERWRITE_TEST
	n+=sizeof(unsigned int);
#endif

	if (re_entrancy==1) {
		ActualMemoryUsage+=n;
	}
	if (n<MAX_ALLOC_SIZE) {
		int index=(n)/ALLOC_STEP;
		{
			FastCriticalSectionClass::LockClass lock(CriticalSections[index]);
			pMemory = allocators[index].Alloc();
		}
	}
   else {
		if (re_entrancy==1) {
			AllocatedWithMalloc+=n;
			AllocatedWithMallocCount++;
		}
      pMemory = ::malloc(n);
	}
#ifdef MEMORY_OVERWRITE_TEST
	*((unsigned int*)((char*)pMemory+n)-1)=0xabbac0de;
#endif

	re_entrancy--;
   *((unsigned int*)pMemory) = n;     //Write modified (augmented by 4) count into first four bytes.
   return ((unsigned int*)pMemory)+1; //return ptr to bytes after it back to user.
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

WWINLINE void FastAllocatorGeneral::Free(void* pAlloc)
{
   if (pAlloc) {
      unsigned int* n = ((unsigned int*)pAlloc)-1; //Subtract four bytes and the count is stored there.

#ifdef MEMORY_OVERWRITE_TEST
		WWASSERT(*((unsigned int*)((char*)n+*n)-1)==0xabbac0de);
#endif

		unsigned size=*n;
		ActualMemoryUsage-=size;

		if (size<MAX_ALLOC_SIZE) {
			int index=size/ALLOC_STEP;
			FastCriticalSectionClass::LockClass lock(CriticalSections[index]);
         allocators[index].Free(n);
		}
      else {
			AllocatedWithMallocCount--;
			AllocatedWithMalloc-=size;
         ::free(n);
		}
   }
}

//ANSI C requires:
//  (1) realloc(NULL, newsize) is equivalent to malloc(newsize).
//  (2) realloc(pblock, 0) is equivalent to free(pblock) (except that NULL is returned).
//  (3) if the realloc() fails, the object pointed to by pblock is left unchanged.
//
WWINLINE void* FastAllocatorGeneral::Realloc(void* pAlloc, unsigned int n){
   if(n){
      void* const pNewAlloc = Alloc(n);      //Allocate the new memory. This never fails.
      if(pAlloc){
         n = *(((unsigned int*)pAlloc)-1);   //Subtract four bytes and the count is stored there.
         ::memcpy(pNewAlloc, pAlloc, n);     //Copy the old memory into the new memory.
         Free(pAlloc);                       //Delete the old memory.
      }
      return pNewAlloc;
   }
   Free(pAlloc);
   return NULL;
}







///////////////////////////////////////////////////////////////////////////////
// FastSTLAllocator
//
// An STL allocator based on a simple fixed size allocator is not going to
// buy you the performance you really want. This is because STL containers
// allocate not just objects of the size of the container, but other objects
// as well. For example, the STL "list" class generally allocates item nodes
// and not items themselves. The STL vector class usually allocates in chunks
// of contiguous items. So your allocator will probably want to have a bucket
// system whereby it maintains buckets for integral sizes.
//

#ifdef _MSC_VER
   //VC++ continues to be the one compiler that lacks the ability to compile
   //standard C++. So we define a version of the STL allocator specifically
   //for VC++, and let other compilers use a standard allocator template.
   template <class T>
   struct FastSTLAllocator{
      typedef size_t    size_type;        //basically, "unsigned int"
      typedef ptrdiff_t difference_type;  //basically, "int"
      typedef T*        pointer;
      typedef const T*  const_pointer;
      typedef T&        reference;
      typedef const T&  const_reference;
      typedef T         value_type;

      T*            address(T& t)       const             { return (&t); } //These two are slightly strange but
      const  T*     address(const T& t) const             { return (&t); } //required functions. Just do it.
      static T*     allocate(size_t n, const void* =NULL) { return (T*)FastAllocatorGeneral::Get_Allocator()->Alloc(n*sizeof(T)); }
      static void   construct(T* ptr, const T& value)     { new(ptr) T(value); }
      static void   deallocate(void* ptr, size_t /*n*/)   { FastAllocatorGeneral::Get_Allocator()->Free(ptr); }
      static void   destroy(T* ptr)                       { ptr->~T(); }
      static size_t max_size()                            { return (size_t)-1; }

      //This _Charalloc is required by VC++5 since it VC++5 predates
      //the language standardization. Allocator behaviour is one of the
      //last things to have been hammered out. Important note: If you
      //decide to write your own fast allocator, containers will allocate
      //random objects through this function but delete them through
      //the above delallocate() function. So your version of deallocate
      //should *not* assume that it will only be given T objects to delete.
      char* _Charalloc(size_t n){ return (char*)FastAllocatorGeneral::Get_Allocator()->Alloc(n*sizeof(char)); }
   };
#else
   //This is a C++ language standard allocator. Most C++ compilers after 1999
   //other than Microsoft C++ compile this fine. Otherwise. you might be able
   //to use the same allocator as VC++ uses above.
   template <class T>
   class FastSTLAllocator{
   public:
     typedef size_t     size_type;
     typedef ptrdiff_t  difference_type;
     typedef T*         pointer;
     typedef const T*   const_pointer;
     typedef T&         reference;
     typedef const T&   const_reference;
     typedef T          value_type;

     template <class T1> struct rebind {
        typedef FastSTLAllocator<T1> other;
     };

     FastSTLAllocator() {}
     FastSTLAllocator(const FastSTLAllocator&) {}
     template <class T1> FastSTLAllocator(const FastSTLAllocator<T1>&) {}
     ~FastSTLAllocator() {}

     pointer address(reference x) const             { return &x; }
     const_pointer address(const_reference x) const { return &x; }

     T* allocate(size_type n, const void* = NULL) { return n != 0 ? static_cast<T*>(FastAllocatorGeneral::Get_Allocator()->Alloc(n*sizeof(T))) : NULL; }
     void deallocate(pointer p, size_type n)      { FastAllocatorGeneral::Get_Allocator()->Free(p); }
     size_type max_size() const                   { return size_t(-1) / sizeof(T); }
     void construct(pointer p, const T& val)      { new(p) T(val); }
     void destroy(pointer p)                      { p->~T(); }
   };
#endif

template<class T>
WWINLINE bool operator==(const FastSTLAllocator<T>&, const FastSTLAllocator<T>&) { return true;  }
template<class T>
WWINLINE bool operator!=(const FastSTLAllocator<T>&, const FastSTLAllocator<T>&) { return false; }
///////////////////////////////////////////////////////////////////////////////





/*
///////////////////////////////////////////////////////////////////////////////
// Example usage of fast allocators in STL.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <string>
#include <memory>
#include <hash_set> //Uncomment this if you have hash containers available.
#include <hash_map>
using namespace std;



///////////////////////////////////////////////////////////////////////////////
// Here's how you declare a custom allocator for every regular STL container class:
//
typedef vector<int, FastSTLAllocator<int> >                            IntArray;
typedef list<int, FastSTLAllocator<int> >                              IntList;
typedef deque<int, FastSTLAllocator<int> >                             IntDequeue;
typedef queue<int, deque<int, FastSTLAllocator<int> > >                IntQueue;
typedef priority_queue<int, vector<int, FastSTLAllocator<int> > >      IntPriorityQueue;
typedef stack<int, deque<int, FastSTLAllocator<int> > >                IntStack;
typedef map<int, int, less<int>, FastSTLAllocator<int> >               IntMap;
typedef multimap<int, int, less<int>, FastSTLAllocator<int> >          IntMultiMap;
typedef set<int, less<int>, FastSTLAllocator<int> >                    IntSet;
typedef multiset<int, less<int>, FastSTLAllocator<int> >               IntMultiSet;

//If you have the hashing containers available, here's how you do it:
typedef hash_map<int, int, hash<int>, equal_to<int>, FastSTLAllocator<int> >        IntHashMap;
typedef hash_multimap<int, int, hash<int>, equal_to<int>, FastSTLAllocator<int> >   IntHashMultiMap;
typedef hash_set<int, hash<int>, equal_to<int>, FastSTLAllocator<int> >             IntHashSet;
typedef hash_multiset<int, hash<int>, equal_to<int>, FastSTLAllocator<int> >        IntHashMultiSet;

typedef basic_string<char, char_traits<char>, FastSTLAllocator<char> > CharString;

//bitset and valarray don't allow custom allocators.

///////////////////////////////////////////////////////////////////////////////


void main(){
   IntArray intArray;
   intArray.push_back(3);
   intArray.pop_back();

   IntList intList;
   intList.push_back(3);
   intList.pop_back();

   IntDequeue intDequeue;
   intDequeue.push_back(3);
   intDequeue.pop_back();

   IntQueue intQueue;
   intQueue.push(3);
   intQueue.pop();

   IntPriorityQueue intPriorityQueue;
   intPriorityQueue.push(3);
   intPriorityQueue.pop();

   IntStack intStack;
   intStack.push(3);
   intStack.pop();

   IntMap intMap;
   intMap.insert(pair<int,int>(3,3));
   intMap.erase(3);

   IntMultiMap intMultiMap;
   intMultiMap.insert(pair<int,int>(3,3));
   intMultiMap.erase(3);

   IntSet intSet;
   intSet.insert(3);
   intSet.erase(3);

   IntMultiSet intMultiSet;
   intMultiSet.insert(3);
   intMultiSet.erase(3);

   IntHashMap intHashMap;
   intHashMap.insert(3);
   intHashMap.erase(3);

   IntHashMultiMap intHashMultiMap;
   intHashMultiMap.insert(3);
   intHashMultiMap.erase(3);

   IntHashSet intHashSet;
   intHashSet.insert(3);
   intHashSet.erase(3);

   IntHashMultiSet intHashMultiSet;
   intHashMultiSet.insert(3);
   intHashMultiSet.erase(3);

   CharString charString;
   charString.append(1, '3');
   charString.erase(charString.length()-1);

   //Shutdown
   printf("\nDone. Press the 'any' key\n");
   getchar();
}
*/
