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
 *                 Project Name : WWLib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/mempool.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/26/01 3:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ObjectPoolClass::ObjectPoolClass -- constructor for ObjectPoolClass                       *
 *   ObjectPoolClass::~ObjectPoolClass -- destructor for ObjectPoolClass                       *
 *   ObjectPoolClass::Allocate_Object -- allocates an object for the user                      *
 *   ObjectPoolClass::Free_Object -- releases obj back into the pool                           *
 *   ObjectPoolClass::Allocate_Object_Memory -- internal function which returns memory for an  *
 *   ObjectPoolClass::Free_Object_Memory -- internal function, returns object's memory to the  *
 *   AutoPoolClass::operator new -- overriden new which calls the internal ObjectPool          *
 *   AutoPoolClass::operator delete -- overriden delete which calls the internal ObjectPool    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "bittype.h"
#include "wwdebug.h"
#include "mutex.h"
#include <new.h>
#include <stdlib.h>
#include <stddef.h>



/**********************************************************************************************
** ObjectPoolClass
**
** This class is designed to allocate blocks of objects of type T and then dole them out
** to you individually.  The motivation for it is the situation where you are allocating
** and freeing lots of little objects directly off the heap.  Through the use of this
** class, far fewer allocations will be actually made.
**
** Example Usage:
**
** ObjectPoolClass<ListNodeClass,256>  NodePool;
** ListNodeClass * node = NodePool.Allocate_Object();
** NodePool.Free_Object(node);
**
**********************************************************************************************/
template<class T,int BLOCK_SIZE = 64> 
class ObjectPoolClass
{
public:

	ObjectPoolClass(void);
	~ObjectPoolClass(void);

	T *		Allocate_Object(void);
	void		Free_Object(T * obj);

	T *		Allocate_Object_Memory(void);
	void		Free_Object_Memory(T * obj);

protected:

	T	*		FreeListHead;			
	uint32 *	BlockListHead;			
	int		FreeObjectCount;
	int		TotalObjectCount;
	FastCriticalSectionClass ObjectPoolCS;

};



/**********************************************************************************************
** AutoPoolClass
**
** This class is designed to be derived from in order to give your class built-in
** object pool behavior.  The new and delete operators for your class will call
** to the internal ObjectPoolClass for fast allocation and de-allocation.  This
** is very well suited to being the base class for a list node class for example.
**
** Notes:
** - The array forms of new and delete are not supported
** - You must define the instance of the static object pool (Allocator)
** - You can't derive a class from a class that is derived from AutoPoolClass 
**   because its size won't match but it will try to use the same pool...
**
** Example Usage:
** --------------
**
** ListNode.h:
** class ListNodeClass : public AutoPoolClass<ListNodeClass,256>
** {
**		ListNodeClass *	Next;
**		void *				Data;
** };
**
** ListNode.cpp:
** DEFINE_AUTO_POOL(ListNodeClass);	
**
** function do_stuff(void) {
**		ListNodeClass * node = new ListNodeClass;
**		delete node;
** }
**
**********************************************************************************************/
template<class T, int BLOCK_SIZE = 64> 
class AutoPoolClass 
{
public:

	static void *	operator new(size_t size);
	static void		operator delete(void * memory);

private:

	// not implemented
	static void *	operator new [] (size_t size);
	static void		operator delete[] (void * memory);

	// This must be staticly declared by user
	static ObjectPoolClass<T,BLOCK_SIZE>	Allocator;

};

/*
** DEFINE_AUTO_POOL(T,BLOCKSIZE)
** Macro to declare the allocator for your class.  Put this in the cpp file for
** the class.
*/
#define DEFINE_AUTO_POOL(T,BLOCKSIZE) \
ObjectPoolClass<T,BLOCKSIZE> AutoPoolClass<T,BLOCKSIZE>::Allocator;


/***********************************************************************************************
 * ObjectPoolClass::ObjectPoolClass -- constructor for ObjectPoolClass                         *
 *                                                                                             *
 * Initializes the object pool to the empty state                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
ObjectPoolClass<T,BLOCK_SIZE>::ObjectPoolClass(void) : 
	FreeListHead(NULL),
	BlockListHead(NULL),
	FreeObjectCount(0),
	TotalObjectCount(0) 
{ 
}
	
/***********************************************************************************************
 * ObjectPoolClass::~ObjectPoolClass -- destructor for ObjectPoolClass                         *
 *                                                                                             *
 * deletes the blocks of memory in use by the object pool.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
ObjectPoolClass<T,BLOCK_SIZE>::~ObjectPoolClass(void)
{
	// assert that the user gave back all of the memory he was using
	WWASSERT(FreeObjectCount == TotalObjectCount);

	// delete all of the blocks we allocated
	int block_count = 0;
	while (BlockListHead != NULL) {
		uint32 * next_block = *(uint32 **)BlockListHead;
		::operator delete(BlockListHead);
		BlockListHead = next_block;
		block_count++;
	}
	WWASSERT(block_count == TotalObjectCount / BLOCK_SIZE);
}



/***********************************************************************************************
 * ObjectPoolClass::Allocate_Object -- allocates an object for the user                        *
 *                                                                                             *
 * If there are no free objects, another block of objects will be allocated.                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
T * ObjectPoolClass<T,BLOCK_SIZE>::Allocate_Object(void)
{
	// allocate memory for the object
	T * obj = Allocate_Object_Memory();
	
	// construct the object in-place
	return new (obj) T;
}

/***********************************************************************************************
 * ObjectPoolClass::Free_Object -- releases obj back into the pool                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
void ObjectPoolClass<T,BLOCK_SIZE>::Free_Object(T * obj)
{
	// destruct the object
	obj->T::~T();

	// release the memory
	Free_Object_Memory(obj);
}

/***********************************************************************************************
 * ObjectPoolClass::Allocate_Object_Memory -- internal function which returns memory for an in *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
T * ObjectPoolClass<T,BLOCK_SIZE>::Allocate_Object_Memory(void)
{
	FastCriticalSectionClass::LockClass lock(ObjectPoolCS);

	if ( FreeListHead == 0 ) {  

		// No free objects, allocate another block
		uint32 * tmp_block_head = BlockListHead;
		BlockListHead = (uint32*)::operator new( sizeof(T) * BLOCK_SIZE + sizeof(uint32 *));
		// Link this block into the block list
		*(void **)BlockListHead = tmp_block_head;

		// Link the objects in the block into the free object list
		FreeListHead = (T*)(BlockListHead + 1);
		for ( int i = 0; i < BLOCK_SIZE; i++ ) {	
			*(T**)(&(FreeListHead[i])) = &(FreeListHead[i+1]);	// link up the elements
		}
		*(T**)(&(FreeListHead[BLOCK_SIZE-1])) = 0;				// Mark the end

		FreeObjectCount += BLOCK_SIZE;
		TotalObjectCount += BLOCK_SIZE;
	}

	T * obj = FreeListHead;						// Get the next free object
	FreeListHead = *(T**)(FreeListHead);	// Bump the Head
	FreeObjectCount--;

	return obj;										
}


/***********************************************************************************************
 * ObjectPoolClass::Free_Object_Memory -- internal function, returns object's memory to the po *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T,int BLOCK_SIZE> 
void ObjectPoolClass<T,BLOCK_SIZE>::Free_Object_Memory(T * obj)
{
	FastCriticalSectionClass::LockClass lock(ObjectPoolCS);

	WWASSERT(obj != NULL);
	*(T**)(obj) = FreeListHead;		// Link to the Head
	FreeListHead = obj;					// Set the Head
	FreeObjectCount++;
}


/***********************************************************************************************
 * AutoPoolClass::operator new -- overriden new which calls the internal ObjectPool            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T, int BLOCK_SIZE>
void * AutoPoolClass<T,BLOCK_SIZE>::operator new( size_t size ) 
{
	WWASSERT(size == sizeof(T));
	return (void *)(Allocator.Allocate_Object_Memory());
}


/***********************************************************************************************
 * AutoPoolClass::operator delete -- overriden delete which calls the internal ObjectPool      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T, int BLOCK_SIZE>
void AutoPoolClass<T,BLOCK_SIZE>::operator delete( void * memory ) 
{
	if ( memory == 0 ) return;
	Allocator.Free_Object_Memory((T*)memory);
}
 


#endif // MEMPOOL_H
