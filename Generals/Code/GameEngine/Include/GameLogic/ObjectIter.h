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

// ObjectIter.h
// Class definitions for various object iterators
// Author: Steven Johnson, September 2001

#pragma once

#include "Common/GameType.h"
#include "Common/GameMemory.h"

// forward declaration
class Object;

//-------------------------------------------------------------------------------------------
/** */
enum IterOrderType CPP_11(: Int)
{
	ITER_FASTEST,										///< iterate in arbitrary order
	ITER_SORTED_NEAR_TO_FAR,				///< iterate in nearest-to-farthest order (may be slower)
	ITER_SORTED_FAR_TO_NEAR,				///< iterate in farthest-to-nearest order (may be slower)
	ITER_SORTED_CHEAP_TO_EXPENSIVE,	///< iterate in cheapest-to-most-expensive order (slower)
	ITER_SORTED_EXPENSIVE_TO_CHEAP	///< iterate in most-expensive-to-cheapest order (slower)
};


//-------------------------------------------------------------------------------------------
/**

	ObjectIterator is an ABC that's used to iterate
	over an arbitrary list of Objects. (duh!)

	As of this writing, only PartitionManager actually
	provides an instance of this, but it seems fairly likely
	that other folks might want to in the future, so
	here it is.

	typical usage:

	iter = ThePartitionManager->iterateInRange(...);
	for (Object *otherObject = iter->first(); otherObject; otherObject = iter->next())
	{
		// do something with other
	}
	deleteInstance(iter);								// you own it, so you must delete it

	note that the iterator is required to deal intelligently with deleted objects;
	in particular, next() will check if an obj has been killed and simply skip it.
*/
class ObjectIterator : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_ABC(ObjectIterator)
public:
	virtual Object *first() = 0;		///< reset iterator and return first item (or null if iter is empty)
	virtual Object *next() = 0;			///< advance and return next item (or null if no more to iterate)
};

inline ObjectIterator::~ObjectIterator() { }


//-------------------------------------------------------------------------------------------
/**
	A basic implementation of ObjectIterator, with (hidden) extensions
	to allow for sorting by a numeric field.
*/
class SimpleObjectIterator : public ObjectIterator
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(SimpleObjectIterator, "SimpleObjectIteratorPool" )
private:

	class Clump : public MemoryPoolObject
	{
		MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Clump, "SimpleObjectIteratorClumpPool" )
	public:

		Clump			*m_nextClump;
		Object		*m_obj;
		Real			m_numeric;	// typically, dist-squared

		Clump();
	//~Clump();
	};

	typedef Real (*ClumpCompareProc)(Clump *a, Clump *b);
	static ClumpCompareProc theClumpCompareProcs[];

	static Real sortNearToFar(Clump *a, Clump *b);
	static Real sortFarToNear(Clump *a, Clump *b);
	static Real sortCheapToExpensive(Clump *a, Clump *b);
	static Real sortExpensiveToCheap(Clump *a, Clump *b);


	Clump				*m_firstClump;
	Clump				*m_curClump;
	Int					m_clumpCount;

	void reset();

public:
	SimpleObjectIterator();
//~SimpleObjectIterator();	// provided by MPO
	Object *first() { return firstWithNumeric(NULL); }
	Object *next() { return nextWithNumeric(NULL); }

	Object *firstWithNumeric(Real *num = NULL) { reset(); return nextWithNumeric(num); }
	Object *nextWithNumeric(Real *num = NULL);

	// methods that are not inherited from ObjectIterator:

	/**
		throw away all contents of the iterator.
	*/
	void makeEmpty();

	/**
		insert an object at the head of the iterator. the given numeric value
		(typically, dist-squared) is used only for subsequent sort() calls;
		if you aren't going to sort, pass 0.0f.
	*/
	void insert(Object *obj, Real numeric = 0.0f);

	/**
		sort the iterator based on the numeric values for objects and the
		order given. Note that some orders (ITER_FASTEST) do nothing!
	*/
	void sort(IterOrderType order);

	/**
		return the total number of objects in the iterator.
	*/
	Int getCount() { return m_clumpCount; }
};
