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
 *                     $Archive:: /Commando/Code/wwlib/simplevec.h                            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/16/01 10:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SimpleVecClass<T>::SimpleVecClass -- Constructor                                          *
 *   SimpleVecClass<T>::~SimpleVecClass -- Destructor                                          *
 *   SimpleVecClass<T>::Resize -- resize the array                                             *
 *   SimpleVecClass<T>::Uninitialised_Grow -- upscale the array, don't copy contents           *
 *   SimpleDynVecClass<T>::~SimpleDynVecClass -- Destructor                                    *
 *   SimpleDynVecClass<T>::SimpleDynVecClass -- Constructor                                    *
 *   SimpleDynVecClass<T>::Resize -- Resize the array                                          *
 *   SimpleDynVecClass<T>::Add -- Add an item to the end of the array                          *
 *   SimpleDynVecClass<T>::Delete -- Delete an item from the array                             *
 *   SimpleDynVecClass<T>::Delete_Range -- delete several items from the array                 *
 *   SimpleDynVecClass<T>::Delete_All -- delete all items from the array                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include <assert.h>


#if (_MSC_VER >= 1200)
#pragma warning (push)
#pragma warning (disable:4702)	// disabling the "unreachable code" warning.
#endif

/**
** SimpleVecClass
** This is a template similar to VectorClass (found in Vector.h) except that it is designed
** specifically to work with data types that are "memcopy-able".
** DON'T USE THIS TEMPLATE IF YOUR CLASS REQUIRES A DESTRUCTOR!!!
*/
template <class T> class SimpleVecClass
{
public:

	SimpleVecClass(int size = 0);
	virtual ~SimpleVecClass(void);

	T & operator[](int index)					{ assert(index < VectorMax); return(Vector[index]); }
	T const & operator[](int index) const	{ assert(index < VectorMax); return(Vector[index]); }

	int				Length(void) const		{ return VectorMax; }
	virtual bool	Resize(int newsize);
	virtual bool	Uninitialised_Grow(int newsize);
	void				Zero_Memory(void)			{ if (Vector != nullptr) { memset(Vector,0,VectorMax * sizeof(T)); } }

protected:

	T *				Vector;
	int				VectorMax;
};


/***********************************************************************************************
 * SimpleVecClass<T>::SimpleVecClass -- Constructor                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 * size - initial size of the vector                                                           *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline SimpleVecClass<T>::SimpleVecClass(int size) :
	Vector(nullptr),
	VectorMax(0)
{
	if (size > 0) {
		Resize(size);
	}
}

/***********************************************************************************************
 * SimpleVecClass<T>::~SimpleVecClass -- Destructor                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline SimpleVecClass<T>::~SimpleVecClass(void)
{
	delete[] Vector;
	Vector = nullptr;
	VectorMax = 0;
}

/***********************************************************************************************
 * SimpleVecClass<T>::Resize -- resize the array                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleVecClass<T>::Resize(int newsize)
{
	if (newsize == VectorMax) {
		return true;
	}

	if (newsize > 0) {

		/*
		**	Allocate a new vector of the size specified. The default constructor
		**	will be called for every object in this vector.
		*/
		T * newptr = W3DNEWARRAY T[newsize];

		/*
		**	If there is an old vector, then it must be copied (as much as is feasible)
		**	to the new vector.
		*/
		if (Vector != nullptr) {

			/*
			**	Mem copy as much of the old vector into the new vector as possible.
			*/
			int copycount = (newsize < VectorMax) ? newsize : VectorMax;
			memcpy(newptr,Vector,copycount * sizeof(T));

			/*
			**	Delete the old vector.
			*/
			delete[] Vector;
			Vector = nullptr;
		}

		/*
		**	Assign the new vector data to this class.
		*/
		Vector = newptr;
		VectorMax = newsize;

	} else {

		/*
		** Delete entire vector and reset counts
		*/
		delete[] Vector;
		Vector = nullptr;
		VectorMax = 0;
	}
	return true;
}

/***********************************************************************************************
 * SimpleVecClass<T>::Uninitialised_Grow -- resize the array if current array is too small.    *
 * Note that it the function doesn't copy the contents so if resising occurs (contents are     *
 * assumed uninitialised after the call).                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/00    jani : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleVecClass<T>::Uninitialised_Grow(int newsize)
{
	if (newsize <= VectorMax) {
		return true;
	}

	if (newsize > 0) {

		/*
		**	Allocate a new vector of the size specified. The default constructor
		**	will be called for every object in this vector.
		*/
		delete[] Vector;
		Vector = W3DNEWARRAY T[newsize];
		VectorMax=newsize;
	}
	return true;
}


/**
** SimpleDynVecClass
** This is also a template designed to work with simple data types.  It is designed to work
** just like DynamicVectorClass except that it assumes that the data type used can be
** legally mem-copied.
** DON'T USE THIS TEMPLATE IF YOUR CLASS REQUIRES A DESTRUCTOR!!!
**
** The automatic resizing behavior for this class is a little different than DynamicVectorClass.
** When the vector needs to be resized, it grows by 25% and if it ever shrinks to 75% usage,
** it resizes downwards.  In addition, when adding objects, you can pass in a "next-size-hint"
** which is the minimum size the vector will need to be once you are done adding your set of
** objects.  This will cause it to resize to at least that size if it needs to resize.  Just
** leave the parameter at its default value for default behavior.
*/
template <class T> class SimpleDynVecClass : public SimpleVecClass<T>
{
protected:
	using SimpleVecClass<T>::Vector;
	using SimpleVecClass<T>::VectorMax;
	using SimpleVecClass<T>::Length;

public:

	SimpleDynVecClass(int size = 0);
	virtual ~SimpleDynVecClass(void);

	// Array-like access (does not grow)
	int				Count(void) const						{ return(ActiveCount); }
	T &				operator[](int index)				{ assert(index < ActiveCount); return(Vector[index]); }
	T const &		operator[](int index) const		{ assert(index < ActiveCount); return(Vector[index]); }

	// Change maximum size of vector
	virtual bool	Resize(int newsize);

	// Add object to vector (growing as necessary).
	bool				Add(T const & object,int new_size_hint = 0);

	// Add room for multiple object to vector. Pointer to first slot added is returned.
	T * 				Add_Multiple( int number_to_add );

	// Delete objects from the vector
	bool				Delete(int index,bool allow_shrink = true);
	bool				Delete(T const & object,bool allow_shrink = true);
	bool				Delete_Range(int start,int count,bool allow_shrink = true);
	void				Delete_All(bool allow_shrink = true);

protected:

	bool				Grow(int new_size_hint);
	bool				Shrink(void);

	int				Find_Index(T const & object);

	int				ActiveCount;
};


/***********************************************************************************************
 * SimpleDynVecClass<T>::SimpleDynVecClass -- Constructor                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 * size - initial size of the allocated vector                                                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline SimpleDynVecClass<T>::SimpleDynVecClass(int size) :
	SimpleVecClass<T>(size),
	ActiveCount(0)
{
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::~SimpleDynVecClass -- Destructor                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline SimpleDynVecClass<T>::~SimpleDynVecClass(void)
{
	delete[] Vector;
	Vector = nullptr;
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Resize -- Resize the array                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 * newsize - new desired size of the array                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Resize(int newsize)
{
	if (SimpleVecClass<T>::Resize(newsize)) {
		if (Length() < ActiveCount) ActiveCount = Length();
		return(true);
	}
	return(false);
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Add -- Add an item to the end of the array                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 * object - object to add to the array                                                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Add(T const & object,int new_size_hint)
{
	if (ActiveCount >= VectorMax) {

		/*
		** We are out of space so tell the vector to grow
		*/
		if (!Grow(new_size_hint)) {
			return false;
		}

	}

	/*
	**	There is room for the new object now. Add it to the end of the object vector.
	*/
	(*this)[ActiveCount++] = object;
	return true;
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Add_Multiple -- Add room for multiple object to vector.					  *
 *                                                                                             *
 * INPUT: number of object slots to add                                                        *
 *                                                                                             *
 * OUTPUT: pointer to first slot added is returned.														  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/01    bmg : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline T *  SimpleDynVecClass<T>::Add_Multiple( int number_to_add )
{
	int index = ActiveCount;
	ActiveCount += number_to_add;

	if (ActiveCount >= VectorMax) {

		/*
		** We are out of space so tell the vector to grow
		*/
		Grow( ActiveCount );
	}

	return &Vector[index];
}


/***********************************************************************************************
 * SimpleDynVecClass<T>::Delete -- Delete an item from the array                               *
 *                                                                                             *
 * This causes the items below the specified index to be mem-moved up.                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 * index - index of the entry to delete                                                        *
 * allow_shrink - should the vector be allowed to resize                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Delete(int index,bool allow_shrink)
{
	assert(index < ActiveCount);

	/*
	**	If there are any objects past the index that was deleted, memcopy
	** those objects to collapse the array.  NOTE: again, this template
	** cannot be used for classes that cannot be memcopied!!
	*/
	if (index < ActiveCount-1) {
		memmove(&(Vector[index]),&(Vector[index+1]),(ActiveCount - index - 1) * sizeof(T));
	}
	ActiveCount--;

	/*
	** We deleted something so we may need to shrink
	*/
	if (allow_shrink) {
		Shrink();
	}

	return true;
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Delete -- Remove the specified object from the vector.                *
 *                                                                                             *
 *    This routine will delete the object referenced from the vector. All objects in the       *
 *    vector that follow the one deleted will be moved "down" to fill the hole.                *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object in this vector that is to be deleted.          *
 *          allow_shrink -- should the vector be allowed to shrink if it is wasting space?     *
 *                                                                                             *
 * OUTPUT:  bool; Was the object deleted successfully?                                         *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Delete(T const & object,bool allow_shrink)
{
	int id = Find_Index(object);
	if (id != -1) {
		return(Delete(id),allow_shrink);
	}
	return(false);
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Delete_Range -- delete several items from the array                   *
 *                                                                                             *
 * This causes the items below the specified index range to be mem-moved up.                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * start - starting index to delete                                                            *
 * count - number of entries to delete                                                         *
 * allow_shrink - should the vector be allowed to shrink                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Delete_Range(int start,int count,bool allow_shrink)
{
	assert(start >= 0);
	assert(start <= ActiveCount - count);

	/*
	**	If there are any objects past the index that was deleted, memcopy
	** those objects to collapse the array.  NOTE: again, this template
	** cannot be used for classes that cannot be memcopied!!
	*/
	if (start < ActiveCount - count) {
		memmove(&(Vector[start]),&(Vector[start + count]),(ActiveCount - start - count) * sizeof(T));
	}

	ActiveCount -= count;

	/*
	** We deleted something so we may need to shrink
	*/
	if (allow_shrink) {
		Shrink();
	}

	return true;
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Delete_All -- delete all items from the array                         *
 *                                                                                             *
 * Internally, this just resets the ActiveCount...                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline void SimpleDynVecClass<T>::Delete_All(bool allow_shrink)
{
	ActiveCount = 0;

	/*
	** We deleted something so we may need to shrink
	*/
	if (allow_shrink) {
		Shrink();
	}
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Grow -- increase the size of the array                                *
 *                                                                                             *
 * Internally, this just resets the ActiveCount...                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Grow(int new_size_hint)
{
	/*
	** Vector should grow to 25% bigger, grow at least 4 elements,
	** and grow at least up to the user's new_size_hint
	*/
	int new_size = MAX(Length() + Length()/4,Length() + 4);
	new_size = MAX(new_size,new_size_hint);

	return Resize(new_size);
}

/***********************************************************************************************
 * SimpleDynVecClass<T>::Shrink -- reduce the size of the array                                *
 *                                                                                             *
 * Internally, this just resets the ActiveCount...                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool SimpleDynVecClass<T>::Shrink(void)
{
	/*
	** Shrink the array if it is wasting more than 25%
	*/
	if (ActiveCount < VectorMax/4) {
		return Resize(ActiveCount);
	}
	return true;
}


/***********************************************************************************************
 * SimpleDynVecClass<T>::Find_Index -- Find matching value in the dynamic vector.              *
 *                                                                                             *
 *    Use this routine to find a matching object (by value) in the vector. Unlike the base     *
 *    class ID function of similar name, this one restricts the scan to the current number     *
 *    of valid objects.                                                                        *
 *                                                                                             *
 * INPUT:   object   -- A reference to the object that a match is to be found in the           *
 *                      vector.                                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the index number of the object that is equivalent to the one          *
 *          specified. If no equivalent object could be found then -1 is returned.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int SimpleDynVecClass<T>::Find_Index(T const & object)
{
	for (int index = 0; index < Count(); index++) {
		if ((*this)[index] == object) return(index);
	}
	return(-1);
}

#if (_MSC_VER >= 1200)
#pragma warning (pop)
#endif
