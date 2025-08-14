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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Library/Vector.H                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/12/98 2:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   VectorClass<T>::VectorClass -- Constructor for vector class.                              *
 *   VectorClass<T>::~VectorClass -- Default destructor for vector class.                      *
 *   VectorClass<T>::VectorClass -- Copy constructor for vector object.                        *
 *   VectorClass<T>::operator = -- The assignment operator.                                    *
 *   VectorClass<T>::operator == -- Equality operator for vector objects.                      *
 *   VectorClass<T>::Clear -- Frees and clears the vector.                                     *
 *   VectorClass<T>::Resize -- Changes the size of the vector.                                 *
 *   DynamicVectorClass<T>::DynamicVectorClass -- Constructor for dynamic vector.              *
 *   DynamicVectorClass<T>::Resize -- Changes the size of a dynamic vector.                    *
 *   DynamicVectorClass<T>::Add -- Add an element to the vector.                               *
 *   DynamicVectorClass<T>::Delete -- Remove the specified object from the vector.             *
 *   DynamicVectorClass<T>::Delete -- Deletes the specified index from the vector.             *
 *   VectorClass<T>::ID -- Pointer based conversion to index number.                           *
 *   VectorClass<T>::ID -- Finds object ID based on value.                                     *
 *   DynamicVectorClass<T>::ID -- Find matching value in the dynamic vector.                   *
 *   DynamicVectorClass<T>::Uninitialized_Add -- Add an empty place to the vector.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VECTOR_H
#define VECTOR_H

#include	"noinit.h"
#include	<assert.h>
#include	<new.h>
#include	<string.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<Utility/stdint_adapter.h>


/**************************************************************************
**	This is a general purpose vector class. A vector is defined by this
**	class, as an array of arbitrary objects where the array can be dynamically
**	sized. Because is deals with arbitrary object types, it can handle everything.
**	As a result of this, it is not terribly efficient for integral objects (such
**	as char or int). It will function correctly, but the copy constructor and
**	equality operator could be highly optimized if the integral type were known.
**	This efficiency can be implemented by deriving an integral vector template
**	from this one in order to supply more efficient routines.
*/

// Why, oh why does Visual C need this!!! It's bugged. <sigh>
#pragma warning(disable : 4505)


template<class T>
class VectorClass
{
	public:
		VectorClass(NoInitClass const &) {};
		VectorClass(int size=0, T const * array=0);
		VectorClass(VectorClass<T> const &);		// Copy constructor.
		virtual ~VectorClass(void);

		T & operator[](int index) {assert(unsigned(index) < unsigned(VectorMax));return(Vector[index]);};
		T const & operator[](int index) const {assert(unsigned(index) < unsigned(VectorMax));return(Vector[index]);};
		VectorClass<T> & operator = (VectorClass<T> const &); // Assignment operator.

		virtual bool operator == (VectorClass<T> const &) const;	// Equality operator.

		virtual bool Resize(int newsize, T const * array=0);
		virtual void Clear(void);
		int Length(void) const {return VectorMax;};
		virtual int ID(T const * ptr);	// Pointer based identification.
		virtual int ID(T const & ptr);	// Value based identification.

	protected:

		/*
		**	This is a pointer to the allocated vector array of elements.
		*/
		T * Vector;

		/*
		**	This is the maximum number of elements allowed in this vector.
		*/
		int VectorMax;

		/*
		**	This indicates if the vector is in a valid (even if empty) state.
		*/
		bool IsValid;

		/*
		**	Does the vector data pointer refer to memory that this class has manually
		**	allocated? If so, then this class is responsible for deleting it.
		*/
		bool IsAllocated;
};


/***********************************************************************************************
 * VectorClass<T>::VectorClass -- Constructor for vector class.                                *
 *                                                                                             *
 *    This constructor for the vector class is passed the initial size of the vector and an    *
 *    optional pointer to a preallocated block of memory that the vector will be placed in.    *
 *    If this optional pointer is NULL (or not provided), then the vector is allocated out     *
 *    of free store (with the "new" operator).                                                 *
 *                                                                                             *
 * INPUT:   size  -- The number of elements to initialize this vector to.                      *
 *                                                                                             *
 *          array -- Optional pointer to a previously allocated memory block to hold the       *
 *                   vector.                                                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::VectorClass(int size, T const * array) :
	Vector(0),
	VectorMax(size),
	IsValid(true),
	IsAllocated(false)
{
	/*
	**	Allocate the vector. The default constructor will be called for every
	**	object in this vector.
	*/
	if (size) {
		if (array) {
			Vector = new((void*)array) T[size];
		} else {
			Vector = new T[size];
			IsAllocated = true;
		}
	}
}


/***********************************************************************************************
 * VectorClass<T>::~VectorClass -- Default destructor for vector class.                        *
 *                                                                                             *
 *    This is the default destructor for the vector class. It will deallocate any memory       *
 *    that it may have allocated.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::~VectorClass(void)
{
	VectorClass<T>::Clear();
}


/***********************************************************************************************
 * VectorClass<T>::VectorClass -- Copy constructor for vector object.                          *
 *                                                                                             *
 *    This is the copy constructor for the vector class. It will duplicate the provided        *
 *    vector into the new vector being created.                                                *
 *                                                                                             *
 * INPUT:   vector   -- Reference to the vector to use as a copy.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::VectorClass(VectorClass<T> const & vector) :
	Vector(0),
	VectorMax(0),
	IsValid(true),
	IsAllocated(false)
{
	*this = vector;
}


/***********************************************************************************************
 * VectorClass<T>::operator = -- The assignment operator.                                      *
 *                                                                                             *
 *    This the the assignment operator for vector objects. It will alter the existing lvalue   *
 *    vector to duplicate the rvalue one.                                                      *
 *                                                                                             *
 * INPUT:   vector   -- The rvalue vector to copy into the lvalue one.                         *
 *                                                                                             *
 * OUTPUT:  Returns with reference to the newly copied vector.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T> & VectorClass<T>::operator =(VectorClass<T> const & vector)
{
	if (this != &vector) {
		Clear();
		VectorMax = vector.Length();
		if (VectorMax) {
			Vector = new T[VectorMax];
			if (Vector) {
				IsAllocated = true;
				for (int index = 0; index < VectorMax; index++) {
					Vector[index] = vector[index];
				}
			}
		} else {
			Vector = 0;
			IsAllocated = false;
		}
	}
	return(*this);
}


/***********************************************************************************************
 * VectorClass<T>::operator == -- Equality operator for vector objects.                        *
 *                                                                                             *
 *    This operator compares two vectors for equality. It does this by performing an object    *
 *    by object comparison between the two vectors.                                            *
 *                                                                                             *
 * INPUT:   vector   -- The right vector expression.                                           *
 *                                                                                             *
 * OUTPUT:  bool; Are the two vectors essentially equal? (do they contain comparable elements  *
 *                in the same order?)                                                          *
 *                                                                                             *
 * WARNINGS:   The equality operator must exist for the objects that this vector contains.     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool VectorClass<T>::operator == (VectorClass<T> const & vector) const
{
	if (VectorMax == vector.Length()) {
		for (int index = 0; index < VectorMax; index++) {
			if (Vector[index] != vector[index]) {
				return(false);
			}
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * VectorClass<T>::ID -- Pointer based conversion to index number.                             *
 *                                                                                             *
 *    Use this routine to convert a pointer to an element in the vector back into the index    *
 *    number of that object. This routine ONLY works with actual pointers to object within     *
 *    the vector. For "equivalent" object index number (such as with similar integral values)  *
 *    then use the "by value" index number ID function.                                        *
 *                                                                                             *
 * INPUT:   pointer  -- Pointer to an actual object in the vector.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the index number for the object pointed to by the parameter.          *
 *                                                                                             *
 * WARNINGS:   This routine is only valid for actual pointers to object that exist within      *
 *             the vector. All other object pointers will yield undefined results.             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int VectorClass<T>::ID(T const * ptr)
{
	if (!IsValid) return(0);
	return(((uintptr_t)ptr - (uintptr_t)&(*this)[0]) / sizeof(T));
}


/***********************************************************************************************
 * VectorClass<T>::ID -- Finds object ID based on value.                                       *
 *                                                                                             *
 *    Use this routine to find the index value of an object with equivalent value in the       *
 *    vector. Typical use of this would be for integral types.                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object that is to be looked up in the vector.         *
 *                                                                                             *
 * OUTPUT:  Returns with the index number of the object that is equivalent to the one          *
 *          specified. If no matching value could be found then -1 is returned.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
int VectorClass<T>::ID(T const & object)
{
	if (!IsValid) return(0);

	for (int index = 0; index < VectorMax; index++) {
		if ((*this)[index] == object) {
			return(index);
		}
	}
	return(-1);
}


/***********************************************************************************************
 * VectorClass<T>::Clear -- Frees and clears the vector.                                       *
 *                                                                                             *
 *    Use this routine to reset the vector to an empty (non-allocated) state. A vector will    *
 *    free all allocated memory when this routine is called. In order for the vector to be     *
 *    useful after this point, the Resize function must be called to give it element space.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void VectorClass<T>::Clear(void)
{
	if (Vector && IsAllocated) {
		delete[] Vector;
		Vector = 0;
	}
	IsAllocated = false;
	VectorMax = 0;
}


/***********************************************************************************************
 * VectorClass<T>::Resize -- Changes the size of the vector.                                   *
 *                                                                                             *
 *    This routine is used to change the size (usually to increase) the size of a vector. This *
 *    is the only way to increase the vector's working room (number of elements).              *
 *                                                                                             *
 * INPUT:   newsize  -- The desired size of the vector.                                        *
 *                                                                                             *
 *          array    -- Optional pointer to a previously allocated memory block that the       *
 *                      array will be located in. If this parameter is not supplied, then      *
 *                      the array will be allocated from free store.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the array resized successfully?                                          *
 *                                                                                             *
 * WARNINGS:   Failure to succeed could be the result of running out of memory.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool VectorClass<T>::Resize(int newsize, T const * array)
{
	if (newsize) {

		/*
		**	Allocate a new vector of the size specified. The default constructor
		**	will be called for every object in this vector.
		*/
		T * newptr;

		/*
		**	Either create a new memory block for the object array or initialize
		**	an existing block as indicated by the array parameter. When creating a new
		**	memory block, flag that the vector object is currently in an invalid
		**	state. This is necessary because the default constructor for the object
		**	elements may look to the vector to fetch their ID number.
		*/
		IsValid = false;
		if (!array) {
			newptr = new T[newsize];
		} else {
			newptr = new((void*)array) T[newsize];
		}
		IsValid = true;
		if (!newptr) {
			return(false);
		}

		/*
		**	If there is an old vector, then it must be copied (as much as is feasible)
		**	to the new vector.
		*/
		if (Vector != NULL) {

			/*
			**	Copy as much of the old vector into the new vector as possible. This
			**	presumes that there is a functional assignment operator for each
			**	of the objects in the vector.
			*/
			int copycount = (newsize < VectorMax) ? newsize : VectorMax;
			for (int index = 0; index < copycount; index++) {
				newptr[index] = Vector[index];
			}

			/*
			**	Delete the old vector. This might cause the destructors to be called
			**	for all of the old elements. This makes the implementation of suitable
			**	assignment operator very important. The default assignment operator will
			**	only work for the simplest of objects.
			*/
			if (IsAllocated) {
				delete[] Vector;
				Vector = 0;
			}
		}

		/*
		**	Assign the new vector data to this class.
		*/
		Vector = newptr;
		VectorMax = newsize;
		IsAllocated = (Vector && !array);

	} else {

		/*
		**	Resizing to zero is the same as clearing the vector.
		*/
		Clear();
	}
	return(true);
}



/**************************************************************************
**	This derivative vector class adds the concept of adding and deleting
**	objects. The objects are packed to the beginning of the vector array.
**	If this is instantiated for a class object, then the assignment operator
**	and the equality operator must be supported. If the vector allocates its
**	own memory, then the vector can grow if it runs out of room adding items.
**	The growth rate is controlled by setting the growth step rate. A growth
**	step rate of zero disallows growing.
*/
template<class T>
class DynamicVectorClass : public VectorClass<T>
{
protected:
		using VectorClass<T>::IsAllocated;
		using VectorClass<T>::Vector;
		using VectorClass<T>::VectorMax;
	public:
		using VectorClass<T>::Length;

	public:
		DynamicVectorClass(unsigned size=0, T const * array=0);

		// Change maximum size of vector.
		virtual bool Resize(int newsize, T const * array=0);

		// Resets and frees the vector array.
		virtual void Clear(void) {ActiveCount = 0;VectorClass<T>::Clear();};

		// Fetch number of "allocated" vector objects.
		int Count(void) const {return(ActiveCount);};

		// Add object to vector (growing as necessary).
		bool Add(T const & object);
		bool Add_Head(T const & object);

		// Delete object just like this from vector.
		bool Delete(T const & object);

		// Delete object at this vector index.
		bool Delete(int index);

		// Deletes all objects in the vector.
		void Delete_All(void) {ActiveCount = 0;};

		// Set amount that vector grows by.
		int Set_Growth_Step(int step) {return(GrowthStep = step);};

		// Fetch current growth step rate.
		int Growth_Step(void) {return GrowthStep;};

		virtual int ID(T const * ptr) {return(VectorClass<T>::ID(ptr));};
		virtual int ID(T const & ptr);

		DynamicVectorClass<T> & operator =(DynamicVectorClass<T> const & rvalue) {
			VectorClass<T>::operator = (rvalue);
			ActiveCount = rvalue.ActiveCount;
			GrowthStep = rvalue.GrowthStep;
			return(*this);
		}

      // Uninitialized Add - does everything an Add does, except copying an
      // object into the 'new' spot in the array. It returns a pointer to
      // the 'new' spot. (NULL if the Add failed). NOTE - you must then fill
      // this memory area with a valid object (e.g. by using placement new),
      // or chaos will result!
      T * Uninitialized_Add(void);

	protected:

		/*
		**	This is a count of the number of active objects in this
		**	vector. The memory array often times is bigger than this
		**	value.
		*/
		int ActiveCount;

		/*
		**	If there is insufficient room in the vector array for a new
		**	object to be added, then the vector will grow by the number
		**	of objects specified by this value. This is controlled by
		**	the Set_Growth_Step() function.
		*/
		int GrowthStep;
};


/***********************************************************************************************
 * DynamicVectorClass<T>::DynamicVectorClass -- Constructor for dynamic vector.                *
 *                                                                                             *
 *    This is the normal constructor for the dynamic vector class. It is similar to the normal *
 *    vector class constructor. The vector is initialized to contain the number of elements    *
 *    specified in the "size" parameter. The memory is allocated from free store unless the    *
 *    optional array parameter is provided. In this case it will place the vector at the       *
 *    memory location specified.                                                               *
 *                                                                                             *
 * INPUT:   size  -- The maximum number of objects allowed in this vector.                     *
 *                                                                                             *
 *          array -- Optional pointer to the memory area to place the vector at.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
DynamicVectorClass<T>::DynamicVectorClass(unsigned size, T const * array)
	: VectorClass<T>(size, array)
{
	GrowthStep = 10;
	ActiveCount = 0;
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Resize -- Changes the size of a dynamic vector.                      *
 *                                                                                             *
 *    Use this routine to change the size of the vector. The size changed is the maximum       *
 *    number of allocated objects within this vector. If a memory buffer is provided, then     *
 *    the vector will be located there. Otherwise, the memory will be allocated out of free    *
 *    store.                                                                                   *
 *                                                                                             *
 * INPUT:   newsize  -- The desired maximum size of this vector.                               *
 *                                                                                             *
 *          array    -- Optional pointer to a previously allocated memory array.               *
 *                                                                                             *
 * OUTPUT:  bool; Was vector successfully resized according to specifications?                 *
 *                                                                                             *
 * WARNINGS:   Failure to resize the vector could be the result of lack of free store.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Resize(int newsize, T const * array)
{
	if (VectorClass<T>::Resize(newsize, array)) {
		if (Length() < ActiveCount) ActiveCount = Length();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::ID -- Find matching value in the dynamic vector.                     *
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
int DynamicVectorClass<T>::ID(T const & object)
{
	for (int index = 0; index < Count(); index++) {
		if ((*this)[index] == object) return(index);
	}
	return(-1);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Add -- Add an element to the vector.                                 *
 *                                                                                             *
 *    Use this routine to add an element to the vector. The vector will automatically be       *
 *    resized to accomodate the new element IF the vector was allocated previously and the     *
 *    growth rate is not zero.                                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object that will be added to the vector.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added successfully? If so, the object is added to the end     *
 *                of the vector.                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Add(T const & object)
{
	if (ActiveCount >= Length()) {
		if ((IsAllocated || !VectorMax) && GrowthStep > 0) {
			if (!Resize(Length() + GrowthStep)) {

				/*
				**	Failure to increase the size of the vector is an error condition.
				**	Return with the error flag.
				*/
				return(false);
			}
		} else {

			/*
			**	Increasing the size of this vector is not allowed! Bail this
			**	routine with the error code.
			*/
			return(false);
		}
	}

	/*
	**	There is room for the new object now. Add it to the end of the object vector.
	*/
	(*this)[ActiveCount++] = object;
	return(true);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Add_Head -- Adds element to head of the list.                        *
 *                                                                                             *
 *    This routine will add the specified element to the head of the vector. If necessary,     *
 *    the vector will be expanded accordingly.                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object to add to the head of this vector.             *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added without error?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Add_Head(T const & object)
{
	if (ActiveCount >= Length()) {
		if ((IsAllocated || !VectorMax) && GrowthStep > 0) {
			if (!Resize(Length() + GrowthStep)) {

				/*
				**	Failure to increase the size of the vector is an error condition.
				**	Return with the error flag.
				*/
				return(false);
			}
		} else {

			/*
			**	Increasing the size of this vector is not allowed! Bail this
			**	routine with the error code.
			*/
			return(false);
		}
	}

	/*
	**	There is room for the new object now. Add it to the end of the object vector.
	*/
	if (ActiveCount) {
		memmove(&(*this)[1], &(*this)[0], ActiveCount * sizeof(T));
	}
	(*this)[0] = object;
	ActiveCount++;
//	(*this)[ActiveCount++] = object;
	return(true);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Delete -- Remove the specified object from the vector.               *
 *                                                                                             *
 *    This routine will delete the object referenced from the vector. All objects in the       *
 *    vector that follow the one deleted will be moved "down" to fill the hole.                *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object in this vector that is to be deleted.          *
 *                                                                                             *
 * OUTPUT:  bool; Was the object deleted successfully? This should always be true.             *
 *                                                                                             *
 * WARNINGS:   Do no pass a reference to an object that is NOT part of this vector. The        *
 *             results of this are undefined and probably catastrophic.                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Delete(T const & object)
{
	int id = ID(object);
	if (id != -1) {
		return(Delete(id));
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Delete -- Deletes the specified index from the vector.               *
 *                                                                                             *
 *    Use this routine to delete the object at the specified index from the objects in the     *
 *    vector. This routine will move all the remaining objects "down" in order to fill the     *
 *    hole.                                                                                    *
 *                                                                                             *
 * INPUT:   index -- The index number of the object in the vector that is to be deleted.       *
 *                                                                                             *
 * OUTPUT:  bool; Was the object index deleted successfully? Failure might mean that the index *
 *                specified was out of bounds.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Delete(int index)
{
	if (index < ActiveCount) {
		ActiveCount--;

		/*
		**	If there are any objects past the index that was deleted, copy those
		**	objects down in order to fill the hole. A simple memory copy is
		**	not sufficient since the vector could contain class objects that
		**	need to use the assignment operator for movement.
		*/
		for (int i = index; i < ActiveCount; i++) {
			(*this)[i] = (*this)[i+1];
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Uninitialized_Add -- Add an empty place to the vector.               *
 *                                                                                             *
 *    To avoid copying when creating an object and adding it to the vector, use this and       *
 *    immediately fill the area that the return value points to with a valid object (by hand   *
 *    for a struct or by using placement new for a class object).                              *
 *    This function does everything Add does except copying an object into the new space,      *
 *    thus leaving an uninitialized area of memory.                                            *
 *                                                                                             *
 * INPUT:   none.                                                                              *
 *                                                                                             *
 * OUTPUT:  T *; Points to the empty space where the new object is to be created. (If the      *
 *               space was not added succesfully, returns NULL).                              *
 *                                                                                             *
 * WARNINGS:   If memory area is left uninitialized, Very Bad Things will happen.              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/04/1998 NH : Created.                                                                  *
 *=============================================================================================*/
template<class T>
T * DynamicVectorClass<T>::Uninitialized_Add(void)
{
	if (ActiveCount >= Length()) {
		if ((IsAllocated || !VectorMax) && GrowthStep > 0) {
			if (!Resize(Length() + GrowthStep)) {

				/*
				**	Failure to increase the size of the vector is an error condition.
				**	Return with the error value.
				*/
				return(NULL);
			}
		} else {

			/*
			**	Increasing the size of this vector is not allowed! Bail this
			**	routine with the error value.
			*/
			return(NULL);
		}
	}

	/*
	**	There is room for the new space now. Add it to the end of the object
   ** vector. and return a pointer to it.
	*/
   return &((*this)[ActiveCount++]);
}


void Set_Bit(void * array, int bit, int value);
int Get_Bit(void const * array, int bit);
int First_True_Bit(void const * array);
int First_False_Bit(void const * array);


/**************************************************************************
**	This is a derivative of a vector class that supports boolean flags. Since
**	a boolean flag can be represented by a single bit, this class packs the
**	array of boolean flags into an array of bytes containing 8 boolean values
**	each. For large boolean arrays, this results in an 87.5% savings. Although
**	the indexing "[]" operator is supported, DO NOT pass pointers to sub elements
**	of this bit vector class. A pointer derived from the indexing operator is
**	only valid until the next call. Because of this, only simple
**	direct use of the "[]" operator is allowed.
*/
class BooleanVectorClass
{
	public:
		BooleanVectorClass(unsigned size=0, unsigned char * array=0);
		BooleanVectorClass(BooleanVectorClass const & vector);

		// Assignment operator.
		BooleanVectorClass & operator =(BooleanVectorClass const & vector);

		// Equivalency operator.
		bool operator == (BooleanVectorClass const & vector);

		// Fetch number of boolean objects in vector.
		int Length(void) {return BitCount;};

		// Set all boolean values to false;
		void Reset(void);

		// Set all boolean values to true.
		void Set(void);

		// Resets vector to zero length (frees memory).
		void Clear(void);

		// Change size of this boolean vector.
		int Resize(unsigned size);

		// Fetch reference to specified index.
		bool const & operator[](int index) const {
			if (LastIndex != index) Fixup(index);
			return(Copy);
		};
		bool & operator[](int index) {
			if (LastIndex != index) Fixup(index);
			return(Copy);
		};

		// Quick check on boolean state.
		bool Is_True(int index) const {
			if (index == LastIndex) return(Copy);
			return(Get_Bit(&BitArray[0], index) != 0);
		};

		// Find first index that is false.
		int First_False(void) const {
			if (LastIndex != -1) Fixup(-1);

			int retval = First_False_Bit(&BitArray[0]);
			if (retval < BitCount) return(retval);

			/*
			**	Failure to find a false boolean value in the vector. Return this
			**	fact in the form of an invalid index number.
			*/
			return(-1);
		}

		// Find first index that is true.
		int First_True(void) const {
			if (LastIndex != -1) Fixup(-1);

			int retval = First_True_Bit(&BitArray[0]);
			if (retval < BitCount) return(retval);

			/*
			**	Failure to find a true boolean value in the vector. Return this
			**	fact in the form of an invalid index number.
			*/
			return(-1);
		}

	private:
		void Fixup(int index=-1) const;

		/*
		**	This is the number of boolean values in the vector. This value is
		**	not necessarily a multiple of 8, even though the underlying character
		**	vector contains a multiple of 8 bits.
		*/
		int BitCount;

		/*
		**	This is a referential copy of an element in the bit vector. The
		**	purpose of this copy is to allow normal reference access to this
		**	object (for speed reasons). This hides the bit packing scheme from
		**	the user of this class.
		*/
		bool Copy;

		/*
		**	This records the index of the value last fetched into the reference
		**	boolean variable. This index is used to properly restore the value
		**	when the reference copy needs updating.
		*/
		int LastIndex;

		/*
		**	This points to the allocated bitfield array.
		*/
		VectorClass<unsigned char> BitArray;
};


template<class T>
int Pointer_Vector_Add(T * ptr, VectorClass<T *> & vec)
{
	int id = 0;
	bool foundspot = false;
	for (int index = 0; index < vec.Length(); index++) {
		if (vec[index] == NULL) {
			id = index;
			foundspot = true;
			break;
		}
	}
	if (!foundspot) {
		id = vec.Length();
		vec.Resize((vec.Length()+1) * 2);
		for (int index = id; index < vec.Length(); index++) {
			vec[index] = NULL;
		}
	}
	vec[id] = ptr;
	return(id);
}


template<class T>
bool Pointer_Vector_Remove(T const * ptr, VectorClass<T *> & vec)
{
	int id = vec.ID((T *)ptr);
	if (id != -1) {
		vec[id] = NULL;
		return(true);
	}
	return(false);
}


#endif

