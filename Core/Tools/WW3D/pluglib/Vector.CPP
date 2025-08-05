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
 *                     $Archive:: /Commando/Code/Tools/pluglib/Vector.CPP                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/98 10:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BooleanVectorClass::BooleanVectorClass -- Copy constructor for boolean array.             *
 *   BooleanVectorClass::BooleanVectorClass -- Explicit data buffer constructor.               *
 *   BooleanVectorClass::Clear -- Resets boolean vector to empty state.                        *
 *   BooleanVectorClass::Fixup -- Updates the boolean vector to a known state.                 *
 *   BooleanVectorClass::Reset -- Clear all boolean values in array.                           *
 *   BooleanVectorClass::Resize -- Resizes a boolean vector object.                            *
 *   BooleanVectorClass::Set -- Forces all boolean elements to true.                           *
 *   BooleanVectorClass::operator = -- Assignment operator.                                    *
 *   BooleanVectorClass::operator == -- Comparison operator for boolean vector.                *
 *   VectorClass<T>::Clear -- Frees and clears the vector.                                     *
 *   VectorClass<T>::ID -- Finds object ID based on value.                                     *
 *   VectorClass<T>::ID -- Pointer based conversion to index number.                           *
 *   VectorClass<T>::Resize -- Changes the size of the vector.                                 *
 *   VectorClass<T>::VectorClass -- Constructor for vector class.                              *
 *   VectorClass<T>::VectorClass -- Copy constructor for vector object.                        *
 *   VectorClass<T>::operator = -- The assignment operator.                                    *
 *   VectorClass<T>::operator == -- Equality operator for vector objects.                      *
 *   VectorClass<T>::~VectorClass -- Default destructor for vector class.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"Vector.H"
#include	<string.h>

/*
**	The following template function can be located here ONLY if all the instantiations are
**	declared in a header file this module includes. By placing the template functions here,
**	it speeds up compiler operation and reduces object module size.
*/

//----------------------------------------------------------------------------------------------

/***********************************************************************************************
 * BooleanVectorClass::BooleanVectorClass -- Explicit data buffer constructor.                 *
 *                                                                                             *
 *    This is the constructor for a boolean array. This constructor takes the memory pointer   *
 *    provided as assigns that as the array data pointer.                                      *
 *                                                                                             *
 * INPUT:   size  -- The size of the array (in bits).                                          *
 *                                                                                             *
 *          array -- Pointer to the memory that the array is to use.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   You must make sure that the memory specified is large enough to contain the     *
 *             bits specified.                                                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BooleanVectorClass::BooleanVectorClass(unsigned size, unsigned char * array) :
	BitCount(size),
	Copy(false),
	LastIndex(-1),
	BitArray(0, 0)
{
	BitArray.Resize(((size + (8-1)) / 8), array);
//	LastIndex = -1;
//	BitCount = size;
}


/***********************************************************************************************
 * BooleanVectorClass::BooleanVectorClass -- Copy constructor of boolean array.                *
 *                                                                                             *
 *    This is the copy constructor for a boolean array. It is used to make a duplicate of the  *
 *    boolean array.                                                                           *
 *                                                                                             *
 * INPUT:   vector   -- Reference to the vector to be duplicated.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BooleanVectorClass::BooleanVectorClass(BooleanVectorClass const & vector)
{
	LastIndex = -1;
	*this = vector;
}


/***********************************************************************************************
 * BooleanVectorClass::operator = -- Assignment operator.                                      *
 *                                                                                             *
 *    This routine will make a copy of the specified boolean vector array. The vector is       *
 *    copied into an already constructed existing vector. The values from the existing vector  *
 *    are destroyed by this copy.                                                              *
 *                                                                                             *
 * INPUT:   vector   -- Reference to the vector to make a copy of.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BooleanVectorClass & BooleanVectorClass::operator =(BooleanVectorClass const & vector)
{
	Fixup();
	Copy = vector.Copy;
	LastIndex = vector.LastIndex;
	BitArray = vector.BitArray;
	BitCount = vector.BitCount;
	return(*this);
}


/***********************************************************************************************
 * BooleanVectorClass::operator == -- Comparison operator for boolean vector.                  *
 *                                                                                             *
 *    This is the comparison operator for a boolean vector class. Boolean vectors are equal    *
 *    if the bit count and bit values are identical.                                           *
 *                                                                                             *
 * INPUT:   vector   -- Reference to the vector to compare to.                                 *
 *                                                                                             *
 * OUTPUT:  Are the boolean vectors identical?                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BooleanVectorClass::operator == (const BooleanVectorClass & vector)
{
	Fixup(LastIndex);
	return(BitCount == vector.BitCount && BitArray == vector.BitArray);
}


/***********************************************************************************************
 * BooleanVectorClass::Resize -- Resizes a boolean vector object.                              *
 *                                                                                             *
 *    This routine will resize the boolean vector object. An index value used with a boolean   *
 *    vector must be less than the value specified in as the new size.                         *
 *                                                                                             *
 * INPUT:   size  -- The new maximum size of this boolean vector.                              *
 *                                                                                             *
 * OUTPUT:  Was the boolean vector sized successfully?                                         *
 *                                                                                             *
 * WARNINGS:   The boolean array might be reallocated or even deleted by this routine.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BooleanVectorClass::Resize(unsigned size)
{
	Fixup();

	if (size > 0) {

		/*
		**	Record the previous bit count of the boolean vector. This is used
		**	to determine if the array has grown in size and thus clearing is
		**	necessary.
		*/
		unsigned oldsize = BitCount;

		/*
		**	Actually resize the bit array. Since this is a bit packed array,
		**	there are 8 elements per byte (rounded up).
		*/
		int success = BitArray.Resize(((size + (8-1)) / 8));

		/*
		**	Since there is no default constructor for bit packed integers, a manual
		**	clearing of the bits is required.
		*/
		BitCount = size;
		if (success && oldsize < size) {
			for (unsigned index = oldsize; index < size; index++) {
				(*this)[index] = 0;
			}
		}

		return(success);
	}

	/*
	**	Resizing to zero is the same as clearing and deallocating the array.
	**	This is always successful.
	*/
	Clear();
	return(true);
}


/***********************************************************************************************
 * BooleanVectorClass::Clear -- Resets boolean vector to empty state.                          *
 *                                                                                             *
 *    This routine will clear out the boolean array. This will free any allocated memory and   *
 *    result in the boolean vector being unusable until the Resize function is subsequently    *
 *    called.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The boolean vector cannot be used until it is resized to a non null condition.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BooleanVectorClass::Clear(void)
{
	Fixup();
	BitCount = 0;
	BitArray.Clear();
}


/***********************************************************************************************
 * BooleanVectorClass::Reset -- Clear all boolean values in array.                             *
 *                                                                                             *
 *    This is the preferred (and quick) method to clear the boolean array to a false condition.*
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BooleanVectorClass::Reset(void)
{
	LastIndex = -1;
	if (BitArray.Length() > 0) {
		memset(&BitArray[0], '\0', BitArray.Length());
	}
}


/***********************************************************************************************
 * BooleanVectorClass::Set -- Forces all boolean elements to true.                             *
 *                                                                                             *
 *    This is the preferred (and fast) way to set all boolean elements to true.                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BooleanVectorClass::Set(void)
{
	LastIndex = -1;
	if (BitArray.Length() > 0) {
		memset(&BitArray[0], '\xFF', BitArray.Length());
	}
}


/***********************************************************************************************
 * BooleanVectorClass::Fixup -- Updates the boolean vector to a known state.                   *
 *                                                                                             *
 *    Use this routine to set the boolean value copy to match the appropriate bit in the       *
 *    boolean array. The boolean array will be updated with any changes from the last time     *
 *    a value was fetched from the boolean vector. By using this update method, the boolean    *
 *    array can be treated as a normal array even though the elements are composed of          *
 *    otherwise inaccessible bits.                                                             *
 *                                                                                             *
 * INPUT:   index -- The index to set the new copy value to. If the index is -1, then the      *
 *                   previous value will be updated into the vector array, but no new value    *
 *                   will be fetched from it.                                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Always call this routine with "-1" if any direct manipulation of the bit        *
 *             array is to occur. This ensures that the bit array is accurate.                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BooleanVectorClass::Fixup(int index) const
{
	/*
	**	If the requested index value is illegal, then force the index
	**	to be -1. This is the default non-index value.
	*/
	if ((unsigned)index >= (unsigned)BitCount) {
		index = -1;
	}

	/*
	**	If the new index is different than the previous index, there might
	**	be some fixing up required.
	*/
	if (index != LastIndex) {

		/*
		**	If the previously fetched boolean value was changed, then update
		**	the boolean array accordingly.
		*/
		if (LastIndex != -1) {
			assert(unsigned(LastIndex) < unsigned(BitCount));
			Set_Bit((void*)&BitArray[0], LastIndex, Copy);
		}

		/*
		**	If this new current index is valid, then fill in the reference boolean
		**	value with the appropriate data from the bit array.
		*/
		if (index != -1) {
			assert(unsigned(index) < unsigned(BitCount));
			((unsigned char &)Copy) = (unsigned char)Get_Bit((void*)&BitArray[0], index);
//			((unsigned char&)Copy) = Get_Bit((void*)&BitArray[0], index);
		}

		((BooleanVectorClass *)this)->LastIndex = index;
	}
}


